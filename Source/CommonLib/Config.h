#pragma once

namespace EastEngine
{
	namespace Config
	{
		bool SetEnable(const std::uint64_t nConfigKey, bool isEnable);
		bool IsEnable(const std::uint64_t nConfigKey);

		bool SetScalar(const std::uint64_t nConfigKey, float fScalar);
		float GetScalar(const std::uint64_t nConfigKey);

		bool SetString(const std::uint64_t nConfigKey, const char* strConfigValue);
		const char* GetString(const std::uint64_t nConfigKey);
	}
}