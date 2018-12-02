#include "stdafx.h"
#include "DirectionalLight.h"

#include "LightManager.h"

namespace eastengine
{
	namespace graphics
	{
		DirectionalLight::DirectionalLight()
			: m_isEnableShadow(false)
			, m_f3Direction(0.f, 0.f, 1.f)
			, m_fIntensity(1.f)
			, m_fAmbientIntensity(0.f)
			, m_fReflectionIntensity(0.f)
			, m_color(math::Color::White)
		{
		}

		DirectionalLight::~DirectionalLight()
		{
		}

		DirectionalLight* DirectionalLight::Create(const string::StringID& strName, const math::float3& f3Direction, const math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity)
		{
			DirectionalLight* pLight = new DirectionalLight;
			pLight->m_strName = strName;
			pLight->m_f3Direction = f3Direction;
			pLight->m_fIntensity = fIntensity;
			pLight->m_fAmbientIntensity = fAmbientIntensity;
			pLight->m_fReflectionIntensity = fReflectionIntensity;
			pLight->m_color = color;

			//if (pCascadeConfig != nullptr)
			//{
			//	if (pCascadeConfig->nLevel < CascadedShadowsConfig::eMinLevel)
			//	{
			//		pCascadeConfig->nLevel = CascadedShadowsConfig::eMinLevel;
			//	}
			//	else if (pCascadeConfig->nLevel > CascadedShadowsConfig::eMaxLevel)
			//	{
			//		pCascadeConfig->nLevel = CascadedShadowsConfig::eMaxLevel;
			//	}

			//	pLight->m_cascadeConfig = *pCascadeConfig;

			//	//CascadedShadows* pCascadedShadows = new CascadedShadows;
			//	//if (pCascadedShadows->Init(pLight, &pLight->m_cascadeConfig) == true)
			//	//{
			//	//	pLight->m_pCascadedShadows = pCascadedShadows;
			//	//	pLight->SetEnableShadow(true);
			//	//}
			//	//else
			//	//{
			//	//	SafeDelete(pCascadedShadows);
			//	//	pLight->SetEnableShadow(false);
			//	//}
			//}

			if (LightManager::GetInstance()->AddLight(pLight) == true)
				return pLight;

			SafeDelete(pLight);
			return nullptr;
		}

		void DirectionalLight::Update(float elapsedTime)
		{
			//if (Config::IsEnable("Shadow"_s) == true && IsEnableShadow() == true && m_pCascadedShadows != nullptr)
			//{
			//	m_pCascadedShadows->Update();
			//}
		}
	}
}