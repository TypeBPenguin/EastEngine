#include "stdafx.h"
#include "AssaoDX11.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/AssaoInterface.h"
#include "Graphics/Interface/Camera.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"

#include "RenderTargetDX11.h"
#include "DepthStencilDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				// ** WARNING ** if changing anything here, update the corresponding shader code! ** WARNING **
				struct ASSAOConstants
				{
					math::float2 ViewportPixelSize; // .zw == 1.0 / ViewportSize.xy
					math::float2 HalfViewportPixelSize; // .zw == 1.0 / ViewportHalfSize.xy

					math::float2 DepthUnpackConsts;
					math::float2 CameraTanHalfFOV;

					math::float2 NDCToViewMul;
					math::float2 NDCToViewAdd;

					math::int2 PerPassFullResCoordOffset;
					math::float2 PerPassFullResUVOffset;

					math::float2 Viewport2xPixelSize;
					math::float2 Viewport2xPixelSize_x_025; // Viewport2xPixelSize* 0.25 (for fusing add+mul into mad)

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
					math::float2 QuarterResPixelSize; // used for importance map only

					math::float4 PatternRotScaleMatrices[5];

					float NormalsUnpackMul;
					float NormalsUnpackAdd;
					float DetailAOStrength;
					float Dummy0;

#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
					math::Matrix NormalsWorldToViewspaceMatrix;
#endif
				};
			}

			// Simplify texture creation and potential future porting
			struct D3D11Texture2D
			{
				ID3D11Texture2D* Texture2D{ nullptr };
				ID3D11ShaderResourceView* SRV{ nullptr };
				ID3D11RenderTargetView* RTV{ nullptr };
				ID3D11UnorderedAccessView* UAV{ nullptr };
				math::uint2 Size;

				~D3D11Texture2D() { Reset(); }

				void Reset() { SafeRelease(Texture2D); SafeRelease(SRV); /*SafeRelease( DSV );*/ SafeRelease(RTV); SafeRelease(UAV); Size = math::uint2::Zero; }
				bool ReCreateIfNeeded(ID3D11Device* device, const math::uint2& size, DXGI_FORMAT format, float& inoutTotalSizeSum, uint32_t mipLevels, uint32_t arraySize, bool supportUAVs);
				bool ReCreateMIPViewIfNeeded(ID3D11Device* device, D3D11Texture2D& original, int mipViewSlice);
				bool ReCreateArrayViewIfNeeded(ID3D11Device* device, D3D11Texture2D& original, int arraySlice);
			};

			bool D3D11Texture2D::ReCreateIfNeeded(ID3D11Device* device, const math::uint2& size, DXGI_FORMAT format, float& inoutTotalSizeSum, uint32_t mipLevels, uint32_t arraySize, bool supportUAVs)
			{
				int approxSize = size.x * size.y * util::GetPixelSizeInBytes(format);
				if (mipLevels != 1)
				{
					approxSize = approxSize * 2; // is this an overestimate?
				}
				inoutTotalSizeSum += approxSize * arraySize;

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
					{
						bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
					}

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

			// Just to reduce clutter in the main code
			class D3D11SSAOStateBackupRAII
			{
				ID3D11DeviceContext* m_dx11Context{ nullptr };

				D3D11_VIEWPORT m_VPs[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				uint32_t m_numViewports;
				D3D11_RECT m_scissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				uint32_t m_numScissorRects;
				ID3D11SamplerState* m_samplerStates[4];
				ID3D11Buffer* m_constantBuffers[1];
				ID3D11ShaderResourceView* m_SRVs[5];
				ID3D11BlendState* m_blendState;
				FLOAT m_blendStateBlendFactor[4];
				uint32_t m_blendStateSampleMask;
				ID3D11DepthStencilState* m_depthStencilState;
				uint32_t m_depthStencilStateStencilRef;
				D3D_PRIMITIVE_TOPOLOGY m_primitiveTopology;
				ID3D11InputLayout* m_inputLayout;
				ID3D11RasterizerState* m_rasterizerState;
				uint32_t m_numRTVs;
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

			struct AssaoInputsDX11 : public IAssaoInputs
			{
				ID3D11DeviceContext* DeviceContext{ nullptr };

				// Hardware screen depths
				//  - R32_FLOAT (R32_TYPELESS) or R24_UNORM_X8_TYPELESS (R24G8_TYPELESS) texture formats are supported.
				//  - Multisampling not yet supported.
				//  - Decoded using provided ProjectionMatrix.
				//  - For custom decoding see PSPrepareDepths/PSPrepareDepthsAndNormals where they get converted to linear viewspace storage.
				ID3D11ShaderResourceView* DepthSRV{ nullptr };

				// Viewspace normals (optional) 
				//  - If NULL, normals are generated from the depth buffer, otherwise provided normals are used for AO. ASSAO is less
				//    costly when input normals are provided, and has a more defined effect. However, aliasing in normals can result in 
				//    aliasing/flickering in the effect so, in some rare cases, normals generated from the depth buffer can look better.
				//  - _FLOAT or _UNORM texture formats are supported.
				//  - Input normals are expected to be in viewspace, encoded in [0, 1] with "encodedNormal.xyz = (normal.xyz* 0.5 + 0.5)" 
				//    or similar. Decode is done in LoadNormal() function with "normal.xyz = (encodedNormal.xyz* 2.0 - 1.0)", which can be
				//    easily modified for any custom decoding.
				//  - Use SSAO_SMOOTHEN_NORMALS for additional normal smoothing to reduce aliasing/flickering. This, however, also reduces
				//    high detail AO and increases cost.
				ID3D11ShaderResourceView* NormalSRV{ nullptr };

				// If not NULL, instead writing into currently bound render target, Draw will use this. Current render target will be restored 
				// to what it was originally after the Draw call.
				ID3D11RenderTargetView* OverrideOutputRTV{ nullptr };
			};

			class Assao::Impl : public IAssaoEffect
			{
			public:
				Impl();
				virtual ~Impl();

			public:
				// ASSAO_Effect implementation
				virtual void PreAllocateVideoMemory(const IAssaoInputs* inputs) override;
				virtual void DeleteAllocatedVideoMemory() override;

				virtual uint32_t GetAllocatedVideoMemory() override;

				virtual void GetVersion(int& major, int& minor) override;

				// Apply the SSAO effect to the currently selected render target using provided config and platform-dependent inputs
				virtual void Draw(const Options::AssaoConfig& config, const IAssaoInputs* inputs) override;

			private:
				bool InitializeDX();
				void CleanupDX();

			private:
				void UpdateTextures(const AssaoInputsDX11* inputs);
				void UpdateConstants(const Options::AssaoConfig& config, const AssaoInputsDX11* inputs, int pass);
				void FullscreenPassDraw(ID3D11DeviceContext* context, ID3D11PixelShader* pixelShader, ID3D11BlendState* blendState = nullptr, ID3D11DepthStencilState* depthStencilState = nullptr);
				void PrepareDepths(const Options::AssaoConfig& config, const AssaoInputsDX11* inputs);
				void GenerateSSAO(const Options::AssaoConfig& config, const AssaoInputsDX11* inputs, bool adaptiveBasePass);

			private:
				struct BufferFormats
				{
					DXGI_FORMAT DepthBufferViewspaceLinear{ DXGI_FORMAT_R16_FLOAT };	// increase this to DXGI_FORMAT_R32_FLOAT if using very low FOVs (for a scope effect) or similar, or in case you suspect artifacts caused by lack of precision; performance will degrade
					DXGI_FORMAT Normals{ DXGI_FORMAT_R8G8B8A8_UNORM };	//Normals = DXGI_FORMAT_R8G8B8A8_UNORM;
					DXGI_FORMAT AOResult{ DXGI_FORMAT_R8G8_UNORM };
					DXGI_FORMAT ImportanceMap{ DXGI_FORMAT_R8_UNORM };
				};
				BufferFormats m_formats;

				math::uint2 m_size;
				math::uint2 m_halfSize;
				math::uint2 m_quarterSize;
				math::uint4 m_fullResOutScissorRect;
				math::uint4 m_halfResOutScissorRect;

				uint32_t m_allocatedVRAM{ 0 };

				ConstantBuffer<shader::ASSAOConstants> m_constantsBuffer;

				ID3D11Device* m_device{ nullptr };

				ID3D11VertexShader* m_pVertexShader{ nullptr };

				ID3D11RasterizerState* m_pRasterizerState{ nullptr };

				ID3D11SamplerState* m_pSamplerStatePointClamp{ nullptr };
				ID3D11SamplerState* m_pSamplerStateLinearClamp{ nullptr };
				ID3D11SamplerState* m_pSamplerStatePointMirror{ nullptr };
				ID3D11SamplerState* m_pSamplerStateViewspaceDepthTap{ nullptr };

				ID3D11BlendState* m_pBlendStateMultiply{ nullptr };
				ID3D11BlendState* m_pBlendStateOpaque{ nullptr };

				ID3D11DepthStencilState* m_pDepthStencilState{ nullptr };

				ID3D11PixelShader* m_pixelShaderPrepareDepths{ nullptr };
				ID3D11PixelShader* m_pixelShaderPrepareDepthsAndNormals{ nullptr };
				ID3D11PixelShader* m_pixelShaderPrepareDepthsHalf{ nullptr };
				ID3D11PixelShader* m_pixelShaderPrepareDepthsAndNormalsHalf{ nullptr };
				ID3D11PixelShader* m_pixelShaderPrepareDepthMip[SSAO_DEPTH_MIP_LEVELS - 1]{};
				ID3D11PixelShader* m_pixelShaderGenerate[5]{};
				ID3D11PixelShader* m_pixelShaderSmartBlur{ nullptr };
				ID3D11PixelShader* m_pixelShaderSmartBlurWide{ nullptr };
				ID3D11PixelShader* m_pixelShaderApply{ nullptr };
				ID3D11PixelShader* m_pixelShaderNonSmartBlur{ nullptr };
				ID3D11PixelShader* m_pixelShaderNonSmartApply{ nullptr };
				ID3D11PixelShader* m_pixelShaderNonSmartHalfApply{ nullptr };
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				ID3D11PixelShader* m_pixelShaderGenerateImportanceMap{ nullptr };
				ID3D11PixelShader* m_pixelShaderPostprocessImportanceMapA{ nullptr };
				ID3D11PixelShader* m_pixelShaderPostprocessImportanceMapB{ nullptr };
#endif

				D3D11Texture2D m_halfDepths[4];
				D3D11Texture2D m_halfDepthsMipViews[4][SSAO_DEPTH_MIP_LEVELS];

				D3D11Texture2D m_pingPongHalfResultA;
				D3D11Texture2D m_pingPongHalfResultB;
				D3D11Texture2D m_finalResults;
				D3D11Texture2D m_finalResultsArrayViews[4];
				D3D11Texture2D m_normals;
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				// Only needed for quality level 3 (adaptive quality)
				D3D11Texture2D m_importanceMap;
				D3D11Texture2D m_importanceMapPong;
				ID3D11Texture1D* m_loadCounter{ nullptr };
				ID3D11ShaderResourceView* m_loadCounterSRV{ nullptr };;
				ID3D11UnorderedAccessView* m_loadCounterUAV{ nullptr };;
#endif

				bool m_requiresClear{ false };
			};

			Assao::Impl::Impl()
			{
				InitializeDX();
			}

			Assao::Impl::~Impl()
			{
				CleanupDX();
			}

			void Assao::Impl::PreAllocateVideoMemory(const IAssaoInputs* _inputs)
			{
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// TODO: dynamic_cast if supported in _DEBUG to check for correct type cast below
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				const AssaoInputsDX11* inputs = static_cast<const AssaoInputsDX11*>(_inputs);

				UpdateTextures(inputs);
			}

			void Assao::Impl::DeleteAllocatedVideoMemory()
			{
			}

			uint32_t Assao::Impl::GetAllocatedVideoMemory()
			{
				return m_allocatedVRAM;
			}

			void Assao::Impl::GetVersion(int& major, int& minor)
			{
				major = 1;
				minor = 0;
			}

			void Assao::Impl::Draw(const Options::AssaoConfig& config, const IAssaoInputs* _inputs)
			{
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// TODO: dynamic_cast if supported in _DEBUG to check for correct type cast below
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				const AssaoInputsDX11* inputs = static_cast<const AssaoInputsDX11*>(_inputs);

				assert(inputs->DeviceContext != nullptr);
				assert(inputs->DepthSRV != nullptr);
				assert(inputs->NormalSRV != nullptr);
				assert(inputs->OverrideOutputRTV != nullptr);

				assert(config.QualityLevel >= -1 && config.QualityLevel <= 3);
#ifndef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				if (config.QualityLevel == 3)
				{
					assert(false);
					return;
				}
#endif
				UpdateTextures(inputs);

				UpdateConstants(config, inputs, 0);

				ID3D11DeviceContext* dx11Context = inputs->DeviceContext;

				{
					// Backup D3D11 states (will be restored when it goes out of scope)
					D3D11SSAOStateBackupRAII d3d11StatesBackup(dx11Context);

					if (m_requiresClear)
					{
						DX_PROFILING(Clear);

						const float fourZeroes[4] = { 0, 0, 0, 0 };
						const float fourOnes[4] = { 1, 1, 1, 1 };
						dx11Context->ClearRenderTargetView(m_halfDepths[0].RTV, fourZeroes);
						dx11Context->ClearRenderTargetView(m_halfDepths[1].RTV, fourZeroes);
						dx11Context->ClearRenderTargetView(m_halfDepths[2].RTV, fourZeroes);
						dx11Context->ClearRenderTargetView(m_halfDepths[3].RTV, fourZeroes);
						dx11Context->ClearRenderTargetView(m_pingPongHalfResultA.RTV, fourOnes);
						dx11Context->ClearRenderTargetView(m_pingPongHalfResultB.RTV, fourZeroes);
						dx11Context->ClearRenderTargetView(m_finalResultsArrayViews[0].RTV, fourOnes);
						dx11Context->ClearRenderTargetView(m_finalResultsArrayViews[1].RTV, fourOnes);
						dx11Context->ClearRenderTargetView(m_finalResultsArrayViews[2].RTV, fourOnes);
						dx11Context->ClearRenderTargetView(m_finalResultsArrayViews[3].RTV, fourOnes);
						if (m_normals.RTV != nullptr)
						{
							dx11Context->ClearRenderTargetView(m_normals.RTV, fourZeroes);
						}
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
						dx11Context->ClearRenderTargetView(m_importanceMap.RTV, fourZeroes);
						dx11Context->ClearRenderTargetView(m_importanceMapPong.RTV, fourZeroes);
#endif
						m_requiresClear = false;
					}

					// Set effect samplers
					ID3D11SamplerState* samplers[] =
					{
						m_pSamplerStatePointClamp,
						m_pSamplerStateLinearClamp,
						m_pSamplerStatePointMirror,
						m_pSamplerStateViewspaceDepthTap,
					};
					dx11Context->PSSetSamplers(0, _countof(samplers), samplers);

					// Set constant buffer
					dx11Context->PSSetConstantBuffers(SSAO_CONSTANTS_BUFFERSLOT, 1, &m_constantsBuffer.pBuffer);

					// Generate depths
					PrepareDepths(config, inputs);

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					// for adaptive quality, importance map pass
					if (config.QualityLevel == 3)
					{
						// Generate simple quality SSAO
						GenerateSSAO(config, inputs, true);

						// Generate importance map
						{
							DX_PROFILING(GenerateImportanceMap);

							const CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_quarterSize.x), static_cast<float>(m_quarterSize.y));
							const CD3D11_RECT rect = CD3D11_RECT(0, 0, m_quarterSize.x, m_quarterSize.y);
							dx11Context->RSSetViewports(1, &viewport);
							dx11Context->RSSetScissorRects(1, &rect);

							ID3D11ShaderResourceView* zeroSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

							// drawing into importanceMap
							dx11Context->OMSetRenderTargets(1, &m_importanceMap.RTV, nullptr);

							// select 4 deinterleaved AO textures (texture array)
							dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT4, 1, &m_finalResults.SRV);
							FullscreenPassDraw(dx11Context, m_pixelShaderGenerateImportanceMap, m_pBlendStateOpaque);

							// postprocess A
							dx11Context->OMSetRenderTargets(1, &m_importanceMapPong.RTV, nullptr);
							dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT3, 1, &m_importanceMap.SRV);
							FullscreenPassDraw(dx11Context, m_pixelShaderPostprocessImportanceMapA, m_pBlendStateOpaque);
							dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT3, 1, zeroSRVs);

							// postprocess B
							uint32_t fourZeroes[4] = { 0, 0, 0, 0 };
							dx11Context->ClearUnorderedAccessViewUint(m_loadCounterUAV, fourZeroes);
							dx11Context->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_importanceMap.RTV, nullptr, SSAO_LOAD_COUNTER_UAV_SLOT, 1, &m_loadCounterUAV, nullptr);
							// select previous pass input importance map
							dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT3, 1, &m_importanceMapPong.SRV);
							FullscreenPassDraw(dx11Context, m_pixelShaderPostprocessImportanceMapB, m_pBlendStateOpaque);
							dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT3, 1, zeroSRVs);
						}
					}
