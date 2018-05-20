#pragma once

#include "CommonLib/Singleton.h"

#include "MotionLoader.h"

namespace eastengine
{
	namespace graphics
	{
		class Motion;
		class IMotion;

		class MotionManager : public Singleton<MotionManager>
		{
			friend Singleton<MotionManager>;
		private:
			MotionManager();
			virtual ~MotionManager();

		public:
			void Flush(bool isEnableGarbageCollector);

		public:
			// FilePath
			IMotion* AllocateMotion(const String::StringID& strKey);

			// FilePath
			IMotion* GetMotion(const String::StringID& strKey);
			IMotion* GetMotion(const size_t nIndex);

			size_t GetMotionCount() const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}