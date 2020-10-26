#pragma once

#include "CommonLib/Singleton.h"

namespace est
{
	namespace Lua
	{
		class System : public Singleton<System>
		{
			friend Singleton<System>;
		private:
			System();
			virtual ~System();

		public:
			bool Initialize(bool isEnableJIT);

			void SetEnableJIT(bool isEnableJIT);

			bool CompileLua(const wchar_t* filePath);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}