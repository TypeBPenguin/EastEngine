#pragma once

#include "GraphicsInterface/Define.h"
#include "GraphicsInterface/Vertex.h"

#include "DefineDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				struct PSOKey
				{
					union
					{
						struct
						{
							uint32_t nMask;
							EmRasterizerState::Type emRasterizerState;
							EmBlendState::Type emBlendState;
							EmDepthStencilState::Type emDepthStencilState;
						};
						uint64_t nKey;
					};

					PSOKey(uint32_t nMask, EmRasterizerState::Type emRasterizerState, EmBlendState::Type emBlendState, EmDepthStencilState::Type emDepthStencilState)
						: nMask(nMask)
						, emRasterizerState(emRasterizerState)
						, emBlendState(emBlendState)
						, emDepthStencilState(emDepthStencilState)
					{
					}

					bool operator == (const PSOKey& other) const { return nKey == other.nKey; }
					bool operator != (const PSOKey& other) const { return nKey != other.nKey; }
				};
			}

			template <typename T>
			struct ConstantBuffer
			{
				std::array<ID3D12Resource*, eFrameBufferCount> pUploadHeaps{ nullptr };
				std::array<uint8_t*, eFrameBufferCount> pViewGPUAddress{ nullptr };
				std::array<D3D12_GPU_VIRTUAL_ADDRESS, eFrameBufferCount> gpuAddress{ 0 };

				static constexpr size_t Size() noexcept { return sizeof(T); }
				static constexpr size_t AlignedSize() noexcept { return util::Align(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT); }

				T* Cast(int nFrameIndex) { return reinterpret_cast<T*>(pViewGPUAddress[nFrameIndex]); }
				T* Cast(int nFrameIndex, size_t nIndex) { return reinterpret_cast<T*>(pViewGPUAddress[nFrameIndex] + (nIndex * AlignedSize())); }
				D3D12_GPU_VIRTUAL_ADDRESS GPUAddress(int nFrameIndex) const { return gpuAddress[nFrameIndex]; }
				D3D12_GPU_VIRTUAL_ADDRESS GPUAddress(int nFrameIndex, size_t nIndex) const { return gpuAddress[nFrameIndex] + (nIndex * AlignedSize()); }

				~ConstantBuffer()
				{
					Destroy();
				}

				void Create(ID3D12Device* pDevice, size_t nHeapCount, const char* strDebugName)
				{
					if (nHeapCount == 0)
					{
						throw_line("heap count is zero");
					}

					const std::wstring wstrDebugName = String::MultiToWide(strDebugName);

					const CD3DX12_RANGE readRange{ 0, 0 };
					for (size_t i = 0; i < eFrameBufferCount; ++i)
					{
						assert(pUploadHeaps[i] == nullptr);

						if (util::CreateConstantBuffer(pDevice, AlignedSize() * nHeapCount, &pUploadHeaps[i], wstrDebugName.c_str()) == false)
						{
							throw_line("failed to create constant buffer");
						}

						HRESULT hr = pUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&pViewGPUAddress[i]));
						if (FAILED(hr))
						{
							throw_line("failed to map, constant buffer upload heap");
						}

						Memory::Clear(pViewGPUAddress[i], AlignedSize() * nHeapCount);

						gpuAddress[i] = pUploadHeaps[i]->GetGPUVirtualAddress();
					}
				}

				void Destroy()
				{
					for (size_t i = 0; i < eFrameBufferCount; ++i)
					{
						SafeRelease(pUploadHeaps[i]);
					}
				}
			};

			namespace util
			{
				inline uint64_t Align(uint64_t nNum, uint64_t nAlignment) { return ((nNum + nAlignment - 1) / nAlignment) * nAlignment; }

				void WaitForFence(ID3D12Fence* pFence, uint64_t nCompletionValue, HANDLE hWaitEvent);

				void GetInputElementDesc(EmVertexFormat::Type emType, const D3D12_INPUT_ELEMENT_DESC** ppInputElements_out, size_t* pElementCount_out);

				bool CompileShader(ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pDefines, const char* strSourceName, const char* strFunctionName, const char* strProfile, ID3DBlob** ppCompliedShaderBlob_out);
				bool CreateConstantBuffer(ID3D12Device* pDevice, size_t nSize, ID3D12Resource** ppBuffer_out, const wchar_t* wstrDebugName = nullptr);

				int GetPixelSizeInBytes(DXGI_FORMAT val);

				D3D12_STATIC_SAMPLER_DESC GetStaticSamplerDesc(EmSamplerState::Type emSamplerState, uint32_t nShaderRegister, uint32_t nRegisterSpace, D3D12_SHADER_VISIBILITY shaderVisibility);
				D3D12_SAMPLER_DESC GetSamplerDesc(EmSamplerState::Type emSamplerState);
				D3D12_RASTERIZER_DESC GetRasterizerDesc(EmRasterizerState::Type emRasterizerState);
				D3D12_BLEND_DESC GetBlendDesc(EmBlendState::Type emBlendState);
				D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(EmDepthStencilState::Type emDepthStencilState);

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice, uint32_t numParameters,
					_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
					uint32_t numStaticSamplers = 0,
					_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
					D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

				void ReportLiveObjects(ID3D12Device* pDevice);
			}
		}
	}
}

namespace std
{
	template <>
	struct hash<eastengine::graphics::dx12::shader::PSOKey>
	{
		std::uint64_t operator()(const eastengine::graphics::dx12::shader::PSOKey& key) const
		{
			return key.nKey;
		}
	};
}