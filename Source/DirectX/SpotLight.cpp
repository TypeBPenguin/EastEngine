#include "stdafx.h"
#include "SpotLight.h"

#include "../CommonLib/Config.h"

#include "LightMgr.h"
#include "Shadow.h"

namespace StrID
{
	RegisterStringID(NoName);
}

namespace EastEngine
{
	namespace Graphics
	{
		SpotLight::SpotLight()
			: m_isEnableShadow(false)
			, m_fAngle(0.f)
			, m_fIntensity(1.f)
			, m_fAmbientIntensity(0.f)
			, m_fReflectionIntensity(0.f)
			, m_color(Math::Color::White)
			, m_pShadowMap(nullptr)
		{
		}

		SpotLight::~SpotLight()
		{
			SafeDelete(m_pShadowMap);
		}

		SpotLight* SpotLight::Create(const String::StringID& strName, const Math::Vector3& f3Position, const Math::Vector3& f3Direction, float fAngle, const Math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity, ShadowConfig* pShadowConfig)
		{
			SpotLight* pLight = new SpotLight;
			pLight->m_strName = strName;
			pLight->m_f3Pos = f3Position;
			pLight->m_f3Direction = f3Direction;
			pLight->m_fAngle = fAngle;
			pLight->m_fIntensity = fIntensity;
			pLight->m_fAmbientIntensity = fAmbientIntensity;
			pLight->m_fReflectionIntensity = fReflectionIntensity;
			pLight->m_color = color;

			if (pShadowConfig != nullptr)
			{
				pLight->m_shadowConfig = *pShadowConfig;

				ShadowMap* pShadowMap = new ShadowMap;
				if (pShadowMap->Init(pLight, &pLight->m_shadowConfig) == true)
				{
					pLight->m_pShadowMap = pShadowMap;
					pLight->SetEnableShadow(true);
				}
				else
				{
					SafeDelete(pShadowMap);
					pLight->SetEnableShadow(false);
				}
			}

			if (LightManager::GetInstance()->AddLight(pLight) == true)
				return pLight;

			SafeDelete(pLight);
			return nullptr;
		}

		void SpotLight::Update(float fElapsedTime)
		{
			if (Config::IsEnableShadow() == true && IsEnableShadow() == true && m_pShadowMap != nullptr)
			{
				m_pShadowMap->Update();
			}
		}
	}
}