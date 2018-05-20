#pragma once

namespace eastengine
{
	namespace graphics
	{
		class Model;
		class Motion;

		namespace XPSImport
		{
			bool LoadModel(Model* pModel, const char* strFilePath, const std::string* pStrDevideModels, size_t nKeywordCount);
			bool LoadMotion(Motion* pMotion, const char* strFilePath);
		}
	}
}