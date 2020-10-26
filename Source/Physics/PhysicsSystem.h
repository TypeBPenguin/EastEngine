#pragma once

#include "CommonLib/Singleton.h"

#include "PhysicsDefine.h"

namespace est
{
	namespace physics
	{
		class System : public Singleton<System>
		{
			friend Singleton<System>;
		private:
			System();
			virtual ~System();

		public:
			bool Initialize(const Initializer& initializer);

		public:
			void Update(float elapsedtime);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}