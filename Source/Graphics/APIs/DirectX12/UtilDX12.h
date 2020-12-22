#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

#include "DefineDX12.h"

namespace est
{
	namespace graphics
	{
		class Camera;
		class IMaterial;

		namespace dx12
		{
			class RenderTarget;
			class DepthStencil;

			struct RenderElement
			{
				Camera* pCamera{ nullptr };
				std::array<RenderTarget*, GBufferTypeCount> pRTVs{ nullptr };
				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[GBufferTypeCount]{ 0 };
				uint32_t rtvCount{ 1 };
				DepthStencil* pDepthStencil{ nullptr };
				std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> dsvHandle;

				void SetDSVHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) { dsvHandle.emplace(handle); }
				void ResetDSVHandle() { dsvHandle.reset(); }
				const D3D12_CPU_DESCRIPTOR_HANDLE* GetDSVHandle() const { return dsvHandle.has_value() ? &dsvHandle.value() : nullptr; };
			};

			namespace shader
			{
				const char VS_CompileVersion[]{ "vs_5_1" };
				const char PS_CompileVersion[]{ "ps_5_1" };
				const char GS_CompileVersion[]{ "gs_5_1" };
				const char HS_CompileVersion[]{ "hs_5_1" };
				const char DS_CompileVersion[]{ "ds_5_1" };
				const char CS_CompileVersion[]{ "cs_5_1" };

				enum MaterialMask : uint64_t
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

				struct MaterialSRVIndexConstants
				{
					uint32_t nTexAlbedoIndex{ 0 };
					uint32_t nTexMaskIndex{ 0 };
					uint32_t nTexNormalMapIndex{ 0 };
					uint32_t nTexRoughnessIndex{ 0 };
					uint32_t nTexMetallicIndex{ 0 };
					uint32_t nTexEmissiveIndex{ 0 };
					uint32_t nTexEmissiveColorIndex{ 0 };
					uint32_t nTexSubsurfaceIndex{ 0 };
					uint32_t nTexSpecularIndex{ 0 };
					uint32_t nTexSpecularTintIndex{ 0 };
					uint32_t nTexAnisotropicIndex{ 0 };
					uint32_t nTexSheenIndex{ 0 };
					uint32_t nTexSheenTintIndex{ 0 };
					uint32_t nTexClearcoatIndex{ 0 };
					uint32_t nTexClearcoatGlossIndex{ 0 };
					uint32_t nSamplerStateIndex{ 0 };
				};

				bool IsValidTexture(const IMaterial* pMaterial, IMaterial::Type emType);
				uint32_t GetMaterialMask(const IMaterial* pMaterial);
				void SetSRVIndex(const IMaterial* pMaterial, IMaterial::Type emType, uint32_t* pSRVIndex_out);
				void SetMaterial(MaterialSRVIndexConstants* pSRVIndexContantBuffer, const IMaterial* pMaterial);
			}

			namespace util
			{
				inline uint64_t Align(uint64_t nNum, uint64_t nAlignment) { return ((nNum + nAlignment - 1) / nAlignment) * nAlignment; }

				void ReleaseResource(ID3D12DeviceChild* pResource);
				void ReleaseResourceRTV(uint32_t& descriptorIndex);
				void ReleaseResourceSRV(uint32_t& descriptorIndex);
				void ReleaseResourceDSV(uint32_t& descriptorIndex);
				void ReleaseResourceUAV(uint32_t& descriptorIndex);

				void WaitForFence(ID3D12Fence* pFence, uint64_t nCompletionValue, HANDLE hWaitEvent);

				void GetInputElementDesc(EmVertexFormat::Type emType, const D3D12_INPUT_ELEMENT_DESC** ppInputElements_out, size_t* pElementCount_out);

				bool CompileShader(ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pDefines, const wchar_t* sourceName, const char* strFunctionName, const char* strProfile, ID3DBlob** ppCompliedShaderBlob_out);
				bool CreateConstantBuffer(ID3D12Device* pDevice, size_t nSize, ID3D12Resource** ppBuffer_out, const wchar_t* wdebugName = nullptr);

				const D3D12_VIEWPORT* Convert(const math::Viewport& viewport);
				int GetPixelSizeInBytes(DXGI_FORMAT val);

				D3D12_STATIC_SAMPLER_DESC GetStaticSamplerDesc(SamplerState::Type samplerState, uint32_t nShaderRegister, uint32_t nRegisterSpace, D3D12_SHADER_VISIBILITY shaderVisibility);
				D3D12_SAMPLER_DESC GetSamplerDesc(SamplerState::Type samplerState);
				D3D12_RASTERIZER_DESC GetRasterizerDesc(RasterizerState::Type rasterizerState);
				D3D12_BLEND_DESC GetBlendDesc(BlendState::Type blendState);
				D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(DepthStencilState::Type depthStencilState);

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice, uint32_t numParameters,
					_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
					uint32_t numStaticSamplers = 0,
					_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
					D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

