#include "stdafx.h"
#include "UIPanel.h"

#include "UIMgr.h"

#include "../DirectX/RenderTarget.h"
#include "../DirectX/ISpriteFont.h"

namespace EastEngine
{
	namespace UI
	{
		CUIPanel::CUIPanel(UIManager* pUIMgr, HWND hWnd)
			: m_pUIMgr(pUIMgr)
			, m_hWnd(hWnd)
			, m_pRenderTarget(nullptr)
			, m_pUIObjectFocus(nullptr)
			, m_pUIObjectPressed(nullptr)
			, m_pUIObjectMouseOver(nullptr)
			, m_bNeedRender(false)
			, m_bVisible(false)
			, m_bDrag(false)
			, m_bKeyboardInput(false)
			, m_bMouseInput(false)
		{
			SetRectEmpty(&m_rect);
			SetRectEmpty(&m_sourceRect);
		}

		CUIPanel::~CUIPanel()
		{
			m_hWnd = nullptr;
			std::for_each(m_umapUIObject.begin(), m_umapUIObject.end(), DeleteSTLMapObject());
			m_umapUIObject.clear();

			SafeDelete(m_pRenderTarget);
			m_pUIObjectFocus = nullptr;
			m_pUIObjectPressed = nullptr;
			m_pUIObjectMouseOver = nullptr;
		}

		bool CUIPanel::Init(const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight)
		{
			m_strID = strID;

			SetRect(&m_rect, 0, 0, nWidth, nHeight);
			OffsetRect(&m_rect, x, y);
			SetRect(&m_sourceRect, 0, 0, nWidth, nHeight);

			Graphics::RenderTargetDesc2D renderTargetInfo;
			renderTargetInfo.Width = nWidth;
			renderTargetInfo.Height = nHeight;
			renderTargetInfo.MipLevels = 1;
			renderTargetInfo.ArraySize = 1;
			renderTargetInfo.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			renderTargetInfo.SampleDesc.Count = 1;
			renderTargetInfo.Usage = D3D11_USAGE_DEFAULT;
			renderTargetInfo.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			renderTargetInfo.CPUAccessFlags = 0;
			renderTargetInfo.MiscFlags = 0;
			renderTargetInfo.Build();
		
			m_pRenderTarget = Graphics::RenderTarget::Create(renderTargetInfo);
			if (m_pRenderTarget == nullptr)
				return false;

			m_bNeedRender = true;
			m_bVisible = true;
			m_bKeyboardInput = true;
			m_bMouseInput = true;

			return true;
		}

		void CUIPanel::Update(float fElapsedTime)
		{
			if (m_bVisible == false)
				return;

			if (m_bNeedRender)
			{
				m_bNeedRender = false;

				m_pRenderTarget->SetClear(Math::Color::Transparent);

				for (auto iter = m_umapUIObject.begin(); iter != m_umapUIObject.end(); ++iter)
				{
					IUIObject* pUIObject = iter->second;

					pUIObject->Update(fElapsedTime);
				}
			}

			drawPanel(m_pRenderTarget->GetTexture(), m_rect, &m_sourceRect, Math::Vector2(), Math::Color::White, 0.f, UI_NEAR_DEPTH);
		}

		bool CUIPanel::HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			if (m_bVisible == false)
				return false;

			if (m_pUIObjectFocus &&
				m_pUIObjectFocus->IsEnable())
			{
				if (m_pUIObjectFocus->HandleMsg(hWnd, nMsg, wParam, lParam))
					return true;
			}


			bool bHandled = false;

			switch (nMsg)
			{
			case WM_SIZE:
			case WM_MOVE:
			{
				// Handle sizing and moving messages so that in case the mouse cursor is moved out
				// of an UI control because of the window adjustment, we can properly
				// unhighlight the highlighted control.
				POINT pt =
				{
					-1, -1
				};
				onMouseMove(pt);
				break;
			}

			case WM_ACTIVATEAPP:
				// Call OnFocusIn()/OnFocusOut() of the control that currently has the focus
				// as the application is activated/deactivated.  This matches the Windows
				// behavior.
				if (m_pUIObjectFocus &&
					m_pUIObjectFocus->IsEnable())
				{
					if (wParam)
					{
						m_pUIObjectFocus->OnFocusIn();
					}
					else
					{
						m_pUIObjectFocus->OnFocusOut();
					}

					Invalidate();
				}
				break;

				// Keyboard messages
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				// If a control is in focus, it belongs to this dialog, and it's enabled, then give
				// it the first chance at handling the message.
				if (m_pUIObjectFocus &&
					m_pUIObjectFocus->IsEnable())
				{
					if (m_pUIObjectFocus->HandleKeyboard(nMsg, wParam, lParam))
						return true;
				}

				// Not yet handled, see if this matches a control's hotkey
				// Activate the hotkey if the focus doesn't belong to an
				// edit box.
				if (nMsg == WM_KEYDOWN &&
					(!m_pUIObjectFocus || m_pUIObjectFocus->GetType() != EmUI::eEditBox))
				{
					for (auto iter = m_umapUIObject.begin(); iter != m_umapUIObject.end(); ++iter)
					{
						IUIObject* pUIObject = iter->second;
						if (pUIObject->GetHotKey() == wParam)
						{
							pUIObject->OnHotKey();
							return true;
						}
					}
				}

				// Not yet handled, check for focus messages
				if (nMsg == WM_KEYDOWN)
				{
					// If keyboard input is not enabled, this message should be ignored
					if (!m_bKeyboardInput)
						return false;

					switch (wParam)
					{
					case VK_RIGHT:
					case VK_DOWN:
						if (m_pUIObjectFocus)
						{
							//return onCycleFocus(true);
							return true;
						}
						break;

					case VK_LEFT:
					case VK_UP:
						if (m_pUIObjectFocus)
						{
							//return onCycleFocus(false);
							return true;
						}
						break;

					case VK_TAB:
					{
						bool isShiftDown = ((GetKeyState(VK_SHIFT) & 0x8000) != 0);
						//return onCycleFocus(isShiftDown == false);
						return true;
					}
					}
				}

				break;
			}

