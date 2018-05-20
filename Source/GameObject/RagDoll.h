#pragma once

namespace eastengine
{
	namespace Physics
	{
		class RigidBody;
		class ConstraintInterface;
	}

	namespace graphics
	{
		class IModelInstance;
		class ISkeletonInstance;
	}

	namespace gameobject
	{
		class RagDoll
		{
		private:
			struct BodyPart
			{
				String::StringID strName;
				Physics::RigidBody* pRigidBody = nullptr;
				graphics::IModelInstance* pPhysicsModelInstance = nullptr;
				graphics::ISkeletonInstance::IBone* pBone = nullptr;

				math::Vector3 f3Offset;
				float fHeight;
				bool isChildBone;
				bool isRootNode = false;

				math::Matrix matOrigin;
				math::Matrix matBodyOrigin;

				BodyPart(const String::StringID& strName, Physics::RigidBody* pRigidBody, graphics::IModelInstance* pPhysicsModelInstance, graphics::ISkeletonInstance::IBone* pBone)
					: strName(strName)
					, pRigidBody(pRigidBody)
					, pPhysicsModelInstance(pPhysicsModelInstance)
					, pBone(pBone)
				{
				}
			};

			struct Joint
			{
				String::StringID strName;
				Physics::ConstraintInterface* pConstraint = nullptr;
				graphics::IModelInstance* pPhysicsModelInstance = nullptr;

				BodyPart* pBodyPartA = nullptr;
				BodyPart* pBodyPartB = nullptr;

				Joint(const String::StringID& strName, Physics::ConstraintInterface* pConstraint, graphics::IModelInstance* pPhysicsModelInstance = nullptr)
					: strName(strName)
					, pConstraint(pConstraint)
					, pPhysicsModelInstance(pPhysicsModelInstance)
				{
				}
			};

		public:
			RagDoll();
			~RagDoll();

			bool BuildBipadRagDoll(graphics::ISkeletonInstance* pSkeleton, const math::Vector3& f3Pos, const math::Quaternion& quatRotation, float fScale);
			void Update(float fElapsedTime);

			BodyPart* AddBodyPart(const String::StringID& strPartName, const Physics::RigidBodyProperty& rigidBodyProperty, graphics::IModelInstance* pPhysicsModelInstance, graphics::ISkeletonInstance::IBone* pBone);
			Joint* AddJoint(const String::StringID& strJointName, const Physics::ConstraintProperty& constraintProperty, BodyPart* pBodyPartA, BodyPart* pBodyPartB, graphics::IModelInstance* pPhysicsModelInstance);

			Physics::RigidBody* GetBodyPort(const String::StringID& strPartName);
			Physics::ConstraintInterface* GetJoint(const String::StringID& strJointName);

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