#endif
					// Generate SSAO
					GenerateSSAO(config, inputs, false);

					if (inputs->OverrideOutputRTV != nullptr)
					{
						// drawing into OverrideOutputRTV
						dx11Context->OMSetRenderTargets(1, &inputs->OverrideOutputRTV, nullptr);
					}
					else
					{
						// restore previous RTs
						d3d11StatesBackup.RestoreRTs();
					}

					// Apply
					{
						DX_PROFILING(Apply);
						// select 4 deinterleaved AO textures (texture array)
						dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT4, 1, &m_finalResults.SRV);

						const CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_size.x), static_cast<float>(m_size.y));
						const CD3D11_RECT rect = CD3D11_RECT(m_fullResOutScissorRect.x, m_fullResOutScissorRect.y, m_fullResOutScissorRect.z, m_fullResOutScissorRect.w);
						dx11Context->RSSetViewports(1, &viewport);
						dx11Context->RSSetScissorRects(1, &rect);

						if (config.QualityLevel < 0)
						{
							FullscreenPassDraw(dx11Context, m_pixelShaderNonSmartHalfApply, m_pBlendStateMultiply);
						}
						else if (config.QualityLevel == 0)
						{
							FullscreenPassDraw(dx11Context, m_pixelShaderNonSmartApply, m_pBlendStateMultiply);
						}
						else
						{
							FullscreenPassDraw(dx11Context, m_pixelShaderApply, m_pBlendStateMultiply);
						}
					}

					// restore previous RTs again (because of the viewport hack)
					d3d11StatesBackup.RestoreRTs();

					// FullscreenPassDraw( dx11Context, m_pixelShaderDebugDraw );
				}
			}

			bool Assao::Impl::InitializeDX()
			{
				m_device = Device::GetInstance()->GetInterface();
				m_device->AddRef();

				HRESULT hr = S_OK;

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\ASSAO\\ASSAO.hlsl");

				// shader load
				ID3DBlob* pShaderBlob = nullptr;
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Model.hlsl");
				}

				// constant buffer
				m_constantsBuffer.Create(m_device, "ASSAOConstants");

				{
					CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
					desc.RenderTarget[0].BlendEnable = false;

					hr = m_device->CreateBlendState(&desc, &m_pBlendStateOpaque);
					if (FAILED(hr))
					{
						CleanupDX();
						throw_line("failed to create blend state");
						return false;
					}

					desc.RenderTarget[0].BlendEnable = true;
					desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
					desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
					desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
					desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;

					hr = m_device->CreateBlendState(&desc, &m_pBlendStateMultiply);
					if (FAILED(hr))
					{
						CleanupDX();
						throw_line("failed to create blend state");
						return false;
					}
				}

				{
					CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
					desc.DepthEnable = false;
					
					hr = m_device->CreateDepthStencilState(&desc, &m_pDepthStencilState);
					if (FAILED(hr))
					{
						CleanupDX();
						throw_line("failed to create depth stencil state");
						return false;
					}
				}

				// samplers
				{
					CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
					desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
					desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					hr = m_device->CreateSamplerState(&desc, &m_pSamplerStatePointClamp);
					if (FAILED(hr))
					{
						CleanupDX();
						throw_line("failed to create sampler state");
						return false;
					}

					desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
					hr = m_device->CreateSamplerState(&desc, &m_pSamplerStatePointMirror);
					if (FAILED(hr))
					{
						CleanupDX();
						throw_line("failed to create sampler state");
						return false;
					}

					desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
					desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					hr = m_device->CreateSamplerState(&desc, &m_pSamplerStateLinearClamp);
					if (FAILED(hr))
					{
						CleanupDX();
						throw_line("failed to create sampler state");
						return false;
					}

					desc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);
					desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
					desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					hr = m_device->CreateSamplerState(&desc, &m_pSamplerStateViewspaceDepthTap);
					if (FAILED(hr))
					{
						CleanupDX();
						throw_line("failed to create sampler state");
						return false;
					}
				}

				// rasterizer state
				{
					CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);
					desc.FillMode = D3D11_FILL_SOLID;
					desc.CullMode = D3D11_CULL_NONE;
					desc.ScissorEnable = true;
					hr = m_device->CreateRasterizerState(&desc, &m_pRasterizerState);
					if (FAILED(hr))
					{
						CleanupDX();
						throw_line("failed to create rasterizer state");
						return false;
					}
				}

				// shaders
				{
					const D3D_SHADER_MACRO shaderMacros[] =
					{
						{ "DX11", "1" },
						{ "SSAO_MAX_TAPS" , SSA_STRINGIZIZER(SSAO_MAX_TAPS) },
						{ "SSAO_ADAPTIVE_TAP_BASE_COUNT" , SSA_STRINGIZIZER(SSAO_ADAPTIVE_TAP_BASE_COUNT) },
						{ "SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT" , SSA_STRINGIZIZER(SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT) },
						{ "SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION", SSA_STRINGIZIZER(SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION) },
						{ nullptr, nullptr }
					};

					// vertex shader
					{
						if (util::CreateVertexShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "VSMain", shader::VS_CompileVersion, &m_pVertexShader, "ASSAO_VSMain") == false)
						{
							CleanupDX();
							throw_line("failed to create vertex shader");
							return false;
						}
					}

					// pixel shaders
					{
						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSPrepareDepths", shader::PS_CompileVersion, &m_pixelShaderPrepareDepths, "PSPrepareDepths") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}
						
						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSPrepareDepthsAndNormals", shader::PS_CompileVersion, &m_pixelShaderPrepareDepthsAndNormals, "PSPrepareDepthsAndNormals") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSPrepareDepthsHalf", shader::PS_CompileVersion, &m_pixelShaderPrepareDepthsHalf, "PSPrepareDepthsHalf") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSPrepareDepthsAndNormalsHalf", shader::PS_CompileVersion, &m_pixelShaderPrepareDepthsAndNormalsHalf, "PSPrepareDepthsAndNormalsHalf") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSPrepareDepthMip1", shader::PS_CompileVersion, &m_pixelShaderPrepareDepthMip[0], "PSPrepareDepthMip1") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSPrepareDepthMip2", shader::PS_CompileVersion, &m_pixelShaderPrepareDepthMip[1], "PSPrepareDepthMip2") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSPrepareDepthMip3", shader::PS_CompileVersion, &m_pixelShaderPrepareDepthMip[2], "PSPrepareDepthMip3") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSGenerateQ0", shader::PS_CompileVersion, &m_pixelShaderGenerate[0], "PSGenerateQ0") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSGenerateQ1", shader::PS_CompileVersion, &m_pixelShaderGenerate[1], "PSGenerateQ1") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSGenerateQ2", shader::PS_CompileVersion, &m_pixelShaderGenerate[2], "PSGenerateQ2") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSGenerateQ3", shader::PS_CompileVersion, &m_pixelShaderGenerate[3], "PSGenerateQ3") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSGenerateQ3Base", shader::PS_CompileVersion, &m_pixelShaderGenerate[4], "PSGenerateQ3Base") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}
