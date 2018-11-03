#include "stdafx.h"
#include "DirectoryMOnitor.h"

#include "CommonLib.h"

#include <strsafe.h>

namespace eastengine
{
	namespace file
	{
		static_assert(FILE_NOTIFY_CHANGE_FILE_NAME == eFileName, "DirectoryMonitor Filter Mismatch");
		static_assert(FILE_NOTIFY_CHANGE_DIR_NAME == eDirName, "DirectoryMonitor Filter Mismatch");
		static_assert(FILE_NOTIFY_CHANGE_ATTRIBUTES == eAttributes, "DirectoryMonitor Filter Mismatch");
		static_assert(FILE_NOTIFY_CHANGE_SIZE == eSize, "DirectoryMonitor Filter Mismatch");
		static_assert(FILE_NOTIFY_CHANGE_LAST_WRITE == eLastWrite, "DirectoryMonitor Filter Mismatch");
		static_assert(FILE_NOTIFY_CHANGE_LAST_ACCESS == eLastAccess, "DirectoryMonitor Filter Mismatch");
		static_assert(FILE_NOTIFY_CHANGE_CREATION == eCreation, "DirectoryMonitor Filter Mismatch");
		static_assert(FILE_NOTIFY_CHANGE_SECURITY == eSecurity, "DirectoryMonitor Filter Mismatch");

		struct HDirMonitor
		{
			OVERLAPPED ol;
			HANDLE hDir = nullptr;
			HANDLE hDirOPPort = nullptr;
			unsigned char buffer[sizeof(FILE_NOTIFY_INFORMATION) * 1024 * 32]{};
			LPARAM lParam{ 0 };
			DWORD dwNotifyFilter{ 0 };
			BOOL isWatchSubTree{ FALSE };
			BOOL isStop{ FALSE };
			DirectoryMonitorCallback callback{ nullptr };

			~HDirMonitor();
		};

		class DirectoryMonitor::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update();

		public:
			void AddDirectoryMonitor(const char* strDirectory, DirectoryMonitorCallback funcCallback, DWORD notifyFilter = Filter::eFileName | Filter::eDirName | Filter::eAttributes | Filter::eSize | Filter::eLastWrite | Filter::eCreation);

		private:
			std::vector<HDirMonitor*> m_vecHDirMonitor;
		};

		void CALLBACK HandleDirectoryMonitorCallback(const char* strPath, DWORD dwAction, LPARAM lParam)
		{

		}

		bool RefreshMonitoring(HDirMonitor* pMonitor, DWORD* pdw)
		{
			return ReadDirectoryChangesW(pMonitor->hDir, pMonitor->buffer, sizeof(pMonitor->buffer), pMonitor->isWatchSubTree,
				pMonitor->dwNotifyFilter, pdw, &pMonitor->ol, nullptr) == TRUE;
		}

		HDirMonitor::~HDirMonitor()
		{
			isStop = TRUE;

			CancelIo(hDir);

			CloseHandle(ol.hEvent);
			CloseHandle(hDir);
		}
		
		DirectoryMonitor::Impl::Impl()
		{
		}

		DirectoryMonitor::Impl::~Impl()
		{
			const size_t nSize = m_vecHDirMonitor.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				SafeDelete(m_vecHDirMonitor[i]);
			}
			m_vecHDirMonitor.clear();
		}

		void DirectoryMonitor::Impl::Update()
		{
			TRACER_EVENT("DirectoryMonitor::Update");
			const size_t nSize = m_vecHDirMonitor.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				HDirMonitor* pMonitor = m_vecHDirMonitor[i];
				if (pMonitor->isStop == TRUE)
					continue;

				DWORD dwNumBytes = 0;
				ULONG_PTR key = reinterpret_cast<ULONG_PTR>(pMonitor);
				if (RefreshMonitoring(pMonitor, &dwNumBytes) == true)
				{
					BOOL bResult = GetQueuedCompletionStatus(pMonitor->hDirOPPort,
						&dwNumBytes, &key, (LPOVERLAPPED*)(&pMonitor->ol), 0);

					if (bResult == TRUE)
					{
						wchar_t temp[MAX_PATH]{};

						DWORD dwOffset = 0;
						FILE_NOTIFY_INFORMATION* pFni = nullptr;

						do
						{
							pFni = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(&pMonitor->buffer[dwOffset]);
							dwOffset += pFni->NextEntryOffset;

							if (pFni->Action != 0)
							{
								StringCbCopyNW(temp, sizeof(temp), pFni->FileName, pFni->FileNameLength);
								pMonitor->callback(string::WideToMulti(temp, CP_ACP).c_str(),
									pFni->Action, pMonitor->lParam);
							}
						} while (pFni->NextEntryOffset > 0);

						Memory::Clear(pMonitor->buffer);
					}
				}
			}
		}

		void DirectoryMonitor::Impl::AddDirectoryMonitor(const char* strDirectory, DirectoryMonitorCallback funcCallback, DWORD notifyFilter)
		{
			HDirMonitor* pMonitor = new HDirMonitor;
			pMonitor->hDir = CreateFile(strDirectory, FILE_LIST_DIRECTORY | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

			if (pMonitor->hDir != INVALID_HANDLE_VALUE)
			{
				pMonitor->ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
				pMonitor->dwNotifyFilter = notifyFilter;
				pMonitor->callback = HandleDirectoryMonitorCallback;
				pMonitor->isWatchSubTree = TRUE;
				pMonitor->isStop = FALSE;
				pMonitor->lParam = 0;

				pMonitor->hDirOPPort = CreateIoCompletionPort(pMonitor->hDir, nullptr, reinterpret_cast<ULONG_PTR>(pMonitor), 1);

				if (RefreshMonitoring(pMonitor, nullptr))
				{
					m_vecHDirMonitor.push_back(pMonitor);
				}
				else
				{
					SafeDelete(pMonitor);
				}
			}
		}

		DirectoryMonitor::DirectoryMonitor()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		DirectoryMonitor::~DirectoryMonitor()
		{
		}
		
		void DirectoryMonitor::Update()
		{
			m_pImpl->Update();
		}

		void DirectoryMonitor::AddDirectoryMonitor(const char* strDirectory, DirectoryMonitorCallback funcCallback, DWORD notifyFilter)
		{
			m_pImpl->AddDirectoryMonitor(strDirectory, funcCallback, notifyFilter);
		}
	}
}