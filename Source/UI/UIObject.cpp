#include "stdafx.h"
#include "UIObject.h"

#include "IUIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		CUIObject::CUIObject(IUIPanel* pUIPanel, String::StringID strID, EmUI::Type emType)
			: m_pUIPanel(pUIPanel)
			, m_strID(strID)
			, m_emType(emType)
			, m_bVisible(true)
			, m_bMouseOver(false)
			, m_bHasFocus(false)
			, m_bDefault(false)
			, m_bEnable(true)
			, m_bMoveAble(false)
			, m_bPressed(false)
			, m_bResizeAble(false)
			, m_nHotKey(0)
			, m_nPosX(0)
			, m_nPosY(0)
			, m_nWidth(0)
			, m_nHeight(0)
			, m_pParent(nullptr)
		{
			SetRectEmpty(&m_rcBoundingBox);
		}

		CUIObject::~CUIObject()
		{
			Release();
		}

		void CUIObject::Release()
		{
			std::for_each(m_vecElements.begin(), m_vecElements.end(), DeleteSTLObject());
			m_vecElements.clear();

			std::for_each(m_umapChildUI.begin(), m_umapChildUI.end(), ReleaseDeleteSTLMapObject());
			m_umapChildUI.clear();
		}

		IUIElement* CUIObject::CreateElement(String::StringID strID, EmUI::ElementType emType)
		{
			uint32_t nSize = m_vecElements.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				if (strID == m_vecElements[i]->GetID())
					return m_vecElements[i];
			}

			IUIElement* pElement = nullptr;

			switch (emType)
			{
			case EmUI::eFont:
			{
				pElement = new CUIElementFont(strID);
				break;
			}
			case EmUI::eTexture:
			{
				pElement = new CUIElementTexture(strID);
				break;
			}
			}

			m_vecElements.push_back(pElement);

			return pElement;
		}

		IUIElement* CUIObject::GetElement(String::StringID strID)
		{
			uint32_t nSize = m_vecElements.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				if (strID == m_vecElements[i]->GetID())
					return m_vecElements[i];
			}

			return nullptr;
		}

		void CUIObject::Invalidate()
		{
			m_pUIPanel->Invalidate();
		}

		IUIObject* CUIObject::GetChildByID(String::StringID strID)
		{
			auto iter = m_umapChildUI.find(strID);
			if (iter == m_umapChildUI.end())
			{
				for (auto iter_child = m_umapChildUI.begin(); iter_child != m_umapChildUI.end(); ++iter_child)
				{
					return iter_child->second->GetChildByID(strID);
				}
			}

			return iter->second;
		}
	}
}