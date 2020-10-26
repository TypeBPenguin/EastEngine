#include "stdafx.h"
#include "TerrainRendererDX12.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"

#include "GBufferDX12.h"
#include "VertexBufferDX12.h"

namespace est
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			namespace shader
			{
				enum
				{
					eMaxJobCount = 1 << 3,
				};

				struct TerrainContents
				{
					float UseDynamicLOD{ 0.f };
					float FrustumCullInHS{ 0.f };
					float DynamicTessFactor{ 50.f };
					float StaticTessFactor{ 12.f };

					math::Matrix matModelViewProjection;
					math::Matrix matPrevModelViewProjectionMatrix;
					math::Matrix matWorld;

					math::float3 CameraPosition;
					float padding0{ 0.f };

					math::float3 CameraDirection;
					float padding1{ 0.f };

					math::float2 f2PatchSize;
					math::float2 f2HeightFieldSize;

					uint32_t nTexHeightFieldIndex{ 0 };
					uint32_t nTexColor{ 0 };
					uint32_t nTexDetail{ 0 };
					uint32_t nTexDetailNormal{ 0 };
				};

				enum CBSlot
				{
					eCB_TerrainContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_SamplerLinearWrap = 0,
					eSampler_SamplerLinearBorder = 1,
					eSampler_SamplerAnisotropicBorder = 2,
				};

				enum PSType
				{
					eSolid = 0,
					eSolid_MotionBlur,

					ePS_Count,
				};

				void SetTerrainContents(TerrainContents* pTerrainContents,
					const RenderJobTerrain& terrain,
					const math::Matrix& matViewProjection, const math::Matrix& matPrevViewProjection,
					const math::float3& f3CameraPosition, const math::float3& f3CameraDirection)
				{
					pTerrainContents->UseDynamicLOD = terrain.isEnableDynamicLOD == true ? 1.f : 0.f;
					pTerrainContents->FrustumCullInHS = terrain.isEnableFrustumCullInHS == true ? 1.f : 0.f;
					pTerrainContents->DynamicTessFactor = terrain.fDynamicTessFactor;
					pTerrainContents->StaticTessFactor = terrain.fStaticTessFactor;

					pTerrainContents->matModelViewProjection = (terrain.matWorld * matViewProjection).Transpose();
					pTerrainContents->matPrevModelViewProjectionMatrix = (terrain.matPrevWorld * matPrevViewProjection).Transpose();
					pTerrainContents->matWorld = terrain.matWorld.Transpose();

					pTerrainContents->CameraPosition = f3CameraPosition;
					pTerrainContents->padding0 = 0.f;

					pTerrainContents->CameraDirection = f3CameraDirection;
					pTerrainContents->padding1 = 0.f;

					pTerrainContents->f2PatchSize = terrain.f2PatchSize;
					pTerrainContents->f2HeightFieldSize = terrain.f2HeightFieldSize;

					pTerrainContents->nTexHeightFieldIndex = static_cast<Texture*>(terrain.pTexHeightField.get())->GetDescriptorIndex();
					pTerrainContents->nTexColor = static_cast<Texture*>(terrain.pTexColorMap.get())->GetDescriptorIndex();
					pTerrainContents->nTexDetail = static_cast<Texture*>(terrain.pTexDetailMap.get())->GetDescriptorIndex();
					pTerrainContents->nTexDetailNormal = static_cast<Texture*>(terrain.pTexDetailNormalMap.get())->GetDescriptorIndex();
				}
			}

			class TerrainRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Render(const RenderElement& renderElement, const math::Matrix& matPrevViewProjection);
				void Cleanup();

			public:
				void PushJob(const RenderJobTerrain& job)
				{
					if (m_vecTerrains.size() < shader::eMaxJobCount)
					{
						m_vecTerrains.emplace_back(job);
					}
					else
					{
						LOG_ERROR(L"Terrain Job Count Over, MaxJobCount[%d]", shader::eMaxJobCount);
					}
				}

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_TerrainContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType);

				shader::TerrainContents* AllocateTerrainContents(uint32_t frameIndex)
				{
					assert(m_nTerrainBufferIndex < shader::eMaxJobCount);
					shader::TerrainContents* pBuffer = m_terrainContents.Cast(frameIndex, m_nTerrainBufferIndex);
					m_terrainBufferGPUAddress = m_terrainContents.GPUAddress(frameIndex, m_nTerrainBufferIndex);
					++m_nTerrainBufferIndex;

					return pBuffer;
				}

				void ResetBloomFilterContents(uint32_t frameIndex)
				{
					m_nTerrainBufferIndex = 0;
					m_terrainBufferGPUAddress = m_terrainContents.GPUAddress(frameIndex, 0);
				}

			private:
				std::array<PSOCache, shader::ePS_Count> m_psoCaches;

				ConstantBuffer<shader::TerrainContents> m_terrainContents;
				D3D12_GPU_VIRTUAL_ADDRESS m_terrainBufferGPUAddress{};
				uint32_t m_nTerrainBufferIndex{ 0 };

				std::vector<RenderJobTerrain> m_vecTerrains;
			};

			TerrainRenderer::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\Terrain\\Terrain.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Terrain.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str(), shader::eSolid);
				CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str(), shader::eSolid_MotionBlur);

				m_terrainContents.Create(pDevice, shader::eMaxJobCount, "TerrainContents");

				SafeRelease(pShaderBlob);
			}

			TerrainRenderer::Impl::~Impl()
			{
				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					m_psoCaches[i].Destroy();
				}

				m_terrainContents.Destroy();
			}

			void TerrainRenderer::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
			}

			void TerrainRenderer::Impl::Render(const RenderElement& renderElement, const math::Matrix& matPrevViewProjection)
			{
				if (m_vecTerrains.empty() == true)
					return;

				TRACER_EVENT(__FUNCTIONW__);
				Device* pDeviceInstance = Device::GetInstance();

				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();

				Camera* pCamera = renderElement.pCamera;

				shader::PSType emPSType = shader::eSolid;

				const math::Matrix matViewProjection = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();

				ResetBloomFilterContents(frameIndex);

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, m_psoCaches[emPSType].pPipelineState);

				pCommandList->SetGraphicsRootSignature(m_psoCaches[emPSType].pRootSignature);

				pCommandList->RSSetViewports(1, pViewport);
				pCommandList->RSSetScissorRects(1, pScissorRect);
				pCommandList->OMSetRenderTargets(renderElement.rtvCount, renderElement.rtvHandles, FALSE, renderElement.GetDSVHandle());

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle());

				pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

				for (auto& terrainJob : m_vecTerrains)
				{
					shader::TerrainContents* pTerrainContents = AllocateTerrainContents(frameIndex);
					shader::SetTerrainContents(pTerrainContents, terrainJob, matViewProjection, matPrevViewProjection, pCamera->GetPosition(), pCamera->GetDirection());

					pCommandList->SetGraphicsRootConstantBufferView(eRP_TerrainContents, m_terrainBufferGPUAddress);

					VertexBuffer* pVertexBuffer = static_cast<VertexBuffer*>(terrainJob.pVertexBuffer.get());
					pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

					pCommandList->DrawInstanced(pVertexBuffer->GetVertexCount(), 1, 0, 0);
				}

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			void TerrainRenderer::Impl::Cleanup()
			{
				m_vecTerrains.clear();
			}

			ID3D12RootSignature* TerrainRenderer::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_ALL);

				CD3DX12_ROOT_PARAMETER& terrainContentsParameter = vecRootParameters.emplace_back();
				terrainContentsParameter.InitAsConstantBufferView(shader::eCB_TerrainContents, 0, D3D12_SHADER_VISIBILITY_ALL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipLinearWrap, shader::eSampler_SamplerLinearWrap, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipLinearBorder, shader::eSampler_SamplerLinearBorder, 100, D3D12_SHADER_VISIBILITY_ALL),
					util::GetStaticSamplerDesc(EmSamplerState::eAnisotropicBorder, shader::eSampler_SamplerAnisotropicBorder, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void TerrainRenderer::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType)
			{
				PSOCache& psoCache = m_psoCaches[emPSType];
				if (pShaderBlob != nullptr)
				{
					std::vector<D3D_SHADER_MACRO> macros;
					macros.emplace_back(D3D_SHADER_MACRO{ "DX12", "1" });
					if (emPSType == shader::eSolid_MotionBlur)
					{
						macros.emplace_back(D3D_SHADER_MACRO{ "USE_MOTION_BLUR", "1" });
					}
					macros.emplace_back(D3D_SHADER_MACRO{ nullptr, nullptr });

					if (psoCache.pVSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros.data(), shaderPath, "PassThroughVS", shader::VS_CompileVersion, &psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros.data(), shaderPath, "HeightFieldPatchPS", shader::PS_CompileVersion, &psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}

					if (psoCache.pHSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros.data(), shaderPath, "PatchHS", shader::HS_CompileVersion, &psoCache.pHSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile hull shader");
						}
					}

					if (psoCache.pDSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros.data(), shaderPath, "HeightFieldPatchDS", shader::DS_CompileVersion, &psoCache.pDSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile domain shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide("HeightFieldPatchPS");

				if (psoCache.pRootSignature == nullptr)
				{
					psoCache.pRootSignature = CreateRootSignature(pDevice);
					psoCache.pRootSignature->SetName(wstrDebugName.c_str());
				}

				if (psoCache.pPipelineState != nullptr)
				{
					util::ReleaseResource(psoCache.pPipelineState);
					psoCache.pPipelineState = nullptr;
				}

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.VS.BytecodeLength = psoCache.pVSBlob->GetBufferSize();
				psoDesc.VS.pShaderBytecode = psoCache.pVSBlob->GetBufferPointer();

				psoDesc.PS.BytecodeLength = psoCache.pPSBlob->GetBufferSize();
				psoDesc.PS.pShaderBytecode = psoCache.pPSBlob->GetBufferPointer();

				psoDesc.HS.BytecodeLength = psoCache.pHSBlob->GetBufferSize();
				psoDesc.HS.pShaderBytecode = psoCache.pHSBlob->GetBufferPointer();

				psoDesc.DS.BytecodeLength = psoCache.pDSBlob->GetBufferSize();
				psoDesc.DS.pShaderBytecode = psoCache.pDSBlob->GetBufferPointer();

				const D3D12_INPUT_ELEMENT_DESC* pInputElements = nullptr;
				size_t nElementCount = 0;
				util::GetInputElementDesc(VertexPos4::Format(), &pInputElements, &nElementCount);

				psoDesc.InputLayout.NumElements = static_cast<uint32_t>(nElementCount);
				psoDesc.InputLayout.pInputElementDescs = pInputElements;

				psoDesc.SampleDesc.Count = 1;

				psoDesc.pRootSignature = psoCache.pRootSignature;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
				psoDesc.SampleMask = 0xffffffff;
				psoDesc.RasterizerState = util::GetRasterizerDesc(EmRasterizerState::eSolidCCW);
				psoDesc.BlendState = util::GetBlendDesc(EmBlendState::eOff);

				if (emPSType == shader::eSolid_MotionBlur)
				{
					psoDesc.NumRenderTargets = GBufferTypeCount;
					psoDesc.RTVFormats[GBufferType::eNormals] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eNormals));
					psoDesc.RTVFormats[GBufferType::eColors] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eColors));
					psoDesc.RTVFormats[GBufferType::eDisneyBRDF] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eDisneyBRDF));
					psoDesc.RTVFormats[GBufferType::eVelocity] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eVelocity));
				}
				else
				{
					psoDesc.NumRenderTargets = GBufferTypeCount - 1;
					psoDesc.RTVFormats[GBufferType::eNormals] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eNormals));
					psoDesc.RTVFormats[GBufferType::eColors] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eColors));
					psoDesc.RTVFormats[GBufferType::eDisneyBRDF] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eDisneyBRDF));
				}

				psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(EmDepthStencilState::eRead_Write_On);

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoCache.pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				psoCache.pPipelineState->SetName(wstrDebugName.c_str());
			}

			TerrainRenderer::TerrainRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			TerrainRenderer::~TerrainRenderer()
			{
			}

			void TerrainRenderer::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void TerrainRenderer::Render(const RenderElement& renderElement, const math::Matrix& matPrevViewProjection)
			{
				m_pImpl->Render(renderElement, matPrevViewProjection);
			}

			void TerrainRenderer::Cleanup()
			{
				m_pImpl->Cleanup();
			}

			void TerrainRenderer::PushJob(const RenderJobTerrain& job)
			{
				m_pImpl->PushJob(job);
			}
		}
	}
}