#include "stdafx.h"
#include "LightMgr.h"

#include "D3DInterface.h"
#include "CameraManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		LightManager::LightManager()
			: m_isInit(false)
		{
			m_pLightBuffers.fill(nullptr);
			m_nLightCountInView.fill(0u);
		}

		LightManager::~LightManager()
		{
			Release();
		}

		bool LightManager::Init()
		{
			if (m_isInit == true)
				return true;

			m_pLightBuffers[EmLight::eDirectional] = IStructuredBuffer::Create(m_directionalLightData.data(), m_directionalLightData.size(), sizeof(DirectionalLightData));
			m_pLightBuffers[EmLight::ePoint] = IStructuredBuffer::Create(m_pointLightData.data(), m_pointLightData.size(), sizeof(PointLightData));
			m_pLightBuffers[EmLight::eSpot] = IStructuredBuffer::Create(m_spotLightData.data(), m_spotLightData.size(), sizeof(SpotLightData));

			m_nLightCountInView.fill(0);

			m_isInit = true;

			return true;
		}

		void LightManager::Release()
		{
			if (m_isInit == false)
				return;

			RemoveAll();

			std::for_each(m_pLightBuffers.begin(), m_pLightBuffers.end(), DeleteSTLObject());

			m_isInit = false;
		}

		void LightManager::Update(float fElapsedTime)
		{
			std::for_each(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), [fElapsedTime](IDirectionalLight* pLight)
			{
				pLight->Update(fElapsedTime);
			});

			std::for_each(m_vecSpotLights.begin(), m_vecSpotLights.end(), [fElapsedTime](ISpotLight* pLight)
			{
				pLight->Update(fElapsedTime);
			});

			std::for_each(m_vecPointLights.begin(), m_vecPointLights.end(), [fElapsedTime](IPointLight* pLight)
			{
				pLight->Update(fElapsedTime);
			});

			updateLightBuffer();
		}

		bool LightManager::AddLight(ILight* pLight)
		{
			switch (pLight->GetType())
			{
			case EmLight::Type::eDirectional:
			{
				auto iter = std::find(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), pLight);
				if (iter != m_vecDirectionalLights.end())
					return false;

				m_vecDirectionalLights.emplace_back(static_cast<IDirectionalLight*>(pLight));
			}
			break;
			case EmLight::Type::ePoint:
			{
				auto iter = std::find(m_vecPointLights.begin(), m_vecPointLights.end(), pLight);
				if (iter != m_vecPointLights.end())
					return false;

				m_vecPointLights.emplace_back(static_cast<IPointLight*>(pLight));
			}
			break;
			case EmLight::Type::eSpot:
			{
				auto iter = std::find(m_vecSpotLights.begin(), m_vecSpotLights.end(), pLight);
				if (iter != m_vecSpotLights.end())
					return false;

				m_vecSpotLights.emplace_back(static_cast<ISpotLight*>(pLight));
			}
			break;
			default:
				return false;
			}

			return true;
		}

		void LightManager::Remove(ILight* pLight)
		{
			switch (pLight->GetType())
			{
			case EmLight::Type::eDirectional:
			{
				auto iter = std::find(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), pLight);
				if (iter != m_vecDirectionalLights.end())
				{
					m_vecDirectionalLights.erase(iter);
				}
			}
			break;
			case EmLight::Type::ePoint:
			{
				auto iter = std::find(m_vecPointLights.begin(), m_vecPointLights.end(), pLight);
				if (iter != m_vecPointLights.end())
				{
					m_vecPointLights.erase(iter);
				}
			}
			break;
			case EmLight::Type::eSpot:
			{
				auto iter = std::find(m_vecSpotLights.begin(), m_vecSpotLights.end(), pLight);
				if (iter != m_vecSpotLights.end())
				{
					m_vecSpotLights.erase(iter);
				}
			}
			break;
			default:
				break;
			}
		}

		void LightManager::Remove(EmLight::Type emType, size_t nIndex)
		{
			switch (emType)
			{
			case EmLight::Type::eDirectional:
			{
				if (nIndex >= m_vecDirectionalLights.size())
					return;

				auto iter = m_vecDirectionalLights.begin();
				std::advance(iter, nIndex);

				if (iter != m_vecDirectionalLights.end())
				{
					m_vecDirectionalLights.erase(iter);
				}
			}
			break;
			case EmLight::Type::ePoint:
			{
				if (nIndex >= m_vecPointLights.size())
					return;

				auto iter = m_vecPointLights.begin();
				std::advance(iter, nIndex);

				if (iter != m_vecPointLights.end())
				{
					m_vecPointLights.erase(iter);
				}
			}
			break;
			case EmLight::Type::eSpot:
			{
				if (nIndex >= m_vecSpotLights.size())
					return;

				auto iter = m_vecSpotLights.begin();
				std::advance(iter, nIndex);

				if (iter != m_vecSpotLights.end())
				{
					m_vecSpotLights.erase(iter);
				}
			}
			break;
			default:
				break;
			}
		}

		void LightManager::RemoveAll()
		{
			std::for_each(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), DeleteSTLObject());
			m_vecDirectionalLights.clear();

			std::for_each(m_vecSpotLights.begin(), m_vecSpotLights.end(), DeleteSTLObject());
			m_vecDirectionalLights.clear();

			std::for_each(m_vecPointLights.begin(), m_vecPointLights.end(), DeleteSTLObject());
			m_vecDirectionalLights.clear();
		}

		ILight* LightManager::GetLight(EmLight::Type emType, uint32_t nIndex)
		{
			switch (emType)
			{
			case EmLight::Type::eDirectional:
			{
				if (nIndex >= m_vecDirectionalLights.size())
					return nullptr;

				return m_vecDirectionalLights[nIndex];
			}
			break;
			case EmLight::Type::ePoint:
			{
				if (nIndex >= m_vecPointLights.size())
					return nullptr;

				return m_vecPointLights[nIndex];
			}
			break;
			case EmLight::Type::eSpot:
			{
				if (nIndex >= m_vecSpotLights.size())
					return nullptr;

				return m_vecSpotLights[nIndex];
			}
			break;
			default:
				return nullptr;
			}
		}

		size_t LightManager::GetLightCount(EmLight::Type emType)
		{
			switch (emType)
			{
			case EmLight::Type::eDirectional:
				return m_vecDirectionalLights.size();
			case EmLight::Type::ePoint:
				return m_vecPointLights.size();
			case EmLight::Type::eSpot:
				return m_vecSpotLights.size();
			default:
				return 0;
			}
		}

		void LightManager::updateLightBuffer()
		{
			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
				return;

			//const Collision::Frustum& frustum = pCamera->GetFrustum();
			m_nLightCountInView.fill(0);

			std::for_each(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), [&](IDirectionalLight* pLight)
			{
				uint32_t& nLightIndex = m_nLightCountInView[EmLight::eDirectional];
				if (nLightIndex < EmLight::eMaxDirectionalLightCount)
				{
					m_directionalLightData[nLightIndex].Set(pLight->GetColor(), pLight->GetDirection(), pLight->GetIntensity(), pLight->GetAmbientIntensity(), pLight->GetReflectionIntensity());
					++nLightIndex;
				}
			});

			std::for_each(m_vecPointLights.begin(), m_vecPointLights.end(), [&](IPointLight* pLight)
			{
				Collision::Sphere sphere(pLight->GetPosition(), Math::PI * pLight->GetIntensity());

				// 이걸 보거든, 방향 및 거리 계산을 계산해서 프러스텀 안에 들어갈때 버퍼에 추가하도록 바꾸시오.
				//if (frustum.Contains(sphere) == Collision::EmContainment::eContains)
				{
					uint32_t& nLightIndex = m_nLightCountInView[EmLight::ePoint];
					if (nLightIndex < EmLight::eMaxPointLightCount)
					{
						m_pointLightData[nLightIndex].Set(pLight->GetColor(), pLight->GetPosition(), pLight->GetIntensity(), pLight->GetAmbientIntensity(), pLight->GetReflectionIntensity());
						++nLightIndex;
					}
				}
			});

			std::for_each(m_vecSpotLights.begin(), m_vecSpotLights.end(), [&](ISpotLight* pLight)
			{
				// 이걸 보거든, 방향 및 거리 계산을 계산해서 프러스텀 안에 들어갈때 버퍼에 추가하도록 바꾸시오.
				if (true)
				{
					uint32_t& nLightIndex = m_nLightCountInView[EmLight::eSpot];
					if (nLightIndex < EmLight::eMaxSpotLightCount)
					{
						m_spotLightData[nLightIndex].Set(pLight->GetColor(), pLight->GetPosition(), pLight->GetDirection(), pLight->GetIntensity(), pLight->GetAmbientIntensity(), pLight->GetReflectionIntensity(), pLight->GetAngle());
						++nLightIndex;
					}
				}
			});

			m_pLightBuffers[EmLight::eDirectional]->UpdateSubresource(0, m_directionalLightData.data(), m_nLightCountInView[EmLight::eDirectional]);
			m_pLightBuffers[EmLight::ePoint]->UpdateSubresource(0, m_pointLightData.data(), m_nLightCountInView[EmLight::ePoint]);
			m_pLightBuffers[EmLight::eSpot]->UpdateSubresource(0, m_spotLightData.data(), m_nLightCountInView[EmLight::eSpot]);
		}
	}
}