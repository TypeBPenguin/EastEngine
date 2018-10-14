#pragma once

namespace eastengine
{
	namespace file
	{
		std::string GetFileName(const std::string& strPath);	// ��θ��� ���ϸ�.Ȯ���� ����
		std::string GetFileNameWithoutExtension(const std::string& strPath);
		std::wstring GetFileName(const std::wstring& strPath);	// ��θ��� ���ϸ�.Ȯ���� ����
		std::wstring GetFileNameWithoutExtension(const std::wstring& strPath);

		std::string GetFileExtension(const std::string& strPath);
		std::wstring GetFileExtension(const std::wstring& strPath);

		std::string GetFilePath(const std::string& strPath);		// ���ϸ� ������ ��θ� ����
		std::wstring GetFilePath(const std::wstring& strPath);		// ���ϸ� ������ ��θ� ����

		bool IsExists(const std::string& strPath);
		void GetFiles(const std::string& strDirectoryPath, const std::string& strExtension, std::vector<std::string>& vecFiles_out);

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
		const char* GetProgramFileName();
	}
}