#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		class Model;

		namespace XPSImport
		{
			bool LoadModel(Model* pModel, const char* strFilePath);
		}
	}
}