#pragma once

#include "Graphics/Model/ModelInterface.h"

namespace est
{
	namespace physics
	{
		class RigidBody;
		class ConstraintInterface;
	}

	namespace gameobject
	{
		class RagDoll
		{
		private:
			struct BodyPart
			{
				string::StringID name;
				physics::RigidBody* pRigidBody = nullptr;
				graphics::IModelInstance* pPhysicsModelInstance = nullptr;
				graphics::ISkeletonInstance::IBone* pBone = nullptr;

				math::float3 f3Offset;
				float fHeight;
				bool isChildBone;
				bool isRootNode = false;

				math::Matrix matOrigin;
				math::Matrix matBodyOrigin;

				BodyPart(const string::StringID& name, physics::RigidBody* pRigidBody, graphics::IModelInstance* pPhysicsModelInstance, graphics::ISkeletonInstance::IBone* pBone)
					: name(name)
					, pRigidBody(pRigidBody)
					, pPhysicsModelInstance(pPhysicsModelInstance)
					, pBone(pBone)
				{
				}
			};

			struct Joint
			{
				string::StringID name;
				physics::ConstraintInterface* pConstraint = nullptr;
				graphics::IModelInstance* pPhysicsModelInstance = nullptr;

				BodyPart* pBodyPartA = nullptr;
				BodyPart* pBodyPartB = nullptr;

				Joint(const string::StringID& name, physics::ConstraintInterface* pConstraint, graphics::IModelInstance* pPhysicsModelInstance = nullptr)
					: name(name)
					, pConstraint(pConstraint)
					, pPhysicsModelInstance(pPhysicsModelInstance)
				{
				}
			};

		public:
			RagDoll();
			~RagDoll();

			bool BuildBipadRagDoll(graphics::ISkeletonInstance* pSkeleton, const math::float3& f3Pos, const math::Quaternion& quatRotation, float fScale);
			void Update(float elapsedTime);

			BodyPart* AddBodyPart(const string::StringID& strPartName, const physics::RigidBodyProperty& rigidBodyProperty, graphics::IModelInstance* pPhysicsModelInstance, graphics::ISkeletonInstance::IBone* pBone);
			Joint* AddJoint(const string::StringID& strJointName, const physics::ConstraintProperty& constraintProperty, BodyPart* pBodyPartA, BodyPart* pBodyPartB, graphics::IModelInstance* pPhysicsModelInstance);

			physics::RigidBody* GetBodyPort(const string::StringID& strPartName);
			physics::ConstraintInterface* GetJoint(const string::StringID& strJointName);

			void Start();
			void End();

		private:
			void copyRagDollStateToModel();
			void copyModelStateToRagDoll();

		private:
			bool m_isRagDollState;

			std::vector<BodyPart*> m_vecBodyParts;
			std::vector<Joint*> m_vecJoints;

			graphics::ISkeletonInstance* m_pSkeletonInst;
		};
	}
}