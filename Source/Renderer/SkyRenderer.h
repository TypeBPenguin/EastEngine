#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTech;

		class SkyRenderer : public IRenderer
		{
		public:
			SkyRenderer();
			virtual ~SkyRenderer();

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void AddRender(const RenderSubsetSky& renderSubset) override { m_vecRenderSubsetSky.emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetSkybox& renderSubset) override { m_vecRenderSubsetSkybox.emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetSkyEffect& renderSubset) override { m_vecRenderSubsetSkyEffect.emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetSkyCloud& renderSubset) override { m_vecRenderSubsetSkyCloud.emplace_back(renderSubset); }

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override { m_vecRenderSubsetSky.clear(); m_vecRenderSubsetSkybox.clear(); m_vecRenderSubsetSkyEffect.clear(); m_vecRenderSubsetSkyCloud.clear(); }

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech);

		private:
			IEffect* m_pEffect;

			std::vector<RenderSubsetSky> m_vecRenderSubsetSky;
			std::vector<RenderSubsetSkybox> m_vecRenderSubsetSkybox;
			std::vector<RenderSubsetSkyEffect> m_vecRenderSubsetSkyEffect;
			std::vector<RenderSubsetSkyCloud> m_vecRenderSubsetSkyCloud;
		};
	}
}