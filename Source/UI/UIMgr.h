#pragma once

#include "CommonLib/Singleton.h"

#include "IUIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		class UIManager : public Singleton<UIManager>
		{
			friend Singleton<UIManager>;
		private:
			UIManager();
			virtual ~UIManager();

		public:
			bool Init(HWND hWnd);
			void Release();

			void Update(float fElapsedTime);

			bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			IUIPanel* AddPanel(String::StringID strID, int x, int y, uint32_t nWidth, uint32_t nHeight);

			/*public:
				IUIObject* ShowUI(String::StringID strID);
				void UnShowUI(String::StringID strID);
				void UnShowUI(IUIObject* pUIObject);*/

		public:
			bool IsMouseOverOnUI();
			bool IsMouseClickOnUI();

		public:
			HWND GetHWND() { return m_hWnd; }

			/*IUIObject* GetMouseOverUI();
			void SetMouseOverUI(String::StringID strID);
			void SetMouseOverUI(IUIObject* pUIObject);*/

			IUIPanel* GetPanelAtPoint(const POINT& pt);
			IUIObject* GetControlAtPoint(const POINT& pt);

			UIFontNode* GetFontNode(String::StringID strNodeName);
			UITextureNode* GetTextureNode(String::StringID strNodeName);

		private:
			bool loadFont();
			bool loadTexture();

		private:
			HWND m_hWnd;

			std::unordered_map<String::StringID, IUIPanel*> m_umapPanel;

			std::vector<UIFontNode*> m_vecFontNode;
			std::vector<UITextureNode*> m_vecTextureNode;

			bool m_bInit;
		};
	}
}