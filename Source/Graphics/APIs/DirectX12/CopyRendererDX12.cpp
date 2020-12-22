#include "stdafx.h"
#include "CopyRendererDX12.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"

#include "DescriptorHeapDX12.h"
#include "RenderTargetDX12.h"
#include "DepthStencilDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				struct CopyContents
				{
					uint32_t texIndex{ 0 };
					math::float3 padding;
				};

				enum CBSlot
				{
					eCB_CopyContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_Point = 0,
				};

				enum SRVSlot
				{
					eSRV_Color = 0,
				};

				enum PSType
				{
					ePS_RGBA = 0,
					ePS_RGB,

					ePS_Count,
				};

				const char* GetCopyPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case ePS_RGBA:
						return "PS_RGBA";
					case ePS_RGB:
						return "PS_RGB";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetCopyContents(CopyContents* pCopyContents, uint32_t texIndex)
				{
					pCopyContents->texIndex = texIndex;
				}
			}

			class CopyRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Copy_RGBA(RenderTarget* pSource, RenderTarget* pResult);
				void Copy_RGB(RenderTarget* pSource, RenderTarget* pResult);

			private:
				void CommonReady(ID3D12GraphicsCommandList2* pCommandList, uint32_t frameIndex, RenderTarget* pSource, RenderTarget* pResult);
				void CommonDraw(ID3D12GraphicsCommandList2* pCommandList, DescriptorHeap* pSRVDescriptorHeap, uint32_t frameIndex, shader::PSType emPSType, RenderTarget* pSource, RenderTarget* pResult);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_CopyContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice, shader::PSType emPSType);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType);
				void CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType);

			private:
				struct RenderPipeline
				{
					PSOCache psoCache;

					std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> pBundles{ nullptr };
				};
				std::array<RenderPipeline, shader::ePS_Count> m_pipelineStates;

				enum
				{
					eCopyContentsCapacity = 32,
				};
				uint32_t m_index{ 0 };
				ConstantBuffer<shader::CopyContents> m_copyContents;
			};

			CopyRenderer::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\Copy.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : CopyRenderer.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str(), emPSType);
				}

				SafeRelease(pShaderBlob);

				m_copyContents.Create(pDevice, eCopyContentsCapacity, "CopyContents");

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreateBundles(pDevice, emPSType);
				}
			}

			CopyRenderer::Impl::~Impl()
			{
				m_copyContents.Destroy();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					for (int j = 0; j < eFrameBufferCount; ++j)
					{
						util::ReleaseResource(m_pipelineStates[i].pBundles[j]);
						m_pipelineStates[i].pBundles[j] = nullptr;
					}

					m_pipelineStates[i].psoCache.Destroy();
				}
			}

			void CopyRenderer::Impl::Copy_RGBA(RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				CommonReady(pCommandList, frameIndex, pSource, pResult);
				CommonDraw(pCommandList, pSRVDescriptorHeap, frameIndex, shader::ePS_RGBA, pSource, pResult);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}
				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			void CopyRenderer::Impl::Copy_RGB(RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				CommonReady(pCommandList, frameIndex, pSource, pResult);
				CommonDraw(pCommandList, pSRVDescriptorHeap, frameIndex, shader::ePS_RGB, pSource, pResult);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}
				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			void CopyRenderer::Impl::CommonReady(ID3D12GraphicsCommandList2* pCommandList, uint32_t frameIndex, RenderTarget* pSource, RenderTarget* pResult)
			{
				shader::SetCopyContents(m_copyContents.Cast(frameIndex, m_index), pSource->GetTexture()->GetDescriptorIndex());

				util::ChangeResourceState(pCommandList, pSource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				util::ChangeResourceState(pCommandList, pResult, D3D12_RESOURCE_STATE_RENDER_TARGET);
			}

			void CopyRenderer::Impl::CommonDraw(ID3D12GraphicsCommandList2* pCommandList, DescriptorHeap* pSRVDescriptorHeap, uint32_t frameIndex, shader::PSType emPSType, RenderTarget* pSource, RenderTarget* pResult)
			{
				const D3D12_RESOURCE_DESC desc = pResult->GetDesc();

				D3D12_VIEWPORT viewport{};
				viewport.TopLeftX = 0.f;
				viewport.TopLeftY = 0.f;
				viewport.Width = static_cast<float>(desc.Width);
				viewport.Height = static_cast<float>(desc.Height);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pCommandList->RSSetViewports(1, &viewport);

				D3D12_RECT scissorRect{};
				scissorRect.left = 0;
				scissorRect.top = 0;
				scissorRect.right = static_cast<long>(desc.Width);
				scissorRect.bottom = static_cast<long>(desc.Height);
				pCommandList->RSSetScissorRects(1, &scissorRect);

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
				{
					pResult->GetCPUHandle(),
				};
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->SetGraphicsRootSignature(m_pipelineStates[emPSType].psoCache.pRootSignature);
				pCommandList->SetGraphicsRootConstantBufferView(eRP_CopyContents, m_copyContents.GPUAddress(frameIndex, m_index));
				pCommandList->ExecuteBundle(m_pipelineStates[emPSType].pBundles[frameIndex]);

				m_index = (m_index + 1) % eCopyContentsCapacity;
			}

			ID3D12RootSignature* CopyRenderer::Impl::CreateRootSignature(ID3D12Device* pDevice, shader::PSType emPSType)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& copyContents = vecRootParameters.emplace_back();
				copyContents.InitAsConstantBufferView(shader::eCB_CopyContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(SamplerState::eMinMagMipPointClamp, shader::eSampler_Point, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void CopyRenderer::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType)
			{
				PSOCache& psoCache = m_pipelineStates[emPSType].psoCache;

				if (pShaderBlob != nullptr)
				{
					const D3D_SHADER_MACRO macros[] =
					{
						{ "DX12", "1" },
						{ nullptr, nullptr },
					};

					if (psoCache.pVSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, "VS", shader::VS_CompileVersion, &psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, shader::GetCopyPSTypeToString(emPSType), shader::PS_CompileVersion, &psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide(shader::GetCopyPSTypeToString(emPSType));

				if (psoCache.pRootSignature == nullptr)
				{
					psoCache.pRootSignature = CreateRootSignature(pDevice, emPSType);
					psoCache.pRootSignature->SetName(wstrDebugName.c_str());
				}

				if (psoCache.pPipelineState != nullptr)
				{
					util::ReleaseResource(psoCache.pPipelineState);
					psoCache.pPipelineState = nullptr;
				}

				D3D12_SHADER_BYTECODE vertexShaderBytecode{};
				vertexShaderBytecode.BytecodeLength = psoCache.pVSBlob->GetBufferSize();
				vertexShaderBytecode.pShaderBytecode = psoCache.pVSBlob->GetBufferPointer();

				D3D12_SHADER_BYTECODE pixelShaderBytecode{};
				pixelShaderBytecode.BytecodeLength = psoCache.pPSBlob->GetBufferSize();
				pixelShaderBytecode.pShaderBytecode = psoCache.pPSBlob->GetBufferPointer();

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.pRootSignature = psoCache.pRootSignature;
				psoDesc.VS = vertexShaderBytecode;
				psoDesc.PS = pixelShaderBytecode;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.SampleDesc = sampleDesc;
				psoDesc.SampleMask = 0xffffffff;
				psoDesc.RasterizerState = util::GetRasterizerDesc(RasterizerState::eSolidCullNone);
				psoDesc.BlendState = util::GetBlendDesc(BlendState::eOff);

				psoDesc.NumRenderTargets = 1;

				D3D12_RESOURCE_DESC desc = Device::GetInstance()->GetSwapChainRenderTarget(0)->GetDesc();
				psoDesc.RTVFormats[0] = desc.Format;

				psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(DepthStencilState::eRead_Write_Off);

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoCache.pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				psoCache.pPipelineState->SetName(wstrDebugName.c_str());
			}

			void CopyRenderer::Impl::CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType)
			{
				DescriptorHeap* pDescriptorHeapSRV = Device::GetInstance()->GetSRVDescriptorHeap();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					if (m_pipelineStates[emPSType].pBundles[i] != nullptr)
					{
						util::ReleaseResource(m_pipelineStates[emPSType].pBundles[i]);
						m_pipelineStates[emPSType].pBundles[i] = nullptr;
					}

					m_pipelineStates[emPSType].pBundles[i] = Device::GetInstance()->CreateBundle(m_pipelineStates[emPSType].psoCache.pPipelineState);
					ID3D12GraphicsCommandList2* pBundles = m_pipelineStates[emPSType].pBundles[i];

					pBundles->SetGraphicsRootSignature(m_pipelineStates[emPSType].psoCache.pRootSignature);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pDescriptorHeapSRV->GetHeap(),
					};
					pBundles->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					pBundles->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pDescriptorHeapSRV->GetStartGPUHandle());

					pBundles->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					pBundles->IASetVertexBuffers(0, 0, nullptr);
					pBundles->IASetIndexBuffer(nullptr);

					pBundles->DrawInstanced(4, 1, 0, 0);

					HRESULT hr = pBundles->Close();
					if (FAILED(hr))
					{
						throw_line("failed to close bundle");
					}
				}
			}

			CopyRenderer::CopyRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			CopyRenderer::~CopyRenderer()
			{
			}

			void CopyRenderer::Copy_RGBA(RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Copy_RGBA(pSource, pResult);
			}

			void CopyRenderer::Copy_RGB(RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Copy_RGB(pSource, pResult);
			}
		}
	}
}