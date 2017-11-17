#pragma once

#include "IUIObject.h"
#include "UIStatic.h"
#include "UIButton.h"
#include "UICheckBox.h"
#include "UIRadioButton.h"
#include "UIComboBox.h"
#include "UISlider.h"
#include "UIEditBox.h"
#include "UIListBox.h"
#include "UIScrollBar.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IRenderTarget;
		class ITexture;
	}

	namespace UI
	{
		class IUIPanel
		{
		public:
			IUIPanel();
			virtual ~IUIPanel();

			virtual void Update(float fElapsedTime) = 0;

			virtual bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) = 0;

			virtual void ClearRadioButtonGroup(uint32_t nGroupID) = 0;

			virtual void RequestFocus(IUIObject* pUIObject) = 0;
			virtual void ClearFocus() = 0;

			virtual void RequestMousePressed(IUIObject* pUIObject) = 0;
			virtual void ClearMousePressed() = 0;

			virtual void SetPosSize(const Math::Rect& rect) = 0;
			virtual int GetPosX() = 0;
			virtual int GetPosY() = 0;
			virtual uint32_t GetWidth() = 0;
			virtual uint32_t GetHeight() = 0;
			virtual void Invalidate() = 0;

		public:
			virtual void DrawText(IUIElement* pElement, const wchar_t* wstrText, Math::Rect& rect, float fDepth) = 0;
			virtual void DrawText(IUIElement* pElement, const std::wstring& wstrText, Math::Rect& rect, float fDepth) = 0;
			virtual void DrawText(IUIElement* pElement, const char* strText, Math::Rect& rect, float fDepth) = 0;
			virtual void DrawText(IUIElement* pElement, const std::string& strText, Math::Rect& rect, float fDepth) = 0;
			virtual void DrawSprite(IUIElement* pElement, Math::Rect& rect, float fDepth) = 0;

		private:
			virtual void drawPanel(std::shared_ptr<Graphics::ITexture> pTexture, const Math::Rect& rect, Math::Rect* sourceRect, const Math::Vector2& origin, const Math::Color& color, float fRotation, float fDepth) = 0;

		public:
			virtual CUIStatic* AddStatic(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight,
				CreateUIElementFontInfo* pFontInfo = nullptr) = 0;
			virtual CUIButton* AddButton(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight,
				CreateUIElementFontInfo* pFontInfo_Button = nullptr, CreateUIElementTextureInfo* pTextureInfo_Button = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_FillLayer = nullptr) = 0;
			virtual CUICheckBox* AddCheckBox(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight, bool bCheck = false,
				CreateUIElementFontInfo* pFontInfo_Title = nullptr, CreateUIElementTextureInfo* pTextureInfo_Box = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Check = nullptr) = 0;
			virtual CUIRadioButton* AddRadioButton(IUIObject* pParent, const String::StringID& strID, const char* strText, int x, int y, uint32_t nWidth, uint32_t nHeight, uint32_t nGroupID, bool bCheck = false,
				CreateUIElementFontInfo* pFontInfo_Title = nullptr, CreateUIElementTextureInfo* pTextureInfo_Box = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Radio = nullptr) = 0;
			virtual CUIComboBox* AddComboBox(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight, uint32_t nDropHeight,
				CreateUIElementFontInfo* pFontInfo_Title = nullptr, CreateUIElementTextureInfo* pTextureInfo_Main = nullptr,
				CreateUIElementFontInfo* pFontInfo_DropDown = nullptr, CreateUIElementTextureInfo* pTextureInfo_DropDown = nullptr,
				CreateUIElementFontInfo* pFontInfo_Selection = nullptr, CreateUIElementTextureInfo* pTextureInfo_Selection = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Button = nullptr) = 0;
			virtual CUISlider* AddSlider(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight,
				int nMin = 0, int nMax = 100,
				CreateUIElementTextureInfo* pTextureInfo_Track = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Button = nullptr) = 0;
			virtual CUIEditBox* AddEditBox(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight,
				CreateUIElementFontInfo* pFontInfo = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Inside = nullptr, CreateUIElementTextureInfo* pTextureInfo_LeftTop = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Top = nullptr, CreateUIElementTextureInfo* pTextureInfo_RightTop = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_Left = nullptr, CreateUIElementTextureInfo* pTextureInfo_Right = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_LeftBottom = nullptr, CreateUIElementTextureInfo* pTextureInfo_Bottom = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_RightBottom = nullptr, CreateUIElementTextureInfo* pTextureInfo_Caret = nullptr) = 0;
			virtual CUIListBox* AddListBox(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight, EmListBox::Style emListBoxStyle,
				CreateUIElementFontInfo* pFontInfo_Main = nullptr, CreateUIElementTextureInfo* pTextureInfo_Main = nullptr,
				CreateUIElementFontInfo* pFontInfo_Selection = nullptr, CreateUIElementTextureInfo* pTextureInfo_Selection = nullptr) = 0;
			virtual CUIScrollBar* AddScrollBar(IUIObject* pParent, const String::StringID& strID, int x, int y, uint32_t nWidth, uint32_t nHeight,
				CreateUIElementTextureInfo* pTextureInfo_Track = nullptr, CreateUIElementTextureInfo* pTextureInfo_UpArrow = nullptr,
				CreateUIElementTextureInfo* pTextureInfo_DownArrow = nullptr, CreateUIElementTextureInfo* pTextureInfo_Button = nullptr) = 0;

			virtual void Remove(const String::StringID& strID) = 0;

		public:
			virtual IUIObject* GetUIObject(const String::StringID& strID) = 0;

			virtual IUIObject* GetFocusUI() = 0;
			virtual void SetFocusUI(const String::StringID& strID) = 0;
			virtual void SetFocusUI(IUIObject* pUIObject) = 0;

			virtual IUIObject* GetPressedUI() = 0;
			virtual IUIObject* GetMouseOverUI() = 0;

			virtual bool IsVisible() = 0;
			virtual void SetVisible(bool bVisible) = 0;

			virtual bool IsKeyboardInput() = 0;
			virtual void SetKeyboardInput(bool bKeyboardInput) = 0;

			virtual bool IsMouseInput() = 0;
			virtual void SetMouseInput(bool bMouseInput) = 0;

		public:
			virtual HWND GetHWND() = 0;

			virtual IUIObject* GetControlAtPoint(const POINT& pt) = 0;
			virtual bool IsContainsPoint(const POINT& pt) = 0;

			virtual Graphics::IRenderTarget* GetRenderTarget() = 0;

		private:
			virtual void onMouseMove(const POINT& pt) = 0;
		};
	}
}