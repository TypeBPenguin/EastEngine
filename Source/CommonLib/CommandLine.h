#pragma once

#include "Singleton.h"

namespace est
{
	namespace Config
	{
		class SCommandLine : public Singleton<SCommandLine>
		{
			friend Singleton<SCommandLine>;
		private:
			SCommandLine();
			virtual ~SCommandLine();

		public:
			bool Init();
			void Release();

			bool GetCommand(const wchar_t* commandLine, std::wstring* pValue = nullptr);

		private:
			bool m_isInitialize{ false };
			std::unordered_map<std::wstring, std::wstring> m_umapCommandLine;
		};
	}
}