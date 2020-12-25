#include "stdafx.h"
#include "TerrainRendererDX11.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "GBufferDX11.h"
#include "LightResourceManagerDX11.h"

#include "TextureDX11.h"
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
				struct TerrainContents
				{
					float UseDynamicLOD{ 0.f };
					float FrustumCullInHS{ 0.f };
					float DynamicTessFactor{ 50.f };
					float StaticTessFactor{ 12.f };

					math::Matrix matModelViewProjection;
					math::Matrix matPrevModelViewProjectionMatrix;
					math::Matrix worldMatrix;

					math::float3 CameraPosition;
					float padding0{ 0.f };

					math::float3 CameraDirection;
					float padding1{ 0.f };

					math::float2 f2PatchSize;
					math::float2 f2HeightFieldSize;
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

				enum SRVSlot
				{
					eSRV_HeightField = 0,
					eSRV_Color = 1,
					eSRV_Detail = 2,
					eSRV_DetailNormal = 3,
				};

				enum PSType
				{
					eSolid = 0,
					eSolid_MotionBlur,
					eDepth,

					ePS_Count,
				};

				void SetTerrainContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<TerrainContents>* pCB_TerrainContents,
					const RenderJobTerrain& terrain, 
					const math::Matrix& matViewProjection, const math::Matrix& matPrevViewProjection,
					const math::float3& f3CameraPosition, const math::float3& f3CameraDirection,
					bool isRenderDepth)
				{
					ID3D11ShaderResourceView* pSRV = static_cast<Texture*>(terrain.pTexHeightField.get())->GetShaderResourceView();
					pDeviceContext->HSSetShaderResources(shader::eSRV_HeightField, 1, &pSRV);
					pDeviceContext->DSSetShaderResources(shader::eSRV_HeightField, 1, &pSRV);

					if (isRenderDepth == false)
					{
						pSRV = static_cast<Texture*>(terrain.pTexColorMap.get())->GetShaderResourceView();
						pDeviceContext->PSSetShaderResources(shader::eSRV_Color, 1, &pSRV);

						pSRV = static_cast<Texture*>(terrain.pTexDetailMap.get())->GetShaderResourceView();
						pDeviceContext->PSSetShaderResources(shader::eSRV_Detail, 1, &pSRV);

						pSRV = static_cast<Texture*>(terrain.pTexDetailNormalMap.get())->GetShaderResourceView();
						pDeviceContext->PSSetShaderResources(shader::eSRV_DetailNormal, 1, &pSRV);
					}

					TerrainContents* pTerrainContents = pCB_TerrainContents->Map(pDeviceContext);
					
					pTerrainContents->UseDynamicLOD = terrain.isEnableDynamicLOD == true ? 1.f : 0.f;
					pTerrainContents->FrustumCullInHS = terrain.isEnableFrustumCullInHS == true ? 1.f : 0.f;
					pTerrainContents->DynamicTessFactor = terrain.fDynamicTessFactor;
					pTerrainContents->StaticTessFactor = terrain.fStaticTessFactor;

					pTerrainContents->matModelViewProjection = (terrain.worldMatrix * matViewProjection).Transpose();
					pTerrainContents->matPrevModelViewProjectionMatrix = (terrain.prevWorldMatrix * matPrevViewProjection).Transpose();
					pTerrainContents->worldMatrix = terrain.worldMatrix.Transpose();

					pTerrainContents->CameraPosition = f3CameraPosition;
					pTerrainContents->padding0 = 0.f;

					pTerrainContents->CameraDirection = f3CameraDirection;
					pTerrainContents->padding1 = 0.f;

					pTerrainContents->f2PatchSize = terrain.f2PatchSize;
					pTerrainContents->f2HeightFieldSize = terrain.f2HeightFieldSize;

					pCB_TerrainContents->Unmap(pDeviceContext);
				}
			}

			class TerrainRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(const RenderElement& element, Group group, const math::Matrix& matPrevViewProjection);
				void AllCleanup();
				void Cleanup();

			public:
				void PushJob(const RenderJobTerrain& job) { m_terrains[UpdateThread()].emplace_back(job); }

			private:
				void Draw(ID3D11DeviceContext* pDeviceContext, const math::Matrix& viewProjectionMatrix, const math::Matrix& prevViewProjectionMatrix, const math::float3& cameraPosition, const math::float3& cameraDirection, bool isRnederDepth);

			private:
				struct PSO
				{
					ID3D11VertexShader* pVertexShader{ nullptr };
					ID3D11HullShader* pHullShader{ nullptr };
					ID3D11DomainShader* pDomainShader{ nullptr };
					ID3D11PixelShader* pPixelShader{ nullptr };
					ID3D11InputLayout* pInputLayout{ nullptr };
				};
				void CreatePSO(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType);

			private:
				std::array<PSO, shader::ePS_Count> m_pso;

				ConstantBuffer<shader::TerrainContents> m_terrainContents;

				std::vector<RenderJobTerrain> m_terrains[2];
			};

			TerrainRenderer::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\Terrain\\Terrain.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Terrain.hlsl");
				}

				CreatePSO(pDevice, pShaderBlob, shaderPath.c_str(), shader::eSolid);
				CreatePSO(pDevice, pShaderBlob, shaderPath.c_str(), shader::eSolid_MotionBlur);
				CreatePSO(pDevice, pShaderBlob, shaderPath.c_str(), shader::eDepth);

				SafeRelease(pShaderBlob);

				m_terrainContents.Create(pDevice, "TerrainContents");
			}

			TerrainRenderer::Impl::~Impl()
			{
				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					SafeRelease(m_pso[i].pVertexShader);
					SafeRelease(m_pso[i].pHullShader);
					SafeRelease(m_pso[i].pDomainShader);
					SafeRelease(m_pso[i].pPixelShader);
					SafeRelease(m_pso[i].pInputLayout);
					SafeRelease(m_pso[i].pVertexShader);
				}
				m_terrainContents.Destroy();
			}

			void TerrainRenderer::Impl::Render(const RenderElement& element, Group group, const math::Matrix& matPrevViewProjection)
			{
				if (m_terrains[RenderThread()].empty())
					return;

				TRACER_EVENT(__FUNCTIONW__);
				DX_PROFILING(TerrainRenderer);

				Device* pDeviceInstance = Device::GetInstance();

				if (group == Group::eShadow)
				{
					LightResourceManager* pLightResourceManager = pDeviceInstance->GetLightResourceManager();
					if (pLightResourceManager->GetShadowCount(ILight::Type::eDirectional) == 0 &&
						pLightResourceManager->GetShadowCount(ILight::Type::ePoint) == 0 &&
						pLightResourceManager->GetShadowCount(ILight::Type::eSpot) == 0)
						return;
				}

				const graphics::Options& graphicsOptions = graphics::RenderOptions();

				Camera* pCamera = element.pCamera;
				const math::Matrix matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();

				ID3D11DeviceContext* pDeviceContext = element.pDeviceContext;
				pDeviceContext->ClearState();

				const math::Viewport& viewport = pDeviceInstance->GetViewport();
				pDeviceContext->RSSetViewports(1, util::Convert(viewport));

				if (graphicsOptions.OnWireframe == true)
				{
					ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eWireframeCullNone);
					pDeviceContext->RSSetState(pRasterizerState);
				}
				else
				{
					ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCCW);
					pDeviceContext->RSSetState(pRasterizerState);
				}

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(BlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(DepthStencilState::eRead_Write_On);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerState = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipLinearBorder);
				pDeviceContext->HSSetSamplers(shader::eSampler_SamplerLinearBorder, 1, &pSamplerState);
				pDeviceContext->DSSetSamplers(shader::eSampler_SamplerLinearBorder, 1, &pSamplerState);

				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();
				if (group != Group::eShadow)
				{
					shader::PSType emPSType;

					if (RenderOptions().OnMotionBlur == true && RenderOptions().motionBlurConfig.IsVelocityMotionBlur() == true)
					{
						emPSType = shader::eSolid_MotionBlur;

						ID3D11RenderTargetView* pRTV[] =
						{
							pGBuffer->GetRenderTarget(GBufferType::eNormals)->GetRenderTargetView(),
							pGBuffer->GetRenderTarget(GBufferType::eColors)->GetRenderTargetView(),
							pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF)->GetRenderTargetView(),
							pGBuffer->GetRenderTarget(GBufferType::eVelocity)->GetRenderTargetView(),
						};
						pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, pGBuffer->GetDepthStencil()->GetDepthStencilView());
					}
					else
					{
						emPSType = shader::eSolid;

						ID3D11RenderTargetView* pRTV[] =
						{
							pGBuffer->GetRenderTarget(GBufferType::eNormals)->GetRenderTargetView(),
							pGBuffer->GetRenderTarget(GBufferType::eColors)->GetRenderTargetView(),
							pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF)->GetRenderTargetView(),
						};
						pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, pGBuffer->GetDepthStencil()->GetDepthStencilView());
					}

					pSamplerState = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipLinearWrap);
					pDeviceContext->PSSetSamplers(shader::eSampler_SamplerLinearWrap, 1, &pSamplerState);

					pSamplerState = pDeviceInstance->GetSamplerState(SamplerState::eAnisotropicBorder);
					pDeviceContext->PSSetSamplers(shader::eSampler_SamplerAnisotropicBorder, 1, &pSamplerState);

					pDeviceContext->VSSetShader(m_pso[emPSType].pVertexShader, nullptr, 0);
					pDeviceContext->HSSetShader(m_pso[emPSType].pHullShader, nullptr, 0);
					pDeviceContext->DSSetShader(m_pso[emPSType].pDomainShader, nullptr, 0);
					pDeviceContext->PSSetShader(m_pso[emPSType].pPixelShader, nullptr, 0);

					pDeviceContext->IASetInputLayout(m_pso[emPSType].pInputLayout);
					pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

					Draw(pDeviceContext, matViewProj, matPrevViewProjection, pCamera->GetPosition(), pCamera->GetDirection(), false);
				}
				else
				{
					const shader::PSType emPSType = shader::eDepth;

					pDeviceContext->VSSetShader(m_pso[emPSType].pVertexShader, nullptr, 0);
					pDeviceContext->HSSetShader(m_pso[emPSType].pHullShader, nullptr, 0);
					pDeviceContext->DSSetShader(m_pso[emPSType].pDomainShader, nullptr, 0);
					//pDeviceContext->PSSetShader(m_pso[emPSType].pPixelShader, nullptr, 0);

					pDeviceContext->IASetInputLayout(m_pso[emPSType].pInputLayout);
					pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

					LightResourceManager* pLightResourceManager = pDeviceInstance->GetLightResourceManager();
					for (uint32_t i = 0; i < ILight::eCount; ++i)
					{
						const ILight::Type type = static_cast<ILight::Type>(i);

						const size_t lightCount = pLightResourceManager->GetLightCount(type);
						for (uint32_t j = 0; j < lightCount; ++j)
						{
							const LightPtr pLight = pLightResourceManager->GetLight(type, j);
							switch (type)
							{
							case ILight::Type::eDirectional:
							{
								IDirectionalLight* pDirectionalLight = static_cast<IDirectionalLight*>(pLight.get());
								const CascadedShadows& cascadedShadows = pDirectionalLight->GetCascadedShadows();
								const CascadedShadowsConfig& cascadedShadowsConfig = cascadedShadows.GetConfig();

								DepthStencil* pCascadedDepthStencil = pLightResourceManager->GetDepthStencil(pDeviceInstance, pDeviceContext, pDirectionalLight);
								if (pCascadedDepthStencil != nullptr)
								{
									pDeviceContext->OMSetRenderTargets(0, nullptr, pCascadedDepthStencil->GetDepthStencilView());

									ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCCW);
									pDeviceContext->RSSetState(pRasterizerState);

									for (uint32_t cascadeLevel = 0; cascadeLevel < cascadedShadowsConfig.numCascades; ++cascadeLevel)
									{
										pDeviceContext->RSSetViewports(1, util::Convert(cascadedShadows.GetViewport(cascadeLevel)));

										const math::Matrix& viewMatrix = cascadedShadows.GetViewMatrix(cascadeLevel);
										const math::Matrix& projectionMatrix = cascadedShadows.GetProjectionMatrix(cascadeLevel);

										const math::Matrix viewProjectionMatrix = viewMatrix * projectionMatrix;
										Draw(pDeviceContext, viewProjectionMatrix, math::Matrix::Identity, pCamera->GetPosition(), pCamera->GetDirection(), true);
									}
								}
							}
							break;
							case ILight::Type::ePoint:
								break;
							case ILight::Type::eSpot:
							{
							}
							break;
							default:
								continue;
							}
						}
					}
				}
			}

			void TerrainRenderer::Impl::AllCleanup()
			{
				m_terrains[UpdateThread()].clear();
				m_terrains[RenderThread()].clear();
			}

			void TerrainRenderer::Impl::Cleanup()
			{
				m_terrains[RenderThread()].clear();
			}

			void TerrainRenderer::Impl::Draw(ID3D11DeviceContext* pDeviceContext, const math::Matrix& viewProjectionMatrix, const math::Matrix& prevViewProjectionMatrix, const math::float3& cameraPosition, const math::float3& cameraDirection, bool isRnederDepth)
			{
				for (auto& renderJob : m_terrains[RenderThread()])
				{
					shader::SetTerrainContents(pDeviceContext, &m_terrainContents, renderJob, viewProjectionMatrix, prevViewProjectionMatrix, cameraPosition, cameraDirection, isRnederDepth);
					pDeviceContext->HSSetConstantBuffers(shader::eCB_TerrainContents, 1, &m_terrainContents.pBuffer);
					pDeviceContext->DSSetConstantBuffers(shader::eCB_TerrainContents, 1, &m_terrainContents.pBuffer);

					VertexBuffer* pVertexBuffer = static_cast<VertexBuffer*>(renderJob.pVertexBuffer.get());
					assert(pVertexBuffer != nullptr);

					ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
					const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
					const uint32_t nOffsets[] = { 0, };
					pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);
					pDeviceContext->Draw(pVertexBuffer->GetVertexCount(), 0);
				}
			}

			void TerrainRenderer::Impl::CreatePSO(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType)
			{
				std::vector<D3D_SHADER_MACRO> macros;
				macros.emplace_back(D3D_SHADER_MACRO{ "DX11", "1" });
				if (emPSType == shader::eSolid_MotionBlur)
				{
					macros.emplace_back(D3D_SHADER_MACRO{ "USE_MOTION_BLUR", "1" });
				}
				else if (emPSType == shader::eDepth)
				{
					macros.emplace_back(D3D_SHADER_MACRO{ "DEPTH", "1" });
				}
				macros.emplace_back(D3D_SHADER_MACRO{ nullptr, nullptr });

				{
					const D3D11_INPUT_ELEMENT_DESC* pInputElements = nullptr;
					size_t nElementCount = 0;

					util::GetInputElementDesc(VertexPos4::Format(), &pInputElements, &nElementCount);

					if (util::CreateVertexShader(pDevice, pShaderBlob, macros.data(), shaderPath, "PassThroughVS", shader::VS_CompileVersion, &m_pso[emPSType].pVertexShader, pInputElements, nElementCount, &m_pso[emPSType].pInputLayout, "Terrain_VS") == false)
					{
						throw_line("failed to create vertex shader");
					}
				}

				{
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros.data(), shaderPath, "HeightFieldPatchPS", shader::PS_CompileVersion, &m_pso[emPSType].pPixelShader, "HeightFieldPatchPS") == false)
					{
						throw_line("failed to create pixel shader");
					}
				}

				{
					if (util::CreateHullShader(pDevice, pShaderBlob, macros.data(), shaderPath, "PatchHS", shader::HS_CompileVersion, &m_pso[emPSType].pHullShader, "Terrain_HS") == false)
					{
						throw_line("failed to create hull shader");
					}
				}

				{
					if (util::CreateDomainShader(pDevice, pShaderBlob, macros.data(), shaderPath, "HeightFieldPatchDS", shader::DS_CompileVersion, &m_pso[emPSType].pDomainShader, "Terrain_DS") == false)
					{
						throw_line("failed to create Domain shader");
					}
				}

			}

			TerrainRenderer::TerrainRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			TerrainRenderer::~TerrainRenderer()
			{
			}

			void TerrainRenderer::Render(const RenderElement& element, Group group, const math::Matrix& matPrevViewProjection)
			{
				m_pImpl->Render(element, group, matPrevViewProjection);
			}

			void TerrainRenderer::AllCleanup()
			{
				m_pImpl->AllCleanup();
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