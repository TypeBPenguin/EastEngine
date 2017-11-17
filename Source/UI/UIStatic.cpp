#include "stdafx.h"
#include "UIStatic.h"

#include "UIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		CUIStatic::CUIStatic(IUIPanel* pUIPanel, String::StringID strID, EmUI::Type emUIType)
			: CUIObject(pUIPanel, strID, emUIType)
		{
		}

		CUIStatic::~CUIStatic()
		{
		}

		void CUIStatic::Update(float fElapsedTime)
		{
			if (!m_bVisible)
				return;

			float fFocusDepth = m_bHasFocus ? -0.5f : 0.f;

			bool bNeedInvalidate = false;

			EmUI::State emState = EmUI::eNormal;

			if (!m_bEnable)
				emState = EmUI::eDisabled;

			IUIElement* pElement = m_vecElements[0];

			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime) == false || bNeedInvalidate;

			m_pUIPanel->DrawText(pElement, m_strText, m_rcBoundingBox, UI_FAR_DEPTH + fFocusDepth);

			if (bNeedInvalidate)
			{
				Invalidate();
			}
		}
	}
}