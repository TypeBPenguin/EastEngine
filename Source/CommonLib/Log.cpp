#include "stdafx.h"
#include "Log.h"

namespace EastEngine
{
	namespace Log
	{
		class SLog
		{
		public:
			SLog() : m_hConsole(nullptr)
			{
			}

			~SLog()
			{
				if (m_hConsole != nullptr)
				{
					m_hConsole = nullptr;
					FreeConsole();
				}
			}

			void Output(const char* msg, WORD textColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
			{
				createConsole();

				SetConsoleTextAttribute(m_hConsole, textColor);
				DWORD len;

				//if (IsUTF8Format())
				//{
				//	std::wstring str = boost:
				//}
				//else
				{
					WriteConsoleA(m_hConsole, msg, strlen(msg), &len, nullptr);
				}

				SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			}

		private:
			void createConsole()
			{
				if (m_hConsole != nullptr && m_hConsole != INVALID_HANDLE_VALUE)
					return;

				AllocConsole();

				FILE* file = nullptr;
				_wfreopen_s(&file, L"CONOUT$", L"wt", stdout);
				m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			}

		private:
			HANDLE m_hConsole;
		};

		SLog s_consoleLog;

		void ConsoleLog(const char* msg, ...)
		{
			static std::mutex mutex;
			std::unique_lock<std::mutex> lock(mutex);

			char buf[65536];

			va_list args;
			va_start(args, msg);
			vsnprintf(buf, 65536, msg, args);
			va_end(args);

			s_consoleLog.Output(buf);
		}
	}
}