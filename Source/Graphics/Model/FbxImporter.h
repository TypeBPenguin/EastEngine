#pragma once

namespace est
{
	namespace graphics
	{
		class Model;
		class Motion;

		class FBXImport
		{
		public:
			FBXImport();
			~FBXImport();

		public:
			bool Initialize();
			void Release();

		public:
			bool LoadModel(Model* pModel, const wchar_t* filePath, float scale, bool isFlipZ);
			bool LoadMotion(Motion* pMotion, const wchar_t* filePath, float scale);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}