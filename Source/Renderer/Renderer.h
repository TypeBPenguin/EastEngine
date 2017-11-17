#pragma once

#include "Instancing.h"
#include "RenderSubsets.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IRenderer
		{
		public:
			IRenderer();
			virtual ~IRenderer() = 0;

		public:
			virtual bool Init(const Math::Viewport& viewport) = 0;

			virtual void AddRender(const RenderSubsetVertex& renderSubset) {}
			virtual void AddRender(const RenderSubsetLine& renderSubset) {}
			virtual void AddRender(const RenderSubsetLineSegment& renderSubset) {}
			virtual void AddRender(const RenderSubsetStatic& renderSubset) {}
			virtual void AddRender(const RenderSubsetSkinned& renderSubset) {}
			virtual void AddRender(const RenderSubsetTerrain& renderSubset) {}
			virtual void AddRender(const RenderSubsetSky& renderSubset) {}
			virtual void AddRender(const RenderSubsetSkyEffect& renderSubset) {}
			virtual void AddRender(const RenderSubsetSkyCloud& renderSubset) {}
			virtual void AddRender(const RenderSubsetParticleEmitter& renderSubset) {}
			virtual void AddRender(const RenderSubsetParticleDecal& renderSubset) {}
			virtual void AddRender(const RenderSubsetUIText& renderSubset) {}
			virtual void AddRender(const RenderSubsetUISprite& renderSubset) {}
			virtual void AddRender(const RenderSubsetUIPanel& renderSubset) {}

			virtual void Render(uint32_t nRenderGroupFlag) = 0;
			virtual void Flush() = 0;
		};
	}
}