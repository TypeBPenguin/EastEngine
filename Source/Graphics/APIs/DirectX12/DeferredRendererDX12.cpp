#include "stdafx.h"
#include "DeferredRendererDX12.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"
#include "Graphics/Interface/LightManager.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "GBufferDX12.h"
#include "LightResourceManagerDX12.h"

#include "DescriptorHeapDX12.h"
#include "RenderTargetDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				struct CommonContents
				{
					math::float3 cameraPosition;
					float farClip{ 0.f };

					uint32_t nTexDiffuseHDRIndex{ 0 };
					uint32_t nTexSpecularHDRIndex{ 0 };
					uint32_t nTexSpecularBRDFIndex{ 0 };
					float padding;
				};

				struct DirectionalLightBuffer
				{
					uint32_t directionalLightCount{ 0 };
					math::float3 padding;

					std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
				};

				struct PointLightBuffer
				{
					uint32_t pointLightCount{ 0 };
					math::float3 padding;

					std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
				};

				struct SpotLightBuffer
				{
					uint32_t spotLightCount{ 0 };
					math::float3 padding;

					std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};
				};

				struct ShadowBuffer
				{
					uint32_t cascadeShadowCount{ 0 };
					math::float3 padding;

					math::uint4 cascadeShadowIndex[ILight::eMaxDirectionalLightCount]{};
					std::array<CascadedShadowData, ILight::eMaxDirectionalLightCount> cascadedShadow{};
				};

				struct DeferredContents
				{
					math::Matrix matInvView;
					math::Matrix matInvProj;

					uint32_t nTexDepthIndex{ 0 };
					uint32_t nTexNormalIndex{ 0 };
					uint32_t nTexAlbedoSpecularIndex{ 0 };
					uint32_t nTexDisneyBRDFIndex{ 0 };
				};

				enum CBSlot
				{
					eCB_DeferredContents = 0,
					eCB_CommonContents = 5,
					eCB_DirectionalLightBuffer = 6,
					eCB_PointLightBuffer = 7,
					eCB_SpotLightBuffer = 8,
					eCB_ShadowBuffer = 9,
				};
			}

			class DeferredRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);

				void Render(const RenderElement& renderElement);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_DeferrecContentsCB,
					eRP_CommonContentsCB,
					eRP_DirectionalLightCB,
					eRP_PointLIghtCB,
					eRP_SpotLightCB,
					eRP_ShadowCB,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath);
				void CreateBundles();

			private:
				PSOCache m_psoCache;

				std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> m_pBundles{ nullptr };

				ConstantBuffer<shader::DeferredContents> m_deferredContentsBuffer;
				ConstantBuffer<shader::CommonContents> m_commonContentsBuffer;
				ConstantBuffer<shader::DirectionalLightBuffer> m_directionalLightBuffer;
				ConstantBuffer<shader::PointLightBuffer> m_pointLightBuffer;
				ConstantBuffer<shader::SpotLightBuffer> m_spotLightBuffer;
				ConstantBuffer<shader::ShadowBuffer> m_shadowBuffer;
			};

			DeferredRenderer::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\Model\\Deferred.hlsl");

				ID3DBlob* pShaderBlob = nullptr;
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Deferred.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
				CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str());

				m_deferredContentsBuffer.Create(pDevice, 1, "DeferredContent");
				m_commonContentsBuffer.Create(pDevice, 1, "CommonContents");
				m_directionalLightBuffer.Create(pDevice, 1, "DirectionalLightBuffer");
				m_pointLightBuffer.Create(pDevice, 1, "PointLightBuffer");
				m_spotLightBuffer.Create(pDevice, 1, "SpotLightBuffer");
				m_shadowBuffer.Create(pDevice, 1, "ShadowBuffer");

				SafeRelease(pShaderBlob);

				CreateBundles();
			}

			DeferredRenderer::Impl::~Impl()
			{
				m_deferredContentsBuffer.Destroy();
				m_commonContentsBuffer.Destroy();
				m_directionalLightBuffer.Destroy();
				m_pointLightBuffer.Destroy();
				m_spotLightBuffer.Destroy();
				m_shadowBuffer.Destroy();

				m_psoCache.Destroy();

				for (auto& pBundle : m_pBundles)
				{
					util::ReleaseResource(pBundle);
					pBundle = nullptr;
				}
			}

			void DeferredRenderer::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				CreatePipelineState(pDevice, nullptr, nullptr);
				CreateBundles();
			}

			void DeferredRenderer::Impl::Render(const RenderElement& renderElement)
			{
				TRACER_EVENT(__FUNCTIONW__);
				Device* pDeviceInstance = Device::GetInstance();
				LightResourceManager* pLightResourceManager = pDeviceInstance->GetLightResourceManager();

				Camera* pCamera = renderElement.pCamera;

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				GBuffer* pGBuffer = pDeviceInstance->GetGBuffer(frameIndex);
				const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				const math::Viewport& viewport= pDeviceInstance->GetViewport();
				const math::Rect& scissorRect = pDeviceInstance->GetScissorRect();

				{
					shader::DeferredContents* pDeferredContents = m_deferredContentsBuffer.Cast(frameIndex);
					pDeferredContents->matInvView = pCamera->GetViewMatrix().Invert().Transpose();
					pDeferredContents->matInvProj = pCamera->GetProjectionMatrix().Invert().Transpose();

					pDeferredContents->nTexDepthIndex = pGBuffer->GetDepthStencil()->GetTexture()->GetDescriptorIndex();
					pDeferredContents->nTexNormalIndex = pGBuffer->GetRenderTarget(GBufferType::eNormals)->GetTexture()->GetDescriptorIndex();
					pDeferredContents->nTexAlbedoSpecularIndex = pGBuffer->GetRenderTarget(GBufferType::eColors)->GetTexture()->GetDescriptorIndex();
					pDeferredContents->nTexDisneyBRDFIndex = pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF)->GetTexture()->GetDescriptorIndex();
				}

				{
					shader::CommonContents* pCommonContents = m_commonContentsBuffer.Cast(frameIndex);
					{
						pCommonContents->cameraPosition = pCamera->GetPosition();

						Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR().get());
						pCommonContents->nTexDiffuseHDRIndex = pDiffuseHDR->GetDescriptorIndex();

						Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR().get());
						pCommonContents->nTexSpecularHDRIndex = pSpecularHDR->GetDescriptorIndex();

						Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF().get());
						pCommonContents->nTexSpecularBRDFIndex = pSpecularBRDF->GetDescriptorIndex();
					}

					shader::DirectionalLightBuffer* pDirectionalLightBuffer = m_directionalLightBuffer.Cast(frameIndex);
					{
						const DirectionalLightData* pDirectionalLightData = nullptr;
						pLightResourceManager->GetDirectionalLightRenderData(&pDirectionalLightData, &pDirectionalLightBuffer->directionalLightCount);
						memory::Copy(pDirectionalLightBuffer->lightDirectional, pDirectionalLightData, sizeof(DirectionalLightData) * pDirectionalLightBuffer->directionalLightCount);
					}

					shader::PointLightBuffer* pPointLightBuffer = m_pointLightBuffer.Cast(frameIndex);
					{
						const PointLightData* pPointLightData = nullptr;
						pLightResourceManager->GetPointLightRenderData(&pPointLightData, &pPointLightBuffer->pointLightCount);
						memory::Copy(pPointLightBuffer->lightPoint, pPointLightData, sizeof(PointLightData) * pPointLightBuffer->pointLightCount);
					}

					shader::SpotLightBuffer* pSpotLightBuffer = m_spotLightBuffer.Cast(frameIndex);
					{
						const SpotLightData* pSpotLightData = nullptr;
						pLightResourceManager->GetSpotLightRenderData(&pSpotLightData, &pSpotLightBuffer->spotLightCount);
						memory::Copy(pSpotLightBuffer->lightSpot, pSpotLightData, sizeof(SpotLightData) * pSpotLightBuffer->spotLightCount);
					}

					shader::ShadowBuffer* pShadowBuffer = m_shadowBuffer.Cast(frameIndex);
					{
						*pShadowBuffer = {};

						const size_t lightCount = pLightResourceManager->GetLightCount(ILight::Type::eDirectional);
						for (size_t i = 0; i < lightCount; ++i)
						{
							DirectionalLightPtr pDirectionalLight = std::static_pointer_cast<IDirectionalLight>(pLightResourceManager->GetLight(ILight::Type::eDirectional, i));
							if (pDirectionalLight != nullptr)
							{
								DepthStencil* pDepthStencil = static_cast<DepthStencil*>(pDirectionalLight->GetDepthMapResource());
								if (pDepthStencil != nullptr)
								{
									const CascadedShadows& cascadedShadows = pDirectionalLight->GetRenderCascadedShadows();

									const uint32_t index = pShadowBuffer->cascadeShadowCount;
									pShadowBuffer->cascadedShadow[index] = cascadedShadows.GetRenderData();
									pShadowBuffer->cascadeShadowIndex[index].x = pDepthStencil->GetTexture()->GetDescriptorIndex();

									++pShadowBuffer->cascadeShadowCount;
								}
							}
						}
					}
				}

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				const size_t lightCount = pLightResourceManager->GetLightCount(ILight::Type::eDirectional);
				for (size_t i = 0; i < lightCount; ++i)
				{
					DirectionalLightPtr pDirectionalLight = std::static_pointer_cast<IDirectionalLight>(pLightResourceManager->GetLight(ILight::Type::eDirectional, i));
					if (pDirectionalLight != nullptr)
					{
						DepthStencil* pDepthStencil = static_cast<DepthStencil*>(pDirectionalLight->GetDepthMapResource());
						if (pDepthStencil != nullptr)
						{
							util::ChangeResourceState(pCommandList, pDepthStencil, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
						}
					}
				}

				util::ChangeResourceState(pCommandList, pGBuffer->GetRenderTarget(GBufferType::eNormals), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				util::ChangeResourceState(pCommandList, pGBuffer->GetRenderTarget(GBufferType::eColors), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				util::ChangeResourceState(pCommandList, pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				util::ChangeResourceState(pCommandList, pGBuffer->GetDepthStencil(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

				pCommandList->RSSetViewports(1, util::Convert(viewport));
				pCommandList->RSSetScissorRects(1, &scissorRect);
				pCommandList->OMSetRenderTargets(renderElement.rtvCount, renderElement.rtvHandles, FALSE, renderElement.GetDSVHandle());

				pCommandList->SetPipelineState(m_psoCache.pPipelineState);
				pCommandList->SetGraphicsRootSignature(m_psoCache.pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle());
				pCommandList->SetGraphicsRootConstantBufferView(eRP_DeferrecContentsCB, m_deferredContentsBuffer.GPUAddress(frameIndex));
				pCommandList->SetGraphicsRootConstantBufferView(eRP_CommonContentsCB, m_commonContentsBuffer.GPUAddress(frameIndex));
				pCommandList->SetGraphicsRootConstantBufferView(eRP_DirectionalLightCB, m_directionalLightBuffer.GPUAddress(frameIndex));
				pCommandList->SetGraphicsRootConstantBufferView(eRP_PointLIghtCB, m_pointLightBuffer.GPUAddress(frameIndex));
				pCommandList->SetGraphicsRootConstantBufferView(eRP_SpotLightCB, m_spotLightBuffer.GPUAddress(frameIndex));
				pCommandList->SetGraphicsRootConstantBufferView(eRP_ShadowCB, m_shadowBuffer.GPUAddress(frameIndex));

				pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				pCommandList->IASetVertexBuffers(0, 0, nullptr);
				pCommandList->IASetIndexBuffer(nullptr);

				pCommandList->DrawInstanced(4, 1, 0, 0);

				//pCommandList->ExecuteBundle(m_pBundles[frameIndex]);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			ID3D12RootSignature* DeferredRenderer::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> rootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = rootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& deferredContentsParameter = rootParameters.emplace_back();
				deferredContentsParameter.InitAsConstantBufferView(shader::eCB_DeferredContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& commonContentsParameter = rootParameters.emplace_back();
				commonContentsParameter.InitAsConstantBufferView(shader::eCB_CommonContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& directionalLightBuffer = rootParameters.emplace_back();
				directionalLightBuffer.InitAsConstantBufferView(shader::eCB_DirectionalLightBuffer, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& pointLightBuffer = rootParameters.emplace_back();
				pointLightBuffer.InitAsConstantBufferView(shader::eCB_PointLightBuffer, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& spotLightBuffer = rootParameters.emplace_back();
				spotLightBuffer.InitAsConstantBufferView(shader::eCB_SpotLightBuffer, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& shadowBuffer = rootParameters.emplace_back();
				shadowBuffer.InitAsConstantBufferView(shader::eCB_ShadowBuffer, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				D3D12_STATIC_SAMPLER_DESC shadowSamplerDesc{};
				shadowSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
				shadowSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
				shadowSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
				shadowSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
				shadowSamplerDesc.MipLODBias = 0.f;
				shadowSamplerDesc.MaxAnisotropy = 0;
				shadowSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
				shadowSamplerDesc.MinLOD = 0.f;
				shadowSamplerDesc.MaxLOD = 0.f;
				shadowSamplerDesc.ShaderRegister = 2;
				shadowSamplerDesc.RegisterSpace = 100;
				shadowSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				shadowSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(SamplerState::eMinMagMipPointClamp, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					util::GetStaticSamplerDesc(SamplerState::eMinMagMipLinearClamp, 1, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					shadowSamplerDesc,
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(rootParameters.size()), rootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void DeferredRenderer::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath)
			{
				if (pShaderBlob != nullptr)
				{
					const D3D_SHADER_MACRO macros[] =
					{
						{ "DX12", "1" },
						{ nullptr, nullptr },
					};

					if (m_psoCache.pVSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, "VS", shader::VS_CompileVersion, &m_psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (m_psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, "PS", shader::PS_CompileVersion, &m_psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				if (m_psoCache.pRootSignature == nullptr)
				{
					m_psoCache.pRootSignature = CreateRootSignature(pDevice);
					m_psoCache.pRootSignature->SetName(L"DeferredRenderer");
				}

				if (m_psoCache.pPipelineState != nullptr)
				{
					util::ReleaseResource(m_psoCache.pPipelineState);
					m_psoCache.pPipelineState = nullptr;
				}

				D3D12_SHADER_BYTECODE vertexShaderBytecode{};
				vertexShaderBytecode.BytecodeLength = m_psoCache.pVSBlob->GetBufferSize();
				vertexShaderBytecode.pShaderBytecode = m_psoCache.pVSBlob->GetBufferPointer();

				D3D12_SHADER_BYTECODE pixelShaderBytecode{};
				pixelShaderBytecode.BytecodeLength = m_psoCache.pPSBlob->GetBufferSize();
				pixelShaderBytecode.pShaderBytecode = m_psoCache.pPSBlob->GetBufferPointer();

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.pRootSignature = m_psoCache.pRootSignature;
				psoDesc.VS = vertexShaderBytecode;
				psoDesc.PS = pixelShaderBytecode;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.SampleDesc = sampleDesc;
				psoDesc.SampleMask = 0xffffffff;
				psoDesc.RasterizerState = util::GetRasterizerDesc(RasterizerState::eSolidCullNone);
				psoDesc.BlendState = util::GetBlendDesc(BlendState::eOff);
				psoDesc.NumRenderTargets = 1;

				if (RenderOptions().OnHDR == true)
				{
					psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}
				else
				{
					D3D12_RESOURCE_DESC desc = Device::GetInstance()->GetSwapChainRenderTarget(0)->GetDesc();
					psoDesc.RTVFormats[0] = desc.Format;
				}

				psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(DepthStencilState::eRead_Write_Off);

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoCache.pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				m_psoCache.pPipelineState->SetName(L"DeferredRenderer");
			}

			void DeferredRenderer::Impl::CreateBundles()
			{
				DescriptorHeap* pSRVDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					if (m_pBundles[i] != nullptr)
					{
						util::ReleaseResource(m_pBundles[i]);
						m_pBundles[i] = nullptr;
					}

					m_pBundles[i] = Device::GetInstance()->CreateBundle(m_psoCache.pPipelineState);

					m_pBundles[i]->SetGraphicsRootSignature(m_psoCache.pRootSignature);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pSRVDescriptorHeap->GetHeap(),
					};
					m_pBundles[i]->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					m_pBundles[i]->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle());
					m_pBundles[i]->SetGraphicsRootConstantBufferView(eRP_DeferrecContentsCB, m_deferredContentsBuffer.GPUAddress(i));
					m_pBundles[i]->SetGraphicsRootConstantBufferView(eRP_CommonContentsCB, m_commonContentsBuffer.GPUAddress(i));
					m_pBundles[i]->SetGraphicsRootConstantBufferView(eRP_DirectionalLightCB, m_directionalLightBuffer.GPUAddress(i));
					m_pBundles[i]->SetGraphicsRootConstantBufferView(eRP_PointLIghtCB, m_pointLightBuffer.GPUAddress(i));
					m_pBundles[i]->SetGraphicsRootConstantBufferView(eRP_SpotLightCB, m_spotLightBuffer.GPUAddress(i));
					m_pBundles[i]->SetGraphicsRootConstantBufferView(eRP_ShadowCB, m_shadowBuffer.GPUAddress(i));

					m_pBundles[i]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					m_pBundles[i]->IASetVertexBuffers(0, 0, nullptr);
					m_pBundles[i]->IASetIndexBuffer(nullptr);

					m_pBundles[i]->DrawInstanced(4, 1, 0, 0);

					HRESULT hr = m_pBundles[i]->Close();
					if (FAILED(hr))
					{
						throw_line("failed to close bundle");
					}
				}
			}

			DeferredRenderer::DeferredRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			DeferredRenderer::~DeferredRenderer()
			{
			}

			void DeferredRenderer::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void DeferredRenderer::Render(const RenderElement& renderElement)
			{
				m_pImpl->Render(renderElement);
			}
		}
	}
}