#include "stdafx.h"
#include "Performance.h"

#include "StringTable.h"
#include "Lock.h"
#include "FileUtil.h"
#include "json.hpp"

namespace eastengine
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
				void End(const char* strSaveFilePath);

				void BeginEvent(const char* strTitle, const char* strCategory);
				void EndEvent();

				template <typename T>
				void PushArgs(const char* strKey, T value)
				{
					if (IsTracing() == false)
						return;

					thread::SRWWriteLock lock(&m_srwLock);

					Event* pEvent = GetLastEvent();
					if (pEvent != nullptr)
					{
						pEvent->args.push_back({ strKey, value });
					}
				}

				template <>
				void PushArgs(const char* strKey, const char* value)
				{
					if (IsTracing() == false)
						return;

					thread::SRWWriteLock lock(&m_srwLock);

					Event* pEvent = GetLastEvent();
					if (pEvent != nullptr)
					{
						pEvent->args.push_back({ strKey, std::string{ value } });
					}
				}

			public:
				float TracingTime() const;
				constexpr bool IsTracing() const noexcept;

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
						std::string strKey;

						std::variant<
							bool,
							int32_t,
							uint32_t,
							int64_t,
							uint64_t,
							float,
							double,
							std::string> variantValue;
					};
					std::vector<Args> args;
				};

				Event* GetLastEvent();

			private:
				thread::SRWLock m_srwLock;

				std::optional<std::chrono::high_resolution_clock::time_point> m_startTime;
				std::unordered_map<uint32_t, std::vector<Event>> m_umapEvents;
				std::unordered_map<uint32_t, std::stack<size_t>> m_umapEventLinkers;

				enum State
				{
					eIdle = 0,

					eStart,

					eRequestStart,
					eRequestEnd,
				};

				State m_emState{ eIdle };
				std::string m_strSaveFilePath;
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

			void TracerImpl::End(const char* strSaveFilePath)
			{
				if (IsTracing() == false)
					return;

				m_strSaveFilePath = strSaveFilePath;
				m_emState = eRequestEnd;
			}

			void TracerImpl::BeginEvent(const char* title, const char* category)
			{
				if (IsTracing() == false)
					return;

				thread::SRWWriteLock lock(&m_srwLock);

				const uint32_t threadID = GetCurrentThreadId();
				const uint32_t processID = GetCurrentProcessId();

				const size_t nIndex = m_umapEvents[threadID].size();
				m_umapEventLinkers[threadID].emplace(nIndex);

				m_umapEvents[threadID].emplace_back(Event::eBegin, title, category, processID, threadID);
			}

			void TracerImpl::EndEvent()
			{
				if (IsTracing() == false)
					return;

				thread::SRWWriteLock lock(&m_srwLock);

				const uint32_t threadID = GetCurrentThreadId();
				if (m_umapEventLinkers[threadID].empty() == true)
					return;

				const size_t nIndex = m_umapEventLinkers[threadID].top();
				m_umapEventLinkers[threadID].pop();
				assert(nIndex < m_umapEvents[threadID].size());

				const Event& beginEvent = m_umapEvents[threadID][nIndex];

				const std::chrono::high_resolution_clock::time_point nowTime = std::chrono::high_resolution_clock::now();
				const std::chrono::microseconds time = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - beginEvent.time);
				if (time.count() > 0)
				{
					const uint32_t processID = GetCurrentProcessId();
					m_umapEvents[threadID].emplace_back(Event::eEnd, StrID::EmptyString, StrID::EmptyString, processID, threadID, nowTime);
				}
				else
				{
					auto iter = m_umapEvents[threadID].begin();
					std::advance(iter, nIndex);
					m_umapEvents[threadID].erase(iter);
				}
			}

			float TracerImpl::TracingTime() const
			{
				if (IsTracing() == false)
					return 0.f;

				return std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - m_startTime.value()).count();
			}

			constexpr bool TracerImpl::IsTracing() const noexcept
			{
				return m_emState == State::eStart;
			}

			void TracerImpl::Save()
			{
				Json root;
				Json& traceEvents = root["traceEvents"];

				for (auto iter = m_umapEvents.begin(); iter != m_umapEvents.end(); ++iter)
				{
					std::vector<Event>& vecEvents = iter->second;
					std::for_each(vecEvents.begin(), vecEvents.end(), [&](Event& event)
					{
						Json data;
						
						switch (event.emType)
						{
						case Event::eBegin:
							data["ph"] = "B";
							data["name"] = event.title.c_str();
							data["cat"] = event.category.c_str();

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
									if constexpr (std::is_same_v<T, bool>)
									{
										argsData[args.strKey] = (arg == true) ? "true" : "false";
									}
									else if constexpr (std::is_same_v<T, int32_t> ||
										std::is_same_v<T, uint32_t> ||
										std::is_same_v<T, int64_t> ||
										std::is_same_v<T, uint64_t> ||
										std::is_same_v<T, float> ||
										std::is_same_v<T, double> ||
										std::is_same_v<T, std::string>)
									{
										argsData[args.strKey] = arg;
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

				const std::string strFileExtension = file::GetFileExtension(m_strSaveFilePath);
				if (strFileExtension.empty() == true)
				{
					m_strSaveFilePath.append(".json");
				}

				std::ofstream stream(m_strSaveFilePath.c_str());
				stream << std::setw(4) << root << std::endl;

				m_startTime.reset();
				m_umapEvents.clear();
			}

			TracerImpl::Event* TracerImpl::GetLastEvent()
			{
				const uint32_t nThreadID = GetCurrentThreadId();
				if (m_umapEventLinkers[nThreadID].empty() == true)
					return nullptr;

				const size_t nIndex = m_umapEventLinkers[nThreadID].top();

				return &m_umapEvents[nThreadID][nIndex];
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

			void End(const char* strSaveFilePath)
			{
				s_tracerImpl.End(strSaveFilePath);
			}

			void BeginEvent(const char* strTitle, const char* strCategory, const char* strFile, int nLine)
			{
				s_tracerImpl.BeginEvent(strTitle, strCategory);

				s_tracerImpl.PushArgs("File", strFile);
				s_tracerImpl.PushArgs("Line", nLine);
			}

			void EndEvent()
			{
				s_tracerImpl.EndEvent();
			}

			template <>
			void PushArgs(const char* strKey, bool value)
			{
				s_tracerImpl.PushArgs(strKey, value);
			}

			template <>
			void PushArgs(const char* strKey, int32_t value)
			{
				s_tracerImpl.PushArgs(strKey, value);
			}

			template <>
			void PushArgs(const char* strKey, uint32_t value)
			{
				s_tracerImpl.PushArgs(strKey, value);
			}

			template <>
			void PushArgs(const char* strKey, int64_t value)
			{
				s_tracerImpl.PushArgs(strKey, value);
			}

			template <>
			void PushArgs(const char* strKey, uint64_t value)
			{
				s_tracerImpl.PushArgs(strKey, value);
			}

			template <>
			void PushArgs(const char* strKey, float value)
			{
				s_tracerImpl.PushArgs(strKey, value);
			}

			template <>
			void PushArgs(const char* strKey, double value)
			{
				s_tracerImpl.PushArgs(strKey, value);
			}

			template <>
			void PushArgs(const char* strKey, const char* value)
			{
				s_tracerImpl.PushArgs(strKey, value);
			}

			float TracingTime()
			{
				return s_tracerImpl.TracingTime();
			}

			bool IsTracing()
			{
				return s_tracerImpl.IsTracing();
			}

			Profiler::Profiler(const char* strTitle, const char* strCategory, const char* strFile, int nLine)
			{
				BeginEvent(strTitle, strCategory, strFile, nLine);
			}

			Profiler::~Profiler()
			{
				EndEvent();
			}
		}
	}
}