#include "stdafx.h"
#include "FileUtil.h"

#include "StringUtil.h"

namespace eastengine
{
	namespace file
	{
		std::string GetFileName(const std::string& strPath)
		{
			return PathFindFileNameA(strPath.c_str());
		}

		std::string GetFileNameWithoutExtension(const std::string& strPath)
		{
			const std::string strName = GetFileName(strPath);
			const size_t pos = strName.find_last_of(".");
			if (pos != std::string::npos)
				return strName.substr(0, pos);

			return strName;
		}

		std::wstring GetFileName(const std::wstring& strPath)
		{
			return PathFindFileNameW(strPath.c_str());
		}

		std::wstring GetFileNameWithoutExtension(const std::wstring& strPath)
		{
			const std::wstring strName = GetFileName(strPath);
			const size_t pos = strName.find_last_of(L".");
			if (pos != std::wstring::npos)
				return strName.substr(0, pos);

			return strName;
		}

		std::string GetFileExtension(const std::string& strPath)
		{
			const char* str = PathFindExtensionA(strPath.c_str());
			if (str != nullptr)
				return str;

			return "";
		}

		std::wstring GetFileExtension(const std::wstring& strPath)
		{
			const wchar_t* str = PathFindExtensionW(strPath.c_str());
			if (str != nullptr)
				return str;

			return L"";
		}

		std::string GetFilePath(const std::string& strPath)
		{
			const size_t pos = strPath.find_last_of("\\");
			if (pos != std::string::npos)
				return strPath.substr(0, pos + 1);

			return strPath;
		}

		std::wstring GetFilePath(const std::wstring& strPath)
		{
			const size_t pos = strPath.find_last_of(L"\\");
			if (pos != std::wstring::npos)
				return strPath.substr(0, pos + 1);

			return strPath;
		}

		bool IsExists(const std::string& strPath)
		{
			return std::experimental::filesystem::exists(strPath);
		}

		void GetFiles(const std::string& strDirectoryPath, const std::string& strExtension, std::vector<std::string>& vecFiles_out)
		{
			const std::experimental::filesystem::path path(strDirectoryPath);

			std::experimental::filesystem::directory_iterator iter_start(path);
			std::experimental::filesystem::directory_iterator iter_end;

			for (; iter_start != iter_end; ++iter_start)
			{
				if (std::experimental::filesystem::is_directory(iter_start->status()) == true)
				{
					const std::string strSubDirectory = iter_start->path().generic_string();
					GetFiles(strSubDirectory, strExtension, vecFiles_out);
				}
				else if (strExtension.empty() == true || strExtension == ".*" || iter_start->path().extension() == strExtension)
				{
					vecFiles_out.emplace_back(iter_start->path().generic_string());
				}
			}
		}

		const char* GetPath(EmPath emPath)
		{
			switch (emPath)
			{
			case EmPath::eFont:
			{
				static std::string strPath;

				if (strPath.empty() == false)
					return strPath.c_str();

				strPath = GetDataPath();
				strPath.append("Font\\");

				return strPath.c_str();
			}
			break;
			case EmPath::eFx:
			{
				static std::string strPath;

				if (strPath.empty() == false)
					return strPath.c_str();

				strPath = GetDataPath();
				strPath.append("Fx\\");

				return strPath.c_str();
			}
			break;
			case EmPath::eLua:
			{
				static std::string strPath;

				if (strPath.empty() == false)
					return strPath.c_str();

				strPath = GetDataPath();
				strPath.append("Lua\\");

				return strPath.c_str();
			}
			break;
			case EmPath::eTexture:
			{
				static std::string strPath;

				if (strPath.empty() == false)
					return strPath.c_str();

				strPath = GetDataPath();
				strPath.append("Texture\\");

				return strPath.c_str();
			}
			break;
			case EmPath::eUI:
			{
				static std::string strPath;

				if (strPath.empty() == false)
					return strPath.c_str();

				strPath = GetDataPath();
				strPath.append("UI\\");

				return strPath.c_str();
			}
			break;
			case EmPath::eXML:
			{
				static std::string strPath;

				if (strPath.empty() == false)
					return strPath.c_str();

				strPath = GetDataPath();
				strPath.append("XML\\");

				return strPath.c_str();
			}
			break;
			case EmPath::eSound:
			{
				static std::string strPath;

				if (strPath.empty() == false)
					return strPath.c_str();

				strPath = GetDataPath();
				strPath.append("Sound\\");

				return strPath.c_str();
			}
			break;
			}

			return GetBinPath();
		}

		const char* GetDataPath()
		{
			static std::string strPath;

			if (strPath.empty() == false)
				return strPath.c_str();

			strPath = GetBinPath();
			strPath.append("Data\\");

			return strPath.c_str();
		}

		const char* GetBinPath()
		{
			static std::string strPath;

			if (strPath.empty() == false)
				return strPath.c_str();

			char strModulePath[MAX_PATH];
			GetModuleFileName(NULL, strModulePath, MAX_PATH);	// 실행파일경로

			// 실행파일명 제거
			strPath = strModulePath;
			strPath = GetFilePath(strPath);

			return strPath.c_str();
		}

		const char* GetProgramFileName()
		{
			static std::string strPath;

			if (strPath.empty() == false)
				return strPath.c_str();

			char strModulePath[MAX_PATH];
			GetModuleFileName(NULL, strModulePath, MAX_PATH);	// 실행파일경로

			strPath = GetFileNameWithoutExtension(strModulePath);

			return strPath.c_str();
		}
	}
}