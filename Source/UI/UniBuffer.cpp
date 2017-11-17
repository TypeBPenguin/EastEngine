#include "stdafx.h"
#include "UniBuffer.h"

#include "../DirectX/ISpriteFont.h"

namespace EastEngine
{
	namespace UI
	{
		CUniBuffer::CUniBuffer(int nInitSize)
			: m_pFont(nullptr)
		{
			if (nInitSize > 0)
			{
				SetBufferSize(nInitSize);
			}
		}

		CUniBuffer::~CUniBuffer()
		{
			m_wstrBuffer.clear();
			m_pFont = nullptr;
		}

		bool CUniBuffer::SetBufferSize(_In_ int nNewSize)
		{
			// If the current size is already the maximum allowed,
			// we can't possibly allocate more.
			int nCapacity = (int)(m_wstrBuffer.capacity());
			if (nCapacity >= MAX_EDITBOXLENGTH)
				return false;

			int nAllocateSize = (nNewSize == -1 || nNewSize < nCapacity * 2) ? (nCapacity ? nCapacity * 2 : 256) : nNewSize * 2;

			// Cap the buffer size at the maximum allowed.
			if (nAllocateSize > MAX_EDITBOXLENGTH)
				nAllocateSize = MAX_EDITBOXLENGTH;

			m_wstrBuffer.reserve(nAllocateSize);

			return true;
		}

		void CUniBuffer::Clear()
		{
			m_wstrBuffer.clear();
		}

		bool CUniBuffer::InsertChar(_In_ int nIndex, _In_ wchar_t str)
		{
			int nLength = (int)(m_wstrBuffer.length());

			if (nIndex < 0 || nIndex > nLength)
				return false;  // invalid index

			// Check for maximum length allowed
			if (nLength + 1 >= MAX_EDITBOXLENGTH)
				return false;

			if (nLength + 1 >= (int)(m_wstrBuffer.capacity()))
			{
				if (!SetBufferSize(-1))
					return false;  // out of memory
			}

			if (nIndex == nLength)
			{
				m_wstrBuffer.push_back(str);
			}
			else
			{
				m_wstrBuffer.insert(m_wstrBuffer.begin() + nIndex, str);
			}

			return true;
		}

		bool CUniBuffer::RemoveChar(_In_ int nIndex)
		{
			if (m_wstrBuffer.empty() || nIndex < 0 || nIndex >= (int)(m_wstrBuffer.length()))
				return false;

			m_wstrBuffer.replace(nIndex, 1, L"");

			return true;
		}

		bool CUniBuffer::InsertString(int nIndex, const wchar_t* pStr, int nCount)
		{
			if (nIndex < 0)
				return false;

			int nLength = (int)(m_wstrBuffer.length());

			if (nIndex > nLength)
				return false;  // invalid index

			if (-1 == nCount)
				nCount = (int)(wcslen(pStr));

			// Check for maximum length allowed
			if (nLength + nCount >= MAX_EDITBOXLENGTH)
				return false;

			if (nLength + nCount >= (int)(m_wstrBuffer.capacity()))
			{
				if (!SetBufferSize(nLength + nCount + 1))
					return false;  // out of memory
			}

			m_wstrBuffer.insert(nIndex, pStr);

			return true;
		}

		bool CUniBuffer::SetText(_In_z_ const wchar_t* wszText)
		{
			if (wszText == nullptr)
				return false;

			size_t nRequired = wcslen(wszText) + 1;

			if (nRequired >= MAX_EDITBOXLENGTH)
				return false;

			while (GetBufferSize() < nRequired)
			{
				if (!SetBufferSize(-1))
					break;
			}

			if (GetBufferSize() >= nRequired)
			{
				m_wstrBuffer = wszText;
				return true;
			}

			return false;
		}

		int CUniBuffer::GetPosXByIndex(int nIndex)
		{
			// 0번째 인덱스라면 위치가 애초에 0 이므로 <= 사용
			if (nIndex <= 0 || nIndex > (int)(m_wstrBuffer.size()))
				return 0;

			int nSize = (int)(m_wstrBuffer.size());
			float fPosX = 0.f;
			int i = 0;
			while (1)
			{
				auto glyph = m_pFont->pSpriteFont->FindGlyph(m_wstrBuffer[i]);

				fPosX += glyph->XOffset < 0 ? 0 : glyph->XOffset;

				if (i >= nIndex)
				{
					if (iswspace(m_wstrBuffer[i]) &&
						glyph->Subrect.GetWidth() <= 1 &&
						glyph->Subrect.GetHeight() <= 1)
					{
						fPosX += (float)(glyph->Subrect.GetWidth()) + glyph->XAdvance;
					}
					break;
				}

				if (i + 1 >= nSize)
				{
					if (!iswspace(m_wstrBuffer[i]) ||
						glyph->Subrect.GetWidth() > 1 ||
						glyph->Subrect.GetHeight() > 1)
					{
						fPosX += (float)(glyph->Subrect.GetWidth());
					}
					break;
				}

				fPosX += (float)(glyph->Subrect.GetWidth()) + glyph->XAdvance;

				++i;
			}

			return (int)(fPosX);
		}

		int CUniBuffer::GetIndexByPosX(int nX)
		{
			float fPosX = 0.f;
			uint32_t nSize = m_wstrBuffer.size();
			uint32_t nIdx = 0;
			for (; nIdx < nSize; ++nIdx)
			{
				auto glyph = m_pFont->pSpriteFont->FindGlyph(m_wstrBuffer[nIdx]);

				fPosX += glyph->XOffset + (float)(glyph->Subrect.GetWidth()) + glyph->XAdvance;

				if (fPosX >= nX)
					break;
			}

			return nIdx;
		}

		int CUniBuffer::GetPriorTextPos(int nIndex)
		{
			// 0번째 텍스트 이전 위치는 없으니깐 <= 사용
			if (nIndex <= 0 || nIndex > (int)(m_wstrBuffer.size()))
				return 0;

			for (int i = nIndex; i >= 0; --i)
			{
				if (iswspace(m_wstrBuffer[i]))
					return i;
			}

			return 0;
		}

		int CUniBuffer::GetNextTextPos(int nIndex)
		{
			int nSize = (int)(m_wstrBuffer.size());
			if (nIndex < 0 || nIndex > nSize)
				return nSize;

			bool bIsSpace = false;
			for (int i = nIndex; i < nSize; ++i)
			{
				if (bIsSpace == false)
				{
					if (iswspace(m_wstrBuffer[i]))
					{
						bIsSpace = true;
					}
				}
				else if (!iswspace(m_wstrBuffer[i]))
				{
					return i;
				}
			}

			return nSize;
		}
	}
}