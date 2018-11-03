#include "stdafx.h"
#include "PointLight.h"

#include "LightManager.h"

namespace StrID
{
	RegisterStringID(NoName);
}

namespace eastengine
{
	namespace graphics
	{
		PointLight::PointLight()
			: m_isEnableShadow(false)
			, m_f3Pos(0.f, 0.f, 0.f)
			, m_fIntensity(1.f)
			, m_fAmbientIntensity(0.f)
			, m_fReflectionIntensity(0.f)
			, m_color(math::Color::White)
		{
		}

		PointLight::~PointLight()
		{
		}

		PointLight* PointLight::Create(const string::StringID& strName, const math::Vector3& f3Position, const math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity)
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
			//if (Config::IsEnable("Shadow"_s) == true && IsEnableShadow() == true && m_pShadowCubeMap != nullptr)
			//{
			//	m_pShadowCubeMap->Update();
			//}
		}
	}
}