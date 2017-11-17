#pragma once

namespace EastEngine
{
	namespace File
	{
		std::string GetFileName(const std::string& strPath);	// ��θ��� ���ϸ�.Ȯ���� ����
		std::string GetFileNameWithoutExtension(const char* strPath);
		std::wstring GetFileName(const std::wstring& strPath);	// ��θ��� ���ϸ�.Ȯ���� ����
		std::wstring GetFileNameWithoutExtension(const wchar_t* strPath);

		std::string GetFileExtension(const std::string& strPath);
		std::wstring GetFileExtension(const std::wstring& strPath);

		std::string GetFilePath(const std::string& strPath);		// ���ϸ� ������ ��θ� ����
		std::string GetFilePath(const char* strPath);				// ���ϸ� ������ ��θ� ����
		std::wstring GetFilePath(const std::wstring& strPath);		// ���ϸ� ������ ��θ� ����
		std::wstring GetFilePath(const wchar_t* strPath);			// ���ϸ� ������ ��θ� ����

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
		const char* GetBinPath();	// ���� ���� ���, Bin Path
	}
}