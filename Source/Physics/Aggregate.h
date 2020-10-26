#pragma once

#include "PhysicsInterface.h"

namespace est
{
	namespace physics
	{
		class Aggregate : public IAggregate
		{
		public:
			Aggregate(physx::PxAggregate* pAggregate);
			virtual ~Aggregate();

		private:
			virtual bool AddActor(IRigidActor* pActor, const IBVHStructure* IBVHStructure = nullptr) override;
			virtual bool RemoveActor(IRigidActor* pActor) override;

			virtual bool AddArticulation(IArticulationBase* pArticulation) override;
			virtual bool RemoveArticulation(IArticulationBase* pArticulation) override;

			virtual uint32_t GetNumActors() const override;
			virtual uint32_t GetMaxNumActors() const override;

			virtual uint32_t GetActors(IRigidActor** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const override;

			virtual bool GetSelfCollision() const override;

		public:
			void Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch = true);

		public:
			physx::PxAggregate* GetInterface() const { return m_pAggregate; }

		private:
			physx::PxAggregate* m_pAggregate{ nullptr };
			bool m_isValid{ false };
		};
	}
}