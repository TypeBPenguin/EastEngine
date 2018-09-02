#include "stdafx.h"
#include "Log.h"

#include "Lock.h"

namespace eastengine
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

			void Output(const char* msg, int length, WORD textColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
			{
				thread::AutoLock autoLock(&m_lock);

				createConsole();

				SetConsoleTextAttribute(m_hConsole, textColor | m_wBackgroundAttributes);

				DWORD len = 0;
				WriteConsoleA(m_hConsole, msg, length, &len, nullptr);

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
			thread::Lock m_lock;

			HANDLE m_hConsole;
			WORD m_wDefaultConsoleTextAttributes;
			WORD m_wBackgroundAttributes;
		};

		SLog s_consoleLog;

		const char s_strMessage[] = "[Message] ";
		const char s_strWarning[] = "[Warning] ";
		const char s_strError[] = "[Error] ";
		const char s_strInfo1[] = " -> [";
		const char s_strInfo2[] = " <";
		const char s_strInfo3[] = ">]\n";

		void Message(const char* file, int line, const char* msg, ...)
		{
			va_list args;
			va_start(args, msg);
			int size = std::vsnprintf(nullptr, 0, msg, args) + 1;
			va_end(args);

			std::unique_ptr<char[]> buf = std::make_unique<char[]>(size);
			va_start(args, msg);
			std::vsnprintf(buf.get(), size, msg, args);
			va_end(args);

			const size_t nInfoLength = sizeof(s_strMessage) + sizeof(s_strInfo1) + sizeof(s_strInfo2) + sizeof(s_strInfo3);
			std::string strLine = std::to_string(line);

			std::string strBuffer;
			strBuffer.resize(static_cast<size_t>(size) + nInfoLength + strlen(file) + strLine.size());

			strBuffer = s_strMessage;
			strBuffer.append(buf.get());
			strBuffer.append(s_strInfo1);
			strBuffer.append(file);
			strBuffer.append(s_strInfo2);
			strBuffer.append(strLine);
			strBuffer.append(s_strInfo3);

			s_consoleLog.Output(strBuffer.c_str(), static_cast<int>(strBuffer.size()));
		}

		void Warning(const char* file, int line, const char* msg, ...)
		{
			va_list args;
			va_start(args, msg);
			int size = std::vsnprintf(nullptr, 0, msg, args) + 1;
			va_end(args);

			std::unique_ptr<char[]> buf = std::make_unique<char[]>(size);
			va_start(args, msg);
			std::vsnprintf(buf.get(), size, msg, args);
			va_end(args);

			const size_t nInfoLength = sizeof(s_strWarning) + sizeof(s_strInfo1) + sizeof(s_strInfo2) + sizeof(s_strInfo3);
			std::string strLine = std::to_string(line);

			std::string strBuffer;
			strBuffer.resize(static_cast<size_t>(size) + nInfoLength + strlen(file) + strLine.size());

			strBuffer = s_strWarning;
			strBuffer.append(buf.get());
			strBuffer.append(s_strInfo1);
			strBuffer.append(file);
			strBuffer.append(s_strInfo2);
			strBuffer.append(strLine);
			strBuffer.append(s_strInfo3);

			s_consoleLog.Output(strBuffer.c_str(), static_cast<int>(strBuffer.size()), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}

		void Error(const char* file, int line, const char* msg, ...)
		{
			va_list args;
			va_start(args, msg);
			int size = std::vsnprintf(nullptr, 0, msg, args) + 1;
			va_end(args);

			std::unique_ptr<char[]> buf = std::make_unique<char[]>(size);
			va_start(args, msg);
			std::vsnprintf(buf.get(), size, msg, args);
			va_end(args);

			const size_t nInfoLength = sizeof(s_strError) + sizeof(s_strInfo1) + sizeof(s_strInfo2) + sizeof(s_strInfo3);
			std::string strLine = std::to_string(line);

			std::string strBuffer;
			strBuffer.resize(static_cast<size_t>(size) + nInfoLength + strlen(file) + strLine.size());

			strBuffer = s_strError;
			strBuffer.append(buf.get());
			strBuffer.append(s_strInfo1);
			strBuffer.append(file);
			strBuffer.append(s_strInfo2);
			strBuffer.append(strLine);
			strBuffer.append(s_strInfo3);

			s_consoleLog.Output(strBuffer.c_str(), static_cast<int>(strBuffer.size()), FOREGROUND_RED | FOREGROUND_INTENSITY);
		}
	}
}