			// Mouse messages
			case WM_MOUSEMOVE:
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
			case WM_LBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_XBUTTONDBLCLK:
			case WM_MOUSEWHEEL:
			{
				// If not accepting mouse input, return false to indicate the message should still 
				// be handled by the application (usually to move the camera).
				if (!m_bMouseInput)
					return false;

				POINT mousePoint =
				{
					short(LOWORD(lParam)), short(HIWORD(lParam))
				};

				mousePoint.x -= GetPosX();
				mousePoint.y -= GetPosY();

				/*mousePoint.x -= m_nPosX;
				mousePoint.y -= m_nPosY;*/

				// If caption is enabled, offset the Y coordinate by the negative of its height.
				/*if (m_bCaption)
				mousePoint.y -= m_nCaptionHeight;*/

				//mousePoint.y += 18;

				// If a control is in focus, it belongs to this dialog, and it's enabled, then give
				// it the first chance at handling the message.
				if (m_pUIObjectFocus &&
					m_pUIObjectFocus->IsEnable())
				{
					if (m_pUIObjectFocus->HandleMouse(nMsg, mousePoint, wParam, lParam))
						return true;
				}

				// Not yet handled, see if the mouse is over any controls
				IUIObject* pUIObject = GetControlAtPoint(mousePoint);
				if (pUIObject != nullptr && pUIObject->IsEnable())
				{
					bHandled = pUIObject->HandleMouse(nMsg, mousePoint, wParam, lParam);
					if (bHandled)
						return true;
				}
				else
				{
					// Mouse not over any controls in this dialog, if there was a control
					// which had focus it just lost it
					if (nMsg == WM_LBUTTONDOWN &&
						m_pUIObjectFocus)
					{
						m_pUIObjectFocus->OnFocusOut();
						m_pUIObjectFocus = nullptr;

						Invalidate();
					}
				}

				// Still not handled, hand this off to the dialog. Return false to indicate the
				// message should still be handled by the application (usually to move the camera).
				switch (nMsg)
				{
				case WM_MOUSEMOVE:
					onMouseMove(mousePoint);
					return false;
				}

				break;
			}

			case WM_CAPTURECHANGED:
			{
				// The application has lost mouse capture.
				// The dialog object may not have received
				// a WM_MOUSEUP when capture changed. Reset
				// m_bDrag so that the dialog does not mistakenly
				// think the mouse button is still held down.
				if (reinterpret_cast<HWND>(lParam) != hWnd)
					m_bDrag = false;
			}
			}

