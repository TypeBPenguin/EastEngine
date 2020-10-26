#pragma once

namespace est
{
	namespace file
	{
		std::string GetFileName(const std::string& path);	// 경로명에서 파일명.확장자 리턴
		std::string GetFileNameWithoutExtension(const std::string& path);
		std::wstring GetFileName(const std::wstring& path);	// 경로명에서 파일명.확장자 리턴
		std::wstring GetFileNameWithoutExtension(const std::wstring& path);

		std::string GetFileExtension(const std::string& path);
		std::wstring GetFileExtension(const std::wstring& path);

		std::string GetFilePath(const std::string& path);		// 파일명 제외한 경로명 리턴
		std::wstring GetFilePath(const std::wstring& path);		// 파일명 제외한 경로명 리턴

		bool IsExists(const std::string& path);
		bool IsExists(const std::wstring& path);
		void GetFiles(const std::string& directoryPath, const std::string& extension, std::vector<std::string>& files_out);
		void GetFiles(const std::wstring& directoryPath, const std::wstring& extension, std::vector<std::wstring>& files_out);

		const wchar_t* GetEngineDataPath();
		const wchar_t* GetBinPath();	// 현재 실행 경로, Bin Path
		const wchar_t* GetProgramFileName();
	}
}