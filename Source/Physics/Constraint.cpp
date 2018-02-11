#include "stdafx.h"
#include "Constraint.h"

#include "PhysicsSystem.h"
#include "MathConvertor.h"
#include "RigidBody.h"

namespace EastEngine
{
	namespace Physics
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
				Math::Matrix matA = Math::Matrix::Compose(Math::Vector3::One, constraintProperty.generic6Dof.originQuatA, constraintProperty.generic6Dof.f3OriginPosA);
				Math::Matrix matB = Math::Matrix::Compose(Math::Vector3::One, constraintProperty.generic6Dof.originQuatB, constraintProperty.generic6Dof.f3OriginPosB);

				pGeneric6DofConstraint = new Generic6DofConstraint;
				pGeneric6DofConstraint->m_pGeneric6DofConstraint = new btGeneric6DofConstraint(*pRigidBodyA->GetInterface(), *pRigidBodyB->GetInterface(), Math::ConvertToBt(matA), Math::ConvertToBt(matB), isEnableLinearReferenceFrameA);

			}
			else if (pRigidBodyA == nullptr || pRigidBodyB != nullptr)
			{
				bool isEnableLinearReferenceFrameA = constraintProperty.generic6Dof.isEnableLinearRefrenceFrameA;
				Math::Matrix matB = Math::Matrix::Compose(Math::Vector3::One, constraintProperty.generic6Dof.originQuatB, constraintProperty.generic6Dof.f3OriginPosB);

				pGeneric6DofConstraint = new Generic6DofConstraint;
				pGeneric6DofConstraint->m_pGeneric6DofConstraint = new btGeneric6DofConstraint(*pRigidBodyB->GetInterface(), Math::ConvertToBt(matB), isEnableLinearReferenceFrameA);
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

		void Generic6DofConstraint::CalculateTransforms(const Math::Matrix& transA, const Math::Matrix& transB)
		{
			m_pGeneric6DofConstraint->calculateTransforms(Math::ConvertToBt(transA), Math::ConvertToBt(transB));
		}

		void Generic6DofConstraint::CalculateTransforms()
		{
			m_pGeneric6DofConstraint->calculateTransforms();
		}

		Math::Matrix Generic6DofConstraint::GetCalculatedTransformA() const
		{
			return Math::Convert(m_pGeneric6DofConstraint->getCalculatedTransformA());
		}

		Math::Matrix Generic6DofConstraint::GetCalculatedTransformB() const
		{
			return Math::Convert(m_pGeneric6DofConstraint->getCalculatedTransformB());
		}

		Math::Matrix Generic6DofConstraint::GetFrameOffSetA() const
		{
			return Math::Convert(m_pGeneric6DofConstraint->getFrameOffsetA());
		}

		Math::Matrix Generic6DofConstraint::GetFrameOffSetB() const
		{
			return Math::Convert(m_pGeneric6DofConstraint->getFrameOffsetB());
		}

		Math::Matrix Generic6DofConstraint::GetFrameOffSetA()
		{
			return Math::Convert(m_pGeneric6DofConstraint->getFrameOffsetA());
		}

		Math::Matrix Generic6DofConstraint::GetFrameOffSetB()
		{
			return Math::Convert(m_pGeneric6DofConstraint->getFrameOffsetB());
		}

		void Generic6DofConstraint::BuildJacobian()
		{
			m_pGeneric6DofConstraint->buildJacobian();
		}

		void Generic6DofConstraint::UpdateRHS(float fTimeStep)
		{
			m_pGeneric6DofConstraint->updateRHS(fTimeStep);
		}

		Math::Vector3 Generic6DofConstraint::GetAxis(int nAxis_index) const
		{
			return Math::Convert(m_pGeneric6DofConstraint->getAxis(nAxis_index));
		}

		float Generic6DofConstraint::GetAngle(int nAxis_index) const
		{
			return m_pGeneric6DofConstraint->getAngle(nAxis_index);
		}

		float Generic6DofConstraint::GetRelativePivotPosition(int nAxis_index) const
		{
			return m_pGeneric6DofConstraint->getRelativePivotPosition(nAxis_index);
		}

		void Generic6DofConstraint::SetFrames(const Math::Matrix& frameA, const Math::Matrix& frameB)
		{
			m_pGeneric6DofConstraint->setFrames(Math::ConvertToBt(frameA), Math::ConvertToBt(frameB));
		}

		bool Generic6DofConstraint::TestAngularLimitMotor(int nAxis_index)
		{
			return m_pGeneric6DofConstraint->testAngularLimitMotor(nAxis_index);
		}

		void Generic6DofConstraint::SetLinearLowerLimit(const Math::Vector3& f3LinearLower)
		{
			m_pGeneric6DofConstraint->setLinearLowerLimit(Math::ConvertToBt(f3LinearLower));
		}

		void Generic6DofConstraint::GetLinearLowerLimit(Math::Vector3& f3LinearLower) const
		{
			btVector3 f3Temp;
			m_pGeneric6DofConstraint->getLinearLowerLimit(f3Temp);

			f3LinearLower = Math::Convert(f3Temp);
		}

		void Generic6DofConstraint::SetLinearUpperLimit(const Math::Vector3& f3LinearUpper)
		{
			m_pGeneric6DofConstraint->setLinearUpperLimit(Math::ConvertToBt(f3LinearUpper));
		}

		void Generic6DofConstraint::GetLinearUpperLimit(Math::Vector3& f3LinearUpper) const
		{
			btVector3 f3Temp;
			m_pGeneric6DofConstraint->getLinearUpperLimit(f3Temp);

			f3LinearUpper = Math::Convert(f3Temp);
		}

		void Generic6DofConstraint::SetAngularLowerLimit(const Math::Vector3& f3AngularLower)
		{
			m_pGeneric6DofConstraint->setAngularLowerLimit(Math::ConvertToBt(f3AngularLower));
		}

		void Generic6DofConstraint::GetAngularLowerLimit(Math::Vector3& f3AngularLower) const
		{
			btVector3 f3Temp;
			m_pGeneric6DofConstraint->getAngularLowerLimit(f3Temp);

			f3AngularLower = Math::Convert(f3Temp);
		}

		void Generic6DofConstraint::SetAngularUpperLimit(const Math::Vector3& f3AngularUpper)
		{
			m_pGeneric6DofConstraint->setAngularUpperLimit(Math::ConvertToBt(f3AngularUpper));
		}

		void Generic6DofConstraint::GetAngularUpperLimit(Math::Vector3& f3AngularUpper) const
		{
			btVector3 f3Temp;
			m_pGeneric6DofConstraint->getAngularUpperLimit(f3Temp);

			f3AngularUpper = Math::Convert(f3Temp);
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

		void Generic6DofConstraint::SetAxis(const Math::Vector3& f3Axis1, const Math::Vector3& f3Axis2)
		{
			m_pGeneric6DofConstraint->setAxis(Math::ConvertToBt(f3Axis1), Math::ConvertToBt(f3Axis2));
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