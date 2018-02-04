#pragma once

#include "Instancing.h"
#include "RenderSubsets.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Camera;

		class IRenderer
		{
		public:
			IRenderer();
			virtual ~IRenderer() = 0;

		public:
			virtual bool Init(const Math::Viewport& viewport) = 0;

			virtual void AddRender(const RenderSubsetVertex&) {}
			virtual void AddRender(const RenderSubsetLine&) {}
			virtual void AddRender(const RenderSubsetLineSegment&) {}
			virtual void AddRender(const RenderSubsetStatic&) {}
			virtual void AddRender(const RenderSubsetSkinned&) {}
			virtual void AddRender(const RenderSubsetTerrain&) {}
			virtual void AddRender(const RenderSubsetSky&) {}
			virtual void AddRender(const RenderSubsetSkybox&) {}
			virtual void AddRender(const RenderSubsetSkyEffect&) {}
			virtual void AddRender(const RenderSubsetSkyCloud&) {}
			virtual void AddRender(const RenderSubsetParticleEmitter&) {}
			virtual void AddRender(const RenderSubsetParticleDecal&) {}
			virtual void AddRender(const RenderSubsetUIText&) {}
			virtual void AddRender(const RenderSubsetUISprite&) {}
			virtual void AddRender(const RenderSubsetUIPanel&) {}

			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) = 0;
			virtual void Flush() = 0;
		};
	}
}