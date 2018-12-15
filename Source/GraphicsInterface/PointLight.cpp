#include "stdafx.h"
#include "PointLight.h"

namespace eastengine
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
		}

		void PointLight::Update(float elapsedTime)
		{
		}
	}
}