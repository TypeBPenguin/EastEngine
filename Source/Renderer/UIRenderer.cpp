#include "stdafx.h"
#include "UIRenderer.h"

#include "DirectX/ISpriteBatch.h"
#include "DirectX/ISpriteFont.h"

namespace StrID
{
	RegisterStringID(UI);
}

namespace EastEngine
{
	namespace Graphics
	{
		class UIRenderer::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void AddRender(const RenderSubsetUIText& renderSubset);
			void AddRender(const RenderSubsetUISprite& renderSubset);
			void AddRender(const RenderSubsetUIPanel& renderSubset);

		public:
			void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag);
			void Flush();

		private:
			void RenderSprtie(ISpriteBatch* pSpriteBatch, std::vector<RenderSubsetUISprite>& vecRenderSubsetSprite);
			void RenderText(ISpriteBatch* pSpriteBatch, std::vector<RenderSubsetUIText>& vecRenderSubsetText);
			void RenderPanel(ISpriteBatch* pSpriteBatch);

		private:
			ISpriteBatch* m_pSpriteBatch{ nullptr };

			size_t m_nPanelIndex{ 0 };

			std::vector<RenderGroupUI> m_vecRenderGroup;
			std::vector<RenderSubsetUIPanel> m_vecRPUIPanel;
		};
		
		UIRenderer::Impl::Impl()
		{
			m_pSpriteBatch = ISpriteBatch::Create(GetImmediateContext()->GetInterface());
			m_pSpriteBatch->SetRotation(DXGI_MODE_ROTATION_IDENTITY);
		}

		UIRenderer::Impl::~Impl()
		{
			SafeDelete(m_pSpriteBatch);
		}

		void UIRenderer::Impl::AddRender(const RenderSubsetUIText& renderSubset)
		{
			size_t nSize = m_vecRenderGroup.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				if (m_vecRenderGroup[i].pRenderTargetView == renderSubset.pRenderTargetView)
				{
					m_vecRenderGroup[i].vecText.emplace_back(renderSubset);
					return;
				}
			}

			RenderGroupUI renderGroup;
			renderGroup.pRenderTargetView = renderSubset.pRenderTargetView;
			renderGroup.vecText.reserve(64);
			renderGroup.vecText.emplace_back(renderSubset);

			m_vecRenderGroup.emplace_back(renderGroup);
		}

		void UIRenderer::Impl::AddRender(const RenderSubsetUISprite& renderSubset)
		{
			size_t nSize = m_vecRenderGroup.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				if (m_vecRenderGroup[i].pRenderTargetView == renderSubset.pRenderTargetView)
				{
					m_vecRenderGroup[i].vecSprite.emplace_back(renderSubset);
					return;
				}
			}

			RenderGroupUI renderGroup;
			renderGroup.pRenderTargetView = renderSubset.pRenderTargetView;
			renderGroup.vecSprite.reserve(64);
			renderGroup.vecSprite.emplace_back(renderSubset);

			m_vecRenderGroup.emplace_back(renderGroup);
		}

		void UIRenderer::Impl::AddRender(const RenderSubsetUIPanel& renderSubset)
		{
			//renderSubset.bNeedRender = true;

			if (m_nPanelIndex >= m_vecRPUIPanel.size())
			{
				m_vecRPUIPanel.emplace_back(renderSubset);
			}
			else
			{
				m_vecRPUIPanel[m_nPanelIndex] = renderSubset;
			}

			++m_nPanelIndex;
		}

		void UIRenderer::Impl::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(pDeviceContext, UIRenderer);

			if (m_vecRPUIPanel.empty())
				return;

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();

			m_pSpriteBatch->SetViewport(GetDevice()->GetViewport());

			if (m_vecRenderGroup.empty() == false)
			{
				for (auto& renderGroup : m_vecRenderGroup)
				{
					renderGroup.pRenderTargetView->OnClear(pDeviceContext);

					pDeviceContext->SetRenderTargets(&renderGroup.pRenderTargetView, 1, nullptr);

					m_pSpriteBatch->Begin(EmSprite::eDeferred);

					RenderSprtie(m_pSpriteBatch, renderGroup.vecSprite);
					RenderText(m_pSpriteBatch, renderGroup.vecText);

					m_pSpriteBatch->End();
				}
			}

			IRenderTarget* pRenderTarget = pDevice->GetLastUseRenderTarget();
			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, false);

			m_pSpriteBatch->Begin(EmSprite::eBackToFront);

			RenderPanel(m_pSpriteBatch);

			m_pSpriteBatch->End();

			pDevice->ReleaseRenderTargets(&pRenderTarget);
		}

		void UIRenderer::Impl::Flush()
		{
			m_vecRenderGroup.clear();
			m_nPanelIndex = 0;
		}

		void UIRenderer::Impl::RenderSprtie(ISpriteBatch* pSpriteBatch, std::vector<RenderSubsetUISprite>& vecRenderSubsetSprite)
		{
			if (vecRenderSubsetSprite.empty())
				return;

			size_t nSize = vecRenderSubsetSprite.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				RenderSubsetUISprite& renderSubset = vecRenderSubsetSprite[i];

				if (renderSubset.pTexture == nullptr)
				{
					pSpriteBatch->Draw(nullptr,
						renderSubset.destRect, renderSubset.sourceRect,
						renderSubset.color, renderSubset.fRot, renderSubset.vOrigin,
						renderSubset.emSpriteEffects, renderSubset.fDepth);
				}
				else
				{
					pSpriteBatch->Draw(renderSubset.pTexture,
						renderSubset.destRect, renderSubset.sourceRect,
						renderSubset.color, renderSubset.fRot, renderSubset.vOrigin,
						renderSubset.emSpriteEffects, renderSubset.fDepth);
				}
			}
		}

		void UIRenderer::Impl::RenderText(ISpriteBatch* pSpriteBatch, std::vector<RenderSubsetUIText>& vecRenderSubsetText)
		{
			if (vecRenderSubsetText.empty())
				return;

			size_t nSize = vecRenderSubsetText.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				RenderSubsetUIText& renderSubset = vecRenderSubsetText[i];

				if (renderSubset.pSpriteFont == nullptr)
					continue;

				renderSubset.pSpriteFont->DrawString(pSpriteBatch,
					renderSubset.wstrText.c_str(),
					renderSubset.rect,
					renderSubset.color,
					renderSubset.fRot,
					renderSubset.vOrigin,
					renderSubset.fScale,
					renderSubset.emSpriteEffects,
					renderSubset.fDepth);
			}
		}

		void UIRenderer::Impl::RenderPanel(ISpriteBatch* pSpriteBatch)
		{
			if (m_vecRPUIPanel.empty())
				return;

			size_t nSize = m_vecRPUIPanel.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				RenderSubsetUIPanel& renderSubset = m_vecRPUIPanel[i];

				if (renderSubset.bNeedRender == false)
					continue;

				if (renderSubset.pTexture == nullptr)
					continue;

				pSpriteBatch->Draw(renderSubset.pTexture,
					renderSubset.destRect, renderSubset.sourceRect,
					renderSubset.color, renderSubset.fRot, renderSubset.vOrigin,
					renderSubset.emSpriteEffects, renderSubset.fDepth);

				renderSubset.bNeedRender = false;
			}
		}

		UIRenderer::UIRenderer()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		UIRenderer::~UIRenderer()
		{
		}

		void UIRenderer::AddRender(const RenderSubsetUIText& renderSubset)
		{
			m_pImpl->AddRender(renderSubset);
		}

		void UIRenderer::AddRender(const RenderSubsetUISprite& renderSubset)
		{
			m_pImpl->AddRender(renderSubset);
		}

		void UIRenderer::AddRender(const RenderSubsetUIPanel& renderSubset)
		{
			m_pImpl->AddRender(renderSubset);
		}

		void UIRenderer::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			m_pImpl->Render(pDevice, pDeviceContext, pCamera, nRenderGroupFlag);
		}

		void UIRenderer::Flush()
		{
			m_pImpl->Flush();
		}
	}
}