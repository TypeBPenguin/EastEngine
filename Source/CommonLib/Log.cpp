#include "stdafx.h"
#include "Log.h"

namespace EastEngine
{
	namespace Log
	{
		class SLog
		{
		public:
			SLog()
				: m_hConsole(nullptr)
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

				SetConsoleTextAttribute(m_hConsole, textColor | m_wBackgroundAttributes);

				DWORD len = 0;
				WriteConsoleA(m_hConsole, msg, strlen(msg), &len, nullptr);

				SetConsoleTextAttribute(m_hConsole, m_wDefaultConsoleTextAttributes);
			}

		private:
			void createConsole()
			{
				if (m_hConsole != nullptr && m_hConsole != INVALID_HANDLE_VALUE)
					return;

				AllocConsole();

				m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

				CONSOLE_SCREEN_BUFFER_INFO csbi;
				GetConsoleScreenBufferInfo(m_hConsole, &csbi);
				m_wDefaultConsoleTextAttributes = csbi.wAttributes;
				m_wBackgroundAttributes = m_wDefaultConsoleTextAttributes & 0x00F0;
			}

		private:
			std::mutex mutex;
			HANDLE m_hConsole;
			WORD m_wDefaultConsoleTextAttributes;
			WORD m_wBackgroundAttributes;
		};

		SLog s_consoleLog;

		void Message(const char* msg, ...)
		{
			va_list args;
			va_start(args, msg);
			int size = std::vsnprintf(nullptr, 0, msg, args) + 1;
			va_end(args);

			std::unique_ptr<char[]> buf(new char[size]);
			va_start(args, msg);
			std::vsnprintf(buf.get(), size, msg, args);
			va_end(args);

			s_consoleLog.Output(buf.get());
		}

		void Warning(const char* msg, ...)
		{
			va_list args;
			va_start(args, msg);
			int size = std::vsnprintf(nullptr, 0, msg, args) + 1;
			va_end(args);

			std::unique_ptr<char[]> buf(new char[size]);
			va_start(args, msg);
			std::vsnprintf(buf.get(), size, msg, args);
			va_end(args);

			s_consoleLog.Output(buf.get(), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}

		void Error(const char* msg, ...)
		{
			va_list args;
			va_start(args, msg);
			int size = std::vsnprintf(nullptr, 0, msg, args) + 1;
			va_end(args);

			std::unique_ptr<char[]> buf(new char[size]);
			va_start(args, msg);
			std::vsnprintf(buf.get(), size, msg, args);
			va_end(args);

			s_consoleLog.Output(buf.get(), FOREGROUND_RED | FOREGROUND_INTENSITY);
		}
	}
}