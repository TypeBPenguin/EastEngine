#pragma once

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class MotionRecoder : public IMotionRecoder
		{
		public:
			MotionRecoder();
			virtual ~MotionRecoder();

		public:
			virtual void SetCaching(const String::StringID& strBoneName, size_t nIndex) override;
			virtual size_t GetCaching(const String::StringID& strBoneName) const override;

			virtual void SetKeyframe(const String::StringID& strBoneName, const IMotion::Keyframe& keyframe) override;
			virtual const IMotion::Keyframe* GetKeyframe(const String::StringID& strBoneName) const override;

		public:
			void Clear();

		private:
			struct keyframeCachingT {};
			using KeyframeCaching = Type<keyframeCachingT, size_t>;

			std::unordered_map<String::StringID, KeyframeCaching> m_umapKeyframeIndexCaching;
			std::unordered_map<String::StringID, IMotion::Keyframe> m_umapKeyframe;
		};
	}
}