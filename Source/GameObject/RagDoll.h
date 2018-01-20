#pragma once

namespace EastEngine
{
	namespace Physics
	{
		class RigidBody;
		class ConstraintInterface;
	}

	namespace Graphics
	{
		class IModelInstance;
		class ISkeletonInstance;
	}

	namespace GameObject
	{
		class RagDoll
		{
		private:
			struct BodyPart
			{
				String::StringID strName;
				Physics::RigidBody* pRigidBody = nullptr;
				Graphics::IModelInstance* pPhysicsModelInstance = nullptr;
				Graphics::ISkeletonInstance::IBone* pBone = nullptr;

				Math::Vector3 f3Offset;
				float fHeight;
				bool isChildBone;
				bool isRootNode = false;

				Math::Matrix matOrigin;
				Math::Matrix matBodyOrigin;

				BodyPart(const String::StringID& strName, Physics::RigidBody* pRigidBody, Graphics::IModelInstance* pPhysicsModelInstance, Graphics::ISkeletonInstance::IBone* pBone)
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
				Graphics::IModelInstance* pPhysicsModelInstance = nullptr;

				BodyPart* pBodyPartA = nullptr;
				BodyPart* pBodyPartB = nullptr;

				Joint(const String::StringID& strName, Physics::ConstraintInterface* pConstraint, Graphics::IModelInstance* pPhysicsModelInstance = nullptr)
					: strName(strName)
					, pConstraint(pConstraint)
					, pPhysicsModelInstance(pPhysicsModelInstance)
				{
				}
			};

		public:
			RagDoll();
			~RagDoll();

			bool BuildBipadRagDoll(Graphics::ISkeletonInstance* pSkeleton, const Math::Vector3& f3Pos, const Math::Quaternion& quatRotation, float fScale);
			void Update(float fElapsedTime);

			BodyPart* AddBodyPart(const String::StringID& strPartName, const Physics::RigidBodyProperty& rigidBodyProperty, Graphics::IModelInstance* pPhysicsModelInstance, Graphics::ISkeletonInstance::IBone* pBone);
			Joint* AddJoint(const String::StringID& strJointName, const Physics::ConstraintProperty& constraintProperty, BodyPart* pBodyPartA, BodyPart* pBodyPartB, Graphics::IModelInstance* pPhysicsModelInstance);

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

			Graphics::ISkeletonInstance* m_pSkeletonInst;
		};
	}
}