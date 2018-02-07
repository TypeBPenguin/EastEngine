#pragma once

#include "Singleton.h"

namespace EastEngine
{
	namespace File
	{
		using DirectoryMonitorCallback = void (CALLBACK*)(const char* strPath, DWORD dwAction, LPARAM lParam);

		enum Filter
		{
			eFileName = 1 << 0,
			eDirName = 1 << 1,
			eAttributes = 1 << 2,
			eSize = 1 << 3,
			eLastWrite = 1 << 4,
			eLastAccess = 1 << 5,
			eCreation = 1 << 6,
			eSecurity = 1 << 8,
		};

		class DirectoryMonitor : public Singleton<DirectoryMonitor>
		{
			friend Singleton<DirectoryMonitor>;
		private:
			DirectoryMonitor();
			virtual ~DirectoryMonitor();

		public:
			void Update();

		public:
			void AddDirectoryMonitor(const char* strDirectory, DirectoryMonitorCallback funcCallback, DWORD notifyFilter = Filter::eFileName | Filter::eDirName | Filter::eAttributes | Filter::eSize | Filter::eLastWrite | Filter::eCreation);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}