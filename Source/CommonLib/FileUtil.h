#pragma once

namespace est
{
	namespace file
	{
		std::string GetFileName(const std::string& path);	// ��θ��� ���ϸ�.Ȯ���� ����
		std::string GetFileNameWithoutExtension(const std::string& path);
		std::wstring GetFileName(const std::wstring& path);	// ��θ��� ���ϸ�.Ȯ���� ����
		std::wstring GetFileNameWithoutExtension(const std::wstring& path);

		std::string GetFileExtension(const std::string& path);
		std::wstring GetFileExtension(const std::wstring& path);

		std::string GetFilePath(const std::string& path);		// ���ϸ� ������ ��θ� ����
		std::wstring GetFilePath(const std::wstring& path);		// ���ϸ� ������ ��θ� ����

		bool IsExists(const std::string& path);
		bool IsExists(const std::wstring& path);
		void GetFiles(const std::string& directoryPath, const std::string& extension, std::vector<std::string>& files_out);
		void GetFiles(const std::wstring& directoryPath, const std::wstring& extension, std::vector<std::wstring>& files_out);

		const wchar_t* GetEngineDataPath();
		const wchar_t* GetBinPath();	// ���� ���� ���, Bin Path
		const wchar_t* GetProgramFileName();
	}
}