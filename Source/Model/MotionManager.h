#pragma once

#include "CommonLib/Singleton.h"

#include "MotionLoader.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IMotion;

		class MotionManager : public Singleton<MotionManager>
		{
			friend Singleton<MotionManager>;
		private:
			MotionManager();
			virtual ~MotionManager();

		public:
			bool Init();
			void Release();

			void Update();
			void Flush();

			void ProcessRequestMotionLoader();

			bool AddMotion(const String::StringID& strMotionName, IMotion* pModel);
			IMotion* GetMotion(const String::StringID& strName);

			const boost::unordered_map<String::StringID, IMotion*>& GetMotions() { return m_umapMotion; }

		private:
			bool m_isInit;

			boost::unordered_map<String::StringID, IMotion*> m_umapMotion;
		};
	}
}