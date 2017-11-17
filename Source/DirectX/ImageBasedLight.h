#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;

		class ImageBasedLight : public IImageBasedLight
		{
		public:
			ImageBasedLight();
			virtual ~ImageBasedLight();

		public:
			bool Init();

		public:
			virtual const std::shared_ptr<ITexture>& GetCubeMap() { return m_pIBLCubeMap; }
			virtual void SetCubeMap(const std::shared_ptr<ITexture>& pIBLCubeMap) { m_pIBLCubeMap = pIBLCubeMap; }

			virtual const std::shared_ptr<ITexture>& GetIrradianceMap() { return m_pIrradianceMap; }
			virtual void SetIrradianceMap(const std::shared_ptr<ITexture>& pIrradianceMap) { m_pIrradianceMap = pIrradianceMap; }

		private:
			std::shared_ptr<ITexture> m_pIBLCubeMap;
			std::shared_ptr<ITexture> m_pIrradianceMap;
		};
	}
}