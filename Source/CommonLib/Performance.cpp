#include "stdafx.h"
#include "Performance.h"

#include "StringTable.h"
#include "Lock.h"
#include "FileUtil.h"
#include "json.hpp"

namespace est
{
	namespace performance
	{
		namespace tracer
		{
			class TracerImpl
			{
			public:
				TracerImpl();
				~TracerImpl();

			public:
				void RefreshState();

			public:
				void Start();
				void End(const wchar_t* saveFilePath);

				void BeginEvent(const wchar_t* title, const wchar_t* category);
				void EndEvent(bool isForceRecord);

				template <typename T>
				void PushArgs(const wchar_t* key, T value)
				{
					if (IsTracing() == false)
						return;

					thread::SRWWriteLock lock(&m_srwLock);

					Event* pEvent = GetLastEvent();
					if (pEvent != nullptr)
					{
						pEvent->args.push_back({ key, value });
					}
				}

				template <>
				void PushArgs(const wchar_t* key, const wchar_t* value)
				{
					if (IsTracing() == false)
						return;

					thread::SRWWriteLock lock(&m_srwLock);

					Event* pEvent = GetLastEvent();
					if (pEvent != nullptr)
					{
						pEvent->args.push_back({ key, std::wstring{ value } });
					}
				}

			public:
				float TracingTime() const;
				bool IsTracing() const;

			private:
				void Save();

			private:
				struct Event
				{
					enum Type
					{
						eBegin = 0,
						eEnd,
					};

					Type emType{ eBegin };

					string::StringID title;
					string::StringID category;

					uint32_t processID{ 0 };
					uint32_t threadID{ 0 };

					std::chrono::high_resolution_clock::time_point time;

					Event() = default;
					Event(Type emType, const string::StringID& title, const string::StringID& category, uint32_t processID, uint32_t threadID, const std::chrono::high_resolution_clock::time_point& time = std::chrono::high_resolution_clock::now())
						: emType(emType)
						, title(title)
						, category(category)
						, processID(processID)
						, threadID(threadID)
						, time(time)
					{
					}

					Event(Event&& source) noexcept
					{
						*this = std::move(source);
					}

					Event& operator = (Event&& source) noexcept
					{
						emType = std::move(source.emType);
						title = std::move(source.title);
						category = std::move(source.category);
						processID = std::move(source.processID);
						threadID = std::move(source.threadID);
						time = std::move(source.time);

						args = std::move(source.args);

						return *this;
					}

					struct Args
					{
						std::wstring key;

						std::variant<
							bool,
							int32_t,
							uint32_t,
							int64_t,
							uint64_t,
							float,
							double,
							std::wstring> variantValue;
					};
					std::vector<Args> args;
				};

				Event* GetLastEvent();

			private:
				thread::SRWLock m_srwLock;

				std::optional<std::chrono::high_resolution_clock::time_point> m_startTime;
				tsl::robin_map<uint32_t, std::vector<Event>> m_umapEvents;
				tsl::robin_map<uint32_t, std::stack<size_t>> m_umapEventLinkers;

				enum State
				{
					eIdle = 0,

					eStart,

					eRequestStart,
					eRequestEnd,
				};

				State m_emState{ eIdle };
				std::wstring m_saveFilePath;
			};

			TracerImpl::TracerImpl()
			{
			}

			TracerImpl::~TracerImpl()
			{
			}

			void TracerImpl::RefreshState()
			{
				switch (m_emState)
				{
				case eRequestStart:
				{
					m_startTime.emplace<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
					m_emState = eStart;
				}
				break;
				case eRequestEnd:
				{
					Save();
					m_emState = eIdle;
				}
				break;
				}
			}

			void TracerImpl::Start()
			{
				m_emState = eRequestStart;
			}

			void TracerImpl::End(const wchar_t* saveFilePath)
			{
				if (IsTracing() == false)
					return;

				m_saveFilePath = saveFilePath;
				m_emState = eRequestEnd;
			}

			void TracerImpl::BeginEvent(const wchar_t* title, const wchar_t* category)
			{
				if (IsTracing() == false)
					return;

				thread::SRWWriteLock lock(&m_srwLock);

				const uint32_t threadID = GetCurrentThreadId();
				const uint32_t processID = GetCurrentProcessId();

				const size_t index = m_umapEvents[threadID].size();
				m_umapEventLinkers[threadID].emplace(index);

				m_umapEvents[threadID].emplace_back(Event::eBegin, title, category, processID, threadID);
			}

			void TracerImpl::EndEvent(bool isForceRecord)
			{
				if (IsTracing() == false)
					return;

				thread::SRWWriteLock lock(&m_srwLock);

				const uint32_t threadID = GetCurrentThreadId();
				if (m_umapEventLinkers[threadID].empty() == true)
					return;

				const size_t index = m_umapEventLinkers[threadID].top();
				m_umapEventLinkers[threadID].pop();
				if (index >= m_umapEvents[threadID].size())
					return;

				enum
				{
					eTracingThreshold = 50,
				};

				const Event& beginEvent = m_umapEvents[threadID][index];

				const std::chrono::high_resolution_clock::time_point nowTime = std::chrono::high_resolution_clock::now();
				const std::chrono::microseconds time = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - beginEvent.time);
				if (isForceRecord == true || time.count() > eTracingThreshold)
				{
					const uint32_t processID = GetCurrentProcessId();
					m_umapEvents[threadID].emplace_back(Event::eEnd, sid::EmptyString, sid::EmptyString, processID, threadID, nowTime);
				}
				else
				{
					auto iter = m_umapEvents[threadID].begin();
					std::advance(iter, index);
					m_umapEvents[threadID].erase(iter);
				}
			}

