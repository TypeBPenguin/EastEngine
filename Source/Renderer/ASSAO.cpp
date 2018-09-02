///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Intel Corporation
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of 
// the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File changes (yyyy-mm-dd)
// 2016-09-07: filip.strugar@intel.com: first commit
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ASSAO.h"

#include "CommonLib/FileUtil.h"

#include "DirectX/Camera.h"
#include "DirectX/Vertex.h"

#include <d3dcompiler.h>

#define SSA_STRINGIZIZER( x ) SSA_STRINGIZIZER_( x )
#define SSA_STRINGIZIZER_( x ) #x

// ** WARNING ** if changing any of the slot numbers, please remember to update the corresponding shader code!
#define SSAO_SAMPLERS_SLOT0				0
#define SSAO_SAMPLERS_SLOT1				1
#define SSAO_SAMPLERS_SLOT2				2
#define SSAO_SAMPLERS_SLOT3				3
#define SSAO_NORMALMAP_OUT_UAV_SLOT		4
#define SSAO_CONSTANTS_BUFFERSLOT		0
#define SSAO_TEXTURE_SLOT0				0
#define SSAO_TEXTURE_SLOT1				1
#define SSAO_TEXTURE_SLOT2				2
#define SSAO_TEXTURE_SLOT3				3
#define SSAO_TEXTURE_SLOT4				4
#define SSAO_LOAD_COUNTER_UAV_SLOT		4

#define SSAO_MAX_TAPS					32
#define SSAO_MAX_REF_TAPS				512
#define SSAO_ADAPTIVE_TAP_BASE_COUNT	5
#define SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT (SSAO_MAX_TAPS-SSAO_ADAPTIVE_TAP_BASE_COUNT)
#define SSAO_DEPTH_MIP_LEVELS			4

#ifdef INTEL_SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
#define SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION 1
#else
#define SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION 0
#endif

namespace StrID
{
	RegisterStringID(PrepareDepths);
	RegisterStringID(PrepareDepthsAndNormals);
	RegisterStringID(PrepareDepthsHalf);
	RegisterStringID(PrepareDepthsAndNormalsHalf);
	RegisterStringID(PrepareDepthMip1);
	RegisterStringID(PrepareDepthMip2);
	RegisterStringID(PrepareDepthMip3);
	RegisterStringID(GenerateQ0);
	RegisterStringID(GenerateQ1);
	RegisterStringID(GenerateQ2);
	RegisterStringID(GenerateQ3);
	RegisterStringID(GenerateQ3Base);
	RegisterStringID(SmartBlur);
	RegisterStringID(SmartBlurWide);
	RegisterStringID(NonSmartBlur);
	RegisterStringID(Apply);
	RegisterStringID(NonSmartApply);
	RegisterStringID(NonSmartHalfApply);
	RegisterStringID(GenerateImportanceMap);
	RegisterStringID(PostprocessImportanceMapA);
	RegisterStringID(PostprocessImportanceMapB);
}

namespace eastengine
{
	namespace graphics
	{
		// ** WARNING ** if changing anything here, update the corresponding shader code! ** WARNING **
		struct ASSAOConstants
		{
			math::Vector2 ViewportPixelSize; // .zw == 1.0 / ViewportSize.xy
			math::Vector2 HalfViewportPixelSize; // .zw == 1.0 / ViewportHalfSize.xy

			math::Vector2 DepthUnpackConsts;
			math::Vector2 CameraTanHalfFOV;

			math::Vector2 NDCToViewMul;
			math::Vector2 NDCToViewAdd;

			math::Int2 PerPassFullResCoordOffset;
			math::Vector2 PerPassFullResUVOffset;

			math::Vector2 Viewport2xPixelSize;
			math::Vector2 Viewport2xPixelSize_x_025; // Viewport2xPixelSize* 0.25 (for fusing add+mul into mad)

			float EffectRadius; // world (viewspace) maximum size of the shadow
			float EffectShadowStrength; // global strength of the effect (0 - 5)
			float EffectShadowPow;
			float EffectShadowClamp;

			float EffectFadeOutMul; // effect fade out from distance (ex. 25)
			float EffectFadeOutAdd; // effect fade out to distance (ex. 100)
			float EffectHorizonAngleThreshold; // limit errors on slopes and caused by insufficient geometry tessellation (0.05 to 0.5)
			float EffectSamplingRadiusNearLimitRec; // if viewspace pixel closer than this, don't enlarge shadow sampling radius anymore (makes no sense to grow beyond some distance, not enough samples to cover everything, so just limit the shadow growth; could be SSAOSettingsFadeOutFrom* 0.1 or less)

			float DepthPrecisionOffsetMod;
			float NegRecEffectRadius; // -1.0 / EffectRadius
			float LoadCounterAvgDiv; // 1.0 / ( halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeX* halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeY )
			float AdaptiveSampleCountLimit;

			float InvSharpness;
			int PassIndex;
			math::Vector2 QuarterResPixelSize; // used for importance map only

			math::Vector4 PatternRotScaleMatrices[5];

			float NormalsUnpackMul;
			float NormalsUnpackAdd;
			float DetailAOStrength;
			float Dummy0;

#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
			math::Matrix NormalsWorldToViewspaceMatrix;
#endif
		};

		// Simplify texture creation and potential future porting
		struct D3D11Texture2D
		{
			ID3D11Texture2D* Texture2D;
			ID3D11ShaderResourceView* SRV;
			//ID3D11DepthStencilView* DSV;
			ID3D11RenderTargetView* RTV;
			ID3D11UnorderedAccessView* UAV;
			math::UInt2 Size;

			D3D11Texture2D() : Texture2D(nullptr), SRV(nullptr), /*DSV( nullptr ),*/ RTV(nullptr), UAV(nullptr), Size(0, 0) { }
			~D3D11Texture2D() { Reset(); }

			void Reset() { SafeRelease(Texture2D); SafeRelease(SRV); /*SafeRelease( DSV );*/ SafeRelease(RTV); SafeRelease(UAV); Size = math::UInt2::Zero; }
			bool ReCreateIfNeeded(ID3D11Device* device, const math::UInt2& size, DXGI_FORMAT format, float& inoutTotalSizeSum, uint32_t mipLevels, uint32_t arraySize, bool supportUAVs);
			bool ReCreateMIPViewIfNeeded(ID3D11Device* device, D3D11Texture2D& original, int mipViewSlice);
			bool ReCreateArrayViewIfNeeded(ID3D11Device* device, D3D11Texture2D& original, int arraySlice);
		};

		// Just to reduce clutter in the main code
		class D3D11SSAOStateBackupRAII
		{
			ID3D11DeviceContext* m_dx11Context;

			D3D11_VIEWPORT m_VPs[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			UINT m_numViewports;
			D3D11_RECT m_scissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			UINT m_numScissorRects;
			ID3D11SamplerState* m_samplerStates[4];
			ID3D11Buffer* m_constantBuffers[1];
			ID3D11ShaderResourceView* m_SRVs[5];
			ID3D11BlendState* m_blendState;
			FLOAT m_blendStateBlendFactor[4];
			UINT m_blendStateSampleMask;
			ID3D11DepthStencilState* m_depthStencilState;
			UINT m_depthStencilStateStencilRef;
			D3D_PRIMITIVE_TOPOLOGY m_primitiveTopology;
			ID3D11InputLayout* m_inputLayout;
			ID3D11RasterizerState* m_rasterizerState;
			UINT m_numRTVs;
			ID3D11RenderTargetView* m_RTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
			ID3D11DepthStencilView* m_DSV;


		public:
			D3D11SSAOStateBackupRAII(ID3D11DeviceContext* dx11Context) : m_dx11Context(dx11Context)
			{
				m_numViewports = _countof(m_VPs);
				m_dx11Context->RSGetViewports(&m_numViewports, m_VPs);
				m_numScissorRects = _countof(m_scissorRects);
				m_dx11Context->RSGetScissorRects(&m_numScissorRects, m_scissorRects);
				m_dx11Context->PSGetSamplers(0, _countof(m_samplerStates), m_samplerStates);
				m_dx11Context->PSGetConstantBuffers(0, _countof(m_constantBuffers), m_constantBuffers);
				m_dx11Context->PSGetShaderResources(0, _countof(m_SRVs), m_SRVs);
				m_dx11Context->OMGetBlendState(&m_blendState, m_blendStateBlendFactor, &m_blendStateSampleMask);
				m_dx11Context->OMGetDepthStencilState(&m_depthStencilState, &m_depthStencilStateStencilRef);
				m_dx11Context->IAGetPrimitiveTopology(&m_primitiveTopology);
				m_dx11Context->IAGetInputLayout(&m_inputLayout);
				m_dx11Context->RSGetState(&m_rasterizerState);

				memset(m_RTVs, sizeof(m_RTVs), 0);
				dx11Context->OMGetRenderTargets(_countof(m_RTVs), m_RTVs, &m_DSV);

#ifdef _DEBUG
				ID3D11UnorderedAccessView* UAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];
				dx11Context->OMGetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, D3D11_PS_CS_UAV_REGISTER_COUNT, UAVs);
				for (int i = 0; i < _countof(UAVs); i++)
				{
					// d3d state backup/restore does not work if UAVs are selected; please backup&restore or reset previously set UAVs manually. 
					assert(UAVs[i] == nullptr);
				}
#endif
			}
			~D3D11SSAOStateBackupRAII()
			{
				// Restore D3D11 states
				m_dx11Context->RSSetViewports(m_numViewports, m_VPs);
				m_dx11Context->RSSetScissorRects(m_numScissorRects, m_scissorRects);
				m_dx11Context->PSSetSamplers(0, _countof(m_samplerStates), m_samplerStates);
				m_dx11Context->PSSetConstantBuffers(0, _countof(m_constantBuffers), m_constantBuffers);
				m_dx11Context->PSSetShaderResources(0, _countof(m_SRVs), m_SRVs);
				m_dx11Context->OMSetRenderTargets(_countof(m_RTVs), m_RTVs, m_DSV);
				m_dx11Context->OMSetBlendState(m_blendState, m_blendStateBlendFactor, m_blendStateSampleMask);
				m_dx11Context->OMSetDepthStencilState(m_depthStencilState, m_depthStencilStateStencilRef);
				m_dx11Context->IASetPrimitiveTopology(m_primitiveTopology);
				m_dx11Context->IASetInputLayout(m_inputLayout);
				m_dx11Context->RSSetState(m_rasterizerState);

				for (auto& p : m_constantBuffers)
				{
					SafeRelease(p);
				}

				for (auto& p : m_samplerStates)
				{
					SafeRelease(p);
				}

				for (auto& p : m_SRVs)
				{
					SafeRelease(p);
				}

				for (auto& p : m_RTVs)
				{
					SafeRelease(p);
				}

				SafeRelease(m_blendState);
				SafeRelease(m_depthStencilState);
				SafeRelease(m_inputLayout);
				SafeRelease(m_rasterizerState);
			}
			void RestoreRTs()
			{
				m_dx11Context->RSSetViewports(m_numViewports, m_VPs);
				m_dx11Context->OMSetRenderTargets(_countof(m_RTVs), m_RTVs, m_DSV);
			}
			const D3D11_VIEWPORT& GetFirstViewport() const { return m_VPs[0]; }
		};

