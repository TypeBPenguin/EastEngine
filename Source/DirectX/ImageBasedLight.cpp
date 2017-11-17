#include "stdafx.h"
#include "ImageBasedLight.h"

namespace EastEngine
{
	namespace Graphics
	{
		ImageBasedLight::ImageBasedLight()
			: m_pIBLCubeMap(nullptr)
			, m_pIrradianceMap(nullptr)
		{
		}

		ImageBasedLight::~ImageBasedLight()
		{
			m_pIBLCubeMap.reset();
			m_pIrradianceMap.reset();
		}

		bool ImageBasedLight::Init()
		{
			return true;
		}
	}
}