#include "stdafx.h"
#include "PointLight.h"

#include "LightManager.h"

namespace est
{
	namespace graphics
	{
		PointLight::PointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData)
			: m_name(name)
			, m_isEnableShadow(isEnableShadow)
			, m_lightData(lightData)
		{
			SetState(IResource::eComplete);
		}

		PointLight::~PointLight()
		{
			if (m_isEnableShadow == true)
			{
				LightManager::GetInstance()->DecreaseShadowCount(ILight::Type::ePoint);
			}
		}

		void PointLight::Update(float elapsedTime)
		{
		}

		void PointLight::SetEnableShadow(bool isEnableShadow)
		{
			if (m_isEnableShadow == isEnableShadow)
				return;

			m_isEnableShadow = isEnableShadow;

			if (m_isEnableShadow == true)
			{
				LightManager::GetInstance()->IncreaseShadowCount(ILight::Type::ePoint);
			}
			else
			{
				LightManager::GetInstance()->DecreaseShadowCount(ILight::Type::ePoint);
			}
		}
	}
}