		static const int cMaxBlurPassCount = 6;

		static GUID c_IID_ID3D11Texture2D = { 0x6f15aaf2,0xd208,0x4e89,{ 0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c } };

		class ASSAODX11 : public ASSAO_Effect
		{
			struct BufferFormats
			{
				DXGI_FORMAT DepthBufferViewspaceLinear;
				DXGI_FORMAT AOResult;
				DXGI_FORMAT Normals;
				DXGI_FORMAT ImportanceMap;

				BufferFormats()
				{
					DepthBufferViewspaceLinear = DXGI_FORMAT_R16_FLOAT; // increase this to DXGI_FORMAT_R32_FLOAT if using very low FOVs (for a scope effect) or similar, or in case you suspect artifacts caused by lack of precision; performance will degrade
					//Normals = DXGI_FORMAT_R8G8B8A8_UNORM;
					Normals = DXGI_FORMAT_R16G16B16A16_FLOAT;
					AOResult = DXGI_FORMAT_R8G8_UNORM;
					ImportanceMap = DXGI_FORMAT_R8_UNORM;
				}
			};

		private:
			BufferFormats m_formats;

			math::UInt2 m_size;
			math::UInt2 m_halfSize;
			math::UInt2 m_quarterSize;
			math::UInt4 m_fullResOutScissorRect;
			math::UInt4 m_halfResOutScissorRect;

			int m_depthMipLevels;

			ID3D11Device* m_device;

			unsigned int m_allocatedVRAM;

			ID3D11Buffer* m_constantsBuffer;

			IVertexBuffer* m_pFullScreenVB;

			ID3D11VertexShader* m_vertexShader;

			IRasterizerState* m_pRasterizerState;

			ISamplerState* m_pSamplerStatePointClamp;
			ISamplerState* m_pSamplerStateLinearClamp;
			ISamplerState* m_pSamplerStatePointMirror;
			ISamplerState* m_pSamplerStateViewspaceDepthTap;

			IBlendState* m_pBlendStateMultiply;
			IBlendState* m_pBlendStateOpaque;

			IDepthStencilState* m_pDepthStencilState;

			ID3D11PixelShader* m_pixelShaderPrepareDepths;
			ID3D11PixelShader* m_pixelShaderPrepareDepthsAndNormals;
			ID3D11PixelShader* m_pixelShaderPrepareDepthsHalf;
			ID3D11PixelShader* m_pixelShaderPrepareDepthsAndNormalsHalf;
			ID3D11PixelShader* m_pixelShaderPrepareDepthMip[SSAO_DEPTH_MIP_LEVELS - 1];
			ID3D11PixelShader* m_pixelShaderGenerate[5];
			ID3D11PixelShader* m_pixelShaderSmartBlur;
			ID3D11PixelShader* m_pixelShaderSmartBlurWide;
			ID3D11PixelShader* m_pixelShaderApply;
			ID3D11PixelShader* m_pixelShaderNonSmartBlur;
			ID3D11PixelShader* m_pixelShaderNonSmartApply;
			ID3D11PixelShader* m_pixelShaderNonSmartHalfApply;
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
			ID3D11PixelShader* m_pixelShaderGenerateImportanceMap;
			ID3D11PixelShader* m_pixelShaderPostprocessImportanceMapA;
			ID3D11PixelShader* m_pixelShaderPostprocessImportanceMapB;
#endif

			D3D11Texture2D m_halfDepths[4];
			D3D11Texture2D m_halfDepthsMipViews[4][SSAO_DEPTH_MIP_LEVELS];
			//D3D11Texture2D m_edges;
			D3D11Texture2D m_pingPongHalfResultA;
			D3D11Texture2D m_pingPongHalfResultB;
			D3D11Texture2D m_finalResults;
			D3D11Texture2D m_finalResultsArrayViews[4];
			D3D11Texture2D m_normals;
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
			// Only needed for quality level 3 (adaptive quality)
			D3D11Texture2D m_importanceMap;
			D3D11Texture2D m_importanceMapPong;
			ID3D11Texture1D* m_loadCounter;
			ID3D11ShaderResourceView* m_loadCounterSRV;
			ID3D11UnorderedAccessView* m_loadCounterUAV;
#endif

			bool m_requiresClear;

		private:
			friend class ASSAO_Effect;

			ASSAODX11(const ASSAO_CreateDesc& createDesc);
			virtual ~ASSAODX11();

			bool InitializeDX(const ASSAO_CreateDesc& createDesc);
			void CleanupDX();

		private:
			// ASSAO_Effect implementation
			virtual void PreAllocateVideoMemory(const ASSAO_Inputs* inputs);
			virtual void DeleteAllocatedVideoMemory();
			virtual unsigned int GetAllocatedVideoMemory();
			virtual void GetVersion(int& major, int& minor) { major = 1; minor = 0; }
			// Apply the SSAO effect to the currently selected render target using provided settings and platform-dependent inputs
			virtual void Draw(const ASSAO_Settings& settings, const ASSAO_Inputs* inputs);

		private:
			void UpdateTextures(const ASSAO_InputsDX11* inputs);
			void UpdateConstants(const ASSAO_Settings& settings, const ASSAO_InputsDX11* inputs, int pass);
			//void FullscreenPassDraw(ID3D11DeviceContext* context, ID3D11PixelShader* pixelShader, ID3D11BlendState* blendState = nullptr, ID3D11DepthStencilState* depthStencilState = nullptr, UINT stencilRef = 0);
			void FullscreenPassDraw(IDeviceContext* context, ID3D11PixelShader* pixelShader, IBlendState* pBlendState = nullptr, IDepthStencilState* pDepthStencilState = nullptr, UINT stencilRef = 0);
			void PrepareDepths(const ASSAO_Settings& settings, const ASSAO_InputsDX11* inputs);
			void GenerateSSAO(const ASSAO_Settings& settings, const ASSAO_InputsDX11* inputs, bool adaptiveBasePass);
		};
		
		ASSAO_Effect* ASSAO_Effect::CreateInstance(const ASSAO_CreateDesc& createDesc)
		{
			ASSAODX11* effect = new ASSAODX11(createDesc);
			if (effect->InitializeDX(createDesc))
			{
				return effect;
			}
			else
			{
				delete effect;
				return nullptr;
			}
		}

		void ASSAO_Effect::DestroyInstance(ASSAO_Effect* effectInstance)
		{
			delete effectInstance;
		}


		static int GetPixelSizeInBytes(DXGI_FORMAT val);

		//////////////////////////////////////////////////////////////////////////
		// ASSAODX11 implementation
		//////////////////////////////////////////////////////////////////////////

		ASSAOdx11::ASSAODX11(const ASSAO_CreateDesc& createDesc)
		{
			m_device = createDesc.Device;
			m_device->AddRef();

			m_size = math::UInt2::Zero;
			m_halfSize = math::UInt2::Zero;
			m_quarterSize = math::UInt2::Zero;
			m_fullResOutScissorRect = math::UInt4::Zero;
			m_halfResOutScissorRect = math::UInt4::Zero;
			m_depthMipLevels = 0;

			m_constantsBuffer = nullptr;
			m_pFullScreenVB = nullptr;
			m_allocatedVRAM = 0;

			m_vertexShader = nullptr;
			m_pRasterizerState = nullptr;

			m_pixelShaderPrepareDepths = nullptr;
			m_pixelShaderPrepareDepthsAndNormals = nullptr;
			m_pixelShaderPrepareDepthsHalf = nullptr;
			m_pixelShaderPrepareDepthsAndNormalsHalf = nullptr;
			for (auto& p : m_pixelShaderPrepareDepthMip)
			{
				p = nullptr;
			}
			for (auto& p : m_pixelShaderGenerate)
			{
				p = nullptr;
			}
			m_pixelShaderSmartBlur = nullptr;
			m_pixelShaderSmartBlurWide = nullptr;
			m_pixelShaderNonSmartBlur = nullptr;
			m_pixelShaderApply = nullptr;
			m_pixelShaderNonSmartApply = nullptr;
			m_pixelShaderNonSmartHalfApply = nullptr;
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
			m_pixelShaderGenerateImportanceMap = nullptr;
			m_pixelShaderPostprocessImportanceMapA = nullptr;
			m_pixelShaderPostprocessImportanceMapB = nullptr;
			m_loadCounter = nullptr;
			m_loadCounterSRV = nullptr;
			m_loadCounterUAV = nullptr;
#endif

			m_pSamplerStatePointClamp = nullptr;
			m_pSamplerStateLinearClamp = nullptr;
			m_pSamplerStatePointMirror = nullptr;
			m_pSamplerStateViewspaceDepthTap = nullptr;
			
			m_pBlendStateMultiply = nullptr;
			m_pBlendStateOpaque = nullptr;

			m_requiresClear = false;
		}

