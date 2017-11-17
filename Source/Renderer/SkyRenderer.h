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

			virtual void AddRender(const RenderSubsetSky& renderSubset) override { m_vecRPSky.emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetSkyEffect& renderSubset) override { m_vecRPSkyEffect.emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetSkyCloud& renderSubset) override { m_vecRPSkyCloud.emplace_back(renderSubset); }

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override { m_vecRPSky.clear(); m_vecRPSkyEffect.clear(); m_vecRPSkyCloud.clear(); }

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech);

		private:
			IEffect* m_pEffect;

			std::vector<RenderSubsetSky> m_vecRPSky;
			std::vector<RenderSubsetSkyEffect> m_vecRPSkyEffect;
			std::vector<RenderSubsetSkyCloud> m_vecRPSkyCloud;
		};
	}
}