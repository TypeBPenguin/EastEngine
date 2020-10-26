#pragma once

namespace ATG
{
	class ExportScene;
}

namespace est
{
	namespace graphics
	{
		class Motion;
		class Model;

		bool WriteModel(Model* pModel, const tsl::robin_map<std::string, math::Matrix>& umapMotionOffset);
		bool WriteMotion(Motion* pMotion);
	}
}