		ASSAOdx11::~ASSAODX11()
		{
			DeleteAllocatedVideoMemory();
			CleanupDX();

			assert(m_allocatedVRAM == 0);
		}

		static HRESULT CompileShader(const ASSAO_CreateDesc& createDesc, CONST D3D_SHADER_MACRO* pDefines, LPCSTR pFunctionName, LPCSTR pProfile, DWORD dwShaderFlags, ID3DBlob** ppShader)
		{
			ID3DBlob* pErrorBlob = nullptr;
			HRESULT hr = D3DCompile(createDesc.ShaderData, createDesc.ShaderDataSize, nullptr, pDefines, nullptr/*D3D_COMPILE_STANDARD_FILE_INCLUDE*/, pFunctionName, pProfile, dwShaderFlags, 0, ppShader, &pErrorBlob);
			if (FAILED(hr))
			{
				MessageBoxA(nullptr, (LPCSTR)pErrorBlob->GetBufferPointer(), "Pixel shader compilation error", MB_OK);

				assert(false);
			}
			if (pErrorBlob != nullptr) pErrorBlob->Release();
			return hr;
		}

		static HRESULT CreateVertexShader(const ASSAO_CreateDesc& createDesc, CONST D3D_SHADER_MACRO* pDefines, LPCSTR pFunctionName, LPCSTR pProfile, DWORD dwShaderFlags, const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, UINT NumElements, ID3D11VertexShader ** ppShader)
		{
			ID3DBlob* shaderBlob = nullptr;

			HRESULT hr = CompileShader(createDesc, pDefines, pFunctionName, pProfile, dwShaderFlags, &shaderBlob);
			if (FAILED(hr)) { assert(false); return hr; }

			hr = createDesc.Device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, ppShader);
			if (FAILED(hr)) { SafeRelease(shaderBlob); assert(false); return hr; }

			SafeRelease(shaderBlob);

			return S_OK;
		}

		static HRESULT CreatePixelShader(const ASSAO_CreateDesc& createDesc, CONST D3D_SHADER_MACRO* pDefines, LPCSTR pFunctionName, LPCSTR pProfile, DWORD dwShaderFlags, ID3D11PixelShader ** ppShader)
		{
			ID3DBlob* shaderBlob = nullptr;

			HRESULT hr = CompileShader(createDesc, pDefines, pFunctionName, pProfile, dwShaderFlags, &shaderBlob);
			if (FAILED(hr)) { assert(false); return hr; }

			hr = createDesc.Device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, ppShader);
			if (FAILED(hr)) { SafeRelease(shaderBlob); assert(false); return hr; }

			SafeRelease(shaderBlob);

			return S_OK;
		}

		bool ASSAOdx11::InitializeDX(const ASSAO_CreateDesc& createDesc)
		{
			HRESULT hr = S_OK;

			// constant buffer
			{
				D3D11_BUFFER_DESC desc;
				desc.ByteWidth = sizeof(ASSAOConstants);
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				desc.MiscFlags = 0;
				desc.StructureByteStride = 0;

				hr = m_device->CreateBuffer(&desc, nullptr, &m_constantsBuffer);
				if (FAILED(hr))
				{
					assert(false);
					CleanupDX();
					return false;
				}
			}

			// fullscreen vertex buffer
			{
				std::vector<VertexPosTex> vertices;
				vertices.resize(3);

				vertices[0].SetVertex(math::Vector3(-1.f, 1.f, 0.f), math::Vector2(0.f, 0.f));
				vertices[1].SetVertex(math::Vector3(3.f, 1.f, 0.f), math::Vector2(2.f, 0.f));
				vertices[2].SetVertex(math::Vector3(-1.f, -3.f, 0.f), math::Vector2(0.f, 2.f));

				m_pFullScreenVB = IVertexBuffer::Create(VertexPosTex::Format(), vertices.size(), &vertices.front(), D3D11_USAGE_IMMUTABLE);
				if (m_pFullScreenVB == nullptr)
				{
					assert(false);
					CleanupDX();
					return false;
				}
			}

			{
				BlendStateDesc desc;
				desc.RenderTarget[0].BlendEnable = false;

				m_pBlendStateOpaque = GetDevice()->GetBlendState(desc);

				desc.RenderTarget[0].BlendEnable = true;
				desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
				desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
				desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
				desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;

				m_pBlendStateMultiply = GetDevice()->GetBlendState(desc);;
			}

			{
				DepthStencilStateDesc desc;
				desc.DepthEnable = false;
				m_pDepthStencilState = GetDevice()->GetDepthStencilState(desc);
				if (m_pDepthStencilState == nullptr)
				{
					assert(false);
					CleanupDX();
					return false;
				}
			}

			// samplers
			{
				SamplerStateDesc desc;
				desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				m_pSamplerStatePointClamp = GetDevice()->GetSamplerState(desc);
				if (m_pSamplerStatePointClamp == nullptr)
				{
					assert(false);
					CleanupDX();
					return false;
				}

				desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
				m_pSamplerStatePointMirror = GetDevice()->GetSamplerState(desc);
				if (m_pSamplerStatePointMirror == nullptr)
				{
					assert(false);
					CleanupDX();
					return false;
				}

				desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				m_pSamplerStateLinearClamp = GetDevice()->GetSamplerState(desc);
				if (m_pSamplerStateLinearClamp == nullptr)
				{
					assert(false);
					CleanupDX();
					return false;
				}

				desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
				desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				m_pSamplerStateViewspaceDepthTap = GetDevice()->GetSamplerState(desc);
				if (m_pSamplerStateViewspaceDepthTap == nullptr)
				{
					assert(false);
					CleanupDX();
					return false;
				}
			}

			// rasterizer state
			{
				RasterizerStateDesc rasterizerStateDesc;
				rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
				rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
				rasterizerStateDesc.ScissorEnable = true;
				m_pRasterizerState = GetDevice()->GetRasterizerState(rasterizerStateDesc);
				if (m_pRasterizerState == nullptr)
				{
					assert(false);
					CleanupDX();
					return false;
				}
			}

			// shaders
			{
				D3D_SHADER_MACRO shaderMacros[] = {
					{ "SSAO_MAX_TAPS" , SSA_STRINGIZIZER(SSAO_MAX_TAPS) },
					{ "SSAO_MAX_REF_TAPS" , SSA_STRINGIZIZER(SSAO_MAX_REF_TAPS) },
					{ "SSAO_ADAPTIVE_TAP_BASE_COUNT" , SSA_STRINGIZIZER(SSAO_ADAPTIVE_TAP_BASE_COUNT) },
					{ "SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT" , SSA_STRINGIZIZER(SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT) },
					{ "SSAO_DEPTH_MIP_LEVELS" , SSA_STRINGIZIZER(SSAO_DEPTH_MIP_LEVELS) },
					{ "SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION", SSA_STRINGIZIZER(SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION) },

					{ nullptr, nullptr }
				};

				DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
				// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
				// Setting this flag improves the shader debugging experience, but still allows 
				// the shaders to be optimized and to run exactly the way they will run in 
				// the release configuration of this program.
				shaderFlags |= D3DCOMPILE_DEBUG;
#endif
				shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
				shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;

				// vertex shader
				{
					D3D11_INPUT_ELEMENT_DESC inputElements[] =
					{
						{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
					};

					hr = CreateVertexShader(createDesc, shaderMacros, "VSMain", "vs_5_0", shaderFlags, inputElements, _countof(inputElements), &m_vertexShader);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }
				}

				// pixel shaders
				{
					hr = CreatePixelShader(createDesc, shaderMacros, "PSPrepareDepths", "ps_5_0", shaderFlags, &m_pixelShaderPrepareDepths);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSPrepareDepthsAndNormals", "ps_5_0", shaderFlags, &m_pixelShaderPrepareDepthsAndNormals);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSPrepareDepthsHalf", "ps_5_0", shaderFlags, &m_pixelShaderPrepareDepthsHalf);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSPrepareDepthsAndNormalsHalf", "ps_5_0", shaderFlags, &m_pixelShaderPrepareDepthsAndNormalsHalf);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSPrepareDepthMip1", "ps_5_0", shaderFlags, &m_pixelShaderPrepareDepthMip[0]);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSPrepareDepthMip2", "ps_5_0", shaderFlags, &m_pixelShaderPrepareDepthMip[1]);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSPrepareDepthMip3", "ps_5_0", shaderFlags, &m_pixelShaderPrepareDepthMip[2]);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSGenerateQ0", "ps_5_0", shaderFlags, &m_pixelShaderGenerate[0]);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSGenerateQ1", "ps_5_0", shaderFlags, &m_pixelShaderGenerate[1]);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSGenerateQ2", "ps_5_0", shaderFlags, &m_pixelShaderGenerate[2]);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					hr = CreatePixelShader(createDesc, shaderMacros, "PSGenerateQ3", "ps_5_0", shaderFlags, &m_pixelShaderGenerate[3]);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSGenerateQ3Base", "ps_5_0", shaderFlags, &m_pixelShaderGenerate[4]);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }
#endif

					hr = CreatePixelShader(createDesc, shaderMacros, "PSSmartBlur", "ps_5_0", shaderFlags, &m_pixelShaderSmartBlur);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSSmartBlurWide", "ps_5_0", shaderFlags, &m_pixelShaderSmartBlurWide);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSNonSmartBlur", "ps_5_0", shaderFlags, &m_pixelShaderNonSmartBlur);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSApply", "ps_5_0", shaderFlags, &m_pixelShaderApply);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSNonSmartApply", "ps_5_0", shaderFlags, &m_pixelShaderNonSmartApply);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSNonSmartHalfApply", "ps_5_0", shaderFlags, &m_pixelShaderNonSmartHalfApply);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					hr = CreatePixelShader(createDesc, shaderMacros, "PSGenerateImportanceMap", "ps_5_0", shaderFlags, &m_pixelShaderGenerateImportanceMap);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSPostprocessImportanceMapA", "ps_5_0", shaderFlags, &m_pixelShaderPostprocessImportanceMapA);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

					hr = CreatePixelShader(createDesc, shaderMacros, "PSPostprocessImportanceMapB", "ps_5_0", shaderFlags, &m_pixelShaderPostprocessImportanceMapB);
					if (FAILED(hr)) { assert(false); CleanupDX(); return false; }
