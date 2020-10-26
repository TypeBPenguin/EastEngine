#include "stdafx.h"
#include "Aggregate.h"

#include "PhysicsUtil.h"

#include "RigidBody.h"
#include "Articulation.h"
#include "BVHStructure.h"

namespace est
{
	namespace physics
	{
		Aggregate::Aggregate(physx::PxAggregate* pAggregate)
			: m_pAggregate(pAggregate)
		{
		}

		Aggregate::~Aggregate()
		{
			Remove(GetScene());
		}

		bool Aggregate::AddActor(IRigidActor* pActor, const IBVHStructure* IBVHStructure)
		{
			if (pActor == nullptr || IBVHStructure == nullptr)
				return false;

			physx::PxActor* pInterface = util::GetInterface(pActor);
			if (pInterface == nullptr)
				return false;

			return m_pAggregate->addActor(*pInterface, static_cast<const BVHStructure*>(IBVHStructure)->GetInterface());
		}

		bool Aggregate::RemoveActor(IRigidActor* pActor)
		{
			if (pActor == nullptr)
				return false;

			physx::PxActor* pInterface = util::GetInterface(pActor);
			if (pInterface == nullptr)
				return false;

			return m_pAggregate->removeActor(*pInterface);
		}

		bool Aggregate::AddArticulation(IArticulationBase* pArticulation)
		{
			if (pArticulation == nullptr)
				return false;

			physx::PxArticulationBase* pInterface = util::GetInterface(pArticulation);
			if (pInterface == nullptr)
				return false;

			return m_pAggregate->addArticulation(*pInterface);
		}

		bool Aggregate::RemoveArticulation(IArticulationBase* pArticulation)
		{
			if (pArticulation == nullptr)
				return false;

			physx::PxArticulationBase* pInterface = util::GetInterface(pArticulation);
			if (pInterface == nullptr)
				return false;

			return m_pAggregate->removeArticulation(*pInterface);
		}

		uint32_t Aggregate::GetNumActors() const
		{
			return m_pAggregate->getNbActors();
		}

		uint32_t Aggregate::GetMaxNumActors() const
		{
			return m_pAggregate->getMaxNbActors();
		}

		uint32_t Aggregate::GetActors(IRigidActor** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex) const
		{
			std::vector<physx::PxActor*> pInterfaces(bufferSize);
			const uint32_t result = m_pAggregate->getActors(pInterfaces.data(), bufferSize, startIndex);

			for (uint32_t i = 0; i < bufferSize; ++i)
			{
				if (pInterfaces[i] != nullptr)
				{
					ppUserBuffer[i] = static_cast<IRigidActor*>(pInterfaces[i]->userData);
				}
			}
			return result;
		}

		bool Aggregate::GetSelfCollision() const
		{
			return m_pAggregate->getSelfCollision();
		}

		void Aggregate::Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch)
		{
			if (m_isValid == true)
			{
				pScene->removeAggregate(*GetInterface(), isEnableWakeOnLostTouch);
				m_isValid = false;
			}
		}
	}
}