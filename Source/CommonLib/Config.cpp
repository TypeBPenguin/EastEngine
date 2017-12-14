#include "stdafx.h"
#include "Config.h"

#include "StringID.h"

namespace EastEngine
{
	namespace Config
	{
		struct ConfigValue
		{
			enum Type
			{
				eBool = 0,
				eScalar,
				eString,
			};

			Type emType;
			std::variant<bool, float, String::StringID> element;
		};
		static std::unordered_map<std::uint64_t, ConfigValue> s_umapConfigValue;

		template <ConfigValue::Type ConfigValueType, typename T>
		bool SetValue(const std::uint64_t nConfigKey, T value)
		{
			auto iter = s_umapConfigValue.find(nConfigKey);
			if (iter != s_umapConfigValue.end())
			{
				ConfigValue& configValue = iter->second;
				if (configValue.emType != ConfigValueType)
					return false;

				T& elementValue = std::get<T>(configValue.element);
				elementValue = value;
			}
			else
			{
				ConfigValue configValue;
				configValue.emType = ConfigValueType;
				configValue.element.emplace<T>(value);

				s_umapConfigValue.emplace(nConfigKey, configValue);
			}

			return true;
		}

		template <ConfigValue::Type ConfigValueType, typename T>
		T GetValue(const std::uint64_t nConfigKey, T defaultValue)
		{
			auto iter = s_umapConfigValue.find(nConfigKey);
			if (iter != s_umapConfigValue.end())
			{
				ConfigValue& configValue = iter->second;
				if (configValue.emType != ConfigValueType)
					return defaultValue;

				T& value = std::get<T>(configValue.element);
				return value;
			}

			return defaultValue;
		}
		
		bool SetEnable(const std::uint64_t nConfigKey, bool isEnable)
		{
			return SetValue<ConfigValue::Type::eBool>(nConfigKey, isEnable);
		}

		bool IsEnable(const std::uint64_t nConfigKey)
		{
			return GetValue<ConfigValue::Type::eBool>(nConfigKey, false);
		}

		bool SetScalar(const std::uint64_t nConfigKey, float fScalar)
		{
			return SetValue<ConfigValue::Type::eScalar>(nConfigKey, fScalar);
		}

		float GetScalar(const std::uint64_t nConfigKey)
		{
			return GetValue<ConfigValue::Type::eScalar>(nConfigKey, false);
		}

		bool SetString(const std::uint64_t nConfigKey, const char* strConfigValue)
		{
			return SetValue<ConfigValue::Type::eString>(nConfigKey, String::StringID(strConfigValue));
		}

		const  char* GetString(const std::uint64_t nConfigKey)
		{
			const String::StringID defaultValue(String::UnregisteredKey);

			return GetValue<ConfigValue::Type::eString>(nConfigKey, defaultValue).c_str();
		}
	}
}