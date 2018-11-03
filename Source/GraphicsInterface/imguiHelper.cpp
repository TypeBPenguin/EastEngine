#include "stdafx.h"
#include "imguiHelper.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace eastengine
{
	namespace imguiHelper
	{
		bool MessageHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
		{
			static std::string strPrevUniCode;

			switch (nMsg)
			{
			case WM_IME_STARTCOMPOSITION:
				return true;
			case WM_IME_ENDCOMPOSITION:
				return false;
			case WM_IME_COMPOSITION:
			{
				HIMC himc = ImmGetContext(hWnd);
				if (himc != nullptr)
				{
					long lRet = 0;
					wchar_t szCompStr[256] = { 0 };

					// ���ڿ� ���� �ϼ�
					// imgui���� ���� �����ڵ� ���ڿ��� �ԷµǴ� �� �Ϻ��ϰ� �������� �ʴ´�.
					// �׷��� ���� ���� ���ڿ��� imgui �� �ִ� �� �����.
					// �ϴ��� �ϼ��� ���ڿ��� imgui �� �߰��ϵ��� ó��.
					if (lParam & GCS_RESULTSTR)
					{
						lRet = ImmGetCompositionStringW(himc, GCS_RESULTSTR, szCompStr, 256) / sizeof(wchar_t);
						szCompStr[lRet] = 0;

						strPrevUniCode = eastengine::string::WideToMulti(szCompStr);

						if (lRet > 0)
						{
							ImGui_ImplWin32_WndProcHandler(hWnd, WM_CHAR, static_cast<WPARAM>(szCompStr[0]), lParam);
						}
					}
				}
				ImmReleaseContext(hWnd, himc);
				return true;
			}
			break;
			default:
			{
				if (nMsg == WM_CHAR && strPrevUniCode.empty() == false)
				{
					static int nIdx = 0;
					static WPARAM temp = 0;
					if (strPrevUniCode[nIdx] == static_cast<char>(wParam))
					{
						if (nIdx == 0)
						{
							temp = wParam;
							++nIdx;
						}
						else
						{
							strPrevUniCode.clear();
							nIdx = 0;
						}
					}
					else
					{
						strPrevUniCode.clear();

						if (nIdx != 0)
						{
							ImGui_ImplWin32_WndProcHandler(hWnd, WM_CHAR, temp, lParam);

							temp = 0;
							nIdx = 0;
						}

						return ImGui_ImplWin32_WndProcHandler(hWnd, nMsg, wParam, lParam) > 0;
					}
				}
				else
				{
					return ImGui_ImplWin32_WndProcHandler(hWnd, nMsg, wParam, lParam) > 0;
				}
			}
			break;
			}

			return false;
		}
	}
}