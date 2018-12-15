#include "stdafx.h"
#include "DirectionalLight.h"

namespace eastengine
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
		}

		void DirectionalLight::Update(float elapsedTime)
		{
		}
	}
}