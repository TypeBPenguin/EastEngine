#include "stdafx.h"
#include "VertexRendererDX11.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Lock.h"

#include "Graphics/Interface/Camera.h"
#include "Graphics/Interface/Instancing.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"

#include "RenderTargetDX11.h"
#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct VertexInstancingData
				{
					TransformInstancingData worldData;
					math::Color color;

					VertexInstancingData() = default;
					VertexInstancingData(const math::Matrix& worldMatrix, const math::Color& color)
						: worldData(worldMatrix)
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

				void SetSingleContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<SingleContents>* pCB_SingleContents, const math::Matrix& matWVP, const math::Color& color)
				{
					SingleContents* pSingleContents = pCB_SingleContents->Map(pDeviceContext);
					{
						pSingleContents->matWVP = matWVP.Transpose();
						pSingleContents->color = color;
					}
					pCB_SingleContents->Unmap(pDeviceContext);
				}

				void Draw(ID3D11DeviceContext* pDeviceContext, const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer)
				{
					ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
					const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
					const uint32_t nOffsets[] = { 0, };
					pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);

					if (pIndexBuffer != nullptr)
					{
						pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
						pDeviceContext->DrawIndexed(pIndexBuffer->GetIndexCount(), 0, 0);
					}
					else
					{
						pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
						pDeviceContext->Draw(pVertexBuffer->GetVertexCount(), 0);
					}
				}

				void DrawInstance(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<InstanceContents>* pCB_InstanceContents,
					const math::Matrix& matViewProjection,
					const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer,
					const VertexInstancingData* pInstanceData, size_t nInstanceCount)
				{
					ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
					const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
					const uint32_t nOffsets[] = { 0, };
					pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);

					if (pIndexBuffer != nullptr)
					{
						pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
					}
					else
					{
						pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
					}

					const size_t loopCount = nInstanceCount / eMaxInstancingCount + 1;
					for (size_t i = 0; i < loopCount; ++i)
					{
						const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), nInstanceCount);
						const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

						if (nDrawInstanceCount <= 0)
							break;

						InstanceContents* pInstanceContents = pCB_InstanceContents->Map(pDeviceContext);
						pInstanceContents->matViewProjection = matViewProjection.Transpose();
						memory::Copy(pInstanceContents->data.data(), sizeof(pInstanceContents->data), &pInstanceData[i * eMaxInstancingCount], sizeof(VertexInstancingData) * nDrawInstanceCount);
						pCB_InstanceContents->Unmap(pDeviceContext);

						if (pIndexBuffer != nullptr)
						{
							pDeviceContext->DrawIndexedInstanced(pIndexBuffer->GetIndexCount(), static_cast<uint32_t>(nDrawInstanceCount), 0, 0, 0);
						}
						else
						{
							pDeviceContext->DrawInstanced(pVertexBuffer->GetVertexCount(), static_cast<uint32_t>(nDrawInstanceCount), 0, 0);
						}
					}
				}
			}

			class VertexRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(const RenderElement& renderElement);
				void AllCleanup();
				void Cleanup();

			public:
				void PushJob(const RenderJobVertex& job);

			private:
				void CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::VSType emVSType);
				void CreatePixelShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, const char* pFunctionName);

			private:
				std::array<ID3D11VertexShader*, shader::VSType::eVS_Count> m_pVertexShader{ nullptr };
				ID3D11PixelShader* m_pPixelShader{ nullptr };
				ID3D11InputLayout* m_pInputLayout{ nullptr };

				thread::SRWLock m_srwLock;

				ConstantBuffer<shader::SingleContents> m_singleContents;
				ConstantBuffer<shader::InstanceContents> m_instanceContents;

				struct JobVertexBatch
				{
					RenderJobVertex job;
					std::vector<shader::VertexInstancingData> instanceData;

					JobVertexBatch(const RenderJobVertex& job)
						: job(job)
					{
						instanceData.emplace_back(job.worldMatrix, job.color);
					}
				};
				tsl::robin_map<VertexBufferPtr, JobVertexBatch> m_umapJobVertexBatchs[2];
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

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				CreateVertexShader(pDevice, pShaderBlob, shaderPath.c_str(), shader::eVertex);
				CreateVertexShader(pDevice, pShaderBlob, shaderPath.c_str(), shader::eVertexInstancing);
				CreatePixelShader(pDevice, pShaderBlob, shaderPath.c_str(), "PS");

				m_singleContents.Create(pDevice, "SingleContents");
				m_instanceContents.Create(pDevice, "InstanceContents");

				SafeRelease(pShaderBlob);

				m_umapJobVertexBatchs[UpdateThread()].reserve(256);
				m_umapJobVertexBatchs[RenderThread()].reserve(256);
			}

			VertexRenderer::Impl::~Impl()
			{
				m_singleContents.Destroy();
				m_instanceContents.Destroy();

				SafeRelease(m_pInputLayout);
				SafeRelease(m_pPixelShader);
				for (int i = 0; i < shader::eVS_Count; ++i)
				{
					SafeRelease(m_pVertexShader[i]);
				}
			}

			void VertexRenderer::Impl::Render(const RenderElement& renderElement)
			{
				if (m_umapJobVertexBatchs[RenderThread()].empty() == true)
					return;

				TRACER_EVENT(__FUNCTIONW__);
				DX_PROFILING(TerrainRenderer);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = renderElement.pDeviceContext;
				Camera* pCamera = renderElement.pCamera;

				const math::Matrix matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();

				pDeviceContext->ClearState();

				const math::Viewport& viewport = pDeviceInstance->GetViewport();
				pDeviceContext->RSSetViewports(1, util::Convert(viewport));

				pDeviceContext->OMSetRenderTargets(renderElement.rtvCount, renderElement.pRTVs, renderElement.pDSV);

				pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

				pDeviceContext->IASetInputLayout(m_pInputLayout);
				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(BlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(DepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eWireframeCullNone);
				pDeviceContext->RSSetState(pRasterizerState);

				const math::Matrix matViewProjection = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();

				for (auto& iter : m_umapJobVertexBatchs[RenderThread()])
				{
					const JobVertexBatch& batch = iter.second;

					const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(batch.job.pVertexBuffer.get());
					const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(batch.job.pIndexBuffer.get());

					const size_t size = batch.instanceData.size();
					if (size == 1)
					{
						pDeviceContext->VSSetShader(m_pVertexShader[shader::eVertex], nullptr, 0);
						pDeviceContext->VSSetConstantBuffers(shader::eCB_SingleContents, 1, &m_singleContents.pBuffer);

						shader::SetSingleContents(pDeviceContext, &m_singleContents, batch.job.worldMatrix * matViewProjection, batch.job.color);
						shader::Draw(pDeviceContext, pVertexBuffer, pIndexBuffer);
					}
					else
					{
						pDeviceContext->VSSetShader(m_pVertexShader[shader::eVertexInstancing], nullptr, 0);
						pDeviceContext->VSSetConstantBuffers(shader::eCB_InstancingContents, 1, &m_instanceContents.pBuffer);

						shader::DrawInstance(pDeviceContext, &m_instanceContents, matViewProjection, pVertexBuffer, pIndexBuffer, batch.instanceData.data(), batch.instanceData.size());
					}
				}
			}

			void VertexRenderer::Impl::AllCleanup()
			{
				m_umapJobVertexBatchs[UpdateThread()].clear();
				m_umapJobVertexBatchs[RenderThread()].clear();
			}

			void VertexRenderer::Impl::Cleanup()
			{
				m_umapJobVertexBatchs[RenderThread()].clear();
			}

			void VertexRenderer::Impl::PushJob(const RenderJobVertex& job)
			{
				thread::SRWWriteLock lock(&m_srwLock);

				auto iter = m_umapJobVertexBatchs[UpdateThread()].find(job.pVertexBuffer);
				if (iter != m_umapJobVertexBatchs[UpdateThread()].end())
				{
					iter.value().instanceData.emplace_back(job.worldMatrix, job.color);
				}
				else
				{
					m_umapJobVertexBatchs[UpdateThread()].emplace(job.pVertexBuffer, job);
				}
			}

			void VertexRenderer::Impl::CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::VSType emVSType)
			{
				std::vector<D3D_SHADER_MACRO> vecMacros;
				vecMacros.push_back({ "DX11", "1" });
				vecMacros.push_back({ nullptr, nullptr });

				if (m_pInputLayout == nullptr)
				{
					const D3D11_INPUT_ELEMENT_DESC* pInputElements = nullptr;
					size_t nElementCount = 0;

					util::GetInputElementDesc(VertexPos::Format(), &pInputElements, &nElementCount);

					if (pInputElements == nullptr || nElementCount == 0)
					{
						throw_line("invalid vertex shader input elements");
					}

					const std::string functionName = shader::GetVertexVSTypeToString(emVSType);
					if (util::CreateVertexShader(pDevice, pShaderBlob, vecMacros.data(), shaderPath, functionName.c_str(), shader::VS_CompileVersion, &m_pVertexShader[emVSType], pInputElements, nElementCount, &m_pInputLayout, functionName.c_str()) == false)
					{
						LOG_ERROR(L"failed to create vertex shader : %s", functionName.c_str());
						return;
					}
				}
				else
				{
					const std::string functionName = shader::GetVertexVSTypeToString(emVSType);
					if (util::CreateVertexShader(pDevice, pShaderBlob, vecMacros.data(), shaderPath, functionName.c_str(), shader::VS_CompileVersion, &m_pVertexShader[emVSType], functionName.c_str()) == false)
					{
						LOG_ERROR(L"failed to create vertex shader : %s", functionName.c_str());
						return;
					}
				}
			}

			void VertexRenderer::Impl::CreatePixelShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, const char* pFunctionName)
			{
				std::vector<D3D_SHADER_MACRO> vecMacros;
				vecMacros.push_back({ "DX11", "1" });
				vecMacros.push_back({ nullptr, nullptr });

				if (util::CreatePixelShader(pDevice, pShaderBlob, vecMacros.data(), shaderPath, pFunctionName, shader::PS_CompileVersion, &m_pPixelShader, "VertexPS") == false)
				{
					LOG_ERROR(L"failed to create pixel shader : %u", "VertexPS");
					return;
				}
			}

			VertexRenderer::VertexRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			VertexRenderer::~VertexRenderer()
			{
			}

			void VertexRenderer::Render(const RenderElement& renderElement)
			{
				m_pImpl->Render(renderElement);
			}

			void VertexRenderer::AllCleanup()
			{
				m_pImpl->AllCleanup();
			}

			void VertexRenderer::Cleanup()
			{
				m_pImpl->Cleanup();
			}

			void VertexRenderer::PushJob(const RenderJobVertex& job)
			{
				m_pImpl->PushJob(job);
			}
		}
	}
}