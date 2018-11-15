#include "stdafx.h"
#include "Light.h"

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

namespace eastengine
{
	namespace graphics
	{
		ILight::ILight()
		{
		}

		ILight::~ILight()
		{
		}

		IDirectionalLight* ILight::CreateDirectionalLight(const string::StringID& strName, const math::float3& f3Direction, const math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity)
		{
			return DirectionalLight::Create(strName, f3Direction, color, fIntensity, fAmbientIntensity, fReflectionIntensity);
		}

		IPointLight* ILight::CreatePointLight(const string::StringID& strName, const math::float3& f3Position, const math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity)
		{
			return PointLight::Create(strName, f3Position, color, fIntensity, fAmbientIntensity, fReflectionIntensity);
		}

		ISpotLight* ILight::CreateSpotLight(const string::StringID& strName, const math::float3& f3Position, const math::float3& f3Direction, float fAngle, const math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity)
		{
			return SpotLight::Create(strName, f3Position, f3Direction, fAngle, color, fIntensity, fAmbientIntensity, fReflectionIntensity);
		}
	}
}