#endif
				}

			}

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
			// load counter stuff, only needed for adaptive quality (quality level 3)
			{
				CD3D11_TEXTURE1D_DESC desc(DXGI_FORMAT_R32_UINT, 1, 1, 1, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);

				hr = m_device->CreateTexture1D(&desc, nullptr, &m_loadCounter);
				if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

				CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(m_loadCounter, D3D11_SRV_DIMENSION_TEXTURE1D);
				hr = m_device->CreateShaderResourceView(m_loadCounter, &srvDesc, &m_loadCounterSRV);
				if (FAILED(hr)) { assert(false); CleanupDX(); return false; }

				CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(m_loadCounter, D3D11_UAV_DIMENSION_TEXTURE1D);
				hr = m_device->CreateUnorderedAccessView(m_loadCounter, &uavDesc, &m_loadCounterUAV);
				if (FAILED(hr)) { assert(false); CleanupDX(); return false; }
			}
#endif

			return true;
		}

		void ASSAOdx11::CleanupDX()
		{
			SafeRelease(m_constantsBuffer);
			SafeDelete(m_pFullScreenVB);

			SafeRelease(m_vertexShader);

			SafeRelease(m_pixelShaderPrepareDepths);
			SafeRelease(m_pixelShaderPrepareDepthsAndNormals);
			SafeRelease(m_pixelShaderPrepareDepthsHalf);
			SafeRelease(m_pixelShaderPrepareDepthsAndNormalsHalf);
			for (auto& p : m_pixelShaderPrepareDepthMip)
			{
				SafeRelease(p);
			}
			for (auto& p : m_pixelShaderGenerate)
			{
				SafeRelease(p);
			}
			SafeRelease(m_pixelShaderSmartBlur);
			SafeRelease(m_pixelShaderSmartBlurWide);
			SafeRelease(m_pixelShaderNonSmartBlur);
			SafeRelease(m_pixelShaderApply);
			SafeRelease(m_pixelShaderNonSmartApply);
			SafeRelease(m_pixelShaderNonSmartHalfApply);
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
			SafeRelease(m_pixelShaderGenerateImportanceMap);
			SafeRelease(m_pixelShaderPostprocessImportanceMapA);
			SafeRelease(m_pixelShaderPostprocessImportanceMapB);
			SafeRelease(m_loadCounter);
			SafeRelease(m_loadCounterSRV);
			SafeRelease(m_loadCounterUAV);
#endif

			SafeRelease(m_device);
		}

		void ASSAOdx11::PreAllocateVideoMemory(const ASSAO_Inputs* _inputs)
		{
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// TODO: dynamic_cast if supported in _DEBUG to check for correct type cast below
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			const ASSAO_InputsDX11* inputs = static_cast<const ASSAO_InputsDX11 *>(_inputs);

			UpdateTextures(inputs);
		}

		void ASSAOdx11::DeleteAllocatedVideoMemory()
		{

		}

		unsigned int ASSAOdx11::GetAllocatedVideoMemory()
		{
			return m_allocatedVRAM;
		}

		//void ASSAOdx11::FullscreenPassDraw(ID3D11DeviceContext* context, ID3D11PixelShader* pixelShader, ID3D11BlendState* blendState, ID3D11DepthStencilState* depthStencilState, UINT stencilRef)
		//{
		//	if (blendState == nullptr) blendState = m_blendStateOpaque;
		//	if (depthStencilState == nullptr) depthStencilState = m_depthStencilState;

		//	// Topology
		//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//	// Vertex buffer
		//	GetDevice()->SetVertexBuffers(context, m_pFullScreenVB, m_pFullScreenVB->GetFormatSize(), 0);
		//	GetDevice()->SetInputLayout(context, EmVertexFormat::ePosTex);

		//	// Shaders and input layout
		//	//context->IASetInputLayout(m_inputLayout);
		//	context->VSSetShader(m_vertexShader, nullptr, 0);
		//	context->PSSetShader(pixelShader, nullptr, 0);

		//	float blendFactor[4] = { 0, 0, 0, 0 };
		//	context->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);
		//	context->OMSetDepthStencilState(depthStencilState, stencilRef);
		//	GetDevice()->SetRasterizerState(context, m_pRasterizerState);

		//	context->Draw(3, 0);
		//}

		void ASSAOdx11::FullscreenPassDraw(IDeviceContext* pDeviceContext, ID3D11PixelShader* pixelShader, IBlendState* pBlendState, IDepthStencilState* pDepthStencilState, UINT stencilRef)
		{
			if (pBlendState == nullptr) pBlendState = m_pBlendStateOpaque;
			if (pDepthStencilState == nullptr) pDepthStencilState = m_pDepthStencilState;

			// Topology
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			// Vertex buffer
			pDeviceContext->SetVertexBuffers(m_pFullScreenVB, m_pFullScreenVB->GetFormatSize(), 0);
			pDeviceContext->SetInputLayout(EmVertexFormat::ePosTex);

			// Shaders and input layout
			//context->IASetInputLayout(m_inputLayout);
			pDeviceContext->GetInterface()->VSSetShader(m_vertexShader, nullptr, 0);
			pDeviceContext->GetInterface()->PSSetShader(pixelShader, nullptr, 0);
			
			pDeviceContext->SetBlendState(pBlendState, math::Vector4::Zero, 0xFFFFFFFF);
			pDeviceContext->SetDepthStencilState(pDepthStencilState, stencilRef);
			//float blendFactor[4] = { 0, 0, 0, 0 };
			//context->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);
			//context->OMSetDepthStencilState(depthStencilState, stencilRef);
			pDeviceContext->SetRasterizerState(m_pRasterizerState);

			pDeviceContext->Draw(3, 0);
		}

		void ASSAOdx11::PrepareDepths(const ASSAO_Settings& settings, const ASSAO_InputsDX11* inputs)
		{
			bool generateNormals = inputs->NormalSRV == nullptr;

			IDeviceContext* dx11Context = inputs->DeviceContext;

			dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 1, &inputs->DepthSRV);

			{
				CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)m_halfSize.x, (float)m_halfSize.y);
				CD3D11_RECT rect = CD3D11_RECT(0, 0, m_halfSize.x, m_halfSize.y);
				dx11Context->GetInterface()->RSSetViewports(1, &viewport);
				dx11Context->GetInterface()->RSSetScissorRects(1, &rect); // no scissor for this
			}

			ID3D11RenderTargetView* fourDepths[] = { m_halfDepths[0].RTV, m_halfDepths[1].RTV, m_halfDepths[2].RTV, m_halfDepths[3].RTV };
			ID3D11RenderTargetView* twoDepths[] = { m_halfDepths[0].RTV, m_halfDepths[3].RTV };
			if (!generateNormals)
			{
				//VA_SCOPE_CPUGPU_TIMER( PrepareDepths, drawContext.APIContext );

				if (settings.QualityLevel < 0)
				{
					dx11Context->GetInterface()->OMSetRenderTargets(_countof(twoDepths), twoDepths, nullptr);
					FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepthsHalf);
				}
				else
				{
					dx11Context->GetInterface()->OMSetRenderTargets(_countof(fourDepths), fourDepths, nullptr);
					FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepths);
				}
			}
			else
			{
				//VA_SCOPE_CPUGPU_TIMER( PrepareDepthsAndNormals, drawContext.APIContext );

				ID3D11UnorderedAccessView* UAVs[] = { m_normals.UAV };
				if (settings.QualityLevel < 0)
				{
					dx11Context->GetInterface()->OMSetRenderTargetsAndUnorderedAccessViews(_countof(twoDepths), twoDepths, nullptr, SSAO_NORMALMAP_OUT_UAV_SLOT, 1, UAVs, nullptr);
					FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepthsAndNormalsHalf);
				}
				else
				{
					dx11Context->GetInterface()->OMSetRenderTargetsAndUnorderedAccessViews(_countof(fourDepths), fourDepths, nullptr, SSAO_NORMALMAP_OUT_UAV_SLOT, 1, UAVs, nullptr);
					FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepthsAndNormals);
				}
			}

			// only do mipmaps for higher quality levels (not beneficial on quality level 1, and detrimental on quality level 0)
			if (settings.QualityLevel > 1)
			{
				//VA_SCOPE_CPUGPU_TIMER( PrepareDepthMips, drawContext.APIContext );

				for (int i = 1; i < m_depthMipLevels; i++)
				{
					ID3D11RenderTargetView* fourDepthMips[] = { m_halfDepthsMipViews[0][i].RTV, m_halfDepthsMipViews[1][i].RTV, m_halfDepthsMipViews[2][i].RTV, m_halfDepthsMipViews[3][i].RTV };

					CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)m_halfDepthsMipViews[0][i].Size.x, (float)m_halfDepthsMipViews[0][i].Size.y);
					dx11Context->GetInterface()->RSSetViewports(1, &viewport);

					ID3D11ShaderResourceView* fourSRVs[] = { m_halfDepthsMipViews[0][i - 1].SRV, m_halfDepthsMipViews[1][i - 1].SRV, m_halfDepthsMipViews[2][i - 1].SRV, m_halfDepthsMipViews[3][i - 1].SRV };

					dx11Context->GetInterface()->OMSetRenderTargets(4, fourDepthMips, nullptr);
					dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 4, fourSRVs);
					FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepthMip[i - 1]);
				}
			}
		}

		void ASSAOdx11::GenerateSSAO(const ASSAO_Settings& settings, const ASSAO_InputsDX11* inputs, bool adaptiveBasePass)
		{
			ID3D11ShaderResourceView* normalmapSRV = (inputs->NormalSRV == nullptr) ? (m_normals.SRV) : (inputs->NormalSRV);

			IDeviceContext* dx11Context = inputs->DeviceContext;

			{
				CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)m_halfSize.x, (float)m_halfSize.y);
				CD3D11_RECT rect = CD3D11_RECT(m_halfResOutScissorRect.x, m_halfResOutScissorRect.y, m_halfResOutScissorRect.z, m_halfResOutScissorRect.w);
				dx11Context->GetInterface()->RSSetViewports(1, &viewport);
				dx11Context->GetInterface()->RSSetScissorRects(1, &rect);
			}

			if (adaptiveBasePass)
			{
				assert(settings.QualityLevel == 3);
			}

			int passCount = 4;

			ID3D11ShaderResourceView* zeroSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

			for (int pass = 0; pass < passCount; pass++)
			{
				if ((settings.QualityLevel < 0) && ((pass == 1) || (pass == 2)))
					continue;

				int blurPasses = settings.BlurPassCount;
				blurPasses = std::min(blurPasses, cMaxBlurPassCount);

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				if (settings.QualityLevel == 3)
				{
					// if adaptive, at least one blur pass needed as the first pass needs to read the final texture results - kind of awkward
					if (adaptiveBasePass)
					{
						blurPasses = 0;
					}
					else
					{
						blurPasses = std::max(1, blurPasses);
					}
				}
				else
#endif
				{
					if (settings.QualityLevel <= 0)
					{
						// just one blur pass allowed for minimum quality 
						blurPasses = std::min(1, settings.BlurPassCount);
					}
				}

				UpdateConstants(settings, inputs, pass);

				D3D11Texture2D* pPingRT = &m_pingPongHalfResultA;
				D3D11Texture2D* pPongRT = &m_pingPongHalfResultB;

				// Generate
				{
					//VA_SCOPE_CPUGPU_TIMER_NAMED( Generate, vaStringTools::Format( "Generate_pass%d", pass ), drawContext.APIContext );

					// remove textures from slots 0, 1, 2, 3 to avoid API complaints
					dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 5, zeroSRVs);

					ID3D11RenderTargetView* rts[] = { pPingRT->RTV };

					// no blur?
					if (blurPasses == 0)
						rts[0] = m_finalResultsArrayViews[pass].RTV;

					dx11Context->GetInterface()->OMSetRenderTargets(_countof(rts), rts, nullptr);

					ID3D11ShaderResourceView* SRVs[] = { m_halfDepths[pass].SRV, normalmapSRV, nullptr, nullptr, nullptr }; // m_loadCounterSRV used only for quality level 3
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					if (!adaptiveBasePass && (settings.QualityLevel == 3))
					{
						SRVs[2] = m_loadCounterSRV;
						SRVs[3] = m_importanceMap.SRV;
						SRVs[4] = m_finalResults.SRV;
					}
#endif
					dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 5, SRVs);

					int shaderIndex = std::max(0, (!adaptiveBasePass) ? (settings.QualityLevel) : (4));
					FullscreenPassDraw(dx11Context, m_pixelShaderGenerate[shaderIndex]);

					// remove textures from slots 0, 1, 2, 3 to avoid API complaints
					dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 5, zeroSRVs);
				}

				// Blur
				if (blurPasses > 0)
				{
					int wideBlursRemaining = std::max(0, blurPasses - 2);

					for (int i = 0; i < blurPasses; i++)
					{
						// remove textures to avoid API complaints
						dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT0, _countof(zeroSRVs), zeroSRVs);

						ID3D11RenderTargetView* rts[] = { pPongRT->RTV };

						// last pass?
						if (i == (blurPasses - 1))
							rts[0] = m_finalResultsArrayViews[pass].RTV;

						dx11Context->GetInterface()->OMSetRenderTargets(_countof(rts), rts, nullptr);

						ID3D11ShaderResourceView* SRVs[] = { pPingRT->SRV };
						dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT2, _countof(SRVs), SRVs);

						if (settings.QualityLevel > 0)
						{
							if (wideBlursRemaining > 0)
							{
								FullscreenPassDraw(dx11Context, m_pixelShaderSmartBlurWide);
								wideBlursRemaining--;
							}
							else
							{
								FullscreenPassDraw(dx11Context, m_pixelShaderSmartBlur);
							}
						}
						else
						{
							FullscreenPassDraw(dx11Context, m_pixelShaderNonSmartBlur); // just for quality level 0 (and -1)
						}

						std::swap(pPingRT, pPongRT);
					}
				}

				// remove textures to avoid API complaints
				dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT0, _countof(zeroSRVs), zeroSRVs);
			}
		}

		void ASSAOdx11::Draw(const ASSAO_Settings& settings, const ASSAO_Inputs* _inputs)
		{
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// TODO: dynamic_cast if supported in _DEBUG to check for correct type cast below
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			const ASSAO_InputsDX11* inputs = static_cast<const ASSAO_InputsDX11 *>(_inputs);

			assert(settings.QualityLevel >= -1 && settings.QualityLevel <= 3);
#ifndef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
			if (settings.QualityLevel == 3)
			{
				assert(false);
				return;
			}
#endif
			{
				D3D_PROFILING(inputs->DeviceContext, UpdateTextures);
				UpdateTextures(inputs);
			}

			{
				D3D_PROFILING(inputs->DeviceContext, UpdateConstants);
				UpdateConstants(settings, inputs, 0);
			}

			IDeviceContext* dx11Context = inputs->DeviceContext;

			{
				// Backup D3D11 states (will be restored when it goes out of scope)
				D3D11SSAOStateBackupRAII d3d11StatesBackup(dx11Context->GetInterface());

				if (m_requiresClear)
				{
					D3D_PROFILING(inputs->DeviceContext, Clear);
					float fourZeroes[4] = { 0, 0, 0, 0 };
					float fourOnes[4] = { 1, 1, 1, 1 };
					dx11Context->GetInterface()->ClearRenderTargetView(m_halfDepths[0].RTV, fourZeroes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_halfDepths[1].RTV, fourZeroes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_halfDepths[2].RTV, fourZeroes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_halfDepths[3].RTV, fourZeroes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_pingPongHalfResultA.RTV, fourOnes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_pingPongHalfResultB.RTV, fourZeroes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_finalResultsArrayViews[0].RTV, fourOnes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_finalResultsArrayViews[1].RTV, fourOnes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_finalResultsArrayViews[2].RTV, fourOnes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_finalResultsArrayViews[3].RTV, fourOnes);
					if (m_normals.RTV != nullptr) dx11Context->GetInterface()->ClearRenderTargetView(m_normals.RTV, fourZeroes);
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					dx11Context->GetInterface()->ClearRenderTargetView(m_importanceMap.RTV, fourZeroes);
					dx11Context->GetInterface()->ClearRenderTargetView(m_importanceMapPong.RTV, fourZeroes);
#endif

					m_requiresClear = false;
				}

				// Set effect samplers
				/*ID3D11SamplerState* samplers[] =
				{
					m_samplerStatePointClamp,
					m_samplerStateLinearClamp,
					m_samplerStatePointMirror,
					m_samplerStateViewspaceDepthTap,
				};*/
				ID3D11SamplerState* samplers[4];
				samplers[0] = m_pSamplerStatePointClamp->GetInterface();
				samplers[1] = m_pSamplerStateLinearClamp->GetInterface();
				samplers[2] = m_pSamplerStatePointMirror->GetInterface();
				samplers[3] = m_pSamplerStateViewspaceDepthTap->GetInterface();

				dx11Context->GetInterface()->PSSetSamplers(0, _countof(samplers), samplers);

				// Set constant buffer
				dx11Context->GetInterface()->PSSetConstantBuffers(SSAO_CONSTANTS_BUFFERSLOT, 1, &m_constantsBuffer);

				// Generate depths
				{
					D3D_PROFILING(inputs->DeviceContext, PrepareDepths);
					PrepareDepths(settings, inputs);
				}

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				// for adaptive quality, importance map pass
				if (settings.QualityLevel == 3)
				{
					// Generate simple quality SSAO
					{
						D3D_PROFILING(inputs->DeviceContext, GenerateSSAO);
						GenerateSSAO(settings, inputs, true);
					}

					// Generate importance map
					{
						D3D_PROFILING(inputs->DeviceContext, GenerateImportanceMap);
						CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)m_quarterSize.x, (float)m_quarterSize.y);
						CD3D11_RECT rect = CD3D11_RECT(0, 0, m_quarterSize.x, m_quarterSize.y);
						dx11Context->GetInterface()->RSSetViewports(1, &viewport);
						dx11Context->GetInterface()->RSSetScissorRects(1, &rect);

						ID3D11ShaderResourceView* zeroSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

						// drawing into importanceMap
						dx11Context->GetInterface()->OMSetRenderTargets(1, &m_importanceMap.RTV, nullptr);

						// select 4 deinterleaved AO textures (texture array)
						dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT4, 1, &m_finalResults.SRV);
						FullscreenPassDraw(dx11Context, m_pixelShaderGenerateImportanceMap, m_pBlendStateOpaque);

						// postprocess A
						dx11Context->GetInterface()->OMSetRenderTargets(1, &m_importanceMapPong.RTV, nullptr);
						dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT3, 1, &m_importanceMap.SRV);
						FullscreenPassDraw(dx11Context, m_pixelShaderPostprocessImportanceMapA, m_pBlendStateOpaque);
						dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT3, 1, zeroSRVs);

						// postprocess B
						UINT fourZeroes[4] = { 0, 0, 0, 0 };
						dx11Context->GetInterface()->ClearUnorderedAccessViewUint(m_loadCounterUAV, fourZeroes);
						dx11Context->GetInterface()->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_importanceMap.RTV, nullptr, SSAO_LOAD_COUNTER_UAV_SLOT, 1, &m_loadCounterUAV, nullptr);
						// select previous pass input importance map
						dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT3, 1, &m_importanceMapPong.SRV);
						FullscreenPassDraw(dx11Context, m_pixelShaderPostprocessImportanceMapB, m_pBlendStateOpaque);
						dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT3, 1, zeroSRVs);
					}
				}
