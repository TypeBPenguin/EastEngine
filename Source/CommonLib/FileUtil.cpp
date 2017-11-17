#include "stdafx.h"
#include "FileUtil.h"

#include "StringUtil.h"

#include <Shlwapi.h>

namespace EastEngine
{
	namespace File
	{
		std::string GetFileName(const std::string& strPath)
		{
			return PathFindFileNameA(strPath.c_str());
		}

		std::string GetFileNameWithoutExtension(const char* strPath)
		{
			std::string strName = GetFileName(strPath);
			uint32_t pos = strName.find_last_of(".");
			if (pos != std::string::npos)
			{
				strName = strName.substr(0, pos);
			}

			return strName;
		}

		std::wstring GetFileName(const std::wstring& strPath)
		{
			return PathFindFileNameW(strPath.c_str());
		}

		std::wstring GetFileNameWithoutExtension(const wchar_t* strPath)
		{
			std::wstring strName = GetFileName(strPath);
			uint32_t pos = strName.find_last_of(L".");
			if (pos != std::wstring::npos)
			{
				strName = strName.substr(0, pos);
			}

			return strName;
		}

		std::string GetFileExtension(const std::string& strPath)
		{
			const char* str = PathFindExtensionA(strPath.c_str());
			if (str != nullptr && String::IsEquals(str, "") == false)
			{
				str += 1;
				return str;
			}

			return "";
		}

		std::wstring GetFileExtension(const std::wstring& strPath)
		{
			const wchar_t* str = PathFindExtensionW(strPath.c_str());
			if (str != nullptr && String::IsEquals(str, L"") == false)
			{
				str += 1;
				return str;
			}

			return L"";
		}

		std::string GetFilePath(const std::string& strPath)
		{
			return GetFilePath(strPath.c_str());
		}

		std::string GetFilePath(const char* strPath)
		{
			std::string str = strPath;

			uint32_t pos = str.find_last_of("\\");
			if (pos != std::string::npos)
			{
				str = str.substr(0, pos + 1);
			}

			return str;
		}

		std::wstring GetFilePath(const std::wstring& strPath)
		{
			return GetFilePath(strPath.c_str());
		}

		std::wstring GetFilePath(const wchar_t* strPath)
		{
			std::wstring str = strPath;

			uint32_t pos = str.find_last_of(L"\\");
			if (pos != std::wstring::npos)
			{
				str = str.substr(0, pos + 1);
			}

			return str;
		}

		std::vector<std::string> GetFilesInFolder(const char* strPath, const char* strType, bool bWithPath)
		{
			std::vector<std::string> vecFiles;

			std::string strFullPath = strPath;
			strFullPath += strType;

			WIN32_FIND_DATA folder;
			HANDLE hFind = ::FindFirstFile(strFullPath.c_str(), &folder);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					if ((folder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
					{
						std::string strFilePath;
						if (bWithPath)
						{
							strFilePath = strPath;
							strFilePath += folder.cFileName;
						}
						else
						{
							strFilePath = folder.cFileName;
						}

						vecFiles.emplace_back(strFilePath);
					}
				} while (::FindNextFile(hFind, &folder));
				::FindClose(hFind);
			}

			return vecFiles;
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
			auto pos = strPath.find_last_of("\\");
			if (pos == std::string::npos)
				return "";

			strPath = strPath.substr(0, pos + 1);

			return strPath.c_str();
		}
	}
}