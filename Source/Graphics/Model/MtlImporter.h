#pragma once

namespace est
{
	namespace graphics
	{
		class MtlImporter
		{
		public:
			MtlImporter();
			~MtlImporter();

			bool Init(const wchar_t* fileName, const wchar_t* path);
			void Release();

			MaterialPtr GetMaterial(const string::StringID& name)
			{
				auto iter = m_umapNewMtrl.find(name);
				if (iter != m_umapNewMtrl.end())
					return iter->second;

				return nullptr;
			}

		private:
			std::unordered_map<string::StringID, MaterialPtr> m_umapNewMtrl;
		};
	}
}