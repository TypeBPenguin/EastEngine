#include "stdafx.h"
#include "Light.h"

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

namespace EastEngine
{
	namespace Graphics
	{
		ILight::ILight()
		{
		}

		ILight::~ILight()
		{
		}

		IDirectionalLight* ILight::CreateDirectionalLight(const String::StringID& strName, const Math::Vector3& f3Direction, const Math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity, CascadedShadowsConfig* pCascadeConfig)
		{
			return DirectionalLight::Create(strName, f3Direction, color, fIntensity, fAmbientIntensity, fReflectionIntensity, pCascadeConfig);
		}

		IPointLight* ILight::CreatePointLight(const String::StringID& strName, const Math::Vector3& f3Position, const Math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity, ShadowConfig* pShadowConfig)
		{
			return PointLight::Create(strName, f3Position, color, fIntensity, fAmbientIntensity, fReflectionIntensity, pShadowConfig);
		}

		ISpotLight* ILight::CreateSpotLight(const String::StringID& strName, const Math::Vector3& f3Position, const Math::Vector3& f3Direction, float fAngle, const Math::Color& color, float fIntensity, float fAmbientIntensity, float fReflectionIntensity, ShadowConfig* pShadowConfig)
		{
			return SpotLight::Create(strName, f3Position, f3Direction, fAngle, color, fIntensity, fAmbientIntensity, fReflectionIntensity, pShadowConfig);
		}
	}
}