			float TracerImpl::TracingTime() const
			{
				if (IsTracing() == false)
					return 0.f;

				return std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - m_startTime.value()).count();
			}

			bool TracerImpl::IsTracing() const
			{
				return m_emState == State::eStart;
			}

			void TracerImpl::Save()
			{
				Json root;
				Json& traceEvents = root["traceEvents"];

				for (auto iter = m_umapEvents.begin(); iter != m_umapEvents.end(); ++iter)
				{
					std::vector<Event>& vecEvents = iter.value();
					std::for_each(vecEvents.begin(), vecEvents.end(), [&](Event& event)
					{
						Json data;
						
						switch (event.emType)
						{
						case Event::eBegin:
							data["ph"] = "B";
							data["name"] = string::WideToMulti(event.title.c_str());
							data["cat"] = string::WideToMulti(event.category.c_str());
							break;
						case Event::eEnd:
							data["ph"] = "E";
							break;
						}

						data["pid"] = event.processID;
						data["tid"] = event.threadID;

						const std::chrono::microseconds time = std::chrono::duration_cast<std::chrono::microseconds>(event.time - m_startTime.value());
						data["ts"] = time.count();

						if (event.args.empty() == false)
						{
							Json& argsList = data["args"];

							std::for_each(event.args.begin(), event.args.end(), [&](const Event::Args& args)
							{
								Json argsData;

								std::visit([&](auto&& arg)
								{
									using T = std::decay_t<decltype(arg)>;
									const std::string key = string::WideToMulti(args.key);
									if constexpr (std::is_same_v<T, bool>)
									{
										argsData[key] = (arg == true) ? "true" : "false";
									}
									else if constexpr (std::is_same_v<T, int32_t> ||
										std::is_same_v<T, uint32_t> ||
										std::is_same_v<T, int64_t> ||
										std::is_same_v<T, uint64_t> ||
										std::is_same_v<T, float> ||
										std::is_same_v<T, double>)
									{
										argsData[key] = arg;
									}
									else if constexpr (std::is_same_v<T, std::wstring>)
									{
										const std::string value = string::WideToMulti(arg);
										argsData[key] = value;
									}
									else
									{
										static_assert(false, "non-exhaustive visitor!");
									}
								}, args.variantValue);

								argsList.push_back(argsData);
							});
						}

						traceEvents.push_back(data);
					});
				}

				root["displayTimeUnit"] = "ms";

				const std::wstring fileExtension = file::GetFileExtension(m_saveFilePath);
				if (fileExtension.empty() == true)
				{
					m_saveFilePath.append(L".json");
				}

				std::ofstream stream(m_saveFilePath.c_str());
				stream << std::setw(4) << root << std::endl;

				m_startTime.reset();
				m_umapEvents.clear();
			}

			TracerImpl::Event* TracerImpl::GetLastEvent()
			{
				const uint32_t nThreadID = GetCurrentThreadId();
				if (m_umapEventLinkers[nThreadID].empty() == true)
					return nullptr;

				const size_t index = m_umapEventLinkers[nThreadID].top();

				return &m_umapEvents[nThreadID][index];
			}

			TracerImpl s_tracerImpl;

			void RefreshState()
			{
				s_tracerImpl.RefreshState();
			}

			void Start()
			{
				s_tracerImpl.Start();
			}

			void End(const wchar_t* saveFilePath)
			{
				s_tracerImpl.End(saveFilePath);
			}

			void BeginEvent(const wchar_t* title, const wchar_t* category, const wchar_t* file, int line)
			{
				s_tracerImpl.BeginEvent(title, category);

				s_tracerImpl.PushArgs(L"File", file);
				s_tracerImpl.PushArgs(L"Line", line);
			}

			void EndEvent(bool isForceRecord)
			{
				s_tracerImpl.EndEvent(isForceRecord);
			}

			template <>
			void PushArgs(const wchar_t* key, bool value)
			{
				s_tracerImpl.PushArgs(key, value);
			}

			template <>
			void PushArgs(const wchar_t* key, int32_t value)
			{
				s_tracerImpl.PushArgs(key, value);
			}

			template <>
			void PushArgs(const wchar_t* key, uint32_t value)
			{
				s_tracerImpl.PushArgs(key, value);
			}

			template <>
			void PushArgs(const wchar_t* key, int64_t value)
			{
				s_tracerImpl.PushArgs(key, value);
			}

			template <>
			void PushArgs(const wchar_t* key, uint64_t value)
			{
				s_tracerImpl.PushArgs(key, value);
			}

			template <>
			void PushArgs(const wchar_t* key, float value)
			{
				s_tracerImpl.PushArgs(key, value);
			}

			template <>
			void PushArgs(const wchar_t* key, double value)
			{
				s_tracerImpl.PushArgs(key, value);
			}

			template <>
			void PushArgs(const wchar_t* key, const wchar_t* value)
			{
				s_tracerImpl.PushArgs(key, value);
			}

			float TracingTime()
			{
				return s_tracerImpl.TracingTime();
			}

			bool IsTracing()
			{
				return s_tracerImpl.IsTracing();
			}

			Profiler::Profiler(const wchar_t* title, const wchar_t* category, const wchar_t* file, int line, bool isForceRecord)
				: m_isForceRecord(isForceRecord)
			{
				BeginEvent(title, category, file, line);
			}

			Profiler::~Profiler()
			{
				EndEvent(m_isForceRecord);
			}
		}
	}
}