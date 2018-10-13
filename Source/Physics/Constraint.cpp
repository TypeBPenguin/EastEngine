#include "stdafx.h"
#include "Constraint.h"

#include "PhysicsSystem.h"
#include "MathConvertor.h"
#include "RigidBody.h"

namespace eastengine
{
	namespace physics
	{
		ConstraintInterface::ConstraintInterface(EmConstraint::Type emConstraintType)
			: m_emConstraintType(emConstraintType)
			, m_pDynamicsWorld(nullptr)
		{
			m_pDynamicsWorld = System::GetInstance()->GetDynamicsWorld();
		}

		ConstraintInterface::~ConstraintInterface()
		{
		}

		ConstraintInterface* ConstraintInterface::Create(const ConstraintProperty& constraintProperty)
		{
			switch (constraintProperty.emType)
			{
			case EmConstraint::eConeTwist:
				break;
			case EmConstraint::eContact:
				break;
			case EmConstraint::eGear:
				break;
			case EmConstraint::eGeneric6Dof:
			{
				return Generic6DofConstraint::Create(constraintProperty);
			}
			break;
			case EmConstraint::eGeneric6DofSpring2:
				break;
			case EmConstraint::eHinge:
				break;
			case EmConstraint::ePoint2Point:
				break;
			case EmConstraint::eSlider:
				break;
			}

			return nullptr;
		}

		Generic6DofConstraint::Generic6DofConstraint()
			: ConstraintInterface(EmConstraint::Type::eGeneric6Dof)
			, m_pGeneric6DofConstraint(nullptr)
		{
		}

		Generic6DofConstraint::~Generic6DofConstraint()
		{
			if (m_pDynamicsWorld != nullptr)
			{
				m_pDynamicsWorld->removeConstraint(m_pGeneric6DofConstraint);
				m_pDynamicsWorld = nullptr;
			}

			SafeDelete(m_pGeneric6DofConstraint);
		}

		Generic6DofConstraint* Generic6DofConstraint::Create(const ConstraintProperty& constraintProperty)
		{
			RigidBody* pRigidBodyA = constraintProperty.generic6Dof.pRigidBodyA;
			RigidBody* pRigidBodyB = constraintProperty.generic6Dof.pRigidBodyB;
			if (pRigidBodyA == nullptr || pRigidBodyB == nullptr)
				return nullptr;
			
			Generic6DofConstraint* pGeneric6DofConstraint = nullptr;
			if (pRigidBodyA != nullptr || pRigidBodyB != nullptr)
			{
				bool isEnableLinearReferenceFrameA = constraintProperty.generic6Dof.isEnableLinearRefrenceFrameA;
				math::Matrix matA = math::Matrix::Compose(math::Vector3::One, constraintProperty.generic6Dof.originQuatA, constraintProperty.generic6Dof.f3OriginPosA);
				math::Matrix matB = math::Matrix::Compose(math::Vector3::One, constraintProperty.generic6Dof.originQuatB, constraintProperty.generic6Dof.f3OriginPosB);

				pGeneric6DofConstraint = new Generic6DofConstraint;
				pGeneric6DofConstraint->m_pGeneric6DofConstraint = new btGeneric6DofConstraint(*pRigidBodyA->GetInterface(), *pRigidBodyB->GetInterface(), math::ConvertToBt(matA), math::ConvertToBt(matB), isEnableLinearReferenceFrameA);

			}
			else if (pRigidBodyA == nullptr || pRigidBodyB != nullptr)
			{
				bool isEnableLinearReferenceFrameA = constraintProperty.generic6Dof.isEnableLinearRefrenceFrameA;
				math::Matrix matB = math::Matrix::Compose(math::Vector3::One, constraintProperty.generic6Dof.originQuatB, constraintProperty.generic6Dof.f3OriginPosB);

				pGeneric6DofConstraint = new Generic6DofConstraint;
				pGeneric6DofConstraint->m_pGeneric6DofConstraint = new btGeneric6DofConstraint(*pRigidBodyB->GetInterface(), math::ConvertToBt(matB), isEnableLinearReferenceFrameA);
			}

			if (pGeneric6DofConstraint == nullptr)
				return nullptr;

			System::GetInstance()->AddConstraint(pGeneric6DofConstraint, constraintProperty.generic6Dof.isEnableCollisionBetweenLinkedBodies);

			return pGeneric6DofConstraint;
		}

		btTypedConstraint* Generic6DofConstraint::GetInterface()
		{
			return m_pGeneric6DofConstraint;
		}

		void Generic6DofConstraint::CalculateTransforms(const math::Matrix& transA, const math::Matrix& transB)
		{
			m_pGeneric6DofConstraint->calculateTransforms(math::ConvertToBt(transA), math::ConvertToBt(transB));
		}

		void Generic6DofConstraint::CalculateTransforms()
		{
			m_pGeneric6DofConstraint->calculateTransforms();
		}

		math::Matrix Generic6DofConstraint::GetCalculatedTransformA() const
		{
			return math::Convert(m_pGeneric6DofConstraint->getCalculatedTransformA());
		}

		math::Matrix Generic6DofConstraint::GetCalculatedTransformB() const
		{
			return math::Convert(m_pGeneric6DofConstraint->getCalculatedTransformB());
		}

		math::Matrix Generic6DofConstraint::GetFrameOffSetA() const
		{
			return math::Convert(m_pGeneric6DofConstraint->getFrameOffsetA());
		}

		math::Matrix Generic6DofConstraint::GetFrameOffSetB() const
		{
			return math::Convert(m_pGeneric6DofConstraint->getFrameOffsetB());
		}

		math::Matrix Generic6DofConstraint::GetFrameOffSetA()
		{
			return math::Convert(m_pGeneric6DofConstraint->getFrameOffsetA());
		}

		math::Matrix Generic6DofConstraint::GetFrameOffSetB()
		{
			return math::Convert(m_pGeneric6DofConstraint->getFrameOffsetB());
		}

		void Generic6DofConstraint::BuildJacobian()
		{
			m_pGeneric6DofConstraint->buildJacobian();
		}

		void Generic6DofConstraint::UpdateRHS(float fTimeStep)
		{
			m_pGeneric6DofConstraint->updateRHS(fTimeStep);
		}

		math::Vector3 Generic6DofConstraint::GetAxis(int nAxis_index) const
		{
			return math::Convert(m_pGeneric6DofConstraint->getAxis(nAxis_index));
		}

		float Generic6DofConstraint::GetAngle(int nAxis_index) const
		{
			return m_pGeneric6DofConstraint->getAngle(nAxis_index);
		}

		float Generic6DofConstraint::GetRelativePivotPosition(int nAxis_index) const
		{
			return m_pGeneric6DofConstraint->getRelativePivotPosition(nAxis_index);
		}

		void Generic6DofConstraint::SetFrames(const math::Matrix& frameA, const math::Matrix& frameB)
		{
			m_pGeneric6DofConstraint->setFrames(math::ConvertToBt(frameA), math::ConvertToBt(frameB));
		}

		bool Generic6DofConstraint::TestAngularLimitMotor(int nAxis_index)
		{
			return m_pGeneric6DofConstraint->testAngularLimitMotor(nAxis_index);
		}

		void Generic6DofConstraint::SetLinearLowerLimit(const math::Vector3& f3LinearLower)
		{
			m_pGeneric6DofConstraint->setLinearLowerLimit(math::ConvertToBt(f3LinearLower));
		}

		void Generic6DofConstraint::GetLinearLowerLimit(math::Vector3& f3LinearLower) const
		{
			btVector3 f3Temp;
			m_pGeneric6DofConstraint->getLinearLowerLimit(f3Temp);

			f3LinearLower = math::Convert(f3Temp);
		}

		void Generic6DofConstraint::SetLinearUpperLimit(const math::Vector3& f3LinearUpper)
		{
			m_pGeneric6DofConstraint->setLinearUpperLimit(math::ConvertToBt(f3LinearUpper));
		}

		void Generic6DofConstraint::GetLinearUpperLimit(math::Vector3& f3LinearUpper) const
		{
			btVector3 f3Temp;
			m_pGeneric6DofConstraint->getLinearUpperLimit(f3Temp);

			f3LinearUpper = math::Convert(f3Temp);
		}

		void Generic6DofConstraint::SetAngularLowerLimit(const math::Vector3& f3AngularLower)
		{
			m_pGeneric6DofConstraint->setAngularLowerLimit(math::ConvertToBt(f3AngularLower));
		}

		void Generic6DofConstraint::GetAngularLowerLimit(math::Vector3& f3AngularLower) const
		{
			btVector3 f3Temp;
			m_pGeneric6DofConstraint->getAngularLowerLimit(f3Temp);

			f3AngularLower = math::Convert(f3Temp);
		}

		void Generic6DofConstraint::SetAngularUpperLimit(const math::Vector3& f3AngularUpper)
		{
			m_pGeneric6DofConstraint->setAngularUpperLimit(math::ConvertToBt(f3AngularUpper));
		}

		void Generic6DofConstraint::GetAngularUpperLimit(math::Vector3& f3AngularUpper) const
		{
			btVector3 f3Temp;
			m_pGeneric6DofConstraint->getAngularUpperLimit(f3Temp);

			f3AngularUpper = math::Convert(f3Temp);
		}

		void Generic6DofConstraint::SetLimit(int nAxis, float lo, float hi)
		{
			m_pGeneric6DofConstraint->setLimit(nAxis, lo, hi);
		}

		bool Generic6DofConstraint::IsLimited(int nLimitIndex) const
		{
			return m_pGeneric6DofConstraint->isLimited(nLimitIndex);
		}

		void Generic6DofConstraint::CalcAnchorPos(void)
		{
			m_pGeneric6DofConstraint->calcAnchorPos();
		}

		bool Generic6DofConstraint::GetUseFrameOffSet() const
		{
			return m_pGeneric6DofConstraint->getUseFrameOffset();
		}

		void Generic6DofConstraint::SetUseFrameOffSet(bool isFrameOffSetOnOff)
		{
			m_pGeneric6DofConstraint->setUseFrameOffset(isFrameOffSetOnOff);
		}

		bool Generic6DofConstraint::GetUseLinearReferenceFrameA() const
		{
			return m_pGeneric6DofConstraint->getUseLinearReferenceFrameA();
		}

		void Generic6DofConstraint::SetUseLinearReferenceFrameA(bool isLinearReferenceFrameA)
		{
			m_pGeneric6DofConstraint->setUseLinearReferenceFrameA(isLinearReferenceFrameA);
		}

		void Generic6DofConstraint::SetParam(int nNum, float fValue, int nAxis)
		{
			m_pGeneric6DofConstraint->setParam(nNum, fValue, nAxis);
		}

		float Generic6DofConstraint::GetParam(int nNum, int nAxis) const
		{
			return m_pGeneric6DofConstraint->getParam(nNum, nAxis);
		}

		void Generic6DofConstraint::SetAxis(const math::Vector3& f3Axis1, const math::Vector3& f3Axis2)
		{
			m_pGeneric6DofConstraint->setAxis(math::ConvertToBt(f3Axis1), math::ConvertToBt(f3Axis2));
		}

		int Generic6DofConstraint::GetFlags() const
		{
			return m_pGeneric6DofConstraint->getFlags();
		}

		int Generic6DofConstraint::CalculateSerializeBufferSize() const
		{
			return m_pGeneric6DofConstraint->calculateSerializeBufferSize();
		}
	}
}