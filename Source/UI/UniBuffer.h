#pragma once

#include <usp10.h>

#include "UIDefine.h"

#define MAX_EDITBOXLENGTH 0xFFFF

namespace EastEngine
{
	namespace UI
	{
		class CUniBuffer
		{
		public:
			CUniBuffer(int nInitSize = MAX_EDITBOXLENGTH);
			~CUniBuffer();

			const wchar_t& operator[](_In_ int n) const { return m_wstrBuffer[n]; }
			wchar_t& operator[](_In_ int n) { return m_wstrBuffer[n]; }

			size_t GetBufferSize() const { return m_wstrBuffer.capacity(); }
			bool SetBufferSize(_In_ int nSize);
			int GetTextSize() const { return m_wstrBuffer.length(); }
			const wchar_t* GetBuffer() const { return &m_wstrBuffer[0]; }

			void Clear();

			bool InsertChar(_In_ int nIndex, _In_ wchar_t str);

			bool RemoveChar(_In_ int nIndex);

			bool InsertString(_In_ int nIndex, _In_z_ const wchar_t* pStr, _In_ int nCount = -1);

			bool SetText(_In_z_ const wchar_t* wszText);
			void SetFont(UIFontNode* pFont) { m_pFont = pFont; }

			int GetPosXByIndex(int nIndex);
			int GetIndexByPosX(int nX);
			int GetPriorTextPos(int nIndex);
			int GetNextTextPos(int nIndex);

		private:
			std::wstring m_wstrBuffer;

			UIFontNode* m_pFont;
		};
	}
}