#include "stdafx.h"
#include "TerrainRendererDX11.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "GBufferDX11.h"

#include "TextureDX11.h"
#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"

namespace eastengine
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
					math::Matrix matWorld;

					math::Vector3 CameraPosition;
					float padding0{ 0.f };

					math::Vector3 CameraDirection;
					float padding1{ 0.f };

					math::Vector2 f2PatchSize;
					math::Vector2 f2HeightFieldSize;
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
					eDepthOnly,

					ePS_Count,
				};

				const char* GetTerrainPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case eSolid:
						return "HeightFieldPatchPS";
					case eDepthOnly:
						return "ColorPS";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetTerrainContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<TerrainContents>* pCB_TerrainContents,
					const RenderJobTerrain& terrain, const math::Matrix& matViewProjection, const math::Vector3& f3CameraPosition, const math::Vector3& f3CameraDirection)
				{
					ID3D11ShaderResourceView* pSRV = static_cast<Texture*>(terrain.pTexHeightField)->GetShaderResourceView();
					pDeviceContext->HSSetShaderResources(shader::eSRV_HeightField, 1, &pSRV);
					pDeviceContext->DSSetShaderResources(shader::eSRV_HeightField, 1, &pSRV);

					pSRV = static_cast<Texture*>(terrain.pTexColorMap)->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(shader::eSRV_Color, 1, &pSRV);

					pSRV = static_cast<Texture*>(terrain.pTexDetailMap)->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(shader::eSRV_Detail, 1, &pSRV);

					pSRV = static_cast<Texture*>(terrain.pTexDetailNormalMap)->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(shader::eSRV_DetailNormal, 1, &pSRV);

					TerrainContents* pTerrainContents = pCB_TerrainContents->Map(pDeviceContext);
					
					pTerrainContents->UseDynamicLOD = terrain.isEnableDynamicLOD == true ? 1.f : 0.f;
					pTerrainContents->FrustumCullInHS = terrain.isEnableFrustumCullInHS == true ? 1.f : 0.f;
					pTerrainContents->DynamicTessFactor = terrain.fDynamicTessFactor;
					pTerrainContents->StaticTessFactor = terrain.fStaticTessFactor;

					pTerrainContents->matModelViewProjection = (terrain.matWorld * matViewProjection).Transpose();
					pTerrainContents->matWorld = terrain.matWorld.Transpose();

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
				void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera);
				void Cleanup();

			public:
				void PushJob(const RenderJobTerrain& job) { m_vecTerrains.emplace_back(job); }

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				ID3D11HullShader* m_pHullShader{ nullptr };
				ID3D11DomainShader* m_pDomainShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShader{ nullptr };
				ID3D11InputLayout* m_pInputLayout{ nullptr };

				ConstantBuffer<shader::TerrainContents> m_terrainContents;

				std::vector<RenderJobTerrain> m_vecTerrains;
			};

			TerrainRenderer::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("Terrain\\Terrain.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(string::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Terrain.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				{
					const D3D11_INPUT_ELEMENT_DESC* pInputElements = nullptr;
					size_t nElementCount = 0;

					util::GetInputElementDesc(VertexPos4::Format(), &pInputElements, &nElementCount);

					if (util::CreateVertexShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "PassThroughVS", shader::VS_CompileVersion, &m_pVertexShader, pInputElements, nElementCount, &m_pInputLayout, "Terrain_VS") == false)
					{
						throw_line("failed to create vertex shader");
					}
				}

				{
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), shader::GetTerrainPSTypeToString(shader::eSolid), shader::PS_CompileVersion, &m_pPixelShader[shader::eSolid], shader::GetTerrainPSTypeToString(shader::eSolid)) == false)
					{
						throw_line("failed to create pixel shader");
					}

					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), shader::GetTerrainPSTypeToString(shader::eDepthOnly), shader::PS_CompileVersion, &m_pPixelShader[shader::eDepthOnly], shader::GetTerrainPSTypeToString(shader::eDepthOnly)) == false)
					{
						throw_line("failed to create pixel shader");
					}
				}

				{
					if (util::CreateHullShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "PatchHS", shader::HS_CompileVersion, &m_pHullShader, "Terrain_HS") == false)
					{
						throw_line("failed to create hull shader");
					}
				}

				{
					if (util::CreateDomainShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "HeightFieldPatchDS", shader::DS_CompileVersion, &m_pDomainShader, "Terrain_DS") == false)
					{
						throw_line("failed to create Domain shader");
					}
				}

				SafeRelease(pShaderBlob);

				m_terrainContents.Create(pDevice, "TerrainContents");
			}

			TerrainRenderer::Impl::~Impl()
			{
				SafeRelease(m_pVertexShader);
				SafeRelease(m_pHullShader);
				SafeRelease(m_pDomainShader);

				for (auto& pPixelShader : m_pPixelShader)
				{
					SafeRelease(pPixelShader);
				}

				SafeRelease(m_pInputLayout);
				SafeRelease(m_pVertexShader);

				m_terrainContents.Destroy();
			}

			void TerrainRenderer::Impl::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera)
			{
				if (m_vecTerrains.empty())
					return;

				DX_PROFILING(TerrainRenderer);

				const graphics::Options& graphicsOptions = graphics::GetOptions();

				Device* pDeviceInstance = Device::GetInstance();

				const math::Matrix matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();

				pDeviceContext->ClearState();

				const D3D11_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				pDeviceContext->RSSetViewports(1, pViewport);

				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();
				ID3D11RenderTargetView* pRTV[] =
				{
					pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetRenderTargetView(),
					pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetRenderTargetView(),
					pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetRenderTargetView(),
				};
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, pGBuffer->GetDepthStencil()->GetDepthStencilView());

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
				pDeviceContext->HSSetShader(m_pHullShader, nullptr, 0);
				pDeviceContext->DSSetShader(m_pDomainShader, nullptr, 0);
				pDeviceContext->PSSetShader(m_pPixelShader[shader::eSolid], nullptr, 0);

				pDeviceContext->IASetInputLayout(m_pInputLayout);
				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

				if (graphicsOptions.OnWireframe == true)
				{
					ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eWireframeCullNone);
					pDeviceContext->RSSetState(pRasterizerState);
				}
				else
				{
					ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
					pDeviceContext->RSSetState(pRasterizerState);
				}

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_On);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerState = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipLinearWrap);
				pDeviceContext->PSSetSamplers(shader::eSampler_SamplerLinearWrap, 1, &pSamplerState);

				pSamplerState = pDeviceInstance->GetSamplerState(EmSamplerState::eAnisotropicBorder);
				pDeviceContext->PSSetSamplers(shader::eSampler_SamplerAnisotropicBorder, 1, &pSamplerState);

				pSamplerState = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipLinearBorder);
				pDeviceContext->HSSetSamplers(shader::eSampler_SamplerLinearBorder, 1, &pSamplerState);
				pDeviceContext->DSSetSamplers(shader::eSampler_SamplerLinearBorder, 1, &pSamplerState);

				for (auto& renderJob : m_vecTerrains)
				{
					shader::SetTerrainContents(pDeviceContext, &m_terrainContents, renderJob, matViewProj, pCamera->GetPosition(), pCamera->GetDir());
					pDeviceContext->HSSetConstantBuffers(shader::eCB_TerrainContents, 1, &m_terrainContents.pBuffer);
					pDeviceContext->DSSetConstantBuffers(shader::eCB_TerrainContents, 1, &m_terrainContents.pBuffer);

					VertexBuffer* pVertexBuffer = static_cast<VertexBuffer*>(renderJob.pVertexBuffer);
					assert(pVertexBuffer != nullptr);

					ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
					const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
					const uint32_t nOffsets[] = { 0, };
					pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);
					pDeviceContext->Draw(pVertexBuffer->GetVertexCount(), 0);
				}
			}

			void TerrainRenderer::Impl::Cleanup()
			{
				m_vecTerrains.clear();
			}

			TerrainRenderer::TerrainRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			TerrainRenderer::~TerrainRenderer()
			{
			}

			void TerrainRenderer::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera)
			{
				m_pImpl->Render(pDevice, pDeviceContext, pCamera);
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