#pragma once

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
				void GetInputElementDesc(EmVertexFormat::Type emType, const D3D11_INPUT_ELEMENT_DESC** ppInputElements_out, size_t& nElementCount_out);

				bool CompileShader(ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pDefines, const char* strSourceName, const char* strFunctionName, const char* strProfile, ID3DBlob** ppCompliedShaderBlob_out);
				bool CreateConstantBuffer(ID3D11Device* pDevice, size_t nSize, ID3D11Buffer** pBuffer_out);

				// debug
				template<UINT TNameLength>
				inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_z_ const char(&name)[TNameLength])
				{
#if defined(_DEBUG) || defined(PROFILE)
					HRESULT nameSet = resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
					ErrorIf(FAILED(nameSet), "Failed to set debug name");
#endif
				}

				template<UINT TNameLength>
				inline void SetDebugObjectName(_In_ IDXGIObject* resource, _In_z_ const char(&name)[TNameLength])
				{
#if defined(_DEBUG) || defined(PROFILE)
					HRESULT nameSet = resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
					ErrorIf(FAILED(nameSet), "Failed to set debug name");
#endif
				}

				inline void SetDebugName(_In_  ID3D11DeviceChild* resource, _In_z_ std::string debugName)
				{
#if defined(_DEBUG) || defined(PROFILE)
					HRESULT nameSet = resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(debugName.size()), debugName.c_str());
					if (FAILED(nameSet))
					{
						throw_line("Failed to set debug name");
					}
#endif
				}

				inline void SetDebugName(_In_  IDXGIObject* resource, _In_z_ std::string debugName)
				{
#if defined(_DEBUG) || defined(PROFILE)
					HRESULT nameSet = resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(debugName.size()), debugName.c_str());
					if (FAILED(nameSet))
					{
						throw_line("Failed to set debug name");
					}
#endif
				}

				inline void SetDebugName(_In_  ID3D11Device* resource, _In_z_ std::string debugName)
				{
#if defined(_DEBUG) || defined(PROFILE)
					HRESULT nameSet = resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(debugName.size()), debugName.c_str());
					if (FAILED(nameSet))
					{
						throw_line("Failed to set debug name");
					}
#endif
				}

				inline void ReportLiveObjects(ID3D11Device* device)
				{
					if (device)
					{
						ID3D11Debug* pDebug = nullptr;
						device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pDebug));
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