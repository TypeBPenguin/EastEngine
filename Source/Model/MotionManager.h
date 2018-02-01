#pragma once

#include "CommonLib/Singleton.h"

#include "MotionLoader.h"

namespace EastEngine
{
	namespace Graphics
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
			void Update();
			void Flush();

		public:
			// FilePath
			IMotion* AllocateMotion(const std::string& strKey);

			// FilePath
			IMotion* GetMotion(const std::string& strKey);
			IMotion* GetMotion(const size_t nIndex);

			size_t GetMotionCount() const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}