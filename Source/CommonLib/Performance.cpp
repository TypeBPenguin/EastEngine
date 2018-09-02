#include "stdafx.h"
#include "Performance.h"

#include "Lock.h"
#include "FileUtil.h"
#include "json.hpp"

namespace eastengine
{
	namespace Performance
	{
		namespace Tracer
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

					thread::AutoLock autoLock(&m_lock);

					Event* pEvent = GetLastEvent();
					if (pEvent != nullptr)
					{
						pEvent->vecArgs.push_back({ strKey, value });
					}
				}

				template <>
				void PushArgs(const char* strKey, const char* value)
				{
					if (IsTracing() == false)
						return;

					thread::AutoLock autoLock(&m_lock);

					Event* pEvent = GetLastEvent();
					if (pEvent != nullptr)
					{
						pEvent->vecArgs.push_back({ strKey, std::string{ value } });
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

					std::string strTitle;
					std::string strCategory;

					uint32_t nProcessID{ 0 };
					uint32_t nThreadID{ 0 };

					std::chrono::system_clock::time_point time;

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
					std::vector<Args> vecArgs;
				};

				Event* GetLastEvent();

			private:
				thread::Lock m_lock;

				std::optional<std::chrono::system_clock::time_point> m_startTime;
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
					m_startTime.emplace<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
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

			void TracerImpl::BeginEvent(const char* strTitle, const char* strCategory)
			{
				if (IsTracing() == false)
					return;

				thread::AutoLock autoLock(&m_lock);

				const uint32_t nThreadID = GetCurrentThreadId();

				Event event;
				event.emType = Event::eBegin;
				event.strTitle = strTitle;
				event.strCategory = strCategory;
				event.nProcessID = GetCurrentProcessId();
				event.nThreadID = nThreadID;
				event.time = std::chrono::system_clock::now();

				const size_t nIndex = m_umapEvents[nThreadID].size();
				m_umapEventLinkers[nThreadID].emplace(nIndex);

				m_umapEvents[nThreadID].emplace_back(event);
			}

			void TracerImpl::EndEvent()
			{
				if (IsTracing() == false)
					return;

				thread::AutoLock autoLock(&m_lock);

				const uint32_t nThreadID = GetCurrentThreadId();
				if (m_umapEventLinkers[nThreadID].empty() == true)
					return;

				const size_t nIndex = m_umapEventLinkers[nThreadID].top();
				m_umapEventLinkers[nThreadID].pop();
				assert(nIndex < m_umapEvents[nThreadID].size());

				const Event& beginEvent = m_umapEvents[nThreadID][nIndex];

				std::chrono::system_clock::time_point nowTime = std::chrono::system_clock::now();

				std::chrono::microseconds time = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - beginEvent.time);
				if (time.count() > 0)
				{
					Event event;
					event.emType = Event::eEnd;
					event.strTitle = beginEvent.strTitle;
					event.strCategory = beginEvent.strCategory;
					event.nProcessID = GetCurrentProcessId();
					event.nThreadID = nThreadID;
					event.time = nowTime;

					m_umapEvents[nThreadID].emplace_back(event);
				}
				else
				{
					auto iter = m_umapEvents[nThreadID].begin();
					std::advance(iter, nIndex);
					m_umapEvents[nThreadID].erase(iter);
				}
			}

			float TracerImpl::TracingTime() const
			{
				if (IsTracing() == false)
					return 0.f;

				return std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::system_clock::now() - m_startTime.value()).count();
			}

			constexpr bool TracerImpl::IsTracing() const noexcept
			{
				return m_emState == State::eStart;
			}

			void TracerImpl::Save()
			{
				Json root;
				Json traceEvents;

				for (auto iter = m_umapEvents.begin(); iter != m_umapEvents.end(); ++iter)
				{
					std::vector<Event>& vecEvents = iter->second;
					std::for_each(vecEvents.begin(), vecEvents.end(), [&](Event& event)
					{
						Json data;
						data["name"] = event.strTitle;
						data["cat"] = event.strCategory;

						switch (event.emType)
						{
						case Event::eBegin:
							data["ph"] = "B";
							break;
						case Event::eEnd:
							data["ph"] = "E";
							break;
						}

						data["pid"] = event.nProcessID;
						data["tid"] = event.nThreadID;

						std::chrono::microseconds time = std::chrono::duration_cast<std::chrono::microseconds>(event.time - m_startTime.value());
						data["ts"] = time.count();

						if (event.vecArgs.empty() == false)
						{
							Json argsList;

							std::for_each(event.vecArgs.begin(), event.vecArgs.end(), [&](const Event::Args& args)
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
										static_assert(always_false<T>::value, "non-exhaustive visitor!");
									}
								}, args.variantValue);

								argsList.push_back(argsData);
							});

							data["args"] = argsList;
						}

						traceEvents.push_back(data);
					});
				}

				root["traceEvents"] = traceEvents;
				root["displayTimeUnit"] = "ms";

				std::string strFileExtension = file::GetFileExtension(m_strSaveFilePath);
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