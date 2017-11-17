#include "stdafx.h"
#include "UIRenderer.h"

#include "../DirectX/ISpriteBatch.h"
#include "../DirectX/ISpriteFont.h"

namespace StrID
{
	RegisterStringID(UI);
}

namespace EastEngine
{
	namespace Graphics
	{
		UIRenderer::UIRenderer()
			: m_pSpriteBatch(nullptr)
			, m_nPanelIndex(0)
		{
		}

		UIRenderer::~UIRenderer()
		{
			SafeDelete(m_pSpriteBatch);
		}

		bool UIRenderer::Init(const Math::Viewport& viewport)
		{
			m_pSpriteBatch = ISpriteBatch::Create(GetDeviceContext()->GetInterface());
			m_pSpriteBatch->SetRotation(DXGI_MODE_ROTATION_IDENTITY);
			m_pSpriteBatch->SetViewport(viewport);

			return true;
		}

		void UIRenderer::AddRender(const RenderSubsetUIText& renderSubset)
		{
			uint32_t nSize = m_vecRenderGroup.size();
			for (uint32_t i = 0; i < nSize; ++i)
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

		void UIRenderer::AddRender(const RenderSubsetUISprite& renderSubset)
		{
			uint32_t nSize = m_vecRenderGroup.size();
			for (uint32_t i = 0; i < nSize; ++i)
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

		void UIRenderer::AddRender(const RenderSubsetUIPanel& renderSubset)
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

		void UIRenderer::Render(uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(UIRenderer);

			if (m_vecRPUIPanel.empty())
				return;

			IDevice* pDevice = GetDevice();
			IDeviceContext* pDeviceContext = GetDeviceContext();

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();

			if (m_vecRenderGroup.empty() == false)
			{
				for (auto& renderGroup : m_vecRenderGroup)
				{
					renderGroup.pRenderTargetView->OnClear(pDeviceContext);

					pDeviceContext->SetRenderTargets(&renderGroup.pRenderTargetView, 1, nullptr);

					m_pSpriteBatch->Begin(EmSprite::eDeferred);

					renderSprtie(m_pSpriteBatch, renderGroup.vecSprite);
					renderText(m_pSpriteBatch, renderGroup.vecText);

					m_pSpriteBatch->End();
				}
			}

			IRenderTarget* pRenderTarget = pDevice->GetLastUseRenderTarget();
			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, false);

			m_pSpriteBatch->Begin(EmSprite::eBackToFront);

			renderPanel(m_pSpriteBatch);

			m_pSpriteBatch->End();

			pDevice->ReleaseRenderTargets(&pRenderTarget);
		}

		void UIRenderer::Flush()
		{
			m_vecRenderGroup.clear();
			m_nPanelIndex = 0;
		}

		void UIRenderer::renderSprtie(ISpriteBatch* pSpriteBatch, std::vector<RenderSubsetUISprite>& vecRenderSubsetSprite)
		{
			if (vecRenderSubsetSprite.empty())
				return;

			uint32_t nSize = vecRenderSubsetSprite.size();
			for (uint32_t i = 0; i < nSize; ++i)
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

		void UIRenderer::renderText(ISpriteBatch* pSpriteBatch, std::vector<RenderSubsetUIText>& vecRenderSubsetText)
		{
			if (vecRenderSubsetText.empty())
				return;

			uint32_t nSize = vecRenderSubsetText.size();
			for (uint32_t i = 0; i < nSize; ++i)
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

		void UIRenderer::renderPanel(ISpriteBatch* pSpriteBatch)
		{
			if (m_vecRPUIPanel.empty())
				return;

			uint32_t nSize = m_vecRPUIPanel.size();
			for (uint32_t i = 0; i < nSize; ++i)
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
	}
}