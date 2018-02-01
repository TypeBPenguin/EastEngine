#pragma once

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class MotionRecorder : public IMotionRecorder
		{
		public:
			MotionRecorder();
			virtual ~MotionRecorder();

		public:
			virtual void SetTransform(const String::StringID& strBoneName, const Math::Transform& keyframe) override;
			virtual const Math::Transform* GetTransform(const String::StringID& strBoneName) const override;

		public:
			void Clear();

		private:
			std::unordered_map<String::StringID, Math::Transform> m_umapMotionData;
		};
	}
}