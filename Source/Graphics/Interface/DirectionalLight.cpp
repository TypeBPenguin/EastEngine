#include "stdafx.h"
#include "DirectionalLight.h"

#include "LightManager.h"

namespace est
{
	namespace graphics
	{
		DirectionalLight::DirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData)
			: m_name(name)
			, m_isEnableShadow(isEnableShadow)
			, m_lightData(lightData)
		{
			SetState(IResource::eComplete);
		}

		DirectionalLight::~DirectionalLight()
		{
			if (m_isEnableShadow == true)
			{
				LightManager::GetInstance()->DecreaseShadowCount(ILight::Type::eDirectional);
			}
		}

		void DirectionalLight::Update(float elapsedTime)
		{
			if (m_isEnableShadow == true)
			{
				m_cascadedShadows.Update(this);
			}
		}

		void DirectionalLight::Cleanup()
		{
			if (m_isEnableShadow == true)
			{
				m_cascadedShadows.Update(this);
			}
		}

		void DirectionalLight::SetEnableShadow(bool isEnableShadow)
		{
			if (m_isEnableShadow == isEnableShadow)
				return;

			m_isEnableShadow = isEnableShadow;

			if (m_isEnableShadow == true)
			{
				LightManager::GetInstance()->IncreaseShadowCount(ILight::Type::eDirectional);
			}
			else
			{
				LightManager::GetInstance()->DecreaseShadowCount(ILight::Type::eDirectional);
			}
		}
	}
}