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

			void AddRender(const RenderSubsetVertex& renderSubset) { m_pRenderer[EmRenderer::eVertex]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetLine& renderSubset) { m_pRenderer[EmRenderer::eVertex]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetLineSegment& renderSubset) { m_pRenderer[EmRenderer::eVertex]->AddRender(renderSubset); }

			void AddRender(const RenderSubsetStatic& renderSubset) { m_pRenderer[EmRenderer::eModel]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetSkinned& renderSubset) { m_pRenderer[EmRenderer::eModel]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetHeightField& renderSubset) { m_pRenderer[EmRenderer::eModel]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetTerrain& renderSubset) { m_pRenderer[EmRenderer::eTerrain]->AddRender(renderSubset); }

			void AddRender(const RenderSubsetSky& renderSubset) { m_pRenderer[EmRenderer::eSky]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetSkyEffect& renderSubset) { m_pRenderer[EmRenderer::eSky]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetSkyCloud& renderSubset) { m_pRenderer[EmRenderer::eSky]->AddRender(renderSubset); }

			void AddRender(const RenderSubsetParticleEmitter& renderSubset) { m_pRenderer[EmRenderer::eParticle]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetParticleDecal& renderSubset) { m_pRenderer[EmRenderer::eParticle]->AddRender(renderSubset); }

			void AddRender(const RenderSubsetUIText& renderSubset) { m_pRenderer[EmRenderer::eUI]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetUISprite& renderSubset) { m_pRenderer[EmRenderer::eUI]->AddRender(renderSubset); }
			void AddRender(const RenderSubsetUIPanel& renderSubset) { m_pRenderer[EmRenderer::eUI]->AddRender(renderSubset); }

		private:
			void CopyToMainRenderTarget();
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech);

		private:
			bool m_isInit;
			IEffect* m_pEffect;

			std::array<IRenderer*, EmRenderer::TypeCount> m_pRenderer;
		};
	}
}