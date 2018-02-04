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

			virtual void AddRender(const RenderSubsetSky& renderSubset) override { m_vecRenderSubsetSky[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetSkybox& renderSubset) override { m_vecRenderSubsetSkybox[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetSkyEffect& renderSubset) override { m_vecRenderSubsetSkyEffect[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetSkyCloud& renderSubset) override { m_vecRenderSubsetSkyCloud[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset); }

			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech);

		private:
			IEffect* m_pEffect;

			std::array<std::vector<RenderSubsetSky>, ThreadCount> m_vecRenderSubsetSky;
			std::array<std::vector<RenderSubsetSkybox>, ThreadCount> m_vecRenderSubsetSkybox;
			std::array<std::vector<RenderSubsetSkyEffect>, ThreadCount> m_vecRenderSubsetSkyEffect;
			std::array<std::vector<RenderSubsetSkyCloud>, ThreadCount> m_vecRenderSubsetSkyCloud;
		};
	}
}