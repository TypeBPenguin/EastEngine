#include "stdafx.h"
#include "SpotLight.h"

#include "LightManager.h"

namespace est
{
	namespace graphics
	{
		SpotLight::SpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData)
			: m_name(name)
			, m_isEnableShadow(isEnableShadow)
			, m_lightData(lightData)
		{
			SetState(IResource::eComplete);
		}

		SpotLight::~SpotLight()
		{
			if (m_isEnableShadow == true)
			{
				LightManager::GetInstance()->DecreaseShadowCount(ILight::Type::eSpot);
			}
		}

		void SpotLight::Update(float elapsedTime)
		{
		}

		void SpotLight::SetEnableShadow(bool isEnableShadow)
		{
			if (m_isEnableShadow == isEnableShadow)
				return;

			m_isEnableShadow = isEnableShadow;

			if (m_isEnableShadow == true)
			{
				LightManager::GetInstance()->IncreaseShadowCount(ILight::Type::eSpot);
			}
			else
			{
				LightManager::GetInstance()->DecreaseShadowCount(ILight::Type::eSpot);
			}
		}
	}
}