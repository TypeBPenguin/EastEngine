#pragma once

namespace eastengine
{
	namespace graphics
	{
		class IMaterial;

		class MtlImporter
		{
		public:
			MtlImporter();
			~MtlImporter();

			bool Init(const char* strFileName, const char* strPath);
			void Release();

			IMaterial* GetMaterial(const String::StringID& strName)
			{
				auto iter = m_umapNewMtrl.find(strName);
				if (iter != m_umapNewMtrl.end())
					return iter->second;

				return nullptr;
			}

		private:
			std::unordered_map<String::StringID, IMaterial*> m_umapNewMtrl;
		};
	}
}