#endif
				// Generate SSAO
				{
					D3D_PROFILING(inputs->DeviceContext, GenerateSSAO);
					GenerateSSAO(settings, inputs, false);
				}

				if (inputs->OverrideOutputRTV != nullptr)
				{
					// drawing into OverrideOutputRTV
					dx11Context->GetInterface()->OMSetRenderTargets(1, &inputs->OverrideOutputRTV, nullptr);
				}
				else
				{
					// restore previous RTs
					d3d11StatesBackup.RestoreRTs();
				}

				// Apply
				{
					D3D_PROFILING(inputs->DeviceContext, Apply);
					// select 4 deinterleaved AO textures (texture array)
					dx11Context->GetInterface()->PSSetShaderResources(SSAO_TEXTURE_SLOT4, 1, &m_finalResults.SRV);

					CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)m_size.x, (float)m_size.y);
					CD3D11_RECT rect = CD3D11_RECT(m_fullResOutScissorRect.x, m_fullResOutScissorRect.y, m_fullResOutScissorRect.z, m_fullResOutScissorRect.w);
					dx11Context->GetInterface()->RSSetViewports(1, &viewport);
					dx11Context->GetInterface()->RSSetScissorRects(1, &rect);

					IBlendState* pBlendState = (inputs->DrawOpaque) ? (m_pBlendStateOpaque) : (m_pBlendStateMultiply);

					if (settings.QualityLevel < 0)
						FullscreenPassDraw(dx11Context, m_pixelShaderNonSmartHalfApply, pBlendState);
					else if (settings.QualityLevel == 0)
						FullscreenPassDraw(dx11Context, m_pixelShaderNonSmartApply, pBlendState);
					else
						FullscreenPassDraw(dx11Context, m_pixelShaderApply, pBlendState);
				}

				// restore previous RTs again (because of the viewport hack)
				d3d11StatesBackup.RestoreRTs();

				// FullscreenPassDraw( dx11Context, m_pixelShaderDebugDraw );

			}

		}

		template<typename OutType>
		static OutType* QueryResourceInterface(ID3D11Resource* d3dResource, REFIID riid)
		{
			OutType* ret = nullptr;
			if (SUCCEEDED(d3dResource->QueryInterface(riid, (void**)&ret)))
			{
				return ret;
			}
			else
			{
				return nullptr;
			}
		}