			return false;
		}

		void CUIPanel::ClearRadioButtonGroup(uint32_t nGroupID)
		{
			for (auto iter = m_umapUIObject.begin(); iter != m_umapUIObject.end(); ++iter)
			{
				IUIObject* pUIObject = iter->second;
				if (pUIObject->GetType() == EmUI::eRadioButton)
				{
					CUIRadioButton* pRadioButton = static_cast<CUIRadioButton*>(pUIObject);

					if (pRadioButton->GetButtonGroup() == nGroupID)
					{
						pRadioButton->SetChecked(false, false);
					}
				}
			}
		}

		void CUIPanel::RequestFocus(IUIObject* pUIObject)
		{
			if (m_pUIObjectFocus == pUIObject)
				return;

			if (!pUIObject->CanHaveFocus())
				return;

			if (m_pUIObjectFocus)
				m_pUIObjectFocus->OnFocusOut();

			pUIObject->OnFocusIn();
			m_pUIObjectFocus = pUIObject;

			Invalidate();
		}

		void CUIPanel::ClearFocus()
		{
			if (m_pUIObjectFocus)
			{
				m_pUIObjectFocus->OnFocusOut();
				m_pUIObjectFocus = nullptr;
			}

			Invalidate();

			ReleaseCapture();
		}

		void CUIPanel::RequestMousePressed(IUIObject* pUIObject)
		{
			if (m_pUIObjectPressed == pUIObject)
				return;

			if (!pUIObject->CanHaveFocus())
				return;

			if (m_pUIObjectPressed)
				m_pUIObjectPressed->OnMousePressedOut();

			pUIObject->OnMousePressedIn();
			m_pUIObjectPressed = pUIObject;

			Invalidate();
		}

		void CUIPanel::ClearMousePressed()
		{
			if (m_pUIObjectPressed)
			{
				m_pUIObjectPressed->OnMousePressedOut();
				m_pUIObjectPressed = nullptr;
			}

			Invalidate();

			ReleaseCapture();
		}

		void CUIPanel::DrawText(IUIElement* pElement, const wchar_t* wstrText, Math::Rect& rect, float fDepth)
		{
			DrawText(pElement, std::wstring(wstrText), rect, fDepth);
		}

		void CUIPanel::DrawText(IUIElement* pElement, const std::wstring& wstrText, Math::Rect& rect, float fDepth)
		{
			if (pElement == nullptr)
				return;

			if (pElement->GetType() != EmUI::eFont)
				return;

			if (wstrText.empty())
				return;

			CUIElementFont* pFont = static_cast<CUIElementFont*>(pElement);

			UIFontNode* fontNode = pFont->GetFontNode();
			if (fontNode == nullptr)
				return;

			if (fontNode->pSpriteFont == nullptr)
				return;

			Math::Rect rc = rect;
			Math::Vector2 vMeasureString(Math::Vector2(fontNode->pSpriteFont->MeasureString(wstrText.c_str())) * 0.5f);
			Math::Vector2 vOrigin(0.f, 0.f);

			switch (pFont->GetAlignHorizontal())
			{
			case EmUI::eLeft:
				rc.left = rect.left;
				break;
			case EmUI::eCenter:
				rc.left = (uint32_t)(rect.right * 0.5f - vMeasureString.x);
				break;
			case EmUI::eRight:
				rc.left = (uint32_t)(std::max((float)(rect.right) - vMeasureString.x * 2.f, (float)(rc.left)));
				break;
			}

			switch (pFont->GetAlignVertical())
			{
			case EmUI::eTop:
				rc.top = rect.top;
				break;
			case EmUI::eMiddle:
				rc.top = (uint32_t)(rect.bottom * 0.5f - vMeasureString.y);
				break;
			case EmUI::eBottom:
				rc.top = (uint32_t)(std::max((float)(rect.bottom) - vMeasureString.y, (float)(rc.top)));
				break;
			}

			rc.left = std::max(rc.left, 0L);
			rc.top = std::max(rc.top, 0L);

			Graphics::RenderSubsetUIText renderSubset;
			renderSubset.Set(fontNode->pSpriteFont, wstrText,
				rc, vOrigin,
				*pFont->GetEffectColor(), pFont->GetRotation(), pFont->GetScale(), fDepth, pFont->GetSpriteEffect(), m_pRenderTarget);

			switch (pFont->GetFontEffect())
			{
			case EmUI::eShadow:
			{
				Math::Rect rcText = rc;
				rcText.Move(1, 1);

				renderSubset.rect = rcText;

				Graphics::RendererManager::GetInstance()->AddRender(renderSubset);

				rcText = rc;
				rcText.Move(-1, 1);

				renderSubset.rect = rcText;

				Graphics::RendererManager::GetInstance()->AddRender(renderSubset);
				break;
			}
			case EmUI::eOutline:
			{
				Math::Rect rcText = rc;
				rcText.Move(1, 1);

				renderSubset.rect = rcText;

				Graphics::RendererManager::GetInstance()->AddRender(renderSubset);

				rcText = rc;
				rcText.Move(-1, 1);

				renderSubset.rect = rcText;

				Graphics::RendererManager::GetInstance()->AddRender(renderSubset);

				rcText = rc;
				rcText.Move(-1, -1);

				renderSubset.rect = rcText;

				Graphics::RendererManager::GetInstance()->AddRender(renderSubset);

				rcText = rc;
				rcText.Move(1, -1);

				renderSubset.rect = rcText;

				Graphics::RendererManager::GetInstance()->AddRender(renderSubset);
				break;
			}
			}

			Graphics::RenderSubsetUIText renderSubsetMain(fontNode->pSpriteFont, wstrText,
				rc, vOrigin,
				pFont->GetColor(), pFont->GetRotation(), pFont->GetScale(), fDepth, pFont->GetSpriteEffect(), m_pRenderTarget);
			Graphics::RendererManager::GetInstance()->AddRender(renderSubsetMain);
		}

		void CUIPanel::DrawText(IUIElement* pElement, const char* strText, Math::Rect& rect, float fDepth)
		{
			DrawText(pElement, String::MultiToWide(strText), rect, fDepth);
		}

		void CUIPanel::DrawText(IUIElement* pElement, const std::string& strText, Math::Rect& rect, float fDepth)
		{
			DrawText(pElement, String::MultiToWide(strText), rect, fDepth);
		}

		void CUIPanel::DrawSprite(IUIElement* pElement, Math::Rect& rect, float fDepth)
		{
			if (pElement == nullptr)
				return;

			if (pElement->GetType() != EmUI::eTexture)
				return;

			CUIElementTexture* pTexture = static_cast<CUIElementTexture*>(pElement);

			Math::Vector2 origin;
			Math::Rect* sourceRect = pTexture->GetTextureRect();
			if (sourceRect == nullptr)
				return;

			UITextureNode* pTextureNode = pTexture->GetTextureNode();
			if (pTextureNode == nullptr || pTextureNode->pTexture == nullptr)
				return;

			Graphics::RenderSubsetUISprite renderSubset(pTextureNode->pTexture,
				rect, sourceRect,
				origin, pTexture->GetColor(),
				0.f, fDepth, pTexture->GetSpriteEffect(), m_pRenderTarget);
			Graphics::RendererManager::GetInstance()->AddRender(renderSubset);
		}

		void CUIPanel::drawPanel(std::shared_ptr<Graphics::ITexture> pTexture, const Math::Rect& rect, Math::Rect* sourceRect, const Math::Vector2& origin, const Math::Color& color, float fRotation, float fDepth)
		{
			if (pTexture == nullptr)
				return;

			Graphics::RenderSubsetUIPanel renderSubset(pTexture,
				rect, sourceRect,
				origin, color,
				fRotation, fDepth, Graphics::EmSprite::eNone, nullptr);
			Graphics::RendererManager::GetInstance()->AddRender(renderSubset);
		}

		CUIStatic* CUIPanel::AddStatic(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight, CreateUIElementFontInfo* pFontInfo)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				if (iter->second->GetType() == EmUI::eStatic)
					return static_cast<CUIStatic*>(iter->second);

				return nullptr;
			}

			CUIStatic* pStatic = new CUIStatic(this, strID);

			pStatic->SetText(strText);
			pStatic->SetPosition(x, y);
			pStatic->SetSize(nWidth, nHeight);

			if (pFontInfo != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo->strFontName);

				IUIElement* pUIElement = pStatic->CreateElement(pFontInfo->strElementName, EmUI::eFont);
				pUIElement->Init(pFontInfo);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::Black, Math::Color(0.7843f, 0.7843f, 0.7843f, 0.7843f));

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pStatic->CreateElement("Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pParent == nullptr)
			{
				m_umapUIObject.insert(std::make_pair(strID, pStatic));
			}
			else
			{
				pParent->AddChild(pStatic->GetID(), pStatic);
			}

			return pStatic;
		}

		CUIButton* CUIPanel::AddButton(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight,
			CreateUIElementFontInfo* pFontInfo_Button, CreateUIElementTextureInfo* pTextureInfo_Button,
			CreateUIElementTextureInfo* pTextureInfo_FillLayer)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				if (iter->second->GetType() == EmUI::eButton)
					return static_cast<CUIButton*>(iter->second);

				return nullptr;
			}

			CUIButton* pButton = new CUIButton(this, strID);

			pButton->SetText(strText);
			pButton->SetPosition(x, y);
			pButton->SetSize(nWidth, nHeight);

			if (pFontInfo_Button != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo_Button->strFontName);

				IUIElement* pUIElement = pButton->CreateElement(pFontInfo_Button->strFontName, EmUI::eFont);
				pUIElement->Init(pFontInfo_Button);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::Black);
				fintInfo.fontColor.SetColor(EmUI::eMouseOver, Math::Color::Black);

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pButton->CreateElement("Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pTextureInfo_Button != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Button->strTextureName);

				IUIElement* pUIElement = pButton->CreateElement(pTextureInfo_Button->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Button);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Button->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "Button_main";
				textureInfo.textureColor.Init(Math::Color(1.f, 1.f, 1.f, 0.5882f));
				textureInfo.textureColor.SetColor(EmUI::ePressed, Math::Color(1.f, 1.f, 1.f, 0.7843f));

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pButton->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_FillLayer != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_FillLayer->strTextureName);

				IUIElement* pUIElement = pButton->CreateElement(pTextureInfo_FillLayer->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_FillLayer);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_FillLayer->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "Button_bg";
				textureInfo.textureColor.Init(Math::Color::Black);
				textureInfo.textureColor.SetColor(EmUI::eMouseOver, Math::Color(1.f, 1.f, 1.f, 0.6274f));
				textureInfo.textureColor.SetColor(EmUI::ePressed, Math::Color(0.f, 0.f, 0.f, 0.2352f));
				textureInfo.textureColor.SetColor(EmUI::eFocus, Math::Color(1.f, 1.f, 1.f, 0.1176f));

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pButton->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pParent == nullptr)
			{
				m_umapUIObject.insert(std::make_pair(strID, pButton));
			}
			else
			{
				pParent->AddChild(pButton->GetID(), pButton);
			}

			return pButton;
		}

		CUICheckBox* CUIPanel::AddCheckBox(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight, bool bCheck,
			CreateUIElementFontInfo* pFontInfo_Title, CreateUIElementTextureInfo* pTextureInfo_Box,
			CreateUIElementTextureInfo* pTextureInfo_Check)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				if (iter->second->GetType() == EmUI::eCheckBox)
					return static_cast<CUICheckBox*>(iter->second);

				return nullptr;
			}

			CUICheckBox* pCheckBox = new CUICheckBox(this, strID);

			pCheckBox->SetText(strText);
			pCheckBox->SetPosition(x, y);
			pCheckBox->SetSize(nWidth, nHeight);
			pCheckBox->SetChecked(bCheck);

			if (pFontInfo_Title != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo_Title->strFontName);

				IUIElement* pUIElement = pCheckBox->CreateElement(pFontInfo_Title->strFontName, EmUI::eFont);
				pUIElement->Init(pFontInfo_Title);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::Black);
				fintInfo.fontColor.SetColor(EmUI::eDisabled, Math::Color(0.7843f, 0.7843f, 0.7843f, 0.7843f));

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pCheckBox->CreateElement("Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pTextureInfo_Box != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Box->strTextureName);

				IUIElement* pUIElement = pCheckBox->CreateElement(pTextureInfo_Box->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Box);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Box->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "CheckBox";
				textureInfo.textureColor.Init(Math::Color(1.f, 1.f, 1.f, 0.5882f));
				textureInfo.textureColor.SetColor(EmUI::eFocus, Math::Color(1.f, 1.f, 1.f, 0.7843f));
				textureInfo.textureColor.SetColor(EmUI::ePressed, Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pCheckBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Check != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Check->strTextureName);

				IUIElement* pUIElement = pCheckBox->CreateElement(pTextureInfo_Check->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Check);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Check->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "Check";
				textureInfo.textureColor.Init(Math::Color::Black);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pCheckBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pParent == nullptr)
			{
				m_umapUIObject.insert(std::make_pair(strID, pCheckBox));
			}
			else
			{
				pParent->AddChild(pCheckBox->GetID(), pCheckBox);
			}

			return pCheckBox;
		}

		CUIRadioButton* CUIPanel::AddRadioButton(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight, uint32_t nGroupID, bool bCheck,
			CreateUIElementFontInfo* pFontInfo_Title, CreateUIElementTextureInfo* pTextureInfo_Box,
			CreateUIElementTextureInfo* pTextureInfo_Radio)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				if (iter->second->GetType() == EmUI::eRadioButton)
					return static_cast<CUIRadioButton*>(iter->second);

				return nullptr;
			}

			CUIRadioButton* pRadioButton = new CUIRadioButton(this, strID);

			pRadioButton->SetText(strText);
			pRadioButton->SetPosition(x, y);
			pRadioButton->SetSize(nWidth, nHeight);
			pRadioButton->SetButtonGroup(nGroupID);
			pRadioButton->SetChecked(bCheck);

			if (pFontInfo_Title != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo_Title->strFontName);

				IUIElement* pUIElement = pRadioButton->CreateElement(pFontInfo_Title->strFontName, EmUI::eFont);
				pUIElement->Init(pFontInfo_Title);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::Black);
				fintInfo.fontColor.SetColor(EmUI::eDisabled, Math::Color(0.7843f, 0.7843f, 0.7843f, 0.7843f));

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pRadioButton->CreateElement("Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pTextureInfo_Box != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Box->strTextureName);

				IUIElement* pUIElement = pRadioButton->CreateElement(pTextureInfo_Box->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Box);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Box->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "RadioBox";
				textureInfo.textureColor.Init(Math::Color(1.f, 1.f, 1.f, 0.5882f));
				textureInfo.textureColor.SetColor(EmUI::eFocus, Math::Color(1.f, 1.f, 1.f, 0.7843f));
				textureInfo.textureColor.SetColor(EmUI::ePressed, Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pRadioButton->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Radio != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Radio->strTextureName);

				IUIElement* pUIElement = pRadioButton->CreateElement(pTextureInfo_Radio->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Radio);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Radio->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "Radio";
				textureInfo.textureColor.Init(Math::Color::Black);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pRadioButton->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pParent == nullptr)
			{
				m_umapUIObject.insert(std::make_pair(strID, pRadioButton));
			}
			else
			{
				pParent->AddChild(pRadioButton->GetID(), pRadioButton);
			}

			return pRadioButton;
		}

		CUIComboBox* CUIPanel::AddComboBox(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight, uint32_t nDropHeight,
			CreateUIElementFontInfo* pFontInfo_Title, CreateUIElementTextureInfo* pTextureInfo_Main,
			CreateUIElementFontInfo* pFontInfo_DropDown, CreateUIElementTextureInfo* pTextureInfo_DropDown,
			CreateUIElementFontInfo* pFontInfo_Selection, CreateUIElementTextureInfo* pTextureInfo_Selection,
			CreateUIElementTextureInfo* pTextureInfo_Button)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				if (iter->second->GetType() == EmUI::eComboBox)
					return static_cast<CUIComboBox*>(iter->second);

				return nullptr;
			}

			CUIComboBox* pComboBox = new CUIComboBox(this, strID);

			pComboBox->SetPosition(x, y);
			pComboBox->SetSize(nWidth, nHeight);
			pComboBox->SetDropHeight(nDropHeight);

			if (pFontInfo_Title != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo_Title->strFontName);

				IUIElement* pUIElement = pComboBox->CreateElement(pFontInfo_Title->strFontName, EmUI::eFont);
				pUIElement->Init(pFontInfo_Title);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::Black, Math::Color(0.7843f, 0.7843f, 0.7843f, 0.7843f));
				fintInfo.fontColor.SetColor(EmUI::eMouseOver, Math::Color::Black);
				fintInfo.fontColor.SetColor(EmUI::ePressed, Math::Color::Black);

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pComboBox->CreateElement("Title_Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pTextureInfo_Main != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Main->strTextureName);

				IUIElement* pUIElement = pComboBox->CreateElement(pTextureInfo_Main->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Main);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Main->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox";
				textureInfo.textureColor.Init(Math::Color(0.7843f, 0.7843f, 0.7843f, 0.5882f), Math::Color(0.7843f, 0.7843f, 0.7843f, 0.2745f));
				textureInfo.textureColor.SetColor(EmUI::eFocus, Math::Color(0.9019f, 0.9019f, 0.9019f, 0.6667f));

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pComboBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pFontInfo_DropDown != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo_DropDown->strFontName);

				IUIElement* pUIElement = pComboBox->CreateElement(pFontInfo_DropDown->strFontName, EmUI::eFont);
				pUIElement->Init(pFontInfo_DropDown);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::Black);

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pComboBox->CreateElement("DropDown_Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pTextureInfo_DropDown != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_DropDown->strTextureName);

				IUIElement* pUIElement = pComboBox->CreateElement(pTextureInfo_DropDown->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_DropDown);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_DropDown->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "DropDown_bg";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pComboBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pFontInfo_Selection != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo_Selection->strFontName);

				IUIElement* pUIElement = pComboBox->CreateElement(pFontInfo_Selection->strFontName, EmUI::eFont);
				pUIElement->Init(pFontInfo_Selection);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::White);

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pComboBox->CreateElement("Selection_Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pTextureInfo_Selection != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Selection->strTextureName);

				IUIElement* pUIElement = pComboBox->CreateElement(pTextureInfo_Selection->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Selection);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Selection->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "Selection";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pComboBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Button != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Selection->strTextureName);

				IUIElement* pUIElement = pComboBox->CreateElement(pTextureInfo_Selection->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Selection);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Selection->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "DropDown_btn";
				textureInfo.textureColor.Init(Math::Color(1.f, 1.f, 1.f, 0.5882f), Math::Color(1.f, 1.f, 1.f, 0.2745f));
				textureInfo.textureColor.SetColor(EmUI::ePressed, Math::Color(0.5882f, 0.5882f, 0.5882f, 1.f));
				textureInfo.textureColor.SetColor(EmUI::eFocus, Math::Color(1.f, 1.f, 1.f, 0.7843f));

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pComboBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pParent == nullptr)
			{
				m_umapUIObject.insert(std::make_pair(strID, pComboBox));
			}
			else
			{
				pParent->AddChild(pComboBox->GetID(), pComboBox);
			}

			return pComboBox;
		}

		CUISlider* CUIPanel::AddSlider(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight,
			int nMin, int nMax,
			CreateUIElementTextureInfo* pTextureInfo_Track,
			CreateUIElementTextureInfo* pTextureInfo_Button)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				if (iter->second->GetType() == EmUI::eSlider)
					return static_cast<CUISlider*>(iter->second);

				return nullptr;
			}

			CUISlider* pSlider = new CUISlider(this, strID);

			pSlider->SetPosition(x, y);
			pSlider->SetSize(nWidth, nHeight);

			if (nMin > nMax)
			{
				int temp = nMax;
				nMin = nMax;
				nMax = temp;
			}

			pSlider->SetRange(nMin, nMax);
			pSlider->SetValue((nMax + nMin) / 2);

			if (pTextureInfo_Track != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Track->strTextureName);

				IUIElement* pUIElement = pSlider->CreateElement(pTextureInfo_Track->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Track);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Track->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "SliderTrack";
				textureInfo.textureColor.Init(Math::Color(1.f, 1.f, 1.f, 0.5882f), Math::Color(1.f, 1.f, 1.f, 0.2745f));
				textureInfo.textureColor.SetColor(EmUI::eFocus, Math::Color(1.f, 1.f, 1.f, 0.7843f));

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pSlider->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Track != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Track->strTextureName);

				IUIElement* pUIElement = pSlider->CreateElement(pTextureInfo_Track->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Track);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Track->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "SliderButton";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pSlider->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pParent == nullptr)
			{
				m_umapUIObject.insert(std::make_pair(strID, pSlider));
			}
			else
			{
				pParent->AddChild(pSlider->GetID(), pSlider);
			}

			return pSlider;
		}

		CUIEditBox* CUIPanel::AddEditBox(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight,
			CreateUIElementFontInfo* pFontInfo,
			CreateUIElementTextureInfo* pTextureInfo_Inside, CreateUIElementTextureInfo* pTextureInfo_LeftTop,
			CreateUIElementTextureInfo* pTextureInfo_Top, CreateUIElementTextureInfo* pTextureInfo_RightTop,
			CreateUIElementTextureInfo* pTextureInfo_Left, CreateUIElementTextureInfo* pTextureInfo_Right,
			CreateUIElementTextureInfo* pTextureInfo_LeftBottom, CreateUIElementTextureInfo* pTextureInfo_Bottom,
			CreateUIElementTextureInfo* pTextureInfo_RightBottom, CreateUIElementTextureInfo* pTextureInfo_Caret)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				if (iter->second->GetType() == EmUI::eEditBox)
					return static_cast<CUIEditBox*>(iter->second);

				return nullptr;
			}

			CUIEditBox* pEditBox = new CUIEditBox(this, strID);

			pEditBox->SetPosition(x, y);
			pEditBox->SetSize(nWidth, nHeight);

			if (pFontInfo != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo->strFontName);

				IUIElement* pUIElement = pEditBox->CreateElement(pFontInfo->strFontName, EmUI::eFont);
				pUIElement->Init(pFontInfo);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::Black);

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pEditBox->CreateElement("Selection_Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pTextureInfo_Inside != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Inside->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_Inside->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Inside);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Inside->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox_Inside";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_LeftTop != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_LeftTop->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_LeftTop->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_LeftTop);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_LeftTop->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox_LeftTop";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Top != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Top->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_Top->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Top);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Top->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox_Top";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_RightTop != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_RightTop->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_RightTop->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_RightTop);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_RightTop->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox_RightTop";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Left != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Left->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_Left->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Left);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Left->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox_Left";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Right != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Right->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_Right->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Right);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Right->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox_Right";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_LeftBottom != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_LeftBottom->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_LeftBottom->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_LeftBottom);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_LeftBottom->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox_LeftBottom";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Bottom != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Bottom->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_Bottom->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Bottom);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Bottom->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox_Bottom";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_RightBottom != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_RightBottom->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_RightBottom->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_RightBottom);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_RightBottom->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "EditBox_RightBottom";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Caret != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Caret->strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(pTextureInfo_Caret->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Caret);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Caret->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "Caret";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pEditBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pParent == nullptr)
			{
				m_umapUIObject.insert(std::make_pair(strID, pEditBox));
			}
			else
			{
				pParent->AddChild(pEditBox->GetID(), pEditBox);
			}

			return pEditBox;
		}

		CUIListBox* CUIPanel::AddListBox(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight, EmListBox::Style emListBoxStyle,
			CreateUIElementFontInfo* pFontInfo_Main, CreateUIElementTextureInfo* pTextureInfo_Main,
			CreateUIElementFontInfo* pFontInfo_Selection, CreateUIElementTextureInfo* pTextureInfo_Selection)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				if (iter->second->GetType() == EmUI::eScrollBar)
					return static_cast<CUIListBox*>(iter->second);

				return nullptr;
			}

			CUIListBox* pListBox = new CUIListBox(this, strID);

			pListBox->SetPosition(x, y);
			pListBox->SetSize(nWidth, nHeight);
			pListBox->SetStyle(emListBoxStyle);

			if (pFontInfo_Main != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo_Main->strFontName);

				IUIElement* pUIElement = pListBox->CreateElement(pFontInfo_Main->strFontName, EmUI::eFont);
				pUIElement->Init(pFontInfo_Main);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::Black);

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pListBox->CreateElement("Main_Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pTextureInfo_Main != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Main->strTextureName);

				IUIElement* pUIElement = pListBox->CreateElement(pTextureInfo_Main->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Main);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Main->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "DropDown_bg";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pListBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pFontInfo_Selection != nullptr)
			{
				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(pFontInfo_Selection->strFontName);

				IUIElement* pUIElement = pListBox->CreateElement(pFontInfo_Selection->strFontName, EmUI::eFont);
				pUIElement->Init(pFontInfo_Selection);
				pUIElement->SetFontNode(pFondNode);
			}
			else
			{
				CreateUIElementFontInfo fintInfo;
				fintInfo.strFontName = "Default";
				fintInfo.fontColor.Init(Math::Color::White);

				UIFontNode* pFondNode = m_pUIMgr->GetFontNode(fintInfo.strFontName);

				IUIElement* pUIElement = pListBox->CreateElement("Selection_Text", EmUI::eFont);
				pUIElement->Init(&fintInfo);
				pUIElement->SetFontNode(pFondNode);
			}

			if (pTextureInfo_Selection != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Selection->strTextureName);

				IUIElement* pUIElement = pListBox->CreateElement(pTextureInfo_Selection->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Selection);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Selection->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "Selection_Inside";
				textureInfo.textureColor.Init(Math::Color::White);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pListBox->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pParent == nullptr)
			{
				m_umapUIObject.insert(std::make_pair(strID, pListBox));
			}
			else
			{
				pParent->AddChild(pListBox->GetID(), pListBox);
			}

			return pListBox;
		}

		CUIScrollBar* CUIPanel::AddScrollBar(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight,
			CreateUIElementTextureInfo* pTextureInfo_Track, CreateUIElementTextureInfo* pTextureInfo_UpArrow,
			CreateUIElementTextureInfo* pTextureInfo_DownArrow, CreateUIElementTextureInfo* pTextureInfo_Button)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				if (iter->second->GetType() == EmUI::eScrollBar)
					return static_cast<CUIScrollBar*>(iter->second);

				return nullptr;
			}

			CUIScrollBar* pScrollBar = new CUIScrollBar(this, strID);

			pScrollBar->SetPosition(x, y);
			pScrollBar->SetSize(nWidth, nHeight);

			if (pTextureInfo_Track != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Track->strTextureName);

				IUIElement* pUIElement = pScrollBar->CreateElement(pTextureInfo_Track->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Track);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Track->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "ScrollTrack";
				textureInfo.textureColor.Init(Math::Color::Black);
				textureInfo.textureColor.SetColor(EmUI::eDisabled, Math::Color(0.7843f, 0.7843f, 0.7843f, 1.f));

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pScrollBar->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_UpArrow != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_UpArrow->strTextureName);

				IUIElement* pUIElement = pScrollBar->CreateElement(pTextureInfo_UpArrow->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_UpArrow);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_UpArrow->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "UpArrow";
				textureInfo.textureColor.Init(Math::Color::Black);
				textureInfo.textureColor.SetColor(EmUI::eDisabled, Math::Color(0.7843f, 0.7843f, 0.7843f, 1.f));

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pScrollBar->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_DownArrow != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_DownArrow->strTextureName);

				IUIElement* pUIElement = pScrollBar->CreateElement(pTextureInfo_DownArrow->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_DownArrow);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_DownArrow->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "DownArrow";
				textureInfo.textureColor.Init(Math::Color::Black);
				textureInfo.textureColor.SetColor(EmUI::eDisabled, Math::Color(0.7843f, 0.7843f, 0.7843f, 1.f));

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pScrollBar->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pTextureInfo_Button != nullptr)
			{
				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(pTextureInfo_Button->strTextureName);

				IUIElement* pUIElement = pScrollBar->CreateElement(pTextureInfo_Button->strElementName, EmUI::eTexture);
				pUIElement->Init(pTextureInfo_Button);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(pTextureInfo_Button->strRectName));
			}
			else
			{
				CreateUIElementTextureInfo textureInfo;
				textureInfo.strTextureName = "Default";
				textureInfo.strRectName = "ScrollButton";
				textureInfo.textureColor.Init(Math::Color::Black);

				UITextureNode* pTextureNode = m_pUIMgr->GetTextureNode(textureInfo.strTextureName);

				IUIElement* pUIElement = pScrollBar->CreateElement(textureInfo.strRectName, EmUI::eTexture);
				pUIElement->Init(&textureInfo);
				pUIElement->SetTextureNode(pTextureNode);
				pUIElement->SetTextureRect(*pTextureNode->GetRect(textureInfo.strRectName));
			}

			if (pParent == nullptr)
			{
				m_umapUIObject.insert(std::make_pair(strID, pScrollBar));
			}
			else
			{
				pParent->AddChild(pScrollBar->GetID(), pScrollBar);
			}

			return pScrollBar;
		}

		void CUIPanel::Remove(const String::StringID& strID)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter != m_umapUIObject.end())
			{
				SafeReleaseDelete(iter->second);
				iter = m_umapUIObject.erase(iter);
			}
		}

		IUIObject* CUIPanel::GetUIObject(const String::StringID& strID)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter == m_umapUIObject.end())
				return nullptr;

			return iter->second;
		}

		void CUIPanel::SetFocusUI(const String::StringID& strID)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter == m_umapUIObject.end())
				return;

			m_pUIObjectFocus = iter->second;

			Invalidate();
		}

		IUIObject* CUIPanel::GetControlAtPoint(const POINT& pt)
		{
			for (auto iter = m_umapUIObject.begin(); iter != m_umapUIObject.end(); ++iter)
			{
				IUIObject* pUIObject = iter->second;

				if (pUIObject->IsContainsPoint(pt) && pUIObject->IsEnable() && pUIObject->IsVisible())
					return pUIObject;
			}

			return nullptr;
		}

		void CUIPanel::onMouseMove(const POINT& pt)
		{
			IUIObject* pControl = GetControlAtPoint(pt);

			if (pControl == m_pUIObjectMouseOver)
				return;

			if (m_pUIObjectMouseOver)
			{
				m_pUIObjectMouseOver->OnMouseLeave();
				Invalidate();
			}

			m_pUIObjectMouseOver = pControl;
			if (pControl)
			{
				m_pUIObjectMouseOver->OnMouseEnter();
				Invalidate();
			}
		}
	}
}