				template <typename T>
				void ChangeResourceState(ID3D12GraphicsCommandList2* pCommandList, T* pResource, D3D12_RESOURCE_STATES state)
				{
					if (pResource->GetResourceState() != state)
					{
						const D3D12_RESOURCE_BARRIER transition[] =
						{
							pResource->Transition(state),
						};
						pCommandList->ResourceBarrier(_countof(transition), transition);
					}
				}

				void ReportLiveObjects();
			}

			struct PSOKey
			{
				union
				{
					struct
					{
						uint32_t mask;
						RasterizerState::Type rasterizerState;
						BlendState::Type blendState;
						DepthStencilState::Type depthStencilState;
					};
					uint64_t key;
				};

				PSOKey(uint32_t mask, RasterizerState::Type rasterizerState, BlendState::Type blendState, DepthStencilState::Type depthStencilState);

				bool operator == (const PSOKey& other) const { return key == other.key; }
				bool operator != (const PSOKey& other) const { return key != other.key; }
			};

			struct PSOCache
			{
				ID3DBlob* pVSBlob{ nullptr };
				ID3DBlob* pPSBlob{ nullptr };
				ID3DBlob* pGSBlob{ nullptr };
				ID3DBlob* pHSBlob{ nullptr };
				ID3DBlob* pDSBlob{ nullptr };
				ID3DBlob* pCSBlob{ nullptr };

				ID3D12PipelineState* pPipelineState{ nullptr };
				ID3D12RootSignature* pRootSignature{ nullptr };

				~PSOCache();

				void Destroy();
			};

			template <typename T>
			struct ConstantBuffer
			{
				std::array<ID3D12Resource*, eFrameBufferCount> pUploadHeaps{ nullptr };
				std::array<uint8_t*, eFrameBufferCount> pViewGPUAddress{ nullptr };
				std::array<D3D12_GPU_VIRTUAL_ADDRESS, eFrameBufferCount> gpuAddress{ 0 };

				static constexpr size_t Size() noexcept { return sizeof(T); }
				static constexpr size_t AlignedSize() noexcept { return util::Align(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT); }

				T* Cast(uint32_t frameIndex) { return reinterpret_cast<T*>(pViewGPUAddress[frameIndex]); }
				T* Cast(uint32_t frameIndex, size_t index) { return reinterpret_cast<T*>(pViewGPUAddress[frameIndex] + (index * AlignedSize())); }
				D3D12_GPU_VIRTUAL_ADDRESS GPUAddress(uint32_t frameIndex) const { return gpuAddress[frameIndex]; }
				D3D12_GPU_VIRTUAL_ADDRESS GPUAddress(uint32_t frameIndex, size_t index) const { return gpuAddress[frameIndex] + (index * AlignedSize()); }

				~ConstantBuffer()
				{
					Destroy();
				}

				void Create(ID3D12Device* pDevice, size_t nHeapCount, const char* debugName)
				{
					if (nHeapCount == 0)
					{
						throw_line("heap count is zero");
					}

					const std::wstring wdebugName = string::MultiToWide(debugName);

					const CD3DX12_RANGE readRange{ 0, 0 };
					for (size_t i = 0; i < eFrameBufferCount; ++i)
					{
						assert(pUploadHeaps[i] == nullptr);

						if (util::CreateConstantBuffer(pDevice, AlignedSize() * nHeapCount, &pUploadHeaps[i], wdebugName.c_str()) == false)
						{
							throw_line("failed to create constant buffer");
						}

						HRESULT hr = pUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&pViewGPUAddress[i]));
						if (FAILED(hr))
						{
							throw_line("failed to map, constant buffer upload heap");
						}

						memory::Clear(pViewGPUAddress[i], AlignedSize() * nHeapCount);

						gpuAddress[i] = pUploadHeaps[i]->GetGPUVirtualAddress();
					}
				}

				void Destroy()
				{
					for (size_t i = 0; i < eFrameBufferCount; ++i)
					{
						util::ReleaseResource(pUploadHeaps[i]);
						pUploadHeaps[i] = nullptr;
					}
				}
			};
		}
	}
}

namespace std
{
	template <>
	struct hash<est::graphics::dx12::PSOKey>
	{
		std::uint64_t operator()(const est::graphics::dx12::PSOKey& key) const
		{
			return key.key;
		}
	};
}