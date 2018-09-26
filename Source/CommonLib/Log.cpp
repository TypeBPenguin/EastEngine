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
			SLog() = default;
			~SLog()
			{
				{
					std::lock_guard<std::mutex> lock(m_mutex);
					m_isStop = true;
				}
				m_condition.notify_one();

				m_thread.join();

				if (m_hConsole != nullptr)
				{
					m_hConsole = nullptr;
					FreeConsole();
				}
			}

			void Push(const std::string& message, WORD textColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
			{
				{
					std::lock_guard<std::mutex> lock(m_mutex);

					CreateConsole();

					m_queueMessages.emplace(message, textColor);
				}
				m_condition.notify_one();
			}

		private:
			struct LogMessage
			{
				std::string message;
				WORD color{ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY };

				LogMessage() = default;
				LogMessage(const std::string& message, WORD color)
					: message(message)
					, color(color)
				{
				}

				LogMessage(LogMessage&& source) noexcept
					: message(std::move(source.message))
					, color(std::move(source.color))
				{
				}

				void operator = (const LogMessage& source)
				{
					message = source.message;
					color = source.color;
				}
			};

		private:
			void Output(const LogMessage& message)
			{
				SetConsoleTextAttribute(m_hConsole, message.color | m_wBackgroundAttributes);

				DWORD len = 0;
				WriteConsoleA(m_hConsole, message.message.c_str(), message.message.length(), &len, nullptr);

				SetConsoleTextAttribute(m_hConsole, m_wDefaultConsoleTextAttributes);
			}

			void CreateConsole()
			{
				if (m_hConsole != nullptr && m_hConsole != INVALID_HANDLE_VALUE)
					return;

				AllocConsole();

				m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

				CONSOLE_SCREEN_BUFFER_INFO csbi;
				GetConsoleScreenBufferInfo(m_hConsole, &csbi);
				m_wDefaultConsoleTextAttributes = csbi.wAttributes;
				m_wBackgroundAttributes = m_wDefaultConsoleTextAttributes & 0x00F0;

				m_thread = std::thread([&]()
				{
					LogMessage message;

					while (true)
					{
						{
							std::unique_lock<std::mutex> lock(m_mutex);
							m_condition.wait(lock, [&]()
							{
								return m_isStop == true || m_queueMessages.empty() == false;
							});

							if (m_isStop == true)
								break;

							message = m_queueMessages.front();
							m_queueMessages.pop();
						}

						Output(message);
					}
				});
			}

		private:
			HANDLE m_hConsole{ nullptr };
			WORD m_wDefaultConsoleTextAttributes;
			WORD m_wBackgroundAttributes;

			std::thread m_thread;

			std::mutex m_mutex;
			std::condition_variable m_condition;

			bool m_isStop{ false };
			std::queue<LogMessage> m_queueMessages;
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

			s_consoleLog.Push(strBuffer);
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

			s_consoleLog.Push(strBuffer, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
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

			s_consoleLog.Push(strBuffer, FOREGROUND_RED | FOREGROUND_INTENSITY);
		}
	}
}