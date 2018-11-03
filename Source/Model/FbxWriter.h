#pragma once

namespace ATG
{
	class ExportScene;
}

namespace eastengine
{
	namespace graphics
	{
		class Motion;
		class Model;

		bool WriteModel(Model* pModel, const std::unordered_map<string::StringID, math::Matrix>& umapMotionOffset);
		bool WriteMotion(Motion* pMotion);
	}
}