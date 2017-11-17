#include "stdafx.h"
#include "DirectoryMOnitor.h"

#include "CommonLib.h"

#include <strsafe.h>

namespace EastEngine
{
	namespace File
	{
		HDirMonitor::HDirMonitor()
			: lParam(0)
			, dwNotifyFilter(0)
			, isWatchSubTree(false)
			, isStop(false)
			, callback(nullptr)
		{
			Memory::Clear(buffer);
		}

		DirectoryMonitor::DirectoryMonitor()
			: m_isInit(false)
		{
		}

		DirectoryMonitor::~DirectoryMonitor()
		{
			Release();
		}

		bool DirectoryMonitor::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			return true;
		}

		void DirectoryMonitor::Release()
		{
			if (m_isInit == false)
				return;

			uint32_t nSize = m_vecHDirMonitor.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				HDirMonitor* pMonitor = m_vecHDirMonitor[i];
				if (pMonitor == nullptr)
					continue;

				pMonitor->isStop = TRUE;

				CancelIo(pMonitor->hDir);

				CloseHandle(pMonitor->ol.hEvent);
				CloseHandle(pMonitor->hDir);

				SafeDelete(pMonitor);
			}
			m_vecHDirMonitor.clear();

			m_isInit = false;
		}

		void DirectoryMonitor::Update()
		{
			uint32_t nSize = m_vecHDirMonitor.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				HDirMonitor* pMonitor = m_vecHDirMonitor[i];
				if (pMonitor == nullptr)
					continue;

				if (pMonitor->isStop == TRUE)
					continue;

				DWORD dwNumBytes = 0;
				ULONG_PTR key = (ULONG_PTR)pMonitor;
				if (RefreshMonitoring(pMonitor, &dwNumBytes) == true)
				{
					BOOL bResult = GetQueuedCompletionStatus(pMonitor->hDirOPPort,
						&dwNumBytes, &key, (LPOVERLAPPED*)(&pMonitor->ol), 0);

					if (bResult == TRUE)
					{
						wchar_t temp[MAX_PATH];
						Memory::Clear(temp);

						DWORD dwOffset = 0;
						FILE_NOTIFY_INFORMATION* pFni = nullptr;

						do
						{
							pFni = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(&pMonitor->buffer[dwOffset]);
							dwOffset += pFni->NextEntryOffset;

							if (pFni->Action != 0)
							{
								StringCbCopyNW(temp, sizeof(temp), pFni->FileName, pFni->FileNameLength);
								pMonitor->callback(String::WideToMulti(temp, CP_ACP).c_str(),
									pFni->Action, pMonitor->lParam);
							}
						} while (pFni->NextEntryOffset > 0);

						Memory::Clear(pMonitor->buffer);
					}
				}
			}
		}

		void DirectoryMonitor::AddDirMonitor(const char* strDirectory, DirectoryMonitorCallback funcCallback, DWORD notifyFilter)
		{
			HDirMonitor* pMonitor = new HDirMonitor;
			Memory::Clear(pMonitor, sizeof(HDirMonitor));

			pMonitor->hDir = CreateFile(strDirectory, FILE_LIST_DIRECTORY | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

			if (pMonitor->hDir != INVALID_HANDLE_VALUE)
			{
				pMonitor->ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
				pMonitor->dwNotifyFilter = notifyFilter;
				pMonitor->callback = funcCallback;
				pMonitor->isWatchSubTree = TRUE;
				pMonitor->isStop = FALSE;
				pMonitor->lParam = 0;

				pMonitor->hDirOPPort = CreateIoCompletionPort(pMonitor->hDir, nullptr, (ULONG_PTR)(pMonitor), 1);

				if (RefreshMonitoring(pMonitor))
				{
					m_vecHDirMonitor.push_back(pMonitor);
				}
				else
				{
					CloseHandle(pMonitor->ol.hEvent);
					CloseHandle(pMonitor->hDir);
				}
			}
		}

		void CALLBACK DirectoryMonitor::MonitorCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
		{
			std::string strFile;
			PFILE_NOTIFY_INFORMATION pNotify;
			HDirMonitor* pMonitor = reinterpret_cast<HDirMonitor*>(lpOverlapped);
			size_t offset = 0;

			if (dwErrorCode == ERROR_SUCCESS)
			{
				do
				{
					pNotify = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(&pMonitor->buffer[offset]);
					offset += pNotify->NextEntryOffset;

# if defined(UNICODE)
					{
						lstrcpynW(szFile, pNotify->FileName,
							min(MAX_PATH, pNotify->FileNameLength / sizeof(WCHAR) + 1));
					}
# else
					{
						strFile = String::WideToMulti(pNotify->FileName, CP_ACP);
					}
# endif

					pMonitor->callback(strFile.c_str(), pNotify->Action, pMonitor->lParam);

				} while (pNotify->NextEntryOffset != 0);
			}

			/*if (pMonitor->bStop == FALSE)
			{
			sRefreshMonitoring(pMonitor);
			}*/
		}

		bool DirectoryMonitor::RefreshMonitoring(HDirMonitor* pMonitor, DWORD* pdw)
		{
			return ReadDirectoryChangesW(pMonitor->hDir, pMonitor->buffer, sizeof(pMonitor->buffer), pMonitor->isWatchSubTree,
				pMonitor->dwNotifyFilter, pdw, &pMonitor->ol, nullptr) == TRUE;
		}
	}
}