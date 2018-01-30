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
			struct MotionData
			{
				struct keyframeCachingT {};
				using KeyframeCaching = PhantomType<keyframeCachingT, size_t>;

				KeyframeCaching caching;
				IMotion::Keyframe keyframe;

				MotionData(size_t nIndex)
					: caching(nIndex)
				{
				}

				MotionData(const IMotion::Keyframe& keyframe)
					: caching(eInvalidCachingIndex)
					, keyframe(keyframe)
				{
				}
			};
			std::unordered_map<String::StringID, MotionData> m_umapMotionData;
		};
	}
}