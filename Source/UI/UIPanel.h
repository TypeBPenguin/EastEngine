#pragma once

#include "IUIPanel.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IRenderTarget;
		class ITexture;
	}

	namespace UI
	{
		class UIManager;

		class CUIPanel : public IUIPanel
		{
		public:
			CUIPanel(UIManager* pUIMgr, HWND hWnd);
			virtual ~CUIPanel();

			bool Init(const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight);

			virtual void Update(float fElapsedTime) override;

			virtual bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) override;

			virtual void ClearRadioButtonGroup(uint32_t nGroupID) override;

			virtual void RequestFocus(IUIObject* pUIObject) override;
			virtual void ClearFocus() override;

			virtual void RequestMousePressed(IUIObject* pUIObject) override;
			virtual void ClearMousePressed() override;

			virtual void SetPosSize(const Math::Rect& rect) override { m_rect = rect; m_sourceRect.Set(0, 0, m_rect.GetWidth(), m_rect.GetHeight()); Invalidate(); }
			virtual int GetPosX() override { return m_rect.left; }
			virtual int GetPosY() override { return m_rect.top; }
			virtual uint32_t GetWidth() override { return m_rect.GetWidth(); }
			virtual uint32_t GetHeight() override { return m_rect.GetHeight(); }
			virtual void Invalidate() override { m_bNeedRender = true; }

		public:
			virtual void DrawText(IUIElement* pElement, const wchar_t* wstrText, Math::Rect& rect, float fDepth) override;
			virtual void DrawText(IUIElement* pElement, const std::wstring& wstrText, Math::Rect& rect, float fDepth) override;
			virtual void DrawText(IUIElement* pElement, const char* strText, Math::Rect& rect, float fDepth) override;
			virtual void DrawText(IUIElement* pElement, const std::string& strText, Math::Rect& rect, float fDepth) override;
			virtual void DrawSprite(IUIElement* pElement, Math::Rect& rect, float fDepth) override;

		private:
			virtual void drawPanel(std::shared_ptr<Graphics::ITexture> pTexture, const Math::Rect& rect, Math::Rect* sourceRect, const Math::Vector2& origin, const Math::Color& color, float fRotation, float fDepth) override;

		public:
			virtual CUIStatic* AddStatic(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight,
				CreateUIElementFontInfo* pFontInfo = nullptr) override;
			virtual CUIButton* AddButton(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight,
				CreateUIElementFontInfo* pFontInfo_Button = nullptr, CreateUIElementTextureInfo* pTextureInfo_Button = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_FillLayer = nullptr) override;
			virtual CUICheckBox* AddCheckBox(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight, bool bCheck = false,
				CreateUIElementFontInfo* pFontInfo_Title = nullptr, CreateUIElementTextureInfo* pTextureInfo_Box = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Check = nullptr) override;
			virtual CUIRadioButton* AddRadioButton(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight, uint32_t nGroupID, bool bCheck = false,
				CreateUIElementFontInfo* pFontInfo_Title = nullptr, CreateUIElementTextureInfo* pTextureInfo_Box = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Radio = nullptr) override;
			virtual CUIComboBox* AddComboBox(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight, uint32_t nDropHeight,
				CreateUIElementFontInfo* pFontInfo_Title = nullptr, CreateUIElementTextureInfo* pTextureInfo_Main = nullptr,
				CreateUIElementFontInfo* pFontInfo_DropDown = nullptr, CreateUIElementTextureInfo* pTextureInfo_DropDown = nullptr,
				CreateUIElementFontInfo* pFontInfo_Selection = nullptr, CreateUIElementTextureInfo* pTextureInfo_Selection = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Button = nullptr) override;
			virtual CUISlider* AddSlider(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight,
				int nMin = 0, int nMax = 100,
				CreateUIElementTextureInfo* pTextureInfo_Track = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Button = nullptr) override;
			virtual CUIEditBox* AddEditBox(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight,
				CreateUIElementFontInfo* pFontInfo = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Inside = nullptr, CreateUIElementTextureInfo* pTextureInfo_LeftTop = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Top = nullptr, CreateUIElementTextureInfo* pTextureInfo_RightTop = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Left = nullptr, CreateUIElementTextureInfo* pTextureInfo_Right = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_LeftBottom = nullptr, CreateUIElementTextureInfo* pTextureInfo_Bottom = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_RightBottom = nullptr, CreateUIElementTextureInfo* pTextureInfo_Caret = nullptr) override;
			virtual CUIListBox* AddListBox(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight, EmListBox::Style emListBoxStyle,
				CreateUIElementFontInfo* pFontInfo_Main = nullptr, CreateUIElementTextureInfo* pTextureInfo_Main = nullptr,
				CreateUIElementFontInfo* pFontInfo_Selection = nullptr, CreateUIElementTextureInfo* pTextureInfo_Selection = nullptr) override;
			virtual CUIScrollBar* AddScrollBar(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight,
				CreateUIElementTextureInfo* pTextureInfo_Track = nullptr, CreateUIElementTextureInfo* pTextureInfo_UpArrow = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_DownArrow = nullptr, CreateUIElementTextureInfo* pTextureInfo_Button = nullptr) override;

			virtual void Remove(const String::StringID& strID) override;

		public:
			virtual const String::StringID& GetID() { return m_strID; }
			virtual IUIObject* GetUIObject(const String::StringID& strID) override;

			virtual IUIObject* GetFocusUI() override { return m_pUIObjectFocus; }
			virtual void SetFocusUI(const String::StringID& strID) override;
			virtual void SetFocusUI(IUIObject* pUIObject) override { m_pUIObjectFocus = pUIObject; }

			virtual IUIObject* GetPressedUI() override { return m_pUIObjectPressed; }
			virtual IUIObject* GetMouseOverUI() override { return m_pUIObjectMouseOver; }

			virtual bool IsVisible() override { return m_bVisible; }
			virtual void SetVisible(bool bVisible) override { m_bVisible = bVisible; }

			virtual bool IsKeyboardInput() override { return m_bKeyboardInput; }
			virtual void SetKeyboardInput(bool bKeyboardInput) override { m_bKeyboardInput = bKeyboardInput; }

			virtual bool IsMouseInput() override { return m_bMouseInput; }
			virtual void SetMouseInput(bool bMouseInput) override { m_bMouseInput = bMouseInput; }

		public:
			virtual HWND GetHWND() override { return m_hWnd; }

			virtual IUIObject* GetControlAtPoint(const POINT& pt) override;
			virtual bool IsContainsPoint(const POINT& pt) override { return PtInRect(&m_rect, pt) == TRUE; }

			virtual Graphics::IRenderTarget* GetRenderTarget() override { return m_pRenderTarget; }

		private:
			virtual void onMouseMove(const POINT& pt) override;

		private:
			UIManager* m_pUIMgr;
			HWND m_hWnd;

			String::StringID m_strID;

			Math::Rect m_rect;
			Math::Rect m_sourceRect;
			Graphics::IRenderTarget* m_pRenderTarget;

			std::unordered_map<String::StringID, IUIObject*> m_umapUIObject;
			IUIObject* m_pUIObjectFocus;
			IUIObject* m_pUIObjectPressed;
			IUIObject* m_pUIObjectMouseOver;

			bool m_bNeedRender;
			bool m_bVisible;
			bool m_bDrag;
				 
			bool m_bKeyboardInput;
			bool m_bMouseInput;
		};
	}
}