#include "stdafx.h"
#include "DeferredRendererDX12.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"
#include "Graphics/Interface/LightManager.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "GBufferDX12.h"

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
					math::float3 f3CameraPos;
					int nEnableShadowCount{ 0 };

					uint32_t nTexDiffuseHDRIndex{ 0 };
					uint32_t nTexSpecularHDRIndex{ 0 };
					uint32_t nTexSpecularBRDFIndex{ 0 };
					uint32_t nTexShadowMapIndex{ 0 };

					uint32_t nDirectionalLightCount{ 0 };
					uint32_t nPointLightCount{ 0 };
					uint32_t nSpotLightCount{ 0 };
					uint32_t padding{ 0 };

					std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
					std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
					std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};
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

				SafeRelease(pShaderBlob);

				CreateBundles();
			}

			DeferredRenderer::Impl::~Impl()
			{
				m_deferredContentsBuffer.Destroy();
				m_commonContentsBuffer.Destroy();

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
				LightManager* pLightManager = LightManager::GetInstance();

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

					pCommonContents->f3CameraPos = pCamera->GetPosition();
					pCommonContents->nEnableShadowCount = 0;

					Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR().get());
					pCommonContents->nTexDiffuseHDRIndex = pDiffuseHDR->GetDescriptorIndex();

					Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR().get());
					pCommonContents->nTexSpecularHDRIndex = pSpecularHDR->GetDescriptorIndex();

					Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF().get());
					pCommonContents->nTexSpecularBRDFIndex = pSpecularBRDF->GetDescriptorIndex();

					const DirectionalLightData* pDirectionalLightData = nullptr;
					pLightManager->GetDirectionalLightRenderData(&pDirectionalLightData, &pCommonContents->nDirectionalLightCount);
					memory::Copy(pCommonContents->lightDirectional.data(), sizeof(pCommonContents->lightDirectional), pDirectionalLightData, sizeof(DirectionalLightData) * pCommonContents->nDirectionalLightCount);

					const PointLightData* pPointLightData = nullptr;
					pLightManager->GetPointLightRenderData(&pPointLightData, &pCommonContents->nPointLightCount);
					memory::Copy(pCommonContents->lightPoint.data(), sizeof(pCommonContents->lightPoint), pPointLightData, sizeof(PointLightData) * pCommonContents->nPointLightCount);

					const SpotLightData* pSpotLightData = nullptr;
					pLightManager->GetSpotLightRenderData(&pSpotLightData, &pCommonContents->nSpotLightCount);
					memory::Copy(pCommonContents->lightSpot.data(), sizeof(pCommonContents->lightSpot), pSpotLightData, sizeof(SpotLightData) * pCommonContents->nSpotLightCount);
				}

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				util::ChangeResourceState(pCommandList, pGBuffer->GetRenderTarget(GBufferType::eNormals), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				util::ChangeResourceState(pCommandList, pGBuffer->GetRenderTarget(GBufferType::eColors), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				util::ChangeResourceState(pCommandList, pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				util::ChangeResourceState(pCommandList, pGBuffer->GetDepthStencil(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

				pCommandList->RSSetViewports(1, util::Convert(viewport));
				pCommandList->RSSetScissorRects(1, &scissorRect);
				pCommandList->OMSetRenderTargets(renderElement.rtvCount, renderElement.rtvHandles, FALSE, renderElement.GetDSVHandle());

				pCommandList->SetGraphicsRootSignature(m_psoCache.pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->ExecuteBundle(m_pBundles[frameIndex]);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			ID3D12RootSignature* DeferredRenderer::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& deferredContentsParameter = vecRootParameters.emplace_back();
				deferredContentsParameter.InitAsConstantBufferView(shader::eCB_DeferredContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& commonContentsParameter = vecRootParameters.emplace_back();
				commonContentsParameter.InitAsConstantBufferView(shader::eCB_CommonContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(SamplerState::eMinMagMipPointClamp, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					util::GetStaticSamplerDesc(SamplerState::eMinMagMipLinearClamp, 1, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
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