#ifdef _DEBUG
		static math::UInt4 GetTextureDimsFromSRV(ID3D11ShaderResourceView* srv)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC desc;
			srv->GetDesc(&desc);

			assert(desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D); if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return math::UInt4::Zero;

			ID3D11Resource* res = nullptr;
			srv->GetResource(&res);

			assert(res != nullptr); if (res == nullptr) return math::UInt4::Zero;

			ID3D11Texture2D* tex = QueryResourceInterface<ID3D11Texture2D>(res, c_IID_ID3D11Texture2D);
			SafeRelease(res);
			assert(tex != nullptr); if (tex == nullptr) return math::UInt4::Zero;

			D3D11_TEXTURE2D_DESC texDesc;
			tex->GetDesc(&texDesc);
			SafeRelease(tex);

			return math::UInt4(texDesc.Width, texDesc.Height, texDesc.MipLevels, texDesc.ArraySize);
		}
#endif

		void ASSAOdx11::UpdateTextures(const ASSAO_InputsDX11* inputs)
		{
#ifdef _DEBUG
			math::UInt4 depthTexDims = GetTextureDimsFromSRV(inputs->DepthSRV);
			assert(depthTexDims.x >= inputs->ViewportWidth);
			assert(depthTexDims.y >= inputs->ViewportHeight);
			assert(depthTexDims.w == 1); // no texture arrays supported

			if (inputs->NormalSRV != nullptr)
			{
				math::UInt4 normTexDims = GetTextureDimsFromSRV(inputs->NormalSRV);
				assert(normTexDims.x >= inputs->ViewportWidth);
				assert(normTexDims.y >= inputs->ViewportHeight);
			}
#endif

			bool needsUpdate = false;

			// We've got input normals? No need to keep ours then.
			if (inputs->NormalSRV != nullptr)
			{
				if (m_normals.SRV != nullptr)
				{
					needsUpdate = true;
					m_normals.Reset();
				}
			}
			else
			{
				if (m_normals.SRV == nullptr)
				{
					needsUpdate = true;
				}
			}

			uint32_t width = inputs->ViewportWidth;
			uint32_t height = inputs->ViewportHeight;

			needsUpdate |= (m_size.x != width) || (m_size.y != height);

			m_size.x = width;
			m_size.y = height;
			m_halfSize.x = (width + 1) / 2;
			m_halfSize.y = (height + 1) / 2;
			m_quarterSize.x = (m_halfSize.x + 1) / 2;
			m_quarterSize.y = (m_halfSize.y + 1) / 2;

			math::UInt4 prevScissorRect = m_fullResOutScissorRect;

			if ((inputs->ScissorRight == 0) || (inputs->ScissorBottom == 0))
				m_fullResOutScissorRect = math::UInt4(0, 0, width, height);
			else
				m_fullResOutScissorRect = math::UInt4(std::max(0u, inputs->ScissorLeft), std::max(0u, inputs->ScissorTop), std::min(width, inputs->ScissorRight), std::min(height, inputs->ScissorBottom));

			needsUpdate |= prevScissorRect != m_fullResOutScissorRect;
			if (!needsUpdate)
				return;

			m_halfResOutScissorRect = math::UInt4(m_fullResOutScissorRect.x / 2, m_fullResOutScissorRect.y / 2, (m_fullResOutScissorRect.z + 1) / 2, (m_fullResOutScissorRect.w + 1) / 2);
			int blurEnlarge = cMaxBlurPassCount + std::max(0, cMaxBlurPassCount - 2); // +1 for max normal blurs, +2 for wide blurs
			m_halfResOutScissorRect = math::UInt4(std::max(0u, m_halfResOutScissorRect.x - blurEnlarge), std::max(0u, m_halfResOutScissorRect.y - blurEnlarge), std::min(m_halfSize.x, m_halfResOutScissorRect.z + blurEnlarge), std::min(m_halfSize.y, m_halfResOutScissorRect.w + blurEnlarge));

			float totalSizeInMB = 0.f;

			m_depthMipLevels = SSAO_DEPTH_MIP_LEVELS;

			for (int i = 0; i < 4; i++)
			{
				if (m_halfDepths[i].ReCreateIfNeeded(m_device, m_halfSize, m_formats.DepthBufferViewspaceLinear, totalSizeInMB, m_depthMipLevels, 1, false))
				{
					for (int j = 0; j < m_depthMipLevels; j++)
					{
						m_halfDepthsMipViews[i][j].Reset();
					}

					for (int j = 0; j < m_depthMipLevels; j++)
					{
						m_halfDepthsMipViews[i][j].ReCreateMIPViewIfNeeded(m_device, m_halfDepths[i], j);
					}
				}

			}
			m_pingPongHalfResultA.ReCreateIfNeeded(m_device, m_halfSize, m_formats.AOResult, totalSizeInMB, 1, 1, false);
			m_pingPongHalfResultB.ReCreateIfNeeded(m_device, m_halfSize, m_formats.AOResult, totalSizeInMB, 1, 1, false);
			m_finalResults.ReCreateIfNeeded(m_device, m_halfSize, m_formats.AOResult, totalSizeInMB, 1, 4, false);
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
			m_importanceMap.ReCreateIfNeeded(m_device, m_quarterSize, m_formats.ImportanceMap, totalSizeInMB, 1, 1, false);
			m_importanceMapPong.ReCreateIfNeeded(m_device, m_quarterSize, m_formats.ImportanceMap, totalSizeInMB, 1, 1, false);
#endif
			for (int i = 0; i < 4; i++)
			{
				m_finalResultsArrayViews[i].ReCreateArrayViewIfNeeded(m_device, m_finalResults, i);
			}

			if (inputs->NormalSRV == nullptr)
			{
				m_normals.ReCreateIfNeeded(m_device, m_size, m_formats.Normals, totalSizeInMB, 1, 1, true);
			}

			totalSizeInMB /= 1024 * 1024;
			// m_debugInfo = vaStringTools::Format( "SSAO (approx. %.2fMB memory used) ", totalSizeInMB );

			// trigger a full buffers clear first time; only really required when using scissor rects
			m_requiresClear = true;
		}

		void ASSAOdx11::UpdateConstants(const ASSAO_Settings& settings, const ASSAO_InputsDX11* inputs, int pass)
		{
			IDeviceContext* dx11Context = inputs->DeviceContext;
			bool generateNormals = inputs->NormalSRV == nullptr;

			// update constants
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (dx11Context->Map(m_constantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
			{
				assert(false); return;
			}
			else
			{
				ASSAOConstants& consts = *((ASSAOConstants*)mappedResource.pData);

				const math::Matrix& proj = inputs->ProjectionMatrix;

				consts.ViewportPixelSize = math::Vector2(1.0f / (float)m_size.x, 1.0f / (float)m_size.y);
				consts.HalfViewportPixelSize = math::Vector2(1.0f / (float)m_halfSize.x, 1.0f / (float)m_halfSize.y);

				consts.Viewport2xPixelSize = math::Vector2(consts.ViewportPixelSize.x* 2.0f, consts.ViewportPixelSize.y* 2.0f);
				consts.Viewport2xPixelSize_x_025 = math::Vector2(consts.Viewport2xPixelSize.x* 0.25f, consts.Viewport2xPixelSize.y* 0.25f);

				float depthLinearizeMul = (inputs->MatricesRowMajorOrder) ? (-proj.m[3][2]) : (-proj.m[2][3]); // float depthLinearizeMul = ( clipFar* clipNear ) / ( clipFar - clipNear );
				float depthLinearizeAdd = (inputs->MatricesRowMajorOrder) ? (proj.m[2][2]) : (proj.m[2][2]); // float depthLinearizeAdd = clipFar / ( clipFar - clipNear );
																													 // correct the handedness issue. need to make sure this below is correct, but I think it is.
				if (depthLinearizeMul* depthLinearizeAdd < 0)
					depthLinearizeAdd = -depthLinearizeAdd;
				consts.DepthUnpackConsts = math::Vector2(depthLinearizeMul, depthLinearizeAdd);

				float tanHalfFOVY = 1.0f / proj.m[1][1]; // = tanf( drawContext.Camera.GetYFOV( )* 0.5f );
				float tanHalfFOVX = 1.0F / proj.m[0][0]; // = tanHalfFOVY* drawContext.Camera.GetAspect( );
				consts.CameraTanHalfFOV = math::Vector2(tanHalfFOVX, tanHalfFOVY);

				consts.NDCToViewMul = math::Vector2(consts.CameraTanHalfFOV.x* 2.0f, consts.CameraTanHalfFOV.y* -2.0f);
				consts.NDCToViewAdd = math::Vector2(consts.CameraTanHalfFOV.x* -1.0f, consts.CameraTanHalfFOV.y* 1.0f);

				consts.EffectRadius = std::clamp(settings.Radius, 0.0f, 100000.0f);
				consts.EffectShadowStrength = std::clamp(settings.ShadowMultiplier* 4.3f, 0.0f, 10.0f);
				consts.EffectShadowPow = std::clamp(settings.ShadowPower, 0.0f, 10.0f);
				consts.EffectShadowClamp = std::clamp(settings.ShadowClamp, 0.0f, 1.0f);
				consts.EffectFadeOutMul = -1.0f / (settings.FadeOutTo - settings.FadeOutFrom);
				consts.EffectFadeOutAdd = settings.FadeOutFrom / (settings.FadeOutTo - settings.FadeOutFrom) + 1.0f;
				consts.EffectHorizonAngleThreshold = std::clamp(settings.HorizonAngleThreshold, 0.0f, 1.0f);

				// 1.2 seems to be around the best trade off - 1.0 means on-screen radius will stop/slow growing when the camera is at 1.0 distance, so, depending on FOV, basically filling up most of the screen
				// This setting is viewspace-dependent and not screen size dependent intentionally, so that when you change FOV the effect stays (relatively) similar.
				float effectSamplingRadiusNearLimit = (settings.Radius* 1.2f);

				// if the depth precision is switched to 32bit float, this can be set to something closer to 1 (0.9999 is fine)
				consts.DepthPrecisionOffsetMod = 0.9992f;

				// consts.RadiusDistanceScalingFunctionPow = 1.0f - Clamp( settings.RadiusDistanceScalingFunction, 0.0f, 1.0f );

				// used to get average load per pixel; 9.0 is there to compensate for only doing every 9th InterlockedAdd in PSPostprocessImportanceMapB for performance reasons
				consts.LoadCounterAvgDiv = 9.0f / (float)(m_quarterSize.x* m_quarterSize.y* 255.0);

				// Special settings for lowest quality level - just nerf the effect a tiny bit
				if (settings.QualityLevel <= 0)
				{
					//consts.EffectShadowStrength *= 0.9f;
					effectSamplingRadiusNearLimit *= 1.50f;

					if (settings.QualityLevel < 0)
					{
						consts.EffectRadius *= 0.8f;
					}
				}
				effectSamplingRadiusNearLimit /= tanHalfFOVY; // to keep the effect same regardless of FOV

				consts.EffectSamplingRadiusNearLimitRec = 1.0f / effectSamplingRadiusNearLimit;

				consts.AdaptiveSampleCountLimit = settings.AdaptiveQualityLimit;

				consts.NegRecEffectRadius = -1.0f / consts.EffectRadius;

				consts.PerPassFullResCoordOffset = math::Int2(pass % 2, pass / 2);
				consts.PerPassFullResUVOffset = math::Vector2(((pass % 2) - 0.0f) / m_size.x, ((pass / 2) - 0.0f) / m_size.y);

				consts.InvSharpness = std::clamp(1.0f - settings.Sharpness, 0.0f, 1.0f);
				consts.PassIndex = pass;
				consts.QuarterResPixelSize = math::Vector2(1.0f / (float)m_quarterSize.x, 1.0f / (float)m_quarterSize.y);

				float additionalAngleOffset = settings.TemporalSupersamplingAngleOffset; // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
				float additionalRadiusScale = settings.TemporalSupersamplingRadiusOffset; // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
				const int subPassCount = 5;
				for (int subPass = 0; subPass < subPassCount; subPass++)
				{
					int a = pass;
					int b = subPass;

					int spmap[5]{ 0, 1, 4, 3, 2 };
					b = spmap[subPass];

					float ca, sa;
					float angle0 = ((float)a + (float)b / (float)subPassCount)* (3.1415926535897932384626433832795f)* 0.5f;
					angle0 += additionalAngleOffset;

					ca = ::cosf(angle0);
					sa = ::sinf(angle0);

					float scale = 1.0f + (a - 1.5f + (b - (subPassCount - 1.0f)* 0.5f) / (float)subPassCount)* 0.07f;
					scale *= additionalRadiusScale;

					consts.PatternRotScaleMatrices[subPass] = math::Vector4(scale* ca, scale* -sa, -scale* sa, -scale* ca);
				}

				if (!generateNormals)
				{
					consts.NormalsUnpackMul = inputs->NormalsUnpackMul;
					consts.NormalsUnpackAdd = inputs->NormalsUnpackAdd;
				}
				else
				{
					consts.NormalsUnpackMul = 2.0f;
					consts.NormalsUnpackAdd = -1.0f;
				}
				consts.DetailAOStrength = settings.DetailShadowStrength;
				consts.Dummy0 = 0.0f;

#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
				if (!generateNormals)
				{
					consts.NormalsWorldToViewspaceMatrix = inputs->NormalsWorldToViewspaceMatrix;
					if (!inputs->MatricesRowMajorOrder)
					{
						consts.NormalsWorldToViewspaceMatrix.Transpose();
					}
				}
				else
				{
					consts.NormalsWorldToViewspaceMatrix = math::Matrix::Identity;
				}
#endif

				dx11Context->Unmap(m_constantsBuffer, 0);
				//m_constantsBuffer.Update( dx11Context, consts );
			}
		}

		static int GetPixelSizeInBytes(DXGI_FORMAT val)
		{
			switch (val)
			{
			case DXGI_FORMAT_UNKNOWN: return 0;
			case DXGI_FORMAT_R32G32B32A32_TYPELESS: return 4* 4;
			case DXGI_FORMAT_R32G32B32A32_FLOAT: return 4* 4;
			case DXGI_FORMAT_R32G32B32A32_UINT: return 4* 4;
			case DXGI_FORMAT_R32G32B32A32_SINT: return 4* 4;
			case DXGI_FORMAT_R32G32B32_TYPELESS: return 3* 4;
			case DXGI_FORMAT_R32G32B32_FLOAT: return 3* 4;
			case DXGI_FORMAT_R32G32B32_UINT: return 3* 4;
			case DXGI_FORMAT_R32G32B32_SINT: return 3* 4;
			case DXGI_FORMAT_R16G16B16A16_TYPELESS: return 4* 2;
			case DXGI_FORMAT_R16G16B16A16_FLOAT: return 4* 2;
			case DXGI_FORMAT_R16G16B16A16_UNORM: return 4* 2;
			case DXGI_FORMAT_R16G16B16A16_UINT: return 4* 2;
			case DXGI_FORMAT_R16G16B16A16_SNORM: return 4* 2;
			case DXGI_FORMAT_R16G16B16A16_SINT: return 4* 2;
			case DXGI_FORMAT_R32G32_TYPELESS: return 2* 4;
			case DXGI_FORMAT_R32G32_FLOAT: return 2* 4;
			case DXGI_FORMAT_R32G32_UINT: return 2* 4;
			case DXGI_FORMAT_R32G32_SINT: return 2* 4;
			case DXGI_FORMAT_R32G8X24_TYPELESS: return 4 + 1;
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return 4 + 1;
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return 4 + 1;
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return 4 + 1;
			case DXGI_FORMAT_R10G10B10A2_TYPELESS: return 4;
			case DXGI_FORMAT_R10G10B10A2_UNORM: return 4;
			case DXGI_FORMAT_R10G10B10A2_UINT: return 4;
			case DXGI_FORMAT_R11G11B10_FLOAT: return 4;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS: return 4;
			case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return 4;
			case DXGI_FORMAT_R8G8B8A8_UINT: return 4;
			case DXGI_FORMAT_R8G8B8A8_SNORM: return 4;
			case DXGI_FORMAT_R8G8B8A8_SINT: return 4;
			case DXGI_FORMAT_R16G16_TYPELESS: return 4;
			case DXGI_FORMAT_R16G16_FLOAT: return 4;
			case DXGI_FORMAT_R16G16_UNORM: return 4;
			case DXGI_FORMAT_R16G16_UINT: return 4;
			case DXGI_FORMAT_R16G16_SNORM: return 4;
			case DXGI_FORMAT_R16G16_SINT: return 4;
			case DXGI_FORMAT_R32_TYPELESS: return 4;
			case DXGI_FORMAT_D32_FLOAT: return 4;
			case DXGI_FORMAT_R32_FLOAT: return 4;
			case DXGI_FORMAT_R32_UINT: return 4;
			case DXGI_FORMAT_R32_SINT: return 4;
			case DXGI_FORMAT_R24G8_TYPELESS: return 4;
			case DXGI_FORMAT_D24_UNORM_S8_UINT: return 4;
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return 4;
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return 4;
			case DXGI_FORMAT_R8G8_TYPELESS: return 2;
			case DXGI_FORMAT_R8G8_UNORM: return 2;
			case DXGI_FORMAT_R8G8_UINT: return 2;
			case DXGI_FORMAT_R8G8_SNORM: return 2;
			case DXGI_FORMAT_R8G8_SINT: return 2;
			case DXGI_FORMAT_R16_TYPELESS: return 2;
			case DXGI_FORMAT_R16_FLOAT: return 2;
			case DXGI_FORMAT_D16_UNORM: return 2;
			case DXGI_FORMAT_R16_UNORM: return 2;
			case DXGI_FORMAT_R16_UINT: return 2;
			case DXGI_FORMAT_R16_SNORM: return 2;
			case DXGI_FORMAT_R16_SINT: return 2;
			case DXGI_FORMAT_R8_TYPELESS: return 1;
			case DXGI_FORMAT_R8_UNORM: return 1;
			case DXGI_FORMAT_R8_UINT: return 1;
			case DXGI_FORMAT_R8_SNORM: return 1;
			case DXGI_FORMAT_R8_SINT: return 1;
			case DXGI_FORMAT_A8_UNORM: return 1;
			case DXGI_FORMAT_R1_UNORM: return 1;
			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return 4;
			case DXGI_FORMAT_R8G8_B8G8_UNORM: return 4;
			case DXGI_FORMAT_G8R8_G8B8_UNORM: return 4;
			case DXGI_FORMAT_BC1_TYPELESS: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC1_UNORM: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC1_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC2_TYPELESS: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC2_UNORM: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC2_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC3_TYPELESS: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC3_UNORM: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC3_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC4_TYPELESS: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC4_UNORM: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC4_SNORM: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC5_TYPELESS: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC5_UNORM: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC5_SNORM: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_B5G6R5_UNORM: return 2;
			case DXGI_FORMAT_B5G5R5A1_UNORM: return 2;
			case DXGI_FORMAT_B8G8R8A8_UNORM: return 4;
			case DXGI_FORMAT_B8G8R8X8_UNORM: return 4;
			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return 4;
			case DXGI_FORMAT_B8G8R8A8_TYPELESS: return 4;
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return 4;
			case DXGI_FORMAT_B8G8R8X8_TYPELESS: return 4;
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return 4;
			case DXGI_FORMAT_BC6H_TYPELESS: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC6H_UF16: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC6H_SF16: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC7_TYPELESS: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC7_UNORM: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_BC7_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
			case DXGI_FORMAT_AYUV: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_Y410: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_Y416: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_NV12: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_P010: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_P016: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_YUY2: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_Y210: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_Y216: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_NV11: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_AI44: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_IA44: assert(false); return 0; // not yet implemented
			case DXGI_FORMAT_P8: return 1;
			case DXGI_FORMAT_A8P8: return 2;
			case DXGI_FORMAT_B4G4R4A4_UNORM: return 2;
			default: break;
			}
			assert(false);
			return 0;
		}

		bool D3D11Texture2D::ReCreateIfNeeded(ID3D11Device* device, const math::UInt2& size, DXGI_FORMAT format, float& inoutTotalSizeSum, uint32_t mipLevels, uint32_t arraySize, bool supportUAVs)
		{
			int approxSize = size.x* size.y* GetPixelSizeInBytes(format);
			if (mipLevels != 1) approxSize = approxSize* 2; // is this an overestimate?
			inoutTotalSizeSum += approxSize* arraySize;

			if ((size.x == 0) || (size.y == 0) || (format == DXGI_FORMAT_UNKNOWN))
			{
				Reset();
			}
			else
			{
				if (Texture2D != nullptr)
				{
					D3D11_TEXTURE2D_DESC desc;
					Texture2D->GetDesc(&desc);
					if ((desc.Format == format) && (desc.Width == size.x) && (desc.Height == size.y) && (desc.MipLevels == mipLevels) && (desc.ArraySize == arraySize))
						return false;
				}

				Reset();

				UINT bindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				if (supportUAVs)
					bindFlags |= D3D11_BIND_UNORDERED_ACCESS;

				HRESULT hr;

				CD3D11_TEXTURE2D_DESC desc(format, size.x, size.y, arraySize, mipLevels, bindFlags);

				hr = device->CreateTexture2D(&desc, nullptr, &Texture2D);
				if (FAILED(hr)) { assert(false); Reset(); return false; }

				CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(Texture2D, (arraySize == 1) ? (D3D11_SRV_DIMENSION_TEXTURE2D) : (D3D11_SRV_DIMENSION_TEXTURE2DARRAY));
				hr = device->CreateShaderResourceView(Texture2D, &srvDesc, &SRV);
				if (FAILED(hr)) { assert(false); Reset(); return false; }

				if (arraySize == 1)
				{
					CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(Texture2D, D3D11_RTV_DIMENSION_TEXTURE2D);
					hr = device->CreateRenderTargetView(Texture2D, &rtvDesc, &RTV);
					if (FAILED(hr)) { assert(false); Reset(); return false; }
				}

				if (supportUAVs)
				{
					CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(Texture2D, D3D11_UAV_DIMENSION_TEXTURE2D);
					hr = device->CreateUnorderedAccessView(Texture2D, &uavDesc, &UAV);
					if (FAILED(hr)) { assert(false); Reset(); return false; }
				}

				this->Size = size;
			}

			return true;
		}

		bool D3D11Texture2D::ReCreateMIPViewIfNeeded(ID3D11Device* device, D3D11Texture2D& original, int mipViewSlice)
		{
			if (original.Texture2D == this->Texture2D)
				return true;

			Reset();

			this->Texture2D = original.Texture2D;
			this->Texture2D->AddRef();

			HRESULT hr;

			CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(Texture2D, D3D11_SRV_DIMENSION_TEXTURE2D);
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = mipViewSlice;
			hr = device->CreateShaderResourceView(Texture2D, &srvDesc, &SRV);
			if (FAILED(hr)) { assert(false); Reset(); return false; }

			CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(Texture2D, D3D11_RTV_DIMENSION_TEXTURE2D);
			rtvDesc.Texture2D.MipSlice = mipViewSlice;
			hr = device->CreateRenderTargetView(Texture2D, &rtvDesc, &RTV);
			if (FAILED(hr)) { assert(false); Reset(); return false; }

			this->Size = original.Size;

			for (int i = 0; i < mipViewSlice; i++)
			{
				this->Size.x = (this->Size.x + 1) / 2;
				this->Size.y = (this->Size.y + 1) / 2;
			}
			this->Size.x = std::max(this->Size.x, 1u);
			this->Size.y = std::max(this->Size.y, 1u);

			return true;
		}

		bool D3D11Texture2D::ReCreateArrayViewIfNeeded(ID3D11Device* device, D3D11Texture2D& original, int arraySlice)
		{
			if (original.Texture2D == this->Texture2D)
				return true;

			Reset();

			this->Texture2D = original.Texture2D;
			this->Texture2D->AddRef();

			HRESULT hr;

			CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(Texture2D, D3D11_RTV_DIMENSION_TEXTURE2DARRAY);

			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.FirstArraySlice = arraySlice;
			rtvDesc.Texture2DArray.ArraySize = 1;

			hr = device->CreateRenderTargetView(Texture2D, &rtvDesc, &RTV);
			if (FAILED(hr)) { assert(false); Reset(); return false; }

			this->Size = original.Size;

			return true;
		}

		ASSAO::ASSAO()
			: m_pEffect(nullptr)
		{
		}

		ASSAO::~ASSAO()
		{
			Release();
		}

		bool ASSAO::Init()
		{
			std::string strPath(file::GetPath(file::eFx));
			strPath += "PostProcessing\\ASSAO\\ASSAO.fx";

			ID3DBlob* pBlob = nullptr;
			if (FAILED(D3DReadFileToBlob(String::MultiToWide(strPath).c_str(), &pBlob)))
				return nullptr;

			m_pEffect = ASSAO_Effect::CreateInstance(ASSAO_CreateDesc(GetDevice()->GetInterface(), pBlob->GetBufferPointer(), pBlob->GetBufferSize()));
			SafeRelease(pBlob);

			if (m_pEffect == nullptr)
			{
				assert(false);
				Release();
				return false;
			}

			return true;
		}

		void ASSAO::Release()
		{
			ASSAO_Effect::DestroyInstance(m_pEffect);
			m_pEffect = nullptr;
		}

		void ASSAO::Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, IRenderTarget* pResult)
		{
			TRACER_EVENT("ASSAO::Apply");
			D3D_PROFILING(pDeviceContext, SSAO);

			int nThreadID = GetThreadID(ThreadType::eRender);

			const math::Viewport& viewport = pDevice->GetViewport();

			ASSAO_InputsDX11 inputs;
			inputs.ScissorLeft = 0;
			inputs.ScissorTop = 0;
			inputs.ScissorRight = static_cast<int>(viewport.width);
			inputs.ScissorBottom = static_cast<int>(viewport.height);
			inputs.DeviceContext = pDeviceContext;
			inputs.ProjectionMatrix = pCamera->GetProjMatrix(nThreadID);
			inputs.ViewportWidth = static_cast<int>(viewport.width);
			inputs.ViewportHeight = static_cast<int>(viewport.height);
			inputs.DepthSRV = GetDevice()->GetMainDepthStencil()->GetTexture()->GetShaderResourceView();
			inputs.NormalSRV = GetGBuffers()->GetGBuffer(EmGBuffer::eNormals)->GetTexture()->GetShaderResourceView();
			inputs.MatricesRowMajorOrder = true;
			inputs.DrawOpaque = false;
#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
			inputs.NormalsWorldToViewspaceMatrix = pCamera->GetViewMatrix(nThreadID).Transpose();
#endif

			pDeviceContext->SetRenderTargets(&pResult, 1, nullptr);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);

			m_pEffect->Draw(m_settings, &inputs);
		}
	}
}