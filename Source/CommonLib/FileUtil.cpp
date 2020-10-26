#include "stdafx.h"
#include "FileUtil.h"

#include "StringUtil.h"

namespace est
{
	namespace file
	{
		std::string GetFileName(const std::string& path)
		{
			return PathFindFileNameA(path.c_str());
		}

		std::string GetFileNameWithoutExtension(const std::string& path)
		{
			const std::string strName = GetFileName(path);
			const size_t pos = strName.find_last_of(".");
			if (pos != std::string::npos)
				return strName.substr(0, pos);

			return strName;
		}

		std::wstring GetFileName(const std::wstring& path)
		{
			return PathFindFileNameW(path.c_str());
		}

		std::wstring GetFileNameWithoutExtension(const std::wstring& path)
		{
			const std::wstring strName = GetFileName(path);
			const size_t pos = strName.find_last_of(L".");
			if (pos != std::wstring::npos)
				return strName.substr(0, pos);

			return strName;
		}

		std::string GetFileExtension(const std::string& path)
		{
			const char* str = PathFindExtensionA(path.c_str());
			if (str != nullptr)
				return str;

			return "";
		}

		std::wstring GetFileExtension(const std::wstring& path)
		{
			const wchar_t* str = PathFindExtensionW(path.c_str());
			if (str != nullptr)
				return str;

			return L"";
		}

		std::string GetFilePath(const std::string& path)
		{
			const size_t pos = path.find_last_of("\\");
			if (pos != std::string::npos)
				return path.substr(0, pos + 1);

			return path;
		}

		std::wstring GetFilePath(const std::wstring& path)
		{
			const size_t pos = path.find_last_of(L"\\");
			if (pos != std::wstring::npos)
				return path.substr(0, pos + 1);

			return path;
		}

		bool IsExists(const std::string& path)
		{
			return std::filesystem::exists(path);
		}

		bool IsExists(const std::wstring& path)
		{
			return std::filesystem::exists(path);
		}

		void GetFiles(const std::string& directoryPath, const std::string& extension, std::vector<std::string>& files_out)
		{
			const std::filesystem::path path(directoryPath);

			std::filesystem::directory_iterator iter_start(path);
			std::filesystem::directory_iterator iter_end;

			for (; iter_start != iter_end; ++iter_start)
			{
				if (std::filesystem::is_directory(iter_start->status()) == true)
				{
					const std::string subDirectory = iter_start->path().generic_string();
					GetFiles(subDirectory, extension, files_out);
				}
				else if (extension.empty() == true || extension == ".*" || iter_start->path().extension() == extension)
				{
					files_out.emplace_back(iter_start->path().generic_string());
				}
			}
		}

		void GetFiles(const std::wstring& directoryPath, const std::wstring& extension, std::vector<std::wstring>& files_out)
		{
			const std::filesystem::path path(directoryPath);

			std::filesystem::directory_iterator iter_start(path);
			std::filesystem::directory_iterator iter_end;

			for (; iter_start != iter_end; ++iter_start)
			{
				if (std::filesystem::is_directory(iter_start->status()) == true)
				{
					const std::wstring subDirectory = iter_start->path();
					GetFiles(subDirectory, extension, files_out);
				}
				else if (extension.empty() == true || extension == L".*" || iter_start->path().extension() == extension)
				{
					files_out.emplace_back(iter_start->path());
				}
			}
		}

		const wchar_t* GetEngineDataPath()
		{
			static std::wstring path;
			if (path.empty() == false)
				return path.c_str();

			path = string::MultiToWide(EST_DATA_PATH);
			return path.c_str();
		}

		const wchar_t* GetBinPath()
		{
			static std::wstring path;
			if (path.empty() == false)
				return path.c_str();

			wchar_t strModulePath[MAX_PATH]{};
			GetModuleFileName(NULL, strModulePath, MAX_PATH);	// 실행파일경로

			// 실행파일명 제거
			path = strModulePath;
			path = GetFilePath(path);

			return path.c_str();
		}

		const wchar_t* GetProgramFileName()
		{
			static std::wstring path;
			if (path.empty() == false)
				return path.c_str();

			wchar_t strModulePath[MAX_PATH]{};
			GetModuleFileName(NULL, strModulePath, MAX_PATH);	// 실행파일경로

			path = GetFileNameWithoutExtension(strModulePath);

			return path.c_str();
		}
	}
}