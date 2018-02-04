#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ISpriteBatch;

		class UIRenderer : public IRenderer
		{
		public:
			UIRenderer();
			virtual ~UIRenderer();

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void AddRender(const RenderSubsetUIText& renderSubset) override;
			virtual void AddRender(const RenderSubsetUISprite& renderSubset) override;
			virtual void AddRender(const RenderSubsetUIPanel& renderSubset) override;

			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			void RenderSprtie(ISpriteBatch* pSpriteBatch, std::vector<RenderSubsetUISprite>& vecRenderSubsetSprite);
			void RenderText(ISpriteBatch* pSpriteBatch, std::vector<RenderSubsetUIText>& vecRenderSubsetText);
			void RenderPanel(ISpriteBatch* pSpriteBatch);

		private:
			ISpriteBatch* m_pSpriteBatch;

			size_t m_nPanelIndex;

			std::vector<RenderGroupUI>		m_vecRenderGroup;
			std::vector<RenderSubsetUIPanel>	m_vecRPUIPanel;
		};
	}
}