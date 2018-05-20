#pragma once

#include "ModelInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class MotionRecorder : public IMotionRecorder
		{
		public:
			MotionRecorder();
			virtual ~MotionRecorder();

		public:
			virtual void SetTransform(const String::StringID& strBoneName, const math::Transform& keyframe) override;
			virtual const math::Transform* GetTransform(const String::StringID& strBoneName) const override;

		public:
			void Clear();

		private:
			std::unordered_map<String::StringID, math::Transform> m_umapMotionData;
		};
	}
}