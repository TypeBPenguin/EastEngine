#pragma once

namespace ATG
{
	class ExportScene;
}

namespace EastEngine
{
	namespace Graphics
	{
		class IMotion;
		class IModel;
		class IModelNode;

		bool WriteModel(IModel* pModel, const std::unordered_map<String::StringID, Math::Matrix>& umapMotionOffset);
		bool WriteMotion(IMotion* pMotion);
	}
}