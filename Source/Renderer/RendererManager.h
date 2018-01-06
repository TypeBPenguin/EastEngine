#pragma once

#include "CommonLib/Singleton.h"

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTech;

		namespace EmRenderer
		{
			enum Type
			{
				ePreProcessing = 0,
				eSky,
				eTerrain,
				eModel,
				eDeferred,
				eWater,
				eVertex,
				ePostProcessing,
				eParticle,
				eUI,

				TypeCount,
			};
		};

		class RendererManager : public Singleton<RendererManager>
		{
			friend Singleton<RendererManager>;
		private:
			RendererManager();
			virtual ~RendererManager();

		public:
			bool Init(const Math::Viewport& viewport);
			void Release();

			void Render();
			void Flush();

		public:
			void AddRender(const RenderSubsetVertex& renderSubset);
			void AddRender(const RenderSubsetLine& renderSubset);
			void AddRender(const RenderSubsetLineSegment& renderSubset);

			void AddRender(const RenderSubsetStatic& renderSubset);
			void AddRender(const RenderSubsetSkinned& renderSubset);
			void AddRender(const RenderSubsetTerrain& renderSubset);

			void AddRender(const RenderSubsetSky& renderSubset);
			void AddRender(const RenderSubsetSkybox& renderSubset);
			void AddRender(const RenderSubsetSkyEffect& renderSubset);
			void AddRender(const RenderSubsetSkyCloud& renderSubset);

			void AddRender(const RenderSubsetParticleEmitter& renderSubset);
			void AddRender(const RenderSubsetParticleDecal& renderSubset);

			void AddRender(const RenderSubsetUIText& renderSubset);
			void AddRender(const RenderSubsetUISprite& renderSubset);
			void AddRender(const RenderSubsetUIPanel& renderSubset);

		private:
			void CopyToMainRenderTarget();
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech);

		private:
			bool m_isInit;

			IEffect* m_pEffect;

			std::array<std::mutex, EmRenderer::TypeCount> m_pMutex;
			std::array<IRenderer*, EmRenderer::TypeCount> m_pRenderer;
		};
	}
}