#include "stdafx.h"
#include "ShadowCube.h"

namespace EastEngine
{
	namespace Graphics
	{
		ShadowCubeMap::ShadowCubeMap()
		{
		}

		ShadowCubeMap::~ShadowCubeMap()
		{
		}

		bool ShadowCubeMap::Init(ISpotLight* pLight, const ShadowConfig* pShadowConfig)
		{
			return true;
		}

		void ShadowCubeMap::Release()
		{
		}

		void ShadowCubeMap::Update()
		{
		}

		const std::shared_ptr<ITexture>& ShadowCubeMap::GetShadowMap() const
		{
			return nullptr;
		}

		void ShadowCubeMap::RefreshShadowResource()
		{
		}
	}
}