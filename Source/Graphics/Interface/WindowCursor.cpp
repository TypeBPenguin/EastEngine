#include "stdafx.h"
#include "WindowCursor.h"

//  Define min max macros required by GDI+ headers.
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#else
#error max macro is already defined
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#else
#error min macro is already defined
#endif

#include <comdef.h>
#include <atlimage.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi")

//  Undefine min max macros so they won't collide with <limits> header content.
#undef min
#undef max

//  Undefine byte macros so it won't collide with <cstddef> header content.
#undef byte

namespace est
{
	namespace graphics
	{
		void GetMaskBitmaps(HBITMAP hSourceBitmap, COLORREF clrTransparent, HBITMAP& hAndMaskBitmap, HBITMAP& hXorMaskBitmap)
		{
			HDC hDC = ::GetDC(NULL);
			HDC hMainDC = ::CreateCompatibleDC(hDC);
			HDC hAndMaskDC = ::CreateCompatibleDC(hDC);
			HDC hXorMaskDC = ::CreateCompatibleDC(hDC);

			//Get the dimensions of the source bitmap
			BITMAP bm;
			::GetObject(hSourceBitmap, sizeof(BITMAP), &bm);

			hAndMaskBitmap = ::CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight);
			hXorMaskBitmap = ::CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight);

			//Select the bitmaps to DC
			HBITMAP hOldMainBitmap = (HBITMAP)::SelectObject(hMainDC, hSourceBitmap);
			HBITMAP hOldAndMaskBitmap = (HBITMAP)::SelectObject(hAndMaskDC, hAndMaskBitmap);
			HBITMAP hOldXorMaskBitmap = (HBITMAP)::SelectObject(hXorMaskDC, hXorMaskBitmap);

			//Scan each pixel of the souce bitmap and create the masks
			COLORREF MainBitPixel;
			for (int x = 0; x<bm.bmWidth; ++x)
			{
				for (int y = 0; y<bm.bmHeight; ++y)
				{
					MainBitPixel = ::GetPixel(hMainDC, x, y);
					if (MainBitPixel == clrTransparent)
					{
						::SetPixel(hAndMaskDC, x, y, RGB(255, 255, 255));
						::SetPixel(hXorMaskDC, x, y, RGB(0, 0, 0));
					}
					else
					{
						::SetPixel(hAndMaskDC, x, y, RGB(0, 0, 0));
						::SetPixel(hXorMaskDC, x, y, MainBitPixel);
					}
				}
			}

			::SelectObject(hMainDC, hOldMainBitmap);
			::SelectObject(hAndMaskDC, hOldAndMaskBitmap);
			::SelectObject(hXorMaskDC, hOldXorMaskBitmap);

			::DeleteDC(hXorMaskDC);
			::DeleteDC(hAndMaskDC);
			::DeleteDC(hMainDC);

			::ReleaseDC(NULL, hDC);
		}

		HCURSOR CreateCursorFromBitmap(HBITMAP hSourceBitmap, COLORREF clrTransparent, uint32_t xHotspot, uint32_t yHotspot)
		{
			HCURSOR hRetCursor = NULL;

			do
			{
				if (NULL == hSourceBitmap)
				{
					break;
				}

				//Create the AND and XOR masks for the bitmap
				HBITMAP hAndMask = NULL;
				HBITMAP hXorMask = NULL;
				GetMaskBitmaps(hSourceBitmap, clrTransparent, hAndMask, hXorMask);
				if (NULL == hAndMask || NULL == hXorMask)
				{
					break;
				}

				//Create the cursor using the masks and the hotspot values provided
				ICONINFO iconinfo = { 0 };
				iconinfo.fIcon = FALSE;
				iconinfo.xHotspot = xHotspot;
				iconinfo.yHotspot = yHotspot;
				iconinfo.hbmMask = hAndMask;
				iconinfo.hbmColor = hXorMask;

				hRetCursor = ::CreateIconIndirect(&iconinfo);
			} while (0);

			return hRetCursor;
		}

		WindowCursor::WindowCursor()
		{
		}

		WindowCursor::~WindowCursor()
		{
			for (auto iter = m_cursors.begin(); iter != m_cursors.end(); ++iter)
			{
				for (auto hCursor : iter.value().hCursors)
				{
					DestroyCursor(hCursor);
				}
			}
			m_cursors.clear();
		}

		void WindowCursor::Update(float elapsedTime)
		{
			if (m_cursorName.empty() == true)
				return;

			auto iter = m_cursors.find(m_cursorName);
			if (iter == m_cursors.end())
				return;

			Cursor& cursor = iter.value();
			if (math::IsZero(cursor.playTime) == true)
				return;

			m_accumTime += elapsedTime;

			const size_t maxFrame = cursor.hCursors.size();
			size_t curFrame = static_cast<size_t>(m_accumTime / cursor.playTime);
			if (cursor.isLoop == false && curFrame >= maxFrame)
			{
				ChangeCursor(m_defaultCursorName);
			}
			else
			{
				if (maxFrame > 1)
				{
					curFrame = curFrame & maxFrame;
				}
				else
				{
					curFrame = 0;
				}
				SetFrame(m_cursorName, curFrame);
			}
		}

		bool WindowCursor::LoadCursor(const string::StringID& key, const std::vector<ImageData>& imageDatas, uint32_t hotSpotX, uint32_t hotSpotY, float playTime, bool isLoop)
		{
			if (imageDatas.empty() == true)
				return false;

			Cursor cursor;
			cursor.hotSpotX = hotSpotX;
			cursor.hotSpotY = hotSpotY;
			cursor.playTime = playTime;
			cursor.isLoop = isLoop;

			for (auto& imageData : imageDatas)
			{
				if (imageData.pData == nullptr || imageData.dataSize == 0)
					return false;

				auto const stream{ ::SHCreateMemStream(reinterpret_cast<const BYTE*>(imageData.pData), imageData.dataSize) };
				if (stream == nullptr)
					return false;

				_COM_SMARTPTR_TYPEDEF(IStream, __uuidof(IStream));
				IStreamPtr sp_stream{ stream, false };

				CImage img{};
				if (FAILED(img.Load(sp_stream)))
					return false;

				HBITMAP hBitmap = img.Detach();
				cursor.hCursors.emplace_back(CreateCursorFromBitmap(hBitmap, RGB(255, 0, 255), hotSpotX, hotSpotY));
				DeleteObject(hBitmap);
			}

			m_cursors.emplace(key, cursor);

			return true;
		}

		bool WindowCursor::IsHasCursor(const string::StringID& key) const
		{
			return m_cursors.find(key) != m_cursors.end();
		}

		void WindowCursor::ChangeCursor(const string::StringID& key)
		{
			auto iter = m_cursors.find(key);
			if (iter == m_cursors.end())
				return;

			if (m_cursorName != key)
			{
				m_cursorName = key;
				ResetFrame();
			}
		}

		void WindowCursor::SetVisible(bool isVisible)
		{
			m_isVisible = isVisible;

			if (m_isVisible == false)
			{
				SetCursor(nullptr);
			}
			else
			{
				auto iter = m_cursors.find(m_cursorName);
				if (iter == m_cursors.end())
					return;

				Cursor& cursor = iter.value();
				SetCursor(cursor.hCursors[0]);
			}
		}

		void WindowCursor::SetFrame(const string::StringID& key, size_t frame)
		{
			auto iter = m_cursors.find(key);
			if (iter == m_cursors.end())
				return;

			Cursor& cursor = iter.value();
			if (frame >= cursor.hCursors.size())
				return;

			SetCursor(cursor.hCursors[frame]);
		}

		void WindowCursor::ResetFrame()
		{
			m_accumTime = 0.f;
			SetFrame(m_cursorName, 0);
		}
	}
}