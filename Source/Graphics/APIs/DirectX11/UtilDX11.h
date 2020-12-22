#pragma once

#include "CommonLib/PhantomType.h"

#include "Graphics/Interface/GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		class Camera;
		class IMaterial;

		namespace dx11
		{
			namespace util
			{
				class DXProfiler
				{
				public:
					DXProfiler(const wchar_t* str);
					~DXProfiler();
				};
#define DX_PROFILING(name)	est::graphics::dx11::util::DXProfiler profiler_##name(L#name)

				void GetInputElementDesc(EmVertexFormat::Type emType, const D3D11_INPUT_ELEMENT_DESC** ppInputElements_out, size_t* pElementCount_out);

				bool CompileShader(ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pDefines, const wchar_t* sourceName, const char* functionName, const char* profile, ID3DBlob** ppCompliedShaderBlob_out);
				bool CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11VertexShader** ppVertexShader, const char* debugName);
				bool CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11VertexShader** ppVertexShader, const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, size_t NumElements, ID3D11InputLayout** ppInputLayout, const char* debugName);
				bool CreatePixelShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11PixelShader** ppPixelShader, const char* debugName);
				bool CreateHullShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11HullShader** ppHullShader, const char* debugName);
				bool CreateDomainShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11DomainShader** ppDomainShader, const char* debugName);
				bool CreateGeometryShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11GeometryShader** ppGeometryShader, const char* debugName);

				bool CreateConstantBuffer(ID3D11Device* pDevice, size_t nSize, ID3D11Buffer** pBuffer_out, const char* debugName);

				const D3D11_VIEWPORT* Convert(const math::Viewport& viewport);
				int GetPixelSizeInBytes(DXGI_FORMAT val);

				DXGI_FORMAT GetCascadedShadowFormat();

				// debug
				inline void SetDebugName(_In_  ID3D11DeviceChild* resource, _In_z_ const std::string& debugName)
				{
#if defined(_DEBUG) || defined(PROFILE)
					HRESULT nameSet = resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(debugName.size()), debugName.c_str());
					if (FAILED(nameSet))
					{
						throw_line("failed to set debug name");
					}
#endif
				}

				inline void SetDebugName(_In_  IDXGIObject* resource, _In_z_ const std::string& debugName)
				{
#if defined(_DEBUG) || defined(PROFILE)
					HRESULT nameSet = resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(debugName.size()), debugName.c_str());
					if (FAILED(nameSet))
					{
						throw_line("failed to set debug name");
					}
#endif
				}

				inline void SetDebugName(_In_  ID3D11Device* resource, _In_z_ const std::string& debugName)
				{
#if defined(_DEBUG) || defined(PROFILE)
					HRESULT nameSet = resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(debugName.size()), debugName.c_str());
					if (FAILED(nameSet))
					{
						throw_line("failed to set debug name");
					}
#endif
				}

				void ReportLiveObjects(ID3D11Device* pDevice);
			}

			class RenderTarget;
			class DepthStencil;

			struct RenderElement
			{
				ID3D11Device* pDevice{ nullptr };
				ID3D11DeviceContext* pDeviceContext{ nullptr };
				Camera* pCamera{ nullptr };
				ID3D11RenderTargetView* pRTVs[GBufferTypeCount]{ nullptr };
				uint32_t rtvCount{ 1 };
				ID3D11DepthStencilView* pDSV{ nullptr };
			};

			template <typename T>
			struct ConstantBuffer
			{
				ID3D11Buffer* pBuffer{ nullptr };

				~ConstantBuffer()
				{
					Destroy();
				}

				void Create(ID3D11Device* pDevice, const char* debugName)
				{
					if (util::CreateConstantBuffer(pDevice, Size(), &pBuffer, debugName) == false)
					{
						throw_line("failed to create constant buffer");
					}
				}

				void Destroy()
				{
					SafeRelease(pBuffer);
				}

				T* Map(ID3D11DeviceContext* pDeviceContext)
				{
					D3D11_MAPPED_SUBRESOURCE mapped{};
					HRESULT hr = pDeviceContext->Map(pBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped);
					if (FAILED(hr))
					{
						throw_line("failed to map constant buffer");
					}

					return reinterpret_cast<T*>(mapped.pData);
				}

				void Unmap(ID3D11DeviceContext* pDeviceContext)
				{
					pDeviceContext->Unmap(pBuffer, 0);
				}

				static constexpr size_t Size() noexcept { return sizeof(T); }
			};

			namespace postprocess
			{
				namespace gaussianblur
				{
					void Apply(const RenderTarget* pSource, RenderTarget* pResult, float sigma);
					void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float sigma);
				}
			}

			namespace shader
			{
				const char VS_CompileVersion[]{ "vs_5_0" };
				const char PS_CompileVersion[]{ "ps_5_0" };
				const char GS_CompileVersion[]{ "gs_5_0" };
				const char HS_CompileVersion[]{ "hs_5_0" };
				const char DS_CompileVersion[]{ "ds_5_0" };

				struct tMaskKey { static constexpr uint32_t DefaultValue() { return 0; } };
				using MaskKey = PhantomType<tMaskKey, uint32_t>;

				enum MaterialMask : uint32_t
				{
					eUseTexAlbedo = 1 << 0,
					eUseTexMask = 1 << 1,
					eUseTexNormal = 1 << 2,
					eUseTexRoughness = 1 << 3,
					eUseTexMetallic = 1 << 4,
					eUseTexEmissive = 1 << 5,
					eUseTexEmissiveColor = 1 << 6,
					eUseTexSubsurface = 1 << 7,
					eUseTexSpecular = 1 << 8,
					eUseTexSpecularTint = 1 << 9,
					eUseTexAnisotropic = 1 << 10,
					eUseTexSheen = 1 << 11,
					eUseTexSheenTint = 1 << 12,
					eUseTexClearcoat = 1 << 13,
					eUseTexClearcoatGloss = 1 << 14,

					MaterialMaskCount = 15,
				};

				bool IsValidTexture(const IMaterial* pMaterial, IMaterial::Type emType);
				uint32_t GetMaterialMask(const IMaterial* pMaterial);
				void SetPSSRV(ID3D11DeviceContext* pDeviceContext, const IMaterial* pMaterial, IMaterial::Type emType, uint32_t slot);
				void SetMaterial(ID3D11DeviceContext* pDeviceContext, const IMaterial* pMaterial, uint32_t materialSamplerSlot, bool isAlphaPrePass);
			}
		}
	}
}

namespace std
{
	template <>
	struct hash<est::graphics::dx11::shader::MaskKey>
	{
		uint32_t operator()(const est::graphics::dx11::shader::MaskKey& key) const
		{
			return key;
		}
	};
}