#include "stdafx.h"
#include "LightResourceManagerDX12.h"

#include "Graphics/Interface/LightManager.h"

#include "DeviceDX12.h"
#include "UtilDX12.h"
#include "DepthStencilDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			LightResourceManager::LightResourceManager(LightManager* pLightManager)
				: m_pLightManager(pLightManager)
			{
			}

			LightResourceManager::~LightResourceManager()
			{
				Cleanup();
			}

			LightPtr LightResourceManager::GetLight(ILight::Type emType, size_t index) const
			{
				return m_pLightManager->GetLight(emType, index);
			}

			size_t LightResourceManager::GetLightCount(ILight::Type emType) const
			{
				return m_pLightManager->GetLightCount(emType);
			}

			uint32_t LightResourceManager::GetLightCountInView(ILight::Type emType) const
			{
				return m_pLightManager->GetLightCountInView(emType);
			}

			void LightResourceManager::GetDirectionalLightRenderData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const
			{
				return m_pLightManager->GetDirectionalLightRenderData(ppDirectionalLightData, pSize);
			}

			void LightResourceManager::GetPointLightRenderData(const PointLightData** ppPointLightData, uint32_t* pSize) const
			{
				return m_pLightManager->GetPointLightRenderData(ppPointLightData, pSize);
			}

			void LightResourceManager::GetSpotLightRenderData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const
			{
				return m_pLightManager->GetSpotLightRenderData(ppSpotLightData, pSize);
			}

			uint32_t LightResourceManager::GetShadowCount(ILight::Type type) const
			{
				return m_pLightManager->GetShadowCount(type);
			}

			void LightResourceManager::Cleanup()
			{
				for (uint32_t i = 0; i < ILight::eCount; ++i)
				{
					const ILight::Type type = static_cast<ILight::Type>(i);

					const size_t lightCount = m_pLightManager->GetLightCount(type);
					for (uint32_t j = 0; j < lightCount; ++j)
					{
						const LightPtr pLight = m_pLightManager->GetLight(type, j);
						switch (type)
						{
						case ILight::Type::eDirectional:
						{
							IDirectionalLight* pDirectionalLight = static_cast<IDirectionalLight*>(pLight.get());
							pDirectionalLight->SetDepthMapResource(nullptr);
						}
						break;
						case ILight::Type::ePoint:
						{
						}
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

				Device* pDeviceInstance = Device::GetInstance();
				for (auto& pDepthStencil : m_depthStencil)
				{
					pDeviceInstance->ReleaseDepthStencil(&pDepthStencil);
				}
				m_depthStencil.clear();
			}

			DepthStencil* LightResourceManager::GetDepthStencil(Device* pDeviceInstance, IDirectionalLight* pDirectionalLight)
			{
				if (pDirectionalLight == nullptr)
					return nullptr;

				DepthStencil* pDepthStencil = static_cast<DepthStencil*>(pDirectionalLight->GetDepthMapResource());
				if (pDepthStencil != nullptr)
					return pDepthStencil;

				const CascadedShadows& cascadedShadows = pDirectionalLight->GetRenderCascadedShadows();
				const CascadedShadowsConfig& cascadedShadowsConfig = cascadedShadows.GetConfig();

				CD3DX12_RESOURCE_DESC depthMapDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, cascadedShadowsConfig.resolution * cascadedShadowsConfig.numCascades, cascadedShadowsConfig.resolution);
				depthMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

				pDepthStencil = pDeviceInstance->GetDepthStencil(&depthMapDesc);
				if (pDepthStencil != nullptr)
				{
					if (pDepthStencil->GetResourceState() != D3D12_RESOURCE_STATE_DEPTH_WRITE)
					{
						ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
						pDeviceInstance->ResetCommandList(0, nullptr);

						util::ChangeResourceState(pCommandList, pDepthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);

						pDepthStencil->Clear(pCommandList);

						pCommandList->Close();
						pDeviceInstance->ExecuteCommandList(pCommandList);
					}

					pDirectionalLight->SetDepthMapResource(pDepthStencil);
					m_depthStencil.emplace_back(pDepthStencil);
				}
				return pDepthStencil;
			}
		}
	}
}