#endif

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSSmartBlur", shader::PS_CompileVersion, &m_pixelShaderSmartBlur, "PSSmartBlur") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSSmartBlurWide", shader::PS_CompileVersion, &m_pixelShaderSmartBlurWide, "PSSmartBlurWide") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSNonSmartBlur", shader::PS_CompileVersion, &m_pixelShaderNonSmartBlur, "PSNonSmartBlur") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSApply", shader::PS_CompileVersion, &m_pixelShaderApply, "PSApply") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSNonSmartApply", shader::PS_CompileVersion, &m_pixelShaderNonSmartApply, "PSNonSmartApply") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSNonSmartHalfApply", shader::PS_CompileVersion, &m_pixelShaderNonSmartHalfApply, "PSNonSmartHalfApply") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSGenerateImportanceMap", shader::PS_CompileVersion, &m_pixelShaderGenerateImportanceMap, "PSGenerateImportanceMap") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSPostprocessImportanceMapA", shader::PS_CompileVersion, &m_pixelShaderPostprocessImportanceMapA, "PSPostprocessImportanceMapA") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}

						if (util::CreatePixelShader(m_device, pShaderBlob, shaderMacros, shaderPath.c_str(), "PSPostprocessImportanceMapB", shader::PS_CompileVersion, &m_pixelShaderPostprocessImportanceMapB, "PSPostprocessImportanceMapB") == false)
						{
							CleanupDX();
							throw_line("failed to create pixel shader");
							return false;
						}
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

			void Assao::Impl::CleanupDX()
			{
				m_constantsBuffer.Destroy();

				SafeRelease(m_pRasterizerState);

				SafeRelease(m_pSamplerStatePointClamp);
				SafeRelease(m_pSamplerStateLinearClamp);
				SafeRelease(m_pSamplerStatePointMirror);
				SafeRelease(m_pSamplerStateViewspaceDepthTap);

				SafeRelease(m_pBlendStateMultiply);
				SafeRelease(m_pBlendStateOpaque);

				SafeRelease(m_pDepthStencilState);

				SafeRelease(m_pVertexShader);

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
			static math::uint4 GetTextureDimsFromSRV(ID3D11ShaderResourceView* srv)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC desc;
				srv->GetDesc(&desc);

				assert(desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D); if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return math::uint4::Zero;

				ID3D11Resource* res = nullptr;
				srv->GetResource(&res);

				assert(res != nullptr); if (res == nullptr) return math::uint4::Zero;

				ID3D11Texture2D* tex = QueryResourceInterface<ID3D11Texture2D>(res, c_IID_ID3D11Texture2D);
				SafeRelease(res);
				assert(tex != nullptr); if (tex == nullptr) return math::uint4::Zero;

				D3D11_TEXTURE2D_DESC texDesc;
				tex->GetDesc(&texDesc);
				SafeRelease(tex);

				return math::uint4(texDesc.Width, texDesc.Height, texDesc.MipLevels, texDesc.ArraySize);
			}
#endif

			void Assao::Impl::UpdateTextures(const AssaoInputsDX11* inputs)
			{
				DX_PROFILING(UpdateTextures);

#ifdef _DEBUG
				math::uint4 depthTexDims = GetTextureDimsFromSRV(inputs->DepthSRV);
				assert(depthTexDims.x >= inputs->ViewportWidth);
				assert(depthTexDims.y >= inputs->ViewportHeight);
				assert(depthTexDims.w == 1); // no texture arrays supported

				if (inputs->NormalSRV != nullptr)
				{
					math::uint4 normTexDims = GetTextureDimsFromSRV(inputs->NormalSRV);
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

				const uint32_t width = inputs->ViewportWidth;
				const uint32_t height = inputs->ViewportHeight;

				needsUpdate |= (m_size.x != width) || (m_size.y != height);

				m_size.x = width;
				m_size.y = height;
				m_halfSize.x = (width + 1) / 2;
				m_halfSize.y = (height + 1) / 2;
				m_quarterSize.x = (m_halfSize.x + 1) / 2;
				m_quarterSize.y = (m_halfSize.y + 1) / 2;

				math::uint4 prevScissorRect = m_fullResOutScissorRect;

				if ((inputs->ScissorRight == 0) || (inputs->ScissorBottom == 0))
				{
					m_fullResOutScissorRect = math::uint4(0, 0, width, height);
				}
				else
				{
					m_fullResOutScissorRect = math::uint4(std::max(0u, inputs->ScissorLeft), std::max(0u, inputs->ScissorTop), std::min(width, inputs->ScissorRight), std::min(height, inputs->ScissorBottom));
				}

				needsUpdate |= prevScissorRect != m_fullResOutScissorRect;
				if (needsUpdate == false)
					return;

				m_halfResOutScissorRect = math::uint4(m_fullResOutScissorRect.x / 2, m_fullResOutScissorRect.y / 2, (m_fullResOutScissorRect.z + 1) / 2, (m_fullResOutScissorRect.w + 1) / 2);
				const int blurEnlarge = cMaxBlurPassCount + std::max(0, cMaxBlurPassCount - 2); // +1 for max normal blurs, +2 for wide blurs
				m_halfResOutScissorRect = math::uint4(std::max(0u, m_halfResOutScissorRect.x - blurEnlarge), std::max(0u, m_halfResOutScissorRect.y - blurEnlarge), std::min(m_halfSize.x, m_halfResOutScissorRect.z + blurEnlarge), std::min(m_halfSize.y, m_halfResOutScissorRect.w + blurEnlarge));

				float totalSizeInMB = 0.f;

				for (int i = 0; i < 4; i++)
				{
					if (m_halfDepths[i].ReCreateIfNeeded(m_device, m_halfSize, m_formats.DepthBufferViewspaceLinear, totalSizeInMB, SSAO_DEPTH_MIP_LEVELS, 1, false))
					{
						for (int j = 0; j < SSAO_DEPTH_MIP_LEVELS; j++)
						{
							m_halfDepthsMipViews[i][j].Reset();
						}

						for (int j = 0; j < SSAO_DEPTH_MIP_LEVELS; j++)
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

			void Assao::Impl::UpdateConstants(const Options::AssaoConfig& config, const AssaoInputsDX11* inputs, int pass)
			{
				DX_PROFILING(UpdateConstants);

				ID3D11DeviceContext* dx11Context = inputs->DeviceContext;
				const bool generateNormals = inputs->NormalSRV == nullptr;

				// update constants
				shader::ASSAOConstants& consts = *m_constantsBuffer.Map(dx11Context);
				{
					const math::Matrix& proj = inputs->ProjectionMatrix;

					consts.ViewportPixelSize = math::float2(1.0f / (float)m_size.x, 1.0f / (float)m_size.y);
					consts.HalfViewportPixelSize = math::float2(1.0f / (float)m_halfSize.x, 1.0f / (float)m_halfSize.y);

					consts.Viewport2xPixelSize = math::float2(consts.ViewportPixelSize.x* 2.0f, consts.ViewportPixelSize.y* 2.0f);
					consts.Viewport2xPixelSize_x_025 = math::float2(consts.Viewport2xPixelSize.x* 0.25f, consts.Viewport2xPixelSize.y* 0.25f);

					float depthLinearizeMul = (inputs->MatricesRowMajorOrder) ? (-proj.m[3][2]) : (-proj.m[2][3]); // float depthLinearizeMul = ( clipFar* clipNear ) / ( clipFar - clipNear );
					float depthLinearizeAdd = (inputs->MatricesRowMajorOrder) ? (proj.m[2][2]) : (proj.m[2][2]); // float depthLinearizeAdd = clipFar / ( clipFar - clipNear );
																												 // correct the handedness issue. need to make sure this below is correct, but I think it is.
					if (depthLinearizeMul* depthLinearizeAdd < 0)
						depthLinearizeAdd = -depthLinearizeAdd;
					consts.DepthUnpackConsts = math::float2(depthLinearizeMul, depthLinearizeAdd);

					float tanHalfFOVY = 1.0f / proj.m[1][1]; // = tanf( drawContext.Camera.GetYFOV( )* 0.5f );
					float tanHalfFOVX = 1.0F / proj.m[0][0]; // = tanHalfFOVY* drawContext.Camera.GetAspect( );
					consts.CameraTanHalfFOV = math::float2(tanHalfFOVX, tanHalfFOVY);

					consts.NDCToViewMul = math::float2(consts.CameraTanHalfFOV.x* 2.0f, consts.CameraTanHalfFOV.y* -2.0f);
					consts.NDCToViewAdd = math::float2(consts.CameraTanHalfFOV.x* -1.0f, consts.CameraTanHalfFOV.y* 1.0f);

					consts.EffectRadius = std::clamp(config.Radius, 0.0f, 100000.0f);
					consts.EffectShadowStrength = std::clamp(config.ShadowMultiplier* 4.3f, 0.0f, 10.0f);
					consts.EffectShadowPow = std::clamp(config.ShadowPower, 0.0f, 10.0f);
					consts.EffectShadowClamp = std::clamp(config.ShadowClamp, 0.0f, 1.0f);
					consts.EffectFadeOutMul = -1.0f / (config.FadeOutTo - config.FadeOutFrom);
					consts.EffectFadeOutAdd = config.FadeOutFrom / (config.FadeOutTo - config.FadeOutFrom) + 1.0f;
					consts.EffectHorizonAngleThreshold = std::clamp(config.HorizonAngleThreshold, 0.0f, 1.0f);

					// 1.2 seems to be around the best trade off - 1.0 means on-screen radius will stop/slow growing when the camera is at 1.0 distance, so, depending on FOV, basically filling up most of the screen
					// This setting is viewspace-dependent and not screen size dependent intentionally, so that when you change FOV the effect stays (relatively) similar.
					float effectSamplingRadiusNearLimit = (config.Radius* 1.2f);

					// if the depth precision is switched to 32bit float, this can be set to something closer to 1 (0.9999 is fine)
					consts.DepthPrecisionOffsetMod = 0.9992f;

					// consts.RadiusDistanceScalingFunctionPow = 1.0f - Clamp( config.RadiusDistanceScalingFunction, 0.0f, 1.0f );

					// used to get average load per pixel; 9.0 is there to compensate for only doing every 9th InterlockedAdd in PSPostprocessImportanceMapB for performance reasons
					consts.LoadCounterAvgDiv = 9.0f / (float)(m_quarterSize.x* m_quarterSize.y* 255.0);

					// Special config for lowest quality level - just nerf the effect a tiny bit
					if (config.QualityLevel <= 0)
					{
						//consts.EffectShadowStrength *= 0.9f;
						effectSamplingRadiusNearLimit *= 1.50f;

						if (config.QualityLevel < 0)
						{
							consts.EffectRadius *= 0.8f;
						}
					}
					effectSamplingRadiusNearLimit /= tanHalfFOVY; // to keep the effect same regardless of FOV

					consts.EffectSamplingRadiusNearLimitRec = 1.0f / effectSamplingRadiusNearLimit;

					consts.AdaptiveSampleCountLimit = config.AdaptiveQualityLimit;

					consts.NegRecEffectRadius = -1.0f / consts.EffectRadius;

					consts.PerPassFullResCoordOffset = math::int2(pass % 2, pass / 2);
					consts.PerPassFullResUVOffset = math::float2(((pass % 2) - 0.0f) / m_size.x, ((pass / 2) - 0.0f) / m_size.y);

					consts.InvSharpness = std::clamp(1.0f - config.Sharpness, 0.0f, 1.0f);
					consts.PassIndex = pass;
					consts.QuarterResPixelSize = math::float2(1.0f / (float)m_quarterSize.x, 1.0f / (float)m_quarterSize.y);

					float additionalAngleOffset = config.TemporalSupersamplingAngleOffset; // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
					float additionalRadiusScale = config.TemporalSupersamplingRadiusOffset; // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
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

						consts.PatternRotScaleMatrices[subPass] = math::float4(scale* ca, scale* -sa, -scale * sa, -scale * ca);
					}

					if (generateNormals == false)
					{
						consts.NormalsUnpackMul = inputs->NormalsUnpackMul;
						consts.NormalsUnpackAdd = inputs->NormalsUnpackAdd;
					}
					else
					{
						consts.NormalsUnpackMul = 2.0f;
						consts.NormalsUnpackAdd = -1.0f;
					}
					consts.DetailAOStrength = config.DetailShadowStrength;
					consts.Dummy0 = 0.0f;

#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
					if (generateNormals == false)
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
				}
				m_constantsBuffer.Unmap(dx11Context);
			}

			void Assao::Impl::FullscreenPassDraw(ID3D11DeviceContext* context, ID3D11PixelShader* pixelShader, ID3D11BlendState* blendState, ID3D11DepthStencilState* depthStencilState)
			{
				DX_PROFILING(FullscreenPassDraw);

				if (blendState == nullptr)
				{
					blendState = m_pBlendStateOpaque;
				}

				if (depthStencilState == nullptr)
				{
					depthStencilState = m_pDepthStencilState;
				}

				// Topology
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				// Shaders and input layout
				context->VSSetShader(m_pVertexShader, nullptr, 0);
				context->PSSetShader(pixelShader, nullptr, 0);

				context->OMSetBlendState(blendState, &math::float4::Zero.x, 0xFFFFFFFF);
				context->OMSetDepthStencilState(depthStencilState, 0);
				
				context->RSSetState(m_pRasterizerState);

				context->Draw(4, 0);
			}

			void Assao::Impl::PrepareDepths(const Options::AssaoConfig& config, const AssaoInputsDX11* inputs)
			{
				DX_PROFILING(PrepareDepths);

				const bool generateNormals = inputs->NormalSRV == nullptr;

				ID3D11DeviceContext* dx11Context = inputs->DeviceContext;

				dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 1, &inputs->DepthSRV);

				{
					const CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)m_halfSize.x, (float)m_halfSize.y);
					const CD3D11_RECT rect = CD3D11_RECT(0, 0, m_halfSize.x, m_halfSize.y);
					dx11Context->RSSetViewports(1, &viewport);
					dx11Context->RSSetScissorRects(1, &rect); // no scissor for this
				}

				ID3D11RenderTargetView* fourDepths[] = { m_halfDepths[0].RTV, m_halfDepths[1].RTV, m_halfDepths[2].RTV, m_halfDepths[3].RTV };
				ID3D11RenderTargetView* twoDepths[] = { m_halfDepths[0].RTV, m_halfDepths[3].RTV };
				if (generateNormals == false)
				{
					if (config.QualityLevel < 0)
					{
						dx11Context->OMSetRenderTargets(_countof(twoDepths), twoDepths, nullptr);
						FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepthsHalf);
					}
					else
					{
						dx11Context->OMSetRenderTargets(_countof(fourDepths), fourDepths, nullptr);
						FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepths);
					}
				}
				else
				{
					ID3D11UnorderedAccessView* UAVs[] = { m_normals.UAV };
					if (config.QualityLevel < 0)
					{
						dx11Context->OMSetRenderTargetsAndUnorderedAccessViews(_countof(twoDepths), twoDepths, nullptr, SSAO_NORMALMAP_OUT_UAV_SLOT, 1, UAVs, nullptr);
						FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepthsAndNormalsHalf);
					}
					else
					{
						dx11Context->OMSetRenderTargetsAndUnorderedAccessViews(_countof(fourDepths), fourDepths, nullptr, SSAO_NORMALMAP_OUT_UAV_SLOT, 1, UAVs, nullptr);
						FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepthsAndNormals);
					}
				}

				// only do mipmaps for higher quality levels (not beneficial on quality level 1, and detrimental on quality level 0)
				if (config.QualityLevel > 1)
				{
					for (int i = 1; i < SSAO_DEPTH_MIP_LEVELS; i++)
					{
						ID3D11RenderTargetView* fourDepthMips[] = { m_halfDepthsMipViews[0][i].RTV, m_halfDepthsMipViews[1][i].RTV, m_halfDepthsMipViews[2][i].RTV, m_halfDepthsMipViews[3][i].RTV };

						CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)m_halfDepthsMipViews[0][i].Size.x, (float)m_halfDepthsMipViews[0][i].Size.y);
						dx11Context->RSSetViewports(1, &viewport);

						ID3D11ShaderResourceView* fourSRVs[] = { m_halfDepthsMipViews[0][i - 1].SRV, m_halfDepthsMipViews[1][i - 1].SRV, m_halfDepthsMipViews[2][i - 1].SRV, m_halfDepthsMipViews[3][i - 1].SRV };

						dx11Context->OMSetRenderTargets(4, fourDepthMips, nullptr);
						dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 4, fourSRVs);
						FullscreenPassDraw(dx11Context, m_pixelShaderPrepareDepthMip[i - 1]);
					}
				}
			}

			void Assao::Impl::GenerateSSAO(const Options::AssaoConfig& config, const AssaoInputsDX11* inputs, bool adaptiveBasePass)
			{
				DX_PROFILING(GenerateSSAO);

				ID3D11ShaderResourceView* normalmapSRV = (inputs->NormalSRV == nullptr) ? (m_normals.SRV) : (inputs->NormalSRV);

				ID3D11DeviceContext* dx11Context = inputs->DeviceContext;

				{
					const CD3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)m_halfSize.x, (float)m_halfSize.y);
					const CD3D11_RECT rect = CD3D11_RECT(m_halfResOutScissorRect.x, m_halfResOutScissorRect.y, m_halfResOutScissorRect.z, m_halfResOutScissorRect.w);
					dx11Context->RSSetViewports(1, &viewport);
					dx11Context->RSSetScissorRects(1, &rect);
				}

				if (adaptiveBasePass)
				{
					assert(config.QualityLevel == 3);
				}

				const int passCount = 4;

				ID3D11ShaderResourceView* zeroSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

				for (int pass = 0; pass < passCount; pass++)
				{
					if ((config.QualityLevel < 0) && ((pass == 1) || (pass == 2)))
						continue;

					int blurPasses = config.BlurPassCount;
					blurPasses = std::min(blurPasses, cMaxBlurPassCount);

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					if (config.QualityLevel == 3)
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
						if (config.QualityLevel <= 0)
						{
							// just one blur pass allowed for minimum quality 
							blurPasses = std::min(1, config.BlurPassCount);
						}
					}

					UpdateConstants(config, inputs, pass);

					D3D11Texture2D* pPingRT = &m_pingPongHalfResultA;
					D3D11Texture2D* pPongRT = &m_pingPongHalfResultB;

					// Generate
					{
						// remove textures from slots 0, 1, 2, 3 to avoid API complaints
						dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 5, zeroSRVs);

						ID3D11RenderTargetView* rts[] = { pPingRT->RTV };

						// no blur?
						if (blurPasses == 0)
						{
							rts[0] = m_finalResultsArrayViews[pass].RTV;
						}

						dx11Context->OMSetRenderTargets(_countof(rts), rts, nullptr);

						ID3D11ShaderResourceView* SRVs[] = { m_halfDepths[pass].SRV, normalmapSRV, nullptr, nullptr, nullptr }; // m_loadCounterSRV used only for quality level 3
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
						if (!adaptiveBasePass && (config.QualityLevel == 3))
						{
							SRVs[2] = m_loadCounterSRV;
							SRVs[3] = m_importanceMap.SRV;
							SRVs[4] = m_finalResults.SRV;
						}
#endif
						dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 5, SRVs);

						int shaderIndex = std::max(0, (!adaptiveBasePass) ? (config.QualityLevel) : (4));
						FullscreenPassDraw(dx11Context, m_pixelShaderGenerate[shaderIndex]);

						// remove textures from slots 0, 1, 2, 3 to avoid API complaints
						dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT0, 5, zeroSRVs);
					}

					// Blur
					if (blurPasses > 0)
					{
						int wideBlursRemaining = std::max(0, blurPasses - 2);

						for (int i = 0; i < blurPasses; i++)
						{
							// remove textures to avoid API complaints
							dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT0, _countof(zeroSRVs), zeroSRVs);

							ID3D11RenderTargetView* rts[] = { pPongRT->RTV };

							// last pass?
							if (i == (blurPasses - 1))
							{
								rts[0] = m_finalResultsArrayViews[pass].RTV;
							}

							dx11Context->OMSetRenderTargets(_countof(rts), rts, nullptr);

							ID3D11ShaderResourceView* SRVs[] = { pPingRT->SRV };
							dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT2, _countof(SRVs), SRVs);

							if (config.QualityLevel > 0)
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
					dx11Context->PSSetShaderResources(SSAO_TEXTURE_SLOT0, _countof(zeroSRVs), zeroSRVs);
				}
			}

			Assao::Assao()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			Assao::~Assao()
			{
			}

			void Assao::Apply(Camera* pCamera, const RenderTarget* pNormalMap, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				TRACER_EVENT(L"ASSAO::Apply");
				DX_PROFILING(ASSAO);

				ID3D11DeviceContext* pDeviceContext = Device::GetInstance()->GetImmediateContext();
				pDeviceContext->ClearState();

				const math::Viewport& viewport = Device::GetInstance()->GetViewport();

				AssaoInputsDX11 inputs;
				inputs.ScissorLeft = 0;
				inputs.ScissorTop = 0;
				inputs.ScissorRight = static_cast<int>(viewport.width);
				inputs.ScissorBottom = static_cast<int>(viewport.height);
				inputs.ProjectionMatrix = pCamera->GetProjectionMatrix();
				inputs.ViewportWidth = static_cast<int>(viewport.width);
				inputs.ViewportHeight = static_cast<int>(viewport.height);
				inputs.MatricesRowMajorOrder = true;
#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
				inputs.NormalsWorldToViewspaceMatrix = pCamera->GetViewMatrix().Transpose();
#endif
				inputs.DeviceContext = pDeviceContext;
				inputs.DepthSRV = pDepth->GetShaderResourceView();
				inputs.NormalSRV = pNormalMap->GetShaderResourceView();
				inputs.OverrideOutputRTV = pResult->GetRenderTargetView();

				const Options::AssaoConfig& config = GetOptions().assaoConfig;
				m_pImpl->Draw(config, &inputs);
			}
		}
	}
}