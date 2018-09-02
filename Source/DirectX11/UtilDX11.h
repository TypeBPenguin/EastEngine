#pragma once

#include "CommonLib/PhantomType.h"

#include "GraphicsInterface/Vertex.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			template <typename T>
			struct ConstantBuffer
			{
				ID3D11Buffer* pBuffer{ nullptr };

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

			namespace shader
			{
				struct tMaskKey {};
				using MaskKey = PhantomType<tMaskKey, const uint32_t>;
			}

			namespace util
			{
				class DXProfiler
				{
				public:
					DXProfiler(const wchar_t* str);
					~DXProfiler();
				};
#define DX_PROFILING(name)	eastengine::graphics::dx11::util::DXProfiler profiler_##name(L#name)

				void GetInputElementDesc(EmVertexFormat::Type emType, const D3D11_INPUT_ELEMENT_DESC** ppInputElements_out, size_t* pElementCount_out);

				bool CompileShader(ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pDefines, const char* strSourceName, const char* strFunctionName, const char* strProfile, ID3DBlob** ppCompliedShaderBlob_out);
				bool CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const char* strShaderPath, const char* strFunctionName, const char* strProfile, ID3D11VertexShader** ppVertexShader, const char* strDebugName);
				bool CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const char* strShaderPath, const char* strFunctionName, const char* strProfile, ID3D11VertexShader** ppVertexShader, const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, size_t NumElements, ID3D11InputLayout** ppInputLayout, const char* strDebugName);
				bool CreatePixelShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const char* strShaderPath, const char* strFunctionName, const char* strProfile, ID3D11PixelShader** ppPixelShader, const char* strDebugName);

				bool CreateConstantBuffer(ID3D11Device* pDevice, size_t nSize, ID3D11Buffer** pBuffer_out, const char* strDebugName);

				int GetPixelSizeInBytes(DXGI_FORMAT val);

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

				inline void ReportLiveObjects(ID3D11Device* pDevice)
				{
					if (pDevice)
					{
						ID3D11Debug* pDebug = nullptr;
						pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pDebug));
						if (pDebug != nullptr)
						{
							pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
							SafeRelease(pDebug);
						}
					}
				}
			}
		}
	}
}

namespace std
{
	template <>
	struct hash<eastengine::graphics::dx11::shader::MaskKey>
	{
		size_t operator()(const eastengine::graphics::dx11::shader::MaskKey& key) const
		{
			return key.value;
		}
	};
}