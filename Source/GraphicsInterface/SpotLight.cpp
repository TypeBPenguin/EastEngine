#include "stdafx.h"
#include "SpotLight.h"

namespace eastengine
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
		}

		void SpotLight::Update(float elapsedTime)
		{
		}
	}
}