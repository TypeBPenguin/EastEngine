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
			std::for_each(m_vecJoints.begin(), m_vecJoints.end(), [](Joint* pJoint)
			{
				SafeDelete(pJoint->pConstraint);
				SafeDelete(pJoint);
			});
			m_vecJoints.clear();

			std::for_each(m_vecBodyParts.begin(), m_vecBodyParts.end(), [](BodyPart* pBodyPart)
			{
				SafeDelete(pBodyPart->pRigidBody);

				Graphics::IModel::DestroyInstance(&pBodyPart->pPhysicsModelInstance);
				pBodyPart->pPhysicsModelInstance = nullptr;

				SafeDelete(pBodyPart);
			});
			m_vecBodyParts.clear();
		}

		bool RagDoll::BuildBipadRagDoll(Graphics::ISkeletonInstance* pSkeletonInst, const Math::Vector3& f3Pos, const Math::Quaternion& quatRotation, float fScale)
		{
			if (pSkeletonInst == nullptr)
				return false;

			m_pSkeletonInst = pSkeletonInst;

			//Math::Matrix matTranslation = Math::Matrix::Compose(Math::Vector3::One, Math::Quaternion::CreateFromYawPitchRoll(Math::ToRadians(180.f), 0.f, 0.f), f3Pos);
			Math::Matrix matTranslation = Math::Matrix::Compose(Math::Vector3::One, quatRotation, f3Pos);

			auto CreateBodyPart = [&](bool isChildBone, const String::StringID& strBoneName, const String::StringID& strName, float fRadius, float fHeight, const Math::Vector3& f3Pos, const Math::Quaternion& quat = Math::Quaternion::Identity)
			{
				float fNewHeight = fHeight;
				Math::Vector3 f3NewPos = f3Pos;
				Math::Vector3 f3Offset;
				Math::Quaternion newQuat = quat;

				Graphics::ISkeletonInstance::IBone* pBone = pSkeletonInst->GetBone(strBoneName);
				//if (isChildBone == true)
				//{
				//	Math::Vector3 f3ScaleChild;
				//	Math::Vector3 f3PosChild;
				//	Math::Quaternion quatChild;
				//	pBone->GetGlobalTransform().Decompose(f3ScaleChild, quatChild, f3PosChild);
				//
				//	Math::Vector3 f3ScaleParent;
				//	Math::Vector3 f3PosParent;
				//	Math::Quaternion quatParent;
				//	pBone->GetParent()->GetGlobalTransform().Decompose(f3ScaleParent, quatParent, f3PosParent);
				//
				//	f3Offset = (f3PosChild - f3PosParent) * 0.5f;
				//	f3NewPos = f3PosParent + f3Offset;
				//
				//	newQuat = quatParent * quat;
				//
				//	fNewHeight = f3Offset.Length() * 1.5f;
				//}

				Graphics::MaterialInfo materialInfo;
				materialInfo.strName = strName;
				materialInfo.colorAlbedo = Math::Color::Red;
				materialInfo.emRasterizerState = Graphics::EmRasterizerState::eWireframeCullNone;
				//materialInfo.emRasterizerState = Graphics::EmRasterizerState::eSolidCCW;

				Graphics::ModelLoader modelLoader;
				modelLoader.InitCapsule(strName, &materialInfo, fRadius, fNewHeight);

				Graphics::IModelInstance* pPhysicsModelInst = Graphics::IModel::CreateInstance(modelLoader);

				Physics::RigidBodyProperty prop;
				prop.fLinearDamping = 0.05f;
				prop.fAngularDamping = 0.85f;
				prop.nCollisionFlag = Physics::EmCollision::eCharacterObject;
				prop.strName = strName;
				prop.f3OriginPos = f3NewPos;
				prop.originQuat = newQuat;
				prop.matOffset = matTranslation;
				prop.shapeInfo.SetCapsule(fRadius, fHeight);

				//Math::Vector3 f3Scale;
				//Math::Quaternion quat2;
				//pBone->GetGlobalTransform().Decompose(f3Scale, quat2, prop.f3OriginPos);

				BodyPart* pBodyPart = AddBodyPart(strName, prop, pPhysicsModelInst, pBone);
				pBodyPart->f3Offset = f3Offset;
				pBodyPart->fHeight = fNewHeight;
				//pBodyPart->isChildBone = isChildBone;

				pBodyPart->pRigidBody->SetActiveState(Physics::EmActiveState::eIslandSleeping);

				return pBodyPart;
			};

			//Physics::RigidBody* pPelvis = CreateBodyPart("Pelvis", StrID::Pelvis, fScale * 0.15f, fScale * 0.2f, Math::Vector3(0.f, fScale * 1.f, 0.f));
			//Physics::RigidBody* pSpine = CreateBodyPart("Spine", StrID::Spine, fScale * 0.15f, fScale * 0.28f, Math::Vector3(0.f, fScale * 1.2f, 0.f));
			//Physics::RigidBody* pHead = CreateBodyPart("Head", StrID::Head, fScale * 0.1f, fScale * 0.05f, Math::Vector3(0.f, fScale * 1.6f, 0.f));
			//Physics::RigidBody* pLeftUpperLeg = CreateBodyPart("L_Thigh1", StrID::LeftUpperLeg, fScale * 0.07f, fScale * 0.45f, Math::Vector3(fScale * -0.18f, fScale * 0.65f, 0.f));
			//Physics::RigidBody* pLeftLowerLeg = CreateBodyPart("L_Knee2", StrID::LeftLowerLeg, fScale * 0.05f, fScale * 0.37f, Math::Vector3(fScale * -0.18f, fScale * 0.2f, 0.f));
			//Physics::RigidBody* pRightUpperLeg = CreateBodyPart("R_Thigh", StrID::RightUpperLeg, fScale * 0.07f, fScale * 0.45f, Math::Vector3(fScale * 0.18f, fScale * 0.65f, 0.f));
			//Physics::RigidBody* pRightLowerLeg = CreateBodyPart("R_Knee", StrID::RightLowerLeg, fScale * 0.05f, fScale * 0.37f, Math::Vector3(fScale * 0.18f, fScale * 0.2f, 0.f));
			//Physics::RigidBody* pLeftUpperArm = CreateBodyPart("L_UpperArm", StrID::LeftUpperArm, fScale * 0.05f, fScale * 0.33f, Math::Vector3(fScale * -0.35f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			//Physics::RigidBody* pLeftLowerArm = CreateBodyPart("L_Forearm", StrID::LeftLowerArm, fScale * 0.04f, fScale * 0.25f, Math::Vector3(fScale * -0.7f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			//Physics::RigidBody* pRightUpperArm = CreateBodyPart("R_UpperArm", StrID::RightUpperArm, fScale * 0.05f, fScale * 0.33f, Math::Vector3(fScale * 0.35f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));
			//Physics::RigidBody* pRightLowerArm = CreateBodyPart("R_Forearm", StrID::RightLowerArm, fScale * 0.04f, fScale * 0.25f, Math::Vector3(fScale * 0.7f, fScale * 1.45f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));

			BodyPart* pPelvis = CreateBodyPart(false, "Character1_Hips", StrID::Pelvis, fScale * 0.15f, fScale * 0.05f, Math::Vector3(0.f, fScale * 1.1f, 0.f));
			pPelvis->isRootNode = true;

			BodyPart* pSpine = CreateBodyPart(false, "Character1_Spine", StrID::Spine, fScale * 0.15f, fScale * 0.20f, Math::Vector3(0.f, fScale * 1.35f, 0.f));
			BodyPart* pHead = CreateBodyPart(false, "Character1_Head", StrID::Head, fScale * 0.1f, fScale * 0.05f, Math::Vector3(0.f, fScale * 1.7f, 0.f));
			/*BodyPart* pLeftUpperLeg = CreateBodyPart(true, "Character1_LeftLeg", StrID::LeftUpperLeg, fScale * 0.07f, fScale * 0.4f, Math::Vector3(fScale * -0.1f, fScale * 0.8f, 0.f));
			BodyPart* pLeftLowerLeg = CreateBodyPart(true, "Character1_LeftFoot", StrID::LeftLowerLeg, fScale * 0.05f, fScale * 0.42f, Math::Vector3(fScale * -0.1f, fScale * 0.35f, 0.f));
			BodyPart* pRightUpperLeg = CreateBodyPart(true, "Character1_RightLeg", StrID::RightUpperLeg, fScale * 0.07f, fScale * 0.4f, Math::Vector3(fScale * 0.1f, fScale * 0.8f, 0.f));
			BodyPart* pRightLowerLeg = CreateBodyPart(true, "Character1_RightFoot", StrID::RightLowerLeg, fScale * 0.05f, fScale * 0.42f, Math::Vector3(fScale * 0.1f, fScale * 0.35f, 0.f));
			BodyPart* pLeftUpperArm = CreateBodyPart(true, "Character1_LeftForeArm", StrID::LeftUpperArm, fScale * 0.05f, fScale * 0.25f, Math::Vector3(fScale * -0.25f, fScale * 1.475f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			BodyPart* pLeftLowerArm = CreateBodyPart(true, "Character1_LeftHand", StrID::LeftLowerArm, fScale * 0.04f, fScale * 0.2f, Math::Vector3(fScale * -0.525f, fScale * 1.475f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			BodyPart* pRightUpperArm = CreateBodyPart(true, "Character1_RightForeArm", StrID::RightUpperArm, fScale * 0.05f, fScale * 0.25f, Math::Vector3(fScale * 0.25f, fScale * 1.475f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));
			BodyPart* pRightLowerArm = CreateBodyPart(true, "Character1_RightHand", StrID::RightLowerArm, fScale * 0.04f, fScale * 0.2f, Math::Vector3(fScale * 0.525f, fScale * 1.475f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));*/

			BodyPart* pLeftUpperLeg = CreateBodyPart(true, "Character1_LeftUpLeg", StrID::LeftUpperLeg, fScale * 0.07f, fScale * 0.4f, Math::Vector3(fScale * -0.1f, fScale * 0.8f, 0.f));
			BodyPart* pLeftLowerLeg = CreateBodyPart(true, "Character1_LeftLeg", StrID::LeftLowerLeg, fScale * 0.05f, fScale * 0.42f, Math::Vector3(fScale * -0.1f, fScale * 0.35f, 0.f));
			BodyPart* pRightUpperLeg = CreateBodyPart(true, "Character1_RightUpLeg", StrID::RightUpperLeg, fScale * 0.07f, fScale * 0.4f, Math::Vector3(fScale * 0.1f, fScale * 0.8f, 0.f));
			BodyPart* pRightLowerLeg = CreateBodyPart(true, "Character1_RightLeg", StrID::RightLowerLeg, fScale * 0.05f, fScale * 0.42f, Math::Vector3(fScale * 0.1f, fScale * 0.35f, 0.f));
			BodyPart* pLeftUpperArm = CreateBodyPart(true, "Character1_LeftArm", StrID::LeftUpperArm, fScale * 0.05f, fScale * 0.25f, Math::Vector3(fScale * -0.25f, fScale * 1.475f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			BodyPart* pLeftLowerArm = CreateBodyPart(true, "Character1_LeftForeArm", StrID::LeftLowerArm, fScale * 0.04f, fScale * 0.2f, Math::Vector3(fScale * -0.525f, fScale * 1.475f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, Math::PIDIV2));
			BodyPart* pRightUpperArm = CreateBodyPart(true, "Character1_RightArm", StrID::RightUpperArm, fScale * 0.05f, fScale * 0.25f, Math::Vector3(fScale * 0.25f, fScale * 1.475f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));
			BodyPart* pRightLowerArm = CreateBodyPart(true, "Character1_RightForeArm", StrID::RightLowerArm, fScale * 0.04f, fScale * 0.2f, Math::Vector3(fScale * 0.525f, fScale * 1.475f, 0.f), Math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -Math::PIDIV2));

			size_t nBodyCount = m_vecBodyParts.size();
			for (size_t i = 0; i < nBodyCount; ++i)
			{
				m_vecBodyParts[i]->pRigidBody->SetDeactivationTime(0.8f);
				m_vecBodyParts[i]->pRigidBody->SetSleepingThresholds(1.6f, 2.5f);
			}

			auto CreateJoint = [&](const String::StringID& strJointName, BodyPart* pBodyPartA, BodyPart* pBodyPartB,
				const Math::Vector3& f3PosA, const Math::Vector3& f3PosB,
				const Math::Quaternion& quatA, const Math::Quaternion& quatB,
				const Math::Vector3& f3AngularLowerLimit, const Math::Vector3& f3AngularUpperLimit)
			{
				/*Graphics::MaterialInfo materialInfo;
				materialInfo.strName = strJointName;
				materialInfo.colorAlbedo = Math::Color::Red;
				materialInfo.emRasterizerState = Graphics::EmRasterizerState::eWireframeCullNone;

				Graphics::ModelLoader modelLoader;
				modelLoader.InitCapsule(strJointName, &materialInfo, fRadius, fHeight);

				Graphics::IModelInstance* pPhysicsModelInst = Graphics::IModel::CreateInstance(modelLoader);*/

				Physics::ConstraintProperty prop;
				prop.SetGeneric6Dof(pBodyPartA->pRigidBody, f3PosA, quatA, pBodyPartB->pRigidBody, f3PosB, quatB, true, true);

				Joint* pJoint = AddJoint(strJointName, prop, pBodyPartA, pBodyPartB, nullptr);

				Physics::Generic6DofConstraint* pConstraint = static_cast<Physics::Generic6DofConstraint*>(pJoint->pConstraint);
				pConstraint->SetAngularLowerLimit(f3AngularLowerLimit);
				pConstraint->SetAngularUpperLimit(f3AngularUpperLimit);

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
				Math::Vector3(0.f, pLeftUpperArm->fHeight * 0.5f, 0.f),
				Math::Vector3(0.f, -pLeftLowerArm->fHeight * 0.5f, 0.f),
				//Math::Vector3(0.f, fScale * 0.18f, 0.f), 
				//Math::Vector3(0.f, fScale * -0.14f, 0.f),
				quatA, quatB,
				Math::Vector3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				Math::Vector3(Math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			// RIGHT ELBOW
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::Identity;
			CreateJoint(StrID::RightElbow, pRightUpperArm, pRightLowerArm,
				Math::Vector3(0.f, pRightUpperArm->fHeight * 0.5f, 0.f),
				Math::Vector3(0.f, -pRightLowerArm->fHeight * 0.5f, 0.f),
				//Math::Vector3(0.f, fScale * 0.18f, 0.f),
				//Math::Vector3(0.f, fScale * -0.14f, 0.f),
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
				Math::Vector3(Math::PI * 0.2f, FLT_EPSILON, Math::PI * 0.3f));
				//Math::Vector3(Math::PI * -0.2f, -FLT_EPSILON, Math::PI * -0.3f),
				//Math::Vector3(Math::PI * 0.2f, FLT_EPSILON, Math::PI * 0.6f));

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
				Math::Vector3(0.f, -pLeftUpperLeg->fHeight * 0.5f, 0.f),
				Math::Vector3(0.f, pLeftLowerLeg->fHeight * 0.5f, 0.f),
				//Math::Vector3(0.f, fScale * -0.25f, 0.f),
				//Math::Vector3(0.f, fScale * 0.25f, 0.f),
				quatA, quatB,
				Math::Vector3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				Math::Vector3(Math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			// RIGHT KNEE
			quatA = Math::Quaternion::Identity;
			quatB = Math::Quaternion::Identity;
			CreateJoint(StrID::RightKnee, pRightUpperLeg, pRightLowerLeg,
				Math::Vector3(0.f, -pRightUpperLeg->fHeight * 0.5f, 0.f),
				Math::Vector3(0.f, pRightLowerLeg->fHeight * 0.5f, 0.f),
				//Math::Vector3(0.f, fScale * -0.25f, 0.f),
				//Math::Vector3(0.f, fScale * 0.25f, 0.f),
				quatA, quatB,
				Math::Vector3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				Math::Vector3(Math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			//copyModelStateToRagDoll();

			return true;
		}

		void RagDoll::Update(float fElapsedTime)
		{
			//if (m_isRagDollState == false)
			//{
			//	copyModelStateToRagDoll();
			//}

			for (auto& pBodyPart : m_vecBodyParts)
			{
				pBodyPart->pRigidBody->Update(fElapsedTime);
			
				Math::Matrix matWorld = pBodyPart->pRigidBody->GetWorldMatrix();
			
				if (pBodyPart->pPhysicsModelInstance != nullptr)
				{
					pBodyPart->pPhysicsModelInstance->Update(fElapsedTime, matWorld);
				}

				//if (m_isRagDollState == true)
				//{
				//	Math::Matrix matInvParent;
				//	Graphics::ISkeletonInstance::IBone* pParentBone = pBodyPart->pBone->GetParent();
				//	if (pParentBone != nullptr)
				//	{
				//		matInvParent = pParentBone->GetGlobalTransform().Invert();
				//	}
				//	
				//	Math::Matrix matWorldMatrix = pBodyPart->pRigidBody->GetWorldMatrix();
				//	pBodyPart->pBone->SetMotionData(matWorldMatrix * matInvParent);
				//}
			}

			if (m_isRagDollState == true)
			{
				copyRagDollStateToModel();
			}
		}

		RagDoll::BodyPart* RagDoll::AddBodyPart(const String::StringID& strPartName, const Physics::RigidBodyProperty& rigidBodyProperty, Graphics::IModelInstance* pPhysicsModelInstance, Graphics::ISkeletonInstance::IBone* pBone)
		{
			if (GetBodyPort(strPartName) != nullptr)
				return nullptr;

			BodyPart* pBodyPart = nullptr;
			Physics::RigidBody* pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);
			if (pRigidBody != nullptr)
			{
				m_vecBodyParts.emplace_back(new BodyPart(strPartName, pRigidBody, pPhysicsModelInstance, pBone));
				pBodyPart = m_vecBodyParts.back();
				pBodyPart->matOrigin = pBone->GetGlobalTransform();
				pBodyPart->matBodyOrigin = Math::Matrix::Compose(Math::Vector3::One, rigidBodyProperty.originQuat, rigidBodyProperty.f3OriginPos);
			}

			return pBodyPart;
		}

		RagDoll::Joint* RagDoll::AddJoint(const String::StringID& strJointName, const Physics::ConstraintProperty& constraintProperty, BodyPart* pBodyPartA, BodyPart* pBodyPartB, Graphics::IModelInstance* pPhysicsModelInstance)
		{
			if (GetJoint(strJointName) != nullptr)
				return nullptr;

			Joint* pJoint = nullptr;
			Physics::ConstraintInterface* pConstraint = Physics::ConstraintInterface::Create(constraintProperty);
			if (pConstraint != nullptr)
			{
				m_vecJoints.emplace_back(new Joint(strJointName, pConstraint, pPhysicsModelInstance));
				pJoint = m_vecJoints.back();
				pJoint->pBodyPartA = pBodyPartA;
				pJoint->pBodyPartB = pBodyPartB;
			}

			return pJoint;
		}

		Physics::RigidBody* RagDoll::GetBodyPort(const String::StringID& strPartName)
		{
			for (auto& pBodyPart : m_vecBodyParts)
			{
				if (pBodyPart->strName == strPartName)
					return pBodyPart->pRigidBody;
			}

			return nullptr;
		}

		Physics::ConstraintInterface* RagDoll::GetJoint(const String::StringID& strJointName)
		{
			for (auto& pJoint : m_vecJoints)
			{
				if (pJoint->strName == strJointName)
					return pJoint->pConstraint;
			}

			return nullptr;
		}

		void RagDoll::Start()
		{
			m_isRagDollState = true;

			//copyModelStateToRagDoll();

			for (auto& pBodyPart : m_vecBodyParts)
			{
				pBodyPart->pRigidBody->SetActiveState(Physics::EmActiveState::eActiveTag);
			}
		}

		void RagDoll::End()
		{
			m_isRagDollState = false;

			//m_pSkeleton->SetManualControl(false);
		}

		void RagDoll::copyRagDollStateToModel()
		{
			static int nFrame = 0;
			++nFrame;

			static int nFrame2 = 0;

			m_pSkeletonInst->SetIdentity();

			std::set<BodyPart*> setBody;
			std::map<Graphics::ISkeletonInstance::IBone*, std::pair<Math::Quaternion, Math::Quaternion>> mapWorldMatrix;

			std::function<void(BodyPart*)> UpdateBodyPart = [&](BodyPart* pBodyPart)
			{
				auto iter = std::find_if(m_vecJoints.begin(), m_vecJoints.end(), [&](const Joint* pJoint)
				{
					return pJoint->pBodyPartB == pBodyPart;
				});

				BodyPart* pParent = nullptr;

				if (iter != m_vecJoints.end())
				{
					Joint* pJoint = *iter;

					auto iter = setBody.find(pJoint->pBodyPartA);
					if (iter == setBody.end())
					{
						UpdateBodyPart(pJoint->pBodyPartA);
						setBody.emplace(pJoint->pBodyPartA);
					}

					pParent = pJoint->pBodyPartA;
				}

				const Math::Matrix& matWorld = pBodyPart->pRigidBody->GetWorldMatrix();

				Math::Vector3 f3ScaleWorld;
				Math::Vector3 f3PosWorld;
				Math::Quaternion quatWorld;
				matWorld.Decompose(f3ScaleWorld, quatWorld, f3PosWorld);

				Math::Vector3 f3ScaleBodyOrigin;
				Math::Vector3 f3PosBodyOrigin;
				Math::Quaternion quatBodyOrigin;
				pBodyPart->matBodyOrigin.Decompose(f3ScaleBodyOrigin, quatBodyOrigin, f3PosBodyOrigin);

				//if (pBodyPart->isRootNode == true)
				//{
				//	Math::Matrix mmm = pBodyPart->pBone->GetMotionOffsetTransform() * pBodyPart->pBone->GetDefaultMotionData();

				//	Math::Vector3 f3ScaleOrigin;
				//	Math::Vector3 f3PosOrigin;
				//	Math::Quaternion quatOrigin;
				//	pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);
				//	//mmm.Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				//	//Math::Matrix mat = Math::Matrix::Compose(f3ScaleOrigin, quatWorld * quatBodyOrigin.Inverse() * quatOrigin, f3PosOrigin);
				//	//Math::Matrix mat = Math::Matrix::Compose(f3ScaleOrigin, quatOrigin * quatWorld, f3PosWorld);
				//	//Math::Matrix mat = Math::Matrix::CreateFromQuaternion(quatWorld * quatBodyOrigin.Inverse() * quatOrigin);
				//	Math::Matrix mat = Math::Matrix::Compose(f3ScaleOrigin, quatWorld * quatBodyOrigin.Inverse() * quatOrigin, f3PosWorld);
				//
				//	pBodyPart->pBone->SetMotionData(mat);

				//	mapWorldMatrix[pBodyPart->pBone] = std::make_pair(quatOrigin, quatWorld);
				//}
				//else
				//{
				//	if (pBodyPart->isChildBone == true)
				//	{
				//		Math::Vector3 f3ScaleOrigin;
				//		Math::Vector3 f3PosOrigin;
				//		Math::Quaternion quatOrigin;
				//		pBodyPart->pBone->GetParent()->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				//		Math::Vector3 f3ScaleOrigin2;
				//		Math::Vector3 f3PosOrigin2;
				//		Math::Quaternion quatOrigin2;
				//		pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin2, quatOrigin2, f3PosOrigin2);

				//		/*if (nFrame >= 1440)
				//		{
				//			nFrame = 0;
				//			++nFrame2;

				//			if (nFrame2 >= 6)
				//			{
				//				int a = 0;
				//				a = 10;
				//			}
				//		}

				//		auto CQ = [&](const Math::Quaternion& a, const Math::Quaternion& b, const Math::Quaternion& c)
				//		{
				//			switch (nFrame / 180)
				//			{
				//			case 0:	return a * b * c;
				//			case 1: return a.Inverse() * b * c;
				//			case 2: return a * b.Inverse() * c;
				//			case 3: return a * b * c.Inverse();
				//			case 4: return a.Inverse() * b.Inverse() * c;
				//			case 5: return a.Inverse() * b * c.Inverse();
				//			case 6: return a * b.Inverse() * c.Inverse();
				//			case 7: return a.Inverse() * b.Inverse() * c.Inverse();
				//			}
				//		};

				//		Math::Quaternion quatRet;

				//		switch (nFrame2)
				//		{
				//		case 0: quatRet = CQ(quatOrigin, quatWorld, quatBodyOrigin); break;
				//		case 1: quatRet = CQ(quatOrigin, quatBodyOrigin, quatWorld); break;
				//		case 2: quatRet = CQ(quatWorld, quatOrigin, quatBodyOrigin); break;
				//		case 3: quatRet = CQ(quatWorld, quatBodyOrigin, quatOrigin); break;
				//		case 4: quatRet = CQ(quatBodyOrigin, quatOrigin, quatWorld); break;
				//		case 5: quatRet = CQ(quatBodyOrigin, quatWorld, quatOrigin); break;
				//		}*/

				//		Math::Matrix mat;
				//		Math::Matrix m = Math::Matrix::Compose(f3ScaleOrigin, quatOrigin * quatWorld.Inverse() * quatBodyOrigin, f3PosOrigin);

				//		if (pParent != nullptr)
				//		{
				//			//Math::Matrix matInv = pParent->pBone->GetGlobalTransform().Invert();
				//			//Math::Matrix matInv = pParent->pRigidBody->GetWorldMatrix();
				//			//mat = matInv * m;
				//			mat = m;
				//		}
				//		else
				//		{
				//			mat = m;
				//		}

				//		pBodyPart->pBone->GetParent()->SetMotionData(mat);
				//	}
				//	else
				//	{
				//		Math::Quaternion quatRoot;
				//		if (pParent->isRootNode == true)
				//		{
				//			Math::Vector3 f3ScaleOrigin;
				//			Math::Vector3 f3PosOrigin;
				//			pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatRoot, f3PosOrigin);
				//		}

				//		Math::Vector3 f3ScaleOrigin;
				//		Math::Vector3 f3PosOrigin;
				//		Math::Quaternion quatOrigin;
				//		pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				//		Math::Matrix matTransform = pParent->pRigidBody->GetWorldMatrix().Invert() * pBodyPart->pRigidBody->GetWorldMatrix();

				//		Math::Vector3 f3ScaleOrigin2;
				//		Math::Vector3 f3PosOrigin2;
				//		Math::Quaternion quatOrigin2;
				//		//pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);
				//		matTransform.Decompose(f3ScaleOrigin2, quatOrigin2, f3PosOrigin2);

				//		//Math::Matrix mat = Math::Matrix::Compose(f3ScaleOrigin, quatOrigin, f3PosOrigin);
				//		//Math::Matrix mat = Math::Matrix::CreateFromQuaternion(mapWorldMatrix[pBodyPart->pBone].first.Inverse() * quatOrigin);
				//		//Math::Matrix mat = Math::Matrix::CreateFromQuaternion(quatOrigin2 * quatRoot * quatOrigin);
				//		//Math::Matrix mat = Math::Matrix::CreateFromQuaternion(quatOrigin2.Inverse() * quatRoot * quatOrigin);
				//		//Math::Matrix mat = Math::Matrix::CreateFromQuaternion(quatWorld);
				//		Math::Matrix mat = Math::Matrix::CreateFromQuaternion(quatOrigin2 * quatBodyOrigin.Inverse() * quatOrigin);

				//		pBodyPart->pBone->SetMotionData(mat);

				//		//Math::Vector3 f3ScaleOrigin;
				//		//Math::Vector3 f3PosOrigin;
				//		//Math::Quaternion quatOrigin;
				//		//pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				//		//Math::Matrix mat;
				//		////Math::Matrix m = Math::Matrix::Compose(f3ScaleOrigin, quatWorld, f3PosWorld);
				//		////Math::Matrix m = Math::Matrix::Compose(f3ScaleOrigin, quatWorld * quatBodyOrigin.Inverse() * quatOrigin, f3PosOrigin);
				//		//Math::Matrix m = Math::Matrix::Compose(f3ScaleOrigin, quatOrigin * quatWorld.Inverse() * quatBodyOrigin, f3PosOrigin);
				//		////Math::Matrix m = Math::Matrix::Compose(f3ScaleOrigin, quatBodyOrigin.Inverse() * quatWorld, f3PosOrigin);

				//		//if (pParent != nullptr)
				//		//{
				//		//	Math::Vector3 f3ScaleOrigin2;
				//		//	Math::Vector3 f3PosOrigin2;
				//		//	Math::Quaternion quatOrigin2; 
				//		//	pParent->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin2, quatOrigin2, f3PosOrigin2);
				//		//	//pParent->pRigidBody->GetWorldMatrix().Invert().Decompose(f3ScaleOrigin2, quatOrigin2, f3PosOrigin2);
				//		//	//mapWorldMatrix[pParent->pBone].Decompose(f3ScaleOrigin2, quatOrigin2, f3PosOrigin2);

				//		//	//m = Math::Matrix::Compose(f3ScaleOrigin, quatWorld, f3PosOrigin);
				//		//	
				//		//	//mat = Math::Matrix::CreateFromQuaternion(quatOrigin2) * m;
				//		//	auto a = mapWorldMatrix[pParent->pBone];
				//		//	mat = Math::Matrix::CreateFromQuaternion(a.first * a.second.Inverse()) * m;

				//		//	//mat = mapWorldMatrix[pParent->pBone].Invert() * m;

				//		//	//Math::Matrix matInv = pParent->pBone->GetMotionOffsetTransform();
				//		//	//Math::Matrix matInv = pParent->pRigidBody->GetWorldMatrix().Invert();
				//		//	//mat = matInv * mat;
				//		//	//mat = m;
				//		//}
				//		//else
				//		//{
				//		//	mat = m;
				//		//}
				//		//
				//		//pBodyPart->pBone->SetMotionData(mat);
				//	}
				//}
			};

			for (auto& pJoint : m_vecJoints)
			{
				auto iter = setBody.find(pJoint->pBodyPartA);
				if (iter == setBody.end())
				{
					UpdateBodyPart(pJoint->pBodyPartA);
					setBody.emplace(pJoint->pBodyPartA);
				}
			}
		}

		void RagDoll::copyModelStateToRagDoll()
		{
			for (auto& pBodyPart : m_vecBodyParts)
			{
				Math::Vector3 f3ScaleOrigin;
				Math::Vector3 f3PosOrigin;
				Math::Quaternion quatOrigin;
				pBodyPart->matBodyOrigin.Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				Math::Vector3 f3Pos;
				Math::Quaternion quat;
				if (pBodyPart->isChildBone == true)
				{
					auto pChildBone = pBodyPart->pBone;
					auto pParentBone = pBodyPart->pBone->GetParent();

					Math::Vector3 f3ScaleChild;
					Math::Vector3 f3PosChild;
					Math::Quaternion quatChild;
					pChildBone->GetGlobalTransform().Decompose(f3ScaleChild, quatChild, f3PosChild);

					Math::Vector3 f3ScaleParent;
					Math::Vector3 f3PosParent;
					Math::Quaternion quatParent;
					pParentBone->GetGlobalTransform().Decompose(f3ScaleParent, quatParent, f3PosParent);

					Math::Vector3 f3Offset = (f3PosChild - f3PosParent) * 0.5f;
					f3Pos = f3PosParent + f3Offset;
					quat = quatOrigin.Inverse() * quatParent;
				}
				else
				{
					// 여기 문제없는지 확인 필요
					Math::Vector3 f3ScaleBone;
					Math::Quaternion quatBone;
					pBodyPart->pBone->GetGlobalTransform().Decompose(f3ScaleBone, quatBone, f3Pos);
				}

				Math::Matrix matWorld = Math::Matrix::Compose(Math::Vector3::One, quat, f3Pos);
				pBodyPart->pRigidBody->SetWorldMatrix(matWorld);
			}
		}
	}
}