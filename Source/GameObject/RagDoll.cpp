#include "stdafx.h"
#include "RagDoll.h"

#include "Model/ModelInterface.h"

namespace StrID
{
	RegisterStringID(Pelvis);
	RegisterStringID(Spine);
	RegisterStringID(Head);
	RegisterStringID(LeftUpperLeg);
	RegisterStringID(LeftLowerLeg);
	RegisterStringID(RightUpperLeg);
	RegisterStringID(RightLowerLeg);
	RegisterStringID(LeftUpperArm);
	RegisterStringID(LeftLowerArm);
	RegisterStringID(RightUpperArm);
	RegisterStringID(RightLowerArm);

	RegisterStringID(Spine_Head);
	RegisterStringID(LeftShoulder);
	RegisterStringID(RightShoulder);
	RegisterStringID(LeftElbow);
	RegisterStringID(RightElbow);
	RegisterStringID(Pelvis_Spine);
	RegisterStringID(LeftHip);
	RegisterStringID(RightHip);
	RegisterStringID(LeftKnee);
	RegisterStringID(RightKnee);
}

namespace EastEngine
{
	namespace GameObject
	{
		RagDoll::RagDoll()
			: m_isRagDollState(false)
			, m_pSkeletonInst(nullptr)
		{
		}

		RagDoll::~RagDoll()
		{
			std::for_each(m_vecJoints.begin(), m_vecJoints.end(), [](Joint& joint)
			{
				SafeDelete(joint.pConstraint);
			});
			m_vecJoints.clear();

			std::for_each(m_vecBodyParts.begin(), m_vecBodyParts.end(), [](BodyPart& bodyPart)
			{
				SafeDelete(bodyPart.pRigidBody);

				Graphics::IModel::DestroyInstance(&bodyPart.pPhysicsModelInstance);
				bodyPart.pPhysicsModelInstance = nullptr;
			});
			m_vecBodyParts.clear();
		}

		bool RagDoll::BuildDefaultHumanRagDoll(Graphics::ISkeletonInstance* pSkeletonInst, const Math::Vector3& f3Pos, const Math::Quaternion& quatRotation, float fScale)
		{
			if (pSkeletonInst == nullptr)
				return false;

			m_pSkeletonInst = pSkeletonInst;

			Math::Matrix matTranslation = Math::Matrix::Compose(Math::Vector3::One, quatRotation, f3Pos);

			auto CreateBodyPart = [&](const String::StringID& strBoneName, const String::StringID& strName, float fRadius, float fHeight, const Math::Vector3& f3Pos, const Math::Quaternion& quat = Math::Quaternion::Identity)
			{
				Graphics::MaterialInfo materialInfo;
				materialInfo.strName = strName;
				materialInfo.colorAlbedo = Math::Color::Red;
				materialInfo.pRasterizerState = Graphics::GetDevice()->GetRasterizerState(Graphics::EmRasterizerState::eWireframeCullNone);

				Graphics::ModelLoader modelLoader;
				modelLoader.InitCapsule(strName, &materialInfo, fRadius, fHeight);

				Graphics::IModelInstance* pPhysicsModelInst = Graphics::IModel::CreateInstance(modelLoader);

				Physics::RigidBodyProperty prop;
				prop.fLinearDamping = 0.05f;
				prop.fAngularDamping = 0.85f;
				prop.nCollisionFlag = Physics::EmCollision::eCharacterObject;
				prop.strName = strName;
				prop.f3OriginPos = f3Pos;
				prop.originQuat = quat;
				prop.matOffset = matTranslation;
				prop.shapeInfo.SetCapsule(fRadius, fHeight);

				Graphics::ISkeletonInstance::IBone* pBone = pSkeletonInst->GetBone(strBoneName);

				Physics::RigidBody* pRigidBody = AddBodyPart(strName, prop, pPhysicsModelInst, pBone);
				pRigidBody->SetActiveState(Physics::EmActiveState::eIslandSleeping);

				return pRigidBody;
			};
/*
			Physics::RigidBody* pPelvis = CreateBodyPart("Pelvis", StrID::Pelvis, fScale * 0.15f, fScale * 0.2f, Math::Vector3(0.f, fScale * 1.f, 0.f));
			Physics::RigidBody* pSpine = CreateBodyPart("Spine", StrID::Spine, fScale * 0.15f, fScale * 0.28f, Math::Vector3(0.f, fScale * 1.2f, 0.f));
			Physics::RigidBody* pHead = CreateBodyPart("Head", StrID::Head, fScale * 0.1f, fScale * 0.05f, Math::Vector3(0.f, fScale * 1.6f, 0.f));
			Physics::RigidBody* pLeftUpperLeg = CreateBodyPart("L_Thigh1", StrID::LeftUpperLeg, fScale * 0.07f, fScale * 0.45f, Math::Vector3(fScale * -0.18f, fScale * 0.65f, 0.f));
			Physics::RigidBody* pLeftLowerLeg = CreateBodyPart("L_Knee2", StrID::LeftLowerLeg, fScale * 0.05f, fScale * 0.37f, Math::Vector3(fScale * -0.18f, fScale * 0.2f, 0.f));
			Physics::RigidBody* pRightUpperLeg = CreateBodyPart("R_Thigh", StrID::RightUpperLeg, fScale * 0.07f, fScale * 0.45f, Math::Vector3(fScale * 0.18f, fScale * 0.65f, 0.f));
			Physics::RigidBody* pRightLowerLeg = CreateBodyPart("R_Knee", StrID::RightLowerLeg, fScale * 0.05f, fScale * 0.37f, Math::Vector3(fScale * 0.18f, fScale * 0.2f, 0.f));
			Physics::RigidBody* pLeftUpperArm = CreateBodyPart("L_UpperArm", StrID::LeftUpperArm, fScale * 0.05f, fScale * 0.33f, Math::Vector3(fScale * -0.35f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			Physics::RigidBody* pLeftLowerArm = CreateBodyPart("L_Forearm", StrID::LeftLowerArm, fScale * 0.04f, fScale * 0.25f, Math::Vector3(fScale * -0.7f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			Physics::RigidBody* pRightUpperArm = CreateBodyPart("R_UpperArm", StrID::RightUpperArm, fScale * 0.05f, fScale * 0.33f, Math::Vector3(fScale * 0.35f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));
			Physics::RigidBody* pRightLowerArm = CreateBodyPart("R_Forearm", StrID::RightLowerArm, fScale * 0.04f, fScale * 0.25f, Math::Vector3(fScale * 0.7f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));
*/

			Physics::RigidBody* pPelvis = CreateBodyPart("Character1_Hips", StrID::Pelvis, fScale * 0.15f, fScale * 0.2f, Math::Vector3(0.f, fScale * 1.f, 0.f));
			Physics::RigidBody* pSpine = CreateBodyPart("Character1_Spine", StrID::Spine, fScale * 0.15f, fScale * 0.28f, Math::Vector3(0.f, fScale * 1.2f, 0.f));
			Physics::RigidBody* pHead = CreateBodyPart("Character1_Head", StrID::Head, fScale * 0.1f, fScale * 0.05f, Math::Vector3(0.f, fScale * 1.6f, 0.f));
			Physics::RigidBody* pLeftUpperLeg = CreateBodyPart("Character1_LeftUpLeg", StrID::LeftUpperLeg, fScale * 0.07f, fScale * 0.45f, Math::Vector3(fScale * -0.18f, fScale * 0.65f, 0.f));
			Physics::RigidBody* pLeftLowerLeg = CreateBodyPart("Character1_LeftLeg", StrID::LeftLowerLeg, fScale * 0.05f, fScale * 0.37f, Math::Vector3(fScale * -0.18f, fScale * 0.2f, 0.f));
			Physics::RigidBody* pRightUpperLeg = CreateBodyPart("Character1_RightUpLeg", StrID::RightUpperLeg, fScale * 0.07f, fScale * 0.45f, Math::Vector3(fScale * 0.18f, fScale * 0.65f, 0.f));
			Physics::RigidBody* pRightLowerLeg = CreateBodyPart("Character1_RightLeg", StrID::RightLowerLeg, fScale * 0.05f, fScale * 0.37f, Math::Vector3(fScale * 0.18f, fScale * 0.2f, 0.f));
			Physics::RigidBody* pLeftUpperArm = CreateBodyPart("Character1_LeftArm", StrID::LeftUpperArm, fScale * 0.05f, fScale * 0.33f, Math::Vector3(fScale * -0.35f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			Physics::RigidBody* pLeftLowerArm = CreateBodyPart("Character1_LeftForeArm", StrID::LeftLowerArm, fScale * 0.04f, fScale * 0.25f, Math::Vector3(fScale * -0.7f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			Physics::RigidBody* pRightUpperArm = CreateBodyPart("Character1_RightArm", StrID::RightUpperArm, fScale * 0.05f, fScale * 0.33f, Math::Vector3(fScale * 0.35f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));
			Physics::RigidBody* pRightLowerArm = CreateBodyPart("Character1_RightForeArm", StrID::RightLowerArm, fScale * 0.04f, fScale * 0.25f, Math::Vector3(fScale * 0.7f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));

			size_t nBodyCount = m_vecBodyParts.size();
			for (size_t i = 0; i < nBodyCount; ++i)
			{
				m_vecBodyParts[i].pRigidBody->SetDeactivationTime(0.8f);
				m_vecBodyParts[i].pRigidBody->SetSleepingThresholds(1.6f, 2.5f);
			}

			auto CreateJoint = [&](const String::StringID& strJointName, Physics::RigidBody* pRigidBodyA, Physics::RigidBody* pRigidBodyB,
				const Math::Vector3& f3PosA, const Math::Vector3& f3PosB,
				const Math::Quaternion& quatA, const Math::Quaternion& quatB,
				const Math::Vector3& f3AngularLowerLimit, const Math::Vector3& f3AngularUpperLimit)
			{
				/*Graphics::MaterialInfo materialInfo;
				materialInfo.strName = strJointName;
				materialInfo.colorAlbedo = Math::Color::Red;
				materialInfo.rasterizerStateKey = Graphics::Device::GetInstance()->GetRasterizerStateKey(Graphics::EmRasterizerState::eWireFrame);

				Graphics::ModelLoader modelLoader;
				modelLoader.InitCapsule(strJointName, &materialInfo, fRadius, fHeight);

				Graphics::IModelInstance* pPhysicsModelInst = Graphics::IModel::CreateInstance(modelLoader);*/

				Physics::ConstraintProperty prop;
				prop.SetGeneric6Dof(pRigidBodyA, f3PosA, quatA, pRigidBodyB, f3PosB, quatB, true, false);

				Physics::Generic6DofConstraint* pJoint = static_cast<Physics::Generic6DofConstraint*>(AddJoint(strJointName, prop, nullptr));
				pJoint->SetAngularLowerLimit(f3AngularLowerLimit);
				pJoint->SetAngularUpperLimit(f3AngularUpperLimit);

				return pJoint;
			};

			// SPINE HEAD
			Math::Quaternion quatA = Math::Quaternion::Identity;
			Math::Quaternion quatB = Math::Quaternion::Identity;
			CreateJoint(StrID::Spine_Head, pSpine, pHead,
				Math::Vector3(0.f, fScale * 0.3f, 0.f),
				Math::Vector3(0.f, fScale * -0.14f, 0.f),
				quatA, quatB,
				Math::Vector3(Math::PI * -0.3f, -FLT_EPSILON, Math::PI * -0.3f),
				Math::Vector3(Math::PI * 0.5f, FLT_EPSILON, Math::PI * 0.3f));

			// LEFT SHOULDER
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::CreateFromYawPitchRoll(0.f, -Math::PIDIV2, Math::PIDIV2);
			CreateJoint(StrID::LeftShoulder, pSpine, pLeftUpperArm, 
				Math::Vector3(fScale * -0.2f, fScale * 0.15f, 0.f), 
				Math::Vector3(0.f, fScale * -0.18f, 0.f),
				quatA, quatB,
				Math::Vector3(Math::PI * -0.8f, -FLT_EPSILON, Math::PI * -0.5f), 
				Math::Vector3(Math::PI * 0.8f, FLT_EPSILON, Math::PI * 0.5f));

			// RIGHT SHOULDER
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::CreateFromYawPitchRoll(0.f, Math::PIDIV2, 0.f);
			CreateJoint(StrID::RightShoulder, pSpine, pRightUpperArm, 
				Math::Vector3(fScale * 0.2f, fScale * 0.15f, 0.f), 
				Math::Vector3(0.f, fScale * -0.18f, 0.f),
				quatA, quatB,
				Math::Vector3(Math::PI * -0.8f, -FLT_EPSILON, Math::PI * -0.5f),
				Math::Vector3(Math::PI * 0.8f, FLT_EPSILON, Math::PI * 0.5f));

			// LEFT ELBOW
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::Identity;
			CreateJoint(StrID::LeftElbow, pLeftUpperArm, pLeftLowerArm, 
				Math::Vector3(0.f, fScale * 0.18f, 0.f), 
				Math::Vector3(0.f, fScale * -0.14f, 0.f),
				quatA, quatB,
				Math::Vector3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				Math::Vector3(Math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			// RIGHT ELBOW
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::Identity;
			CreateJoint(StrID::RightElbow, pRightUpperArm, pRightLowerArm,
				Math::Vector3(0.f, fScale * 0.18f, 0.f),
				Math::Vector3(0.f, fScale * -0.14f, 0.f),
				quatA, quatB,
				Math::Vector3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				Math::Vector3(Math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			// PELVIS
			quatA = Math::Quaternion::CreateFromYawPitchRoll(Math::PIDIV2, 0.f, 0.f);
			quatB = Math::Quaternion::CreateFromYawPitchRoll(Math::PIDIV2, 0.f, 0.f);
			CreateJoint(StrID::Pelvis, pPelvis, pSpine,
				Math::Vector3(0.f, fScale * 0.15f, 0.f),
				Math::Vector3(0.f, fScale * -0.15f, 0.f),
				quatA, quatB,
				Math::Vector3(Math::PI * -0.2f, -FLT_EPSILON, Math::PI * -0.3f),
				Math::Vector3(Math::PI * 0.2f, FLT_EPSILON, Math::PI * 0.6f));

			// LEFT HIP
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::Identity;
			CreateJoint(StrID::LeftHip, pPelvis, pLeftUpperLeg,
				Math::Vector3(fScale * -0.18f, fScale * -0.1f, 0.f),
				Math::Vector3(0.f, fScale * 0.225f, 0.f),
				quatA, quatB,
				Math::Vector3(Math::PIDIV2 * -0.5f, -FLT_EPSILON, -FLT_EPSILON),
				Math::Vector3(Math::PIDIV2 * 0.8f, FLT_EPSILON, Math::PIDIV2 * 0.6f));

			// RIGHT HIP
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::Identity;
			CreateJoint(StrID::RightHip, pPelvis, pRightUpperLeg,
				Math::Vector3(fScale * 0.18f, fScale * -0.1f, 0.f),
				Math::Vector3(0.f, fScale * 0.225f, 0.f),
				quatA, quatB,
				Math::Vector3(Math::PIDIV2 * -0.5f, -FLT_EPSILON, Math::PIDIV2 * -0.6f),
				Math::Vector3(Math::PIDIV2 * 0.8f, FLT_EPSILON, FLT_EPSILON));

			// LEFT KNEE
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::Identity;
			CreateJoint(StrID::LeftKnee, pLeftUpperLeg, pLeftLowerLeg,
				Math::Vector3(0.f, fScale * -0.225f, 0.f),
				Math::Vector3(0.f, fScale * 0.185f, 0.f),
				quatA, quatB,
				Math::Vector3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				Math::Vector3(Math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			// RIGHT KNEE
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::Identity;
			CreateJoint(StrID::RightKnee, pRightUpperLeg, pRightLowerLeg,
				Math::Vector3(0.f, fScale * -0.225f, 0.f),
				Math::Vector3(0.f, fScale * 0.185f, 0.f),
				quatA, quatB,
				Math::Vector3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				Math::Vector3(Math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			return true;
		}

		void RagDoll::Update(float fElapsedTime)
		{
			for (auto& bodyPart : m_vecBodyParts)
			{
				bodyPart.pRigidBody->Update(fElapsedTime);
			
				Math::Matrix matWorld = bodyPart.pRigidBody->GetWorldMatrix();
			
				if (bodyPart.pPhysicsModelInstance != nullptr)
				{
					bodyPart.pPhysicsModelInstance->Update(fElapsedTime, matWorld);
				}

				//if (m_isRagDollState == true)
				//{
				//	Math::Matrix matInvParent;
				//	Graphics::ISkeletonInstance::IBone* pParentBone = bodyPart.pBone->GetParent();
				//	if (pParentBone != nullptr)
				//	{
				//		matInvParent = pParentBone->GetLocalTransform().Invert();
				//	}
				//	
				//	Math::Matrix matWorldMatrix = bodyPart.pRigidBody->GetWorldMatrix();
				//	bodyPart.pBone->SetMotionData(matWorldMatrix * matInvParent);
				//}
			}
		}

		Physics::RigidBody* RagDoll::AddBodyPart(const String::StringID& strPartName, const Physics::RigidBodyProperty& rigidBodyProperty, Graphics::IModelInstance* pPhysicsModelInstance, Graphics::ISkeletonInstance::IBone* pBone)
		{
			if (GetBodyPort(strPartName) != nullptr)
				return nullptr;

			Physics::RigidBody* pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);
			if (pRigidBody != nullptr)
			{
				Math::Quaternion quatBodyOrigin = pRigidBody->GetOrientation();
				Math::Quaternion quatBoneOrigin /*= pBone->GetRotation()*/;
				m_vecBodyParts.emplace_back(strPartName, pRigidBody, pPhysicsModelInstance, pBone, quatBodyOrigin, quatBoneOrigin);
			}

			return pRigidBody;
		}

		Physics::ConstraintInterface* RagDoll::AddJoint(const String::StringID& strJointName, const Physics::ConstraintProperty& constraintProperty, Graphics::IModelInstance* pPhysicsModelInstance)
		{
			if (GetJoint(strJointName) != nullptr)
				return nullptr;

			Physics::ConstraintInterface* pConstraint = Physics::ConstraintInterface::Create(constraintProperty);
			if (pConstraint != nullptr)
			{
				m_vecJoints.emplace_back(strJointName, pConstraint, pPhysicsModelInstance);
			}

			return pConstraint;
		}

		Physics::RigidBody* RagDoll::GetBodyPort(const String::StringID& strPartName)
		{
			for (auto& bodyPart : m_vecBodyParts)
			{
				if (bodyPart.strName == strPartName)
					return bodyPart.pRigidBody;
			}

			return nullptr;
		}

		Physics::ConstraintInterface* RagDoll::GetJoint(const String::StringID& strJointName)
		{
			for (auto& joint : m_vecJoints)
			{
				if (joint.strName == strJointName)
					return joint.pConstraint;
			}

			return nullptr;
		}

		void RagDoll::Start()
		{
			m_isRagDollState = true;

			for (auto& bodyPart : m_vecBodyParts)
			{
				bodyPart.pRigidBody->SetActiveState(Physics::EmActiveState::eActiveTag);

				//bodyPart.pBone->SetManualControl(true);
			}

			//m_pSkeleton->SetManualControl(true);
		}

		void RagDoll::End()
		{
			m_isRagDollState = false;

			//m_pSkeleton->SetManualControl(false);
		}

		void RagDoll::copyRagDollStateToModel()
		{
			for (auto& bodyPart : m_vecBodyParts)
			{
				Math::Matrix matParentTransform;
				Graphics::ISkeletonInstance::IBone* pPrarentBone = bodyPart.pBone->GetParent();
				if (pPrarentBone != nullptr)
				{
					matParentTransform = pPrarentBone->GetMotionTransform();
				}

				Math::Vector3 f3Scale;
				Math::Vector3 f3Pos;
				Math::Quaternion quat;
				matParentTransform.Decompose(f3Scale, quat, f3Pos);

				/*bodyPart.pBone->SetRotation(quat.Inverse() *
					bodyPart.pRigidBody->GetOrientation() *
					bodyPart.quatBodyOrigin.Inverse() *
					bodyPart.quatBoneOrigin);*/
			}
		}

		void RagDoll::copyModelStateToRagDoll()
		{
			for (auto& bodyPart : m_vecBodyParts)
			{
				Math::Matrix matTransform = bodyPart.pRigidBody->GetWorldMatrix();
				Math::Vector3 f3Scale;
				Math::Vector3 f3Pos;
				Math::Quaternion quat;
				matTransform.Decompose(f3Scale, quat, f3Pos);

				//quat *= (bodyPart.pBone->GetRotation() * bodyPart.quatBoneOrigin.Inverse());
				//f3Pos = (bodyPart.pBone->GetMotionTransform;
			}
		}
	}
}