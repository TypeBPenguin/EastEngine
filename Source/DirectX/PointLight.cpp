#include "stdafx.h"
#include "PointLight.h"

#include "LightMgr.h"

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
		{
		}

		PointLight::~PointLight()
		{
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

			if (LightManager::GetInstance()->AddLight(pLight) == true)
				return pLight;

			SafeDelete(pLight);
			return nullptr;
		}

		void PointLight::Update(float fElapsedTime)
		{
		}
	}
}