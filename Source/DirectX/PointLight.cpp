#include "stdafx.h"
#include "PointLight.h"

#include "CommonLib\Config.h"

#include "LightMgr.h"
#include "ShadowCube.h"

namespace StrID
{
	RegisterStringID(NoName);
}

namespace EastEngine
{
	namespace Graphics
	{
		PointLight::PointLight()
			: m_isEnableShadow(false)
			, m_f3Pos(0.f, 0.f, 0.f)
			, m_fIntensity(1.f)
			, m_fAmbientIntensity(0.f)
			, m_fReflectionIntensity(0.f)
			, m_color(Math::Color::White)
			, m_pShadowCubeMap(nullptr)
		{
		}

		PointLight::~PointLight()
		{
			SafeDelete(m_pShadowCubeMap);
		}

		PointLight* PointLight::Create(const String::StringID& strName, const Math::Vector3& f3Position, const Math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity, ShadowConfig* pShadowConfig)
		{
			PointLight* pLight = new PointLight;
			pLight->m_strName = strName;
			pLight->m_f3Pos = f3Position;
			pLight->m_fIntensity = fIntensity;
			pLight->m_fAmbientIntensity = fAmbientIntensity;
			pLight->m_fReflectionIntensity = fReflectionIntensity;
			pLight->m_color = color;

			if (pShadowConfig != nullptr)
			{
				pLight->m_shadowConfig = *pShadowConfig;

				ShadowCubeMap* pShadowMap = new ShadowCubeMap;
				if (pShadowMap->Init(pLight, &pLight->m_shadowConfig) == true)
				{
					pLight->m_pShadowCubeMap = pShadowMap;
					//pLight->SetEnableShadow(true);

					// 으아아아아아아 감 잡을때까지 false로 봉인
					pLight->SetEnableShadow(false);
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

		void PointLight::Update(float fElapsedTime)
		{
			if (Config::IsEnable("Shadow"_s) == true && IsEnableShadow() == true && m_pShadowCubeMap != nullptr)
			{
				m_pShadowCubeMap->Update();
			}
		}
	}
}