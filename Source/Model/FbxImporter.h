#pragma once

namespace eastengine
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
			bool LoadModel(Model* pModel, const char* strFilePath, float fScale, bool isFlipZ);
			bool LoadMotion(Motion* pMotion, const char* strFilePath, float fScale);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}