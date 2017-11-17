#pragma once

#include "Singleton.h"

namespace EastEngine
{
	namespace File
	{
		typedef void (CALLBACK *DirectoryMonitorCallback)(const char* strPath, DWORD dwAction, LPARAM lParam);

		struct HDirMonitor
		{
			OVERLAPPED ol;
			HANDLE hDir = nullptr;
			HANDLE hDirOPPort = nullptr;
			unsigned char buffer[sizeof(FILE_NOTIFY_INFORMATION) * 1024 * 32];
			LPARAM lParam;
			DWORD dwNotifyFilter;
			BOOL isWatchSubTree;
			BOOL isStop;
			DirectoryMonitorCallback callback;

			HDirMonitor();
		};

		class DirectoryMonitor : public Singleton<DirectoryMonitor>
		{
			friend Singleton<DirectoryMonitor>;
		private:
			DirectoryMonitor();
			virtual ~DirectoryMonitor();

		public:
			bool Init();
			void Release();

			void Update();

			void AddDirMonitor(const char* strDirectory, DirectoryMonitorCallback funcCallback, DWORD notifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION);
			static void CALLBACK MonitorCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

		private:
			static bool RefreshMonitoring(HDirMonitor* pMonitor, DWORD* pdw = nullptr);

		private:
			std::vector<HDirMonitor*> m_vecHDirMonitor;

			bool m_isInit;
		};
	}
}