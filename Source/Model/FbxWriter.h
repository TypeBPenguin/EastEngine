#pragma once

namespace ATG
{
	class ExportScene;
}

namespace EastEngine
{
	namespace Graphics
	{
		class Motion;
		class Model;

		bool WriteModel(Model* pModel, const std::unordered_map<String::StringID, Math::Matrix>& umapMotionOffset);
		bool WriteMotion(Motion* pMotion);
	}
}