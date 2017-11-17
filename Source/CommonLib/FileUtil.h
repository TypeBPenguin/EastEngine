#pragma once

namespace EastEngine
{
	namespace File
	{
		std::string GetFileName(const std::string& strPath);	// 경로명에서 파일명.확장자 리턴
		std::string GetFileNameWithoutExtension(const char* strPath);
		std::wstring GetFileName(const std::wstring& strPath);	// 경로명에서 파일명.확장자 리턴
		std::wstring GetFileNameWithoutExtension(const wchar_t* strPath);

		std::string GetFileExtension(const std::string& strPath);
		std::wstring GetFileExtension(const std::wstring& strPath);

		std::string GetFilePath(const std::string& strPath);		// 파일명 제외한 경로명 리턴
		std::string GetFilePath(const char* strPath);				// 파일명 제외한 경로명 리턴
		std::wstring GetFilePath(const std::wstring& strPath);		// 파일명 제외한 경로명 리턴
		std::wstring GetFilePath(const wchar_t* strPath);			// 파일명 제외한 경로명 리턴

		std::vector<std::string> GetFilesInFolder(const char* strPath, const char* strType = "*.*", bool bWithPath = false);

		enum EmPath
		{
			eFont = 0,
			eFx,
			eLua,
			eTexture,
			eUI,
			eXML,
			eSound,
		};

		const char* GetPath(EmPath emPath);
		const char* GetDataPath();
		const char* GetBinPath();	// 현재 실행 경로, Bin Path
	}
}