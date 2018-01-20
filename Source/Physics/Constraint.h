#pragma once

#include "PhysicsDefine.h"

class btTypedConstraint;
class btGeneric6DofConstraint;

namespace EastEngine
{
	namespace Physics
	{
		class RigidBody;

		namespace EmConstraint
		{
			enum Type
			{
				eConeTwist = 0,
				eContact,
				eGear,
				eGeneric6Dof,
				eGeneric6DofSpring2,
				eHinge,
				ePoint2Point,
				eSlider,
			};
		}

		struct ConstraintProperty
		{
			EmConstraint::Type emType;

			struct Generic6Dof
			{
				RigidBody* pRigidBodyA;
				Math::Vector3 f3OriginPosA;
				Math::Quaternion originQuatA;
				RigidBody* pRigidBodyB;
				Math::Vector3 f3OriginPosB;
				Math::Quaternion originQuatB;
				bool isEnableLinearRefrenceFrameA;
				bool isEnableCollisionBetweenLinkedBodies;
			};

			union
			{
				Generic6Dof generic6Dof;
			};

			ConstraintProperty()
			{
			}

			void SetGeneric6Dof(RigidBody* pRigidBodyA, const Math::Vector3& f3OriginPosA, const Math::Quaternion& originQuatA, RigidBody* pRigidBodyB, const Math::Vector3& f3OriginPosB, const Math::Quaternion& originQuatB, bool isEnableLinearRefrenceFrameA, bool isEnableCollisionBetweenLinkedBodies = true)
			{
				generic6Dof.pRigidBodyA = pRigidBodyA;
				generic6Dof.f3OriginPosA = f3OriginPosA;
				generic6Dof.originQuatA = originQuatA;
				generic6Dof.pRigidBodyB = pRigidBodyB;
				generic6Dof.f3OriginPosB = f3OriginPosB;
				generic6Dof.originQuatB = originQuatB;
				generic6Dof.isEnableLinearRefrenceFrameA = isEnableLinearRefrenceFrameA;
				generic6Dof.isEnableCollisionBetweenLinkedBodies = isEnableCollisionBetweenLinkedBodies;
				emType = EmConstraint::eGeneric6Dof;
			}

			void SetGeneric6Dof(RigidBody* pRigidBodyB, const Math::Vector3& f3OriginPosB, const Math::Quaternion& originQuatB, bool isEnableLinearRefrenceFrameA, bool isEnableCollisionBetweenLinkedBodies = true)
			{
				generic6Dof.pRigidBodyA = nullptr;
				generic6Dof.f3OriginPosA = Math::Vector3::Zero;
				generic6Dof.originQuatA = Math::Quaternion::Identity;
				generic6Dof.pRigidBodyB = pRigidBodyB;
				generic6Dof.f3OriginPosB = f3OriginPosB;
				generic6Dof.originQuatB = originQuatB;
				generic6Dof.isEnableLinearRefrenceFrameA = isEnableLinearRefrenceFrameA;
				generic6Dof.isEnableCollisionBetweenLinkedBodies = isEnableCollisionBetweenLinkedBodies;
				emType = EmConstraint::eGeneric6Dof;
			}
		};

		class ConstraintInterface
		{
		public:
			ConstraintInterface(EmConstraint::Type emConstraintType);
			virtual ~ConstraintInterface() = 0;

			static ConstraintInterface* Create(const ConstraintProperty& constraintProperty);

			virtual btTypedConstraint* GetInterface() = 0;

		protected:
			EmConstraint::Type m_emConstraintType;
			btDiscreteDynamicsWorld* m_pDynamicsWorld;
		};

		class Generic6DofConstraint : public ConstraintInterface
		{
		private:
			Generic6DofConstraint();

		public:
			virtual ~Generic6DofConstraint();

			static Generic6DofConstraint* Create(const ConstraintProperty& constraintProperty);

			virtual btTypedConstraint* GetInterface() override;
			
		public:
			void CalculateTransforms(const Math::Matrix& transA, const Math::Matrix& transB);
			void CalculateTransforms();

			Math::Matrix GetCalculatedTransformA() const;
			Math::Matrix GetCalculatedTransformB() const;

			Math::Matrix GetFrameOffSetA() const;
			Math::Matrix GetFrameOffSetB() const;

			Math::Matrix GetFrameOffSetA();
			Math::Matrix GetFrameOffSetB();

			void BuildJacobian();

			void UpdateRHS(float fTimeStep);

			// BuildJacobian must be called previously.
			Math::Vector3 GetAxis(int nAxis_index) const;

			// CalculateTransforms() must be called previously.
			float GetAngle(int nAxis_index) const;

			// CalculateTransforms() must be called previously.
			float GetRelativePivotPosition(int nAxis_index) const;

			void SetFrames(const Math::Matrix& frameA, const Math::Matrix& frameB);

			/*
			Calculates angular correction and returns true if limit needs to be corrected.
			CalculateTransforms() must be called previously.
			*/
			bool TestAngularLimitMotor(int nAxis_index);

			void SetLinearLowerLimit(const Math::Vector3& f3LinearLower);
			void GetLinearLowerLimit(Math::Vector3& f3LinearLower) const;

			void SetLinearUpperLimit(const Math::Vector3& f3LinearUpper);
			void GetLinearUpperLimit(Math::Vector3& f3LinearUpper) const;

			void SetAngularLowerLimit(const Math::Vector3& f3AngularLower);
			void GetAngularLowerLimit(Math::Vector3& f3AngularLower) const;

			void SetAngularUpperLimit(const Math::Vector3& f3AngularUpper);
			void GetAngularUpperLimit(Math::Vector3& f3AngularUpper) const;

			//first 3 are linear, next 3 are angular
			void SetLimit(int nAxis, float lo, float hi);

			//! Test limit
			/*!
			- free means upper < lower,
			- locked means upper == lower
			- limited means upper > lower
			- limitIndex: first 3 are linear, next 3 are angular
			*/
			bool IsLimited(int nLimitIndex) const;

			void CalcAnchorPos(void);

			// access for UseFrameOffSet
			bool GetUseFrameOffSet() const;
			void SetUseFrameOffSet(bool isFrameOffSetOnOff);

			bool GetUseLinearReferenceFrameA() const;
			void SetUseLinearReferenceFrameA(bool isLinearReferenceFrameA);

			///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5). 
			///If no axis is provided, it uses the default axis for this constraint.
			void SetParam(int nNum, float fValue, int nAxis = -1);

			///return the local value of parameter
			float GetParam(int nNum, int nAxis = -1) const;

			void SetAxis(const Math::Vector3& f3Axis1, const Math::Vector3& f3Axis2);

			int GetFlags() const;

			int CalculateSerializeBufferSize() const;

		private:
			btGeneric6DofConstraint* m_pGeneric6DofConstraint;
		};
	}
}