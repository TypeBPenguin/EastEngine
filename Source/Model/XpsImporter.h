#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		class Model;
		class Motion;

		namespace XPSImport
		{
			bool LoadModel(Model* pModel, const char* strFilePath);
			bool LoadMotion(Motion* pMotion, const char* strFilePath);
		}
	}
}