#pragma once

namespace est
{
	namespace graphics
	{
		class Model;
		class Motion;

		namespace XPSImport
		{
			bool LoadModel(Model* pModel, const wchar_t* filePath, const std::string* pStrDevideModels, size_t nKeywordCount);
			bool LoadMotion(Motion* pMotion, const wchar_t* filePath);
		}
	}
}