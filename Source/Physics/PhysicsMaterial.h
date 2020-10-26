#pragma once

#include "PhysicsInterface.h"
#include "PhysicsUtil.h"

namespace est
{
	namespace physics
	{
		class Material : public IMaterial
		{
		public:
			Material(physx::PxMaterial* pMaterial);
			virtual ~Material();

		public:
			virtual void SetDynamicFriction(float coef) override { m_pMaterial->setDynamicFriction(coef); }
			virtual float GetDynamicFriction() const override { return m_pMaterial->getDynamicFriction(); }

			virtual void SetStaticFriction(float coef) override { m_pMaterial->setStaticFriction(coef); }
			virtual float GetStaticFriction() const override { return m_pMaterial->getStaticFriction(); }

			virtual void SetRestitution(float rest) override { m_pMaterial->setRestitution(rest); }
			virtual float GetRestitution() const override { return m_pMaterial->getRestitution(); }

			virtual void SetFlag(Flag flag, bool isEnable) override { m_pMaterial->setFlag(Convert<physx::PxMaterialFlag::Enum>(flag), isEnable); }
			virtual void SetFlags(Flags flags) override { m_pMaterial->setFlags(Convert<physx::PxMaterialFlags>(flags)); }

			virtual Flags GetFlags() const override
			{
				const physx::PxMaterialFlags flags = m_pMaterial->getFlags();
				return Convert<const Flags>(flags);
			}

			virtual void SetFrictionCombineMode(CombineMode combMode) override { m_pMaterial->setFrictionCombineMode(Convert<physx::PxCombineMode::Enum>(combMode)); }
			virtual CombineMode GetFrictionCombineMode() const override
			{
				const physx::PxCombineMode::Enum combMode = m_pMaterial->getFrictionCombineMode();
				return Convert<const CombineMode>(combMode);
			}

			virtual void SetRestitutionCombineMode(CombineMode combMode) override { m_pMaterial->setRestitutionCombineMode(Convert<physx::PxCombineMode::Enum>(combMode)); }
			virtual CombineMode GetRestitutionCombineMode() const override
			{
				const physx::PxCombineMode::Enum combMode = m_pMaterial->getRestitutionCombineMode();
				return Convert<const CombineMode>(combMode);
			}

		public:
			physx::PxMaterial* GetInterface() const { return m_pMaterial; }

		private:
			physx::PxMaterial* m_pMaterial{ nullptr };
		};
	}
}