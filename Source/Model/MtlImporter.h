#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		class IMaterial;

		class CMtlImporter
		{
		public:
			CMtlImporter();
			~CMtlImporter();

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
			boost::unordered_map<String::StringID, IMaterial*> m_umapNewMtrl;
		};
	}
}