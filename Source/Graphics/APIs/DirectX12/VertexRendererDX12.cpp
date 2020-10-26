#include "stdafx.h"
#include "VertexRendererDX12.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Lock.h"

#include "Graphics/Interface/Camera.h"
#include "Graphics/Interface/Instancing.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"

#include "RenderTargetDX12.h"
#include "VertexBufferDX12.h"
#include "IndexBufferDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				enum
				{
					eMaxJobCount = 2048,
					eMaxInstancingJobCount = eMaxJobCount / 2,
				};

				struct VertexInstancingData
				{
					TransformInstancingData worldData;
					math::Color color;

					VertexInstancingData() = default;
					VertexInstancingData(const math::Matrix& matWorld, const math::Color& color)
						: worldData(matWorld)
						, color(color)
					{
					}
				};

				struct SingleContents
				{
					math::Matrix matWVP;
					math::Color color{ math::Color::White };
				};

				struct InstanceContents
				{
					math::Matrix matViewProjection;
					std::array<VertexInstancingData, eMaxInstancingCount> data;
				};

				enum CBSlot
				{
					eCB_SingleContents = 0,
					eCB_InstancingContents = 1,
				};

				enum VSType
				{
					eVertex = 0,
					eVertexInstancing,

					eVS_Count,
				};

				const char* GetVertexVSTypeToString(VSType emVSType)
				{
					switch (emVSType)
					{
					case eVertex:
						return "VS";
					case eVertexInstancing:
						return "VS_Instancing";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetSingleContents(SingleContents* pSingleContents, const math::Matrix& matWVP, const math::Color& color)
				{
					pSingleContents->matWVP = matWVP.Transpose();
					pSingleContents->color = color;
				}
			}

			class VertexRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Render(const RenderElement& renderElement);
				void Cleanup();

			public:
				void PushJob(const RenderJobVertex& job);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_SingleContents,
					eRP_InstanceContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

			private:
				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice, shader::VSType emVSType);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* strshaderPath, shader::VSType emVSType);

				shader::SingleContents* AllocateSingleContents(uint32_t frameIndex)
				{
					assert(m_singleContentsIndex < shader::eMaxJobCount);
					shader::SingleContents* pBuffer = m_singleContents.Cast(frameIndex, m_singleContentsIndex);
					m_singleContentsGPUAddress = m_singleContents.GPUAddress(frameIndex, m_singleContentsIndex);
					++m_singleContentsIndex;

					return pBuffer;
				}

				void ReseSingleContents(uint32_t frameIndex)
				{
					m_singleContentsIndex = 0;
					m_singleContentsGPUAddress = m_singleContents.GPUAddress(frameIndex, 0);
				}

				shader::InstanceContents* AllocateInstanceContents(uint32_t frameIndex)
				{
					assert(m_instanceContentsIndex < shader::eMaxInstancingJobCount);
					shader::InstanceContents* pBuffer = m_instanceContents.Cast(frameIndex, m_instanceContentsIndex);
					m_instanceContentsGPUAddress = m_instanceContents.GPUAddress(frameIndex, m_instanceContentsIndex);
					++m_instanceContentsIndex;

					return pBuffer;
				}

				void ResetInstanceContents(uint32_t frameIndex)
				{
					m_instanceContentsIndex = 0;
					m_instanceContentsGPUAddress = m_instanceContents.GPUAddress(frameIndex, 0);
				}

			private:
				std::array<PSOCache, shader::eVS_Count> m_psoCaches;

				ConstantBuffer<shader::SingleContents> m_singleContents;
				D3D12_GPU_VIRTUAL_ADDRESS m_singleContentsGPUAddress{};
				uint32_t m_singleContentsIndex{ 0 };

				ConstantBuffer<shader::InstanceContents> m_instanceContents;
				D3D12_GPU_VIRTUAL_ADDRESS m_instanceContentsGPUAddress{};
				uint32_t m_instanceContentsIndex{ 0 };

				thread::SRWLock m_srwLock;

				struct JobVertexBatch
				{
					RenderJobVertex job;
					std::vector<shader::VertexInstancingData> instanceData;

					JobVertexBatch(const RenderJobVertex& job)
						: job(job)
					{
						instanceData.emplace_back(job.matWorld, job.color);
					}
				};
				tsl::robin_map<VertexBufferPtr, JobVertexBatch> m_umapJobVertexBatchs;
			};

			VertexRenderer::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\Vertex\\Vertex.hlsl");

				ID3DBlob* pShaderBlob = nullptr;
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Vertex.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str(), shader::eVertex);
				CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str(), shader::eVertexInstancing);

				m_singleContents.Create(pDevice, shader::eMaxJobCount, "SingleContents");
				m_instanceContents.Create(pDevice, shader::eMaxInstancingJobCount, "InstanceContents");

				SafeRelease(pShaderBlob);

				m_umapJobVertexBatchs.reserve(256);
			}

			VertexRenderer::Impl::~Impl()
			{
				m_singleContents.Destroy();
				m_instanceContents.Destroy();

				for (int i = 0; i < shader::eVS_Count; ++i)
				{
					m_psoCaches[i].Destroy();
				}
			}

			void VertexRenderer::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				for (int i = 0; i < shader::eVS_Count; ++i)
				{
					const shader::VSType emVSType = static_cast<shader::VSType>(i);
					CreatePipelineState(pDevice, nullptr, nullptr, emVSType);
				}
			}

			void VertexRenderer::Impl::Render(const RenderElement& renderElement)
			{
				if (m_umapJobVertexBatchs.empty() == true)
					return;

				TRACER_EVENT(__FUNCTIONW__);
				Device* pDeviceInstance = Device::GetInstance();
				Camera* pCamera = renderElement.pCamera;

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();

				ReseSingleContents(frameIndex);
				ResetInstanceContents(frameIndex);

				const math::Matrix matViewProjection = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				pCommandList->RSSetViewports(1, pViewport);
				pCommandList->RSSetScissorRects(1, pScissorRect);
				pCommandList->OMSetRenderTargets(renderElement.rtvCount, renderElement.rtvHandles, FALSE, renderElement.GetDSVHandle());
				pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				pCommandList->SetPipelineState(m_psoCaches[shader::eVertex].pPipelineState);
				pCommandList->SetGraphicsRootSignature(m_psoCaches[shader::eVertex].pRootSignature);

				for (auto& iter : m_umapJobVertexBatchs)
				{
					const JobVertexBatch& batch = iter.second;

					const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(batch.job.pVertexBuffer.get());
					const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(batch.job.pIndexBuffer.get());

					const size_t size = batch.instanceData.size();
					if (size == 1)
					{
						shader::SingleContents* pSingleContents = AllocateSingleContents(frameIndex);
						shader::SetSingleContents(pSingleContents, batch.job.matWorld * matViewProjection, batch.job.color);

						pCommandList->SetGraphicsRootConstantBufferView(0, m_singleContentsGPUAddress);

						pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

						if (batch.job.pIndexBuffer != nullptr)
						{
							pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());
							pCommandList->DrawIndexedInstanced(pIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
						}
						else
						{
							pCommandList->IASetIndexBuffer(nullptr);
							pCommandList->DrawInstanced(pVertexBuffer->GetVertexCount(), 1, 0, 0);
						}
					}
				}

				pCommandList->SetPipelineState(m_psoCaches[shader::eVertexInstancing].pPipelineState);
				pCommandList->SetGraphicsRootSignature(m_psoCaches[shader::eVertexInstancing].pRootSignature);

				for (auto& iter : m_umapJobVertexBatchs)
				{
					const JobVertexBatch& batch = iter.second;

					const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(batch.job.pVertexBuffer.get());
					const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(batch.job.pIndexBuffer.get());

					const size_t size = batch.instanceData.size();
					if (size > 1)
					{
						const shader::VertexInstancingData* pInstanceData = batch.instanceData.data();
						const size_t nInstanceCount = batch.instanceData.size();

						const size_t loopCount = nInstanceCount / eMaxInstancingCount + 1;
						for (size_t i = 0; i < loopCount; ++i)
						{
							const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), nInstanceCount);
							const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

							if (nDrawInstanceCount <= 0)
								break;

							shader::InstanceContents* pInstanceContents = AllocateInstanceContents(frameIndex);
							pInstanceContents->matViewProjection = matViewProjection.Transpose();
							memory::Copy(pInstanceContents->data.data(), sizeof(pInstanceContents->data),
								&pInstanceData[i * eMaxInstancingCount], sizeof(shader::VertexInstancingData) * nDrawInstanceCount);

							pCommandList->SetGraphicsRootConstantBufferView(0, m_instanceContentsGPUAddress);

							pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

							if (pIndexBuffer != nullptr)
							{
								pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());
								pCommandList->DrawIndexedInstanced(pIndexBuffer->GetIndexCount(), static_cast<uint32_t>(nDrawInstanceCount), 0, 0, 0);
							}
							else
							{
								pCommandList->IASetIndexBuffer(nullptr);
								pCommandList->DrawInstanced(pVertexBuffer->GetVertexCount(), static_cast<uint32_t>(nDrawInstanceCount), 0, 0);
							}
						}
					}
				}

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			void VertexRenderer::Impl::Cleanup()
			{
				m_umapJobVertexBatchs.clear();
			}

			void VertexRenderer::Impl::PushJob(const RenderJobVertex& job)
			{
				thread::SRWWriteLock lock(&m_srwLock);

				auto iter = m_umapJobVertexBatchs.find(job.pVertexBuffer);
				if (iter != m_umapJobVertexBatchs.end())
				{
					iter.value().instanceData.emplace_back(job.matWorld, job.color);
				}
				else
				{
					m_umapJobVertexBatchs.emplace(job.pVertexBuffer, job);
				}
			}

			ID3D12RootSignature* VertexRenderer::Impl::CreateRootSignature(ID3D12Device* pDevice, shader::VSType emVSType)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;

				switch (emVSType)
				{
				case shader::eVertex:
				{
					CD3DX12_ROOT_PARAMETER& singleContentsParameter = vecRootParameters.emplace_back();
					singleContentsParameter.InitAsConstantBufferView(shader::eCB_SingleContents, 0, D3D12_SHADER_VISIBILITY_VERTEX);
				}
				break;
				case shader::eVertexInstancing:
				{
					CD3DX12_ROOT_PARAMETER& instancingContentsParameter = vecRootParameters.emplace_back();
					instancingContentsParameter.InitAsConstantBufferView(shader::eCB_InstancingContents, 0, D3D12_SHADER_VISIBILITY_VERTEX);
				}
				break;
				}

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					0, nullptr,
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void VertexRenderer::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::VSType emVSType)
			{
				if (m_psoCaches[emVSType].pVSBlob == nullptr || m_psoCaches[emVSType].pPSBlob == nullptr)
				{
					std::vector<D3D_SHADER_MACRO> vecMacros;
					vecMacros.push_back({ "DX11", "1" });
					vecMacros.push_back({ nullptr, nullptr });

					if (m_psoCaches[emVSType].pVSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, vecMacros.data(), shaderPath, shader::GetVertexVSTypeToString(emVSType), shader::VS_CompileVersion, &m_psoCaches[emVSType].pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (m_psoCaches[emVSType].pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, vecMacros.data(), shaderPath, "PS", shader::PS_CompileVersion, &m_psoCaches[emVSType].pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide(shader::GetVertexVSTypeToString(emVSType));

				if (m_psoCaches[emVSType].pRootSignature == nullptr)
				{
					m_psoCaches[emVSType].pRootSignature = CreateRootSignature(pDevice, emVSType);
					m_psoCaches[emVSType].pRootSignature->SetName(wstrDebugName.c_str());
				}

				if (m_psoCaches[emVSType].pPipelineState != nullptr)
				{
					util::ReleaseResource(m_psoCaches[emVSType].pPipelineState);
					m_psoCaches[emVSType].pPipelineState = nullptr;
				}

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.VS.BytecodeLength = m_psoCaches[emVSType].pVSBlob->GetBufferSize();
				psoDesc.VS.pShaderBytecode = m_psoCaches[emVSType].pVSBlob->GetBufferPointer();

				psoDesc.PS.BytecodeLength = m_psoCaches[emVSType].pPSBlob->GetBufferSize();
				psoDesc.PS.pShaderBytecode = m_psoCaches[emVSType].pPSBlob->GetBufferPointer();

				const D3D12_INPUT_ELEMENT_DESC* pInputElements = nullptr;
				size_t nElementCount = 0;
				util::GetInputElementDesc(VertexPos::Format(), &pInputElements, &nElementCount);

				psoDesc.InputLayout.NumElements = static_cast<uint32_t>(nElementCount);
				psoDesc.InputLayout.pInputElementDescs = pInputElements;

				psoDesc.SampleDesc.Count = 1;

				psoDesc.pRootSignature = m_psoCaches[emVSType].pRootSignature;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.SampleMask = 0xffffffff;
				psoDesc.RasterizerState = util::GetRasterizerDesc(EmRasterizerState::Type::eWireframeCullNone);
				psoDesc.BlendState = util::GetBlendDesc(EmBlendState::eOff);

				psoDesc.NumRenderTargets = 1;

				if (GetOptions().OnHDR == true)
				{
					psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}
				else
				{
					RenderTarget* pRenderTarget = Device::GetInstance()->GetSwapChainRenderTarget(0);
					psoDesc.RTVFormats[0] = pRenderTarget->GetDesc().Format;
				}

				psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(EmDepthStencilState::eRead_Write_Off);

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoCaches[emVSType].pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				m_psoCaches[emVSType].pPipelineState->SetName(wstrDebugName.c_str());
			}

			VertexRenderer::VertexRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			VertexRenderer::~VertexRenderer()
			{
			}

			void VertexRenderer::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void VertexRenderer::Render(const RenderElement& renderElement)
			{
				m_pImpl->Render(renderElement);
			}

			void VertexRenderer::Cleanup()
			{
				m_pImpl->Cleanup();
			}

			void VertexRenderer::PushJob(const RenderJobVertex& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}
		}
	}
}