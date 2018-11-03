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

					// 문자열 조합 완성
					// imgui에서 아직 유니코드 문자열이 입력되는 걸 완벽하게 지원하지 않는다.
					// 그래서 조합 중인 문자열을 imgui 에 넣는 건 힘들다.
					// 일단은 완성된 문자열만 imgui 에 추가하도록 처리.
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