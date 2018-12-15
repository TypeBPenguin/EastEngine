#include "stdafx.h"
#include "RagDoll.h"

#include "Model/ModelLoader.h"

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

namespace eastengine
{
	namespace gameobject
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

				graphics::IModel::DestroyInstance(&pBodyPart->pPhysicsModelInstance);
				pBodyPart->pPhysicsModelInstance = nullptr;

				SafeDelete(pBodyPart);
			});
			m_vecBodyParts.clear();
		}

		bool RagDoll::BuildBipadRagDoll(graphics::ISkeletonInstance* pSkeletonInst, const math::float3& f3Pos, const math::Quaternion& quatRotation, float fScale)
		{
			if (pSkeletonInst == nullptr)
				return false;

			m_pSkeletonInst = pSkeletonInst;

			//math::Matrix matTranslation = math::Matrix::Compose(math::float3::One, math::Quaternion::CreateFromYawPitchRoll(math::ToRadians(180.f), 0.f, 0.f), f3Pos);
			math::Matrix matTranslation = math::Matrix::Compose(math::float3::One, quatRotation, f3Pos);

			auto CreateBodyPart = [&](bool isChildBone, const string::StringID& strBoneName, const string::StringID& strName, float fRadius, float fHeight, const math::float3& f3Pos, const math::Quaternion& quat = math::Quaternion::Identity)
			{
				float fNewHeight = fHeight;
				math::float3 f3NewPos = f3Pos;
				math::float3 f3Offset;
				math::Quaternion newQuat = quat;

				graphics::ISkeletonInstance::IBone* pBone = pSkeletonInst->GetBone(strBoneName);
				//if (isChildBone == true)
				//{
				//	math::float3 f3ScaleChild;
				//	math::float3 f3PosChild;
				//	math::Quaternion quatChild;
				//	pBone->GetGlobalMatrix().Decompose(f3ScaleChild, quatChild, f3PosChild);
				//
				//	math::float3 f3ScaleParent;
				//	math::float3 f3PosParent;
				//	math::Quaternion quatParent;
				//	pBone->GetParent()->GetGlobalMatrix().Decompose(f3ScaleParent, quatParent, f3PosParent);
				//
				//	f3Offset = (f3PosChild - f3PosParent) * 0.5f;
				//	f3NewPos = f3PosParent + f3Offset;
				//
				//	newQuat = quatParent * quat;
				//
				//	fNewHeight = f3Offset.Length() * 1.5f;
				//}

				graphics::MaterialInfo materialInfo;
				materialInfo.name = strName;
				materialInfo.colorAlbedo = math::Color::Red;
				materialInfo.emRasterizerState = graphics::EmRasterizerState::eWireframeCullNone;
				//materialInfo.emRasterizerState = graphics::EmRasterizerState::eSolidCCW;

				graphics::ModelLoader modelLoader;
				modelLoader.InitCapsule(strName, &materialInfo, fRadius, fNewHeight);

				graphics::IModelInstance* pPhysicsModelInst = graphics::IModel::CreateInstance(modelLoader);

				physics::RigidBodyProperty prop;
				prop.fLinearDamping = 0.05f;
				prop.fAngularDamping = 0.85f;
				prop.nCollisionFlag = physics::CollisionFlag::eCharacterObject;
				prop.strName = strName;
				prop.f3OriginPos = f3NewPos;
				prop.originQuat = newQuat;
				prop.matOffset = matTranslation;
				prop.shapeInfo.SetCapsule(fRadius, fHeight);

				//math::float3 f3Scale;
				//math::Quaternion quat2;
				//pBone->GetGlobalMatrix().Decompose(f3Scale, quat2, prop.f3OriginPos);

				BodyPart* pBodyPart = AddBodyPart(strName, prop, pPhysicsModelInst, pBone);
				pBodyPart->f3Offset = f3Offset;
				pBodyPart->fHeight = fNewHeight;
				//pBodyPart->isChildBone = isChildBone;

				pBodyPart->pRigidBody->SetActiveState(physics::ActiveStateType::eIslandSleeping);

				return pBodyPart;
			};

			//physics::RigidBody* pPelvis = CreateBodyPart("Pelvis", StrID::Pelvis, fScale * 0.15f, fScale * 0.2f, math::float3(0.f, fScale * 1.f, 0.f));
			//physics::RigidBody* pSpine = CreateBodyPart("Spine", StrID::Spine, fScale * 0.15f, fScale * 0.28f, math::float3(0.f, fScale * 1.2f, 0.f));
			//physics::RigidBody* pHead = CreateBodyPart("Head", StrID::Head, fScale * 0.1f, fScale * 0.05f, math::float3(0.f, fScale * 1.6f, 0.f));
			//physics::RigidBody* pLeftUpperLeg = CreateBodyPart("L_Thigh1", StrID::LeftUpperLeg, fScale * 0.07f, fScale * 0.45f, math::float3(fScale * -0.18f, fScale * 0.65f, 0.f));
			//physics::RigidBody* pLeftLowerLeg = CreateBodyPart("L_Knee2", StrID::LeftLowerLeg, fScale * 0.05f, fScale * 0.37f, math::float3(fScale * -0.18f, fScale * 0.2f, 0.f));
			//physics::RigidBody* pRightUpperLeg = CreateBodyPart("R_Thigh", StrID::RightUpperLeg, fScale * 0.07f, fScale * 0.45f, math::float3(fScale * 0.18f, fScale * 0.65f, 0.f));
			//physics::RigidBody* pRightLowerLeg = CreateBodyPart("R_Knee", StrID::RightLowerLeg, fScale * 0.05f, fScale * 0.37f, math::float3(fScale * 0.18f, fScale * 0.2f, 0.f));
			//physics::RigidBody* pLeftUpperArm = CreateBodyPart("L_UpperArm", StrID::LeftUpperArm, fScale * 0.05f, fScale * 0.33f, math::float3(fScale * -0.35f, fScale * 1.45f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, math::PIDIV2));
			//physics::RigidBody* pLeftLowerArm = CreateBodyPart("L_Forearm", StrID::LeftLowerArm, fScale * 0.04f, fScale * 0.25f, math::float3(fScale * -0.7f, fScale * 1.45f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, math::PIDIV2));
			//physics::RigidBody* pRightUpperArm = CreateBodyPart("R_UpperArm", StrID::RightUpperArm, fScale * 0.05f, fScale * 0.33f, math::float3(fScale * 0.35f, fScale * 1.45f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -math::PIDIV2));
			//physics::RigidBody* pRightLowerArm = CreateBodyPart("R_Forearm", StrID::RightLowerArm, fScale * 0.04f, fScale * 0.25f, math::float3(fScale * 0.7f, fScale * 1.45f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -math::PIDIV2));

			BodyPart* pPelvis = CreateBodyPart(false, "Character1_Hips", StrID::Pelvis, fScale * 0.15f, fScale * 0.05f, math::float3(0.f, fScale * 1.1f, 0.f));
			pPelvis->isRootNode = true;

			BodyPart* pSpine = CreateBodyPart(false, "Character1_Spine", StrID::Spine, fScale * 0.15f, fScale * 0.20f, math::float3(0.f, fScale * 1.35f, 0.f));
			BodyPart* pHead = CreateBodyPart(false, "Character1_Head", StrID::Head, fScale * 0.1f, fScale * 0.05f, math::float3(0.f, fScale * 1.7f, 0.f));
			/*BodyPart* pLeftUpperLeg = CreateBodyPart(true, "Character1_LeftLeg", StrID::LeftUpperLeg, fScale * 0.07f, fScale * 0.4f, math::float3(fScale * -0.1f, fScale * 0.8f, 0.f));
			BodyPart* pLeftLowerLeg = CreateBodyPart(true, "Character1_LeftFoot", StrID::LeftLowerLeg, fScale * 0.05f, fScale * 0.42f, math::float3(fScale * -0.1f, fScale * 0.35f, 0.f));
			BodyPart* pRightUpperLeg = CreateBodyPart(true, "Character1_RightLeg", StrID::RightUpperLeg, fScale * 0.07f, fScale * 0.4f, math::float3(fScale * 0.1f, fScale * 0.8f, 0.f));
			BodyPart* pRightLowerLeg = CreateBodyPart(true, "Character1_RightFoot", StrID::RightLowerLeg, fScale * 0.05f, fScale * 0.42f, math::float3(fScale * 0.1f, fScale * 0.35f, 0.f));
			BodyPart* pLeftUpperArm = CreateBodyPart(true, "Character1_LeftForeArm", StrID::LeftUpperArm, fScale * 0.05f, fScale * 0.25f, math::float3(fScale * -0.25f, fScale * 1.475f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, math::PIDIV2));
			BodyPart* pLeftLowerArm = CreateBodyPart(true, "Character1_LeftHand", StrID::LeftLowerArm, fScale * 0.04f, fScale * 0.2f, math::float3(fScale * -0.525f, fScale * 1.475f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, math::PIDIV2));
			BodyPart* pRightUpperArm = CreateBodyPart(true, "Character1_RightForeArm", StrID::RightUpperArm, fScale * 0.05f, fScale * 0.25f, math::float3(fScale * 0.25f, fScale * 1.475f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -math::PIDIV2));
			BodyPart* pRightLowerArm = CreateBodyPart(true, "Character1_RightHand", StrID::RightLowerArm, fScale * 0.04f, fScale * 0.2f, math::float3(fScale * 0.525f, fScale * 1.475f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -math::PIDIV2));*/

			BodyPart* pLeftUpperLeg = CreateBodyPart(true, "Character1_LeftUpLeg", StrID::LeftUpperLeg, fScale * 0.07f, fScale * 0.4f, math::float3(fScale * -0.1f, fScale * 0.8f, 0.f));
			BodyPart* pLeftLowerLeg = CreateBodyPart(true, "Character1_LeftLeg", StrID::LeftLowerLeg, fScale * 0.05f, fScale * 0.42f, math::float3(fScale * -0.1f, fScale * 0.35f, 0.f));
			BodyPart* pRightUpperLeg = CreateBodyPart(true, "Character1_RightUpLeg", StrID::RightUpperLeg, fScale * 0.07f, fScale * 0.4f, math::float3(fScale * 0.1f, fScale * 0.8f, 0.f));
			BodyPart* pRightLowerLeg = CreateBodyPart(true, "Character1_RightLeg", StrID::RightLowerLeg, fScale * 0.05f, fScale * 0.42f, math::float3(fScale * 0.1f, fScale * 0.35f, 0.f));
			BodyPart* pLeftUpperArm = CreateBodyPart(true, "Character1_LeftArm", StrID::LeftUpperArm, fScale * 0.05f, fScale * 0.25f, math::float3(fScale * -0.25f, fScale * 1.475f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, math::PIDIV2));
			BodyPart* pLeftLowerArm = CreateBodyPart(true, "Character1_LeftForeArm", StrID::LeftLowerArm, fScale * 0.04f, fScale * 0.2f, math::float3(fScale * -0.525f, fScale * 1.475f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, math::PIDIV2));
			BodyPart* pRightUpperArm = CreateBodyPart(true, "Character1_RightArm", StrID::RightUpperArm, fScale * 0.05f, fScale * 0.25f, math::float3(fScale * 0.25f, fScale * 1.475f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -math::PIDIV2));
			BodyPart* pRightLowerArm = CreateBodyPart(true, "Character1_RightForeArm", StrID::RightLowerArm, fScale * 0.04f, fScale * 0.2f, math::float3(fScale * 0.525f, fScale * 1.475f, 0.f), math::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, -math::PIDIV2));

			size_t nBodyCount = m_vecBodyParts.size();
			for (size_t i = 0; i < nBodyCount; ++i)
			{
				m_vecBodyParts[i]->pRigidBody->SetDeactivationTime(0.8f);
				m_vecBodyParts[i]->pRigidBody->SetSleepingThresholds(1.6f, 2.5f);
			}

			auto CreateJoint = [&](const string::StringID& strJointName, BodyPart* pBodyPartA, BodyPart* pBodyPartB,
				const math::float3& f3PosA, const math::float3& f3PosB,
				const math::Quaternion& quatA, const math::Quaternion& quatB,
				const math::float3& f3AngularLowerLimit, const math::float3& f3AngularUpperLimit)
			{
				/*graphics::MaterialInfo materialInfo;
				materialInfo.name = strJointName;
				materialInfo.colorAlbedo = math::Color::Red;
				materialInfo.emRasterizerState = graphics::EmRasterizerState::eWireframeCullNone;

				graphics::ModelLoader modelLoader;
				modelLoader.InitCapsule(strJointName, &materialInfo, fRadius, fHeight);

				graphics::IModelInstance* pPhysicsModelInst = graphics::IModel::CreateInstance(modelLoader);*/

				physics::ConstraintProperty prop;
				prop.SetGeneric6Dof(pBodyPartA->pRigidBody, f3PosA, quatA, pBodyPartB->pRigidBody, f3PosB, quatB, true, true);

				Joint* pJoint = AddJoint(strJointName, prop, pBodyPartA, pBodyPartB, nullptr);

				physics::Generic6DofConstraint* pConstraint = static_cast<physics::Generic6DofConstraint*>(pJoint->pConstraint);
				pConstraint->SetAngularLowerLimit(f3AngularLowerLimit);
				pConstraint->SetAngularUpperLimit(f3AngularUpperLimit);

				return pJoint;
			};

			// SPINE HEAD
			math::Quaternion quatA = math::Quaternion::Identity;
			math::Quaternion quatB = math::Quaternion::Identity;
			CreateJoint(StrID::Spine_Head, pSpine, pHead,
				math::float3(0.f, fScale * 0.3f, 0.f),
				math::float3(0.f, fScale * -0.14f, 0.f),
				quatA, quatB,
				math::float3(math::PI * -0.3f, -FLT_EPSILON, math::PI * -0.3f),
				math::float3(math::PI * 0.5f, FLT_EPSILON, math::PI * 0.3f));

			// LEFT SHOULDER
			quatA = math::Quaternion::Identity;
			quatB = math::Quaternion::CreateFromYawPitchRoll(0.f, -math::PIDIV2, math::PIDIV2);
			CreateJoint(StrID::LeftShoulder, pSpine, pLeftUpperArm, 
				math::float3(fScale * -0.2f, fScale * 0.15f, 0.f), 
				math::float3(0.f, fScale * -0.18f, 0.f),
				quatA, quatB,
				math::float3(math::PI * -0.8f, -FLT_EPSILON, math::PI * -0.5f), 
				math::float3(math::PI * 0.8f, FLT_EPSILON, math::PI * 0.5f));

			// RIGHT SHOULDER
			quatA = math::Quaternion::Identity;
			quatB = math::Quaternion::CreateFromYawPitchRoll(0.f, math::PIDIV2, 0.f);
			CreateJoint(StrID::RightShoulder, pSpine, pRightUpperArm, 
				math::float3(fScale * 0.2f, fScale * 0.15f, 0.f), 
				math::float3(0.f, fScale * -0.18f, 0.f),
				quatA, quatB,
				math::float3(math::PI * -0.8f, -FLT_EPSILON, math::PI * -0.5f),
				math::float3(math::PI * 0.8f, FLT_EPSILON, math::PI * 0.5f));

			// LEFT ELBOW
			quatA = math::Quaternion::Identity;
			quatB = math::Quaternion::Identity;
			CreateJoint(StrID::LeftElbow, pLeftUpperArm, pLeftLowerArm,
				math::float3(0.f, pLeftUpperArm->fHeight * 0.5f, 0.f),
				math::float3(0.f, -pLeftLowerArm->fHeight * 0.5f, 0.f),
				//math::float3(0.f, fScale * 0.18f, 0.f), 
				//math::float3(0.f, fScale * -0.14f, 0.f),
				quatA, quatB,
				math::float3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				math::float3(math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			// RIGHT ELBOW
			quatA = math::Quaternion::Identity;
			quatB = math::Quaternion::Identity;
			CreateJoint(StrID::RightElbow, pRightUpperArm, pRightLowerArm,
				math::float3(0.f, pRightUpperArm->fHeight * 0.5f, 0.f),
				math::float3(0.f, -pRightLowerArm->fHeight * 0.5f, 0.f),
				//math::float3(0.f, fScale * 0.18f, 0.f),
				//math::float3(0.f, fScale * -0.14f, 0.f),
				quatA, quatB,
				math::float3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				math::float3(math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			// PELVIS
			quatA = math::Quaternion::CreateFromYawPitchRoll(math::PIDIV2, 0.f, 0.f);
			quatB = math::Quaternion::CreateFromYawPitchRoll(math::PIDIV2, 0.f, 0.f);
			CreateJoint(StrID::Pelvis, pPelvis, pSpine,
				math::float3(0.f, fScale * 0.15f, 0.f),
				math::float3(0.f, fScale * -0.15f, 0.f),
				quatA, quatB,
				math::float3(math::PI * -0.2f, -FLT_EPSILON, math::PI * -0.3f),
				math::float3(math::PI * 0.2f, FLT_EPSILON, math::PI * 0.3f));
				//math::float3(math::PI * -0.2f, -FLT_EPSILON, math::PI * -0.3f),
				//math::float3(math::PI * 0.2f, FLT_EPSILON, math::PI * 0.6f));

			// LEFT HIP
			quatA = math::Quaternion::Identity;
			quatB = math::Quaternion::Identity;
			CreateJoint(StrID::LeftHip, pPelvis, pLeftUpperLeg,
				math::float3(fScale * -0.18f, fScale * -0.1f, 0.f),
				math::float3(0.f, fScale * 0.225f, 0.f),
				quatA, quatB,
				math::float3(math::PIDIV2 * -0.5f, -FLT_EPSILON, -FLT_EPSILON),
				math::float3(math::PIDIV2 * 0.8f, FLT_EPSILON, math::PIDIV2 * 0.6f));

			// RIGHT HIP
			quatA = math::Quaternion::Identity;
			quatB = math::Quaternion::Identity;
			CreateJoint(StrID::RightHip, pPelvis, pRightUpperLeg,
				math::float3(fScale * 0.18f, fScale * -0.1f, 0.f),
				math::float3(0.f, fScale * 0.225f, 0.f),
				quatA, quatB,
				math::float3(math::PIDIV2 * -0.5f, -FLT_EPSILON, math::PIDIV2 * -0.6f),
				math::float3(math::PIDIV2 * 0.8f, FLT_EPSILON, FLT_EPSILON));

			// LEFT KNEE
			quatA = math::Quaternion::Identity;
			quatB = math::Quaternion::Identity;
			CreateJoint(StrID::LeftKnee, pLeftUpperLeg, pLeftLowerLeg,
				math::float3(0.f, -pLeftUpperLeg->fHeight * 0.5f, 0.f),
				math::float3(0.f, pLeftLowerLeg->fHeight * 0.5f, 0.f),
				//math::float3(0.f, fScale * -0.25f, 0.f),
				//math::float3(0.f, fScale * 0.25f, 0.f),
				quatA, quatB,
				math::float3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				math::float3(math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			// RIGHT KNEE
			quatA = math::Quaternion::Identity;
			quatB = math::Quaternion::Identity;
			CreateJoint(StrID::RightKnee, pRightUpperLeg, pRightLowerLeg,
				math::float3(0.f, -pRightUpperLeg->fHeight * 0.5f, 0.f),
				math::float3(0.f, pRightLowerLeg->fHeight * 0.5f, 0.f),
				//math::float3(0.f, fScale * -0.25f, 0.f),
				//math::float3(0.f, fScale * 0.25f, 0.f),
				quatA, quatB,
				math::float3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON),
				math::float3(math::PI * 0.7f, FLT_EPSILON, FLT_EPSILON));

			//copyModelStateToRagDoll();

			return true;
		}

		void RagDoll::Update(float elapsedTime)
		{
			//if (m_isRagDollState == false)
			//{
			//	copyModelStateToRagDoll();
			//}

			for (auto& pBodyPart : m_vecBodyParts)
			{
				pBodyPart->pRigidBody->Update(elapsedTime);
			
				math::Matrix matWorld = pBodyPart->pRigidBody->GetWorldMatrix();
			
				if (pBodyPart->pPhysicsModelInstance != nullptr)
				{
					pBodyPart->pPhysicsModelInstance->Update(elapsedTime, matWorld);
				}

				//if (m_isRagDollState == true)
				//{
				//	math::Matrix matInvParent;
				//	graphics::ISkeletonInstance::IBone* pParentBone = pBodyPart->pBone->GetParent();
				//	if (pParentBone != nullptr)
				//	{
				//		matInvParent = pParentBone->GetGlobalMatrix().Invert();
				//	}
				//	
				//	math::Matrix matWorldMatrix = pBodyPart->pRigidBody->GetWorldMatrix();
				//	pBodyPart->pBone->SetMotionTransform(matWorldMatrix * matInvParent);
				//}
			}

			if (m_isRagDollState == true)
			{
				copyRagDollStateToModel();
			}
		}

		RagDoll::BodyPart* RagDoll::AddBodyPart(const string::StringID& strPartName, const physics::RigidBodyProperty& rigidBodyProperty, graphics::IModelInstance* pPhysicsModelInstance, graphics::ISkeletonInstance::IBone* pBone)
		{
			if (GetBodyPort(strPartName) != nullptr)
				return nullptr;

			BodyPart* pBodyPart = nullptr;
			physics::RigidBody* pRigidBody = physics::RigidBody::Create(rigidBodyProperty);
			if (pRigidBody != nullptr)
			{
				m_vecBodyParts.emplace_back(new BodyPart(strPartName, pRigidBody, pPhysicsModelInstance, pBone));
				pBodyPart = m_vecBodyParts.back();
				pBodyPart->matOrigin = pBone->GetGlobalMatrix();
				pBodyPart->matBodyOrigin = math::Matrix::Compose(math::float3::One, rigidBodyProperty.originQuat, rigidBodyProperty.f3OriginPos);
			}

			return pBodyPart;
		}

		RagDoll::Joint* RagDoll::AddJoint(const string::StringID& strJointName, const physics::ConstraintProperty& constraintProperty, BodyPart* pBodyPartA, BodyPart* pBodyPartB, graphics::IModelInstance* pPhysicsModelInstance)
		{
			if (GetJoint(strJointName) != nullptr)
				return nullptr;

			Joint* pJoint = nullptr;
			physics::ConstraintInterface* pConstraint = physics::ConstraintInterface::Create(constraintProperty);
			if (pConstraint != nullptr)
			{
				m_vecJoints.emplace_back(new Joint(strJointName, pConstraint, pPhysicsModelInstance));
				pJoint = m_vecJoints.back();
				pJoint->pBodyPartA = pBodyPartA;
				pJoint->pBodyPartB = pBodyPartB;
			}

			return pJoint;
		}

		physics::RigidBody* RagDoll::GetBodyPort(const string::StringID& strPartName)
		{
			for (auto& pBodyPart : m_vecBodyParts)
			{
				if (pBodyPart->strName == strPartName)
					return pBodyPart->pRigidBody;
			}

			return nullptr;
		}

		physics::ConstraintInterface* RagDoll::GetJoint(const string::StringID& strJointName)
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
				pBodyPart->pRigidBody->SetActiveState(physics::ActiveStateType::eActiveTag);
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
			std::map<graphics::ISkeletonInstance::IBone*, std::pair<math::Quaternion, math::Quaternion>> mapWorldMatrix;

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

					auto iter_find = setBody.find(pJoint->pBodyPartA);
					if (iter_find == setBody.end())
					{
						UpdateBodyPart(pJoint->pBodyPartA);
						setBody.emplace(pJoint->pBodyPartA);
					}

					pParent = pJoint->pBodyPartA;
				}

				const math::Matrix& matWorld = pBodyPart->pRigidBody->GetWorldMatrix();

				math::float3 f3ScaleWorld;
				math::float3 f3PosWorld;
				math::Quaternion quatWorld;
				matWorld.Decompose(f3ScaleWorld, quatWorld, f3PosWorld);

				math::float3 f3ScaleBodyOrigin;
				math::float3 f3PosBodyOrigin;
				math::Quaternion quatBodyOrigin;
				pBodyPart->matBodyOrigin.Decompose(f3ScaleBodyOrigin, quatBodyOrigin, f3PosBodyOrigin);

				//if (pBodyPart->isRootNode == true)
				//{
				//	math::Matrix mmm = pBodyPart->pBone->GetMotionOffsetTransform() * pBodyPart->pBone->GetDefaultMotionData();

				//	math::float3 f3ScaleOrigin;
				//	math::float3 f3PosOrigin;
				//	math::Quaternion quatOrigin;
				//	pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);
				//	//mmm.Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				//	//math::Matrix mat = math::Matrix::Compose(f3ScaleOrigin, quatWorld * quatBodyOrigin.Inverse() * quatOrigin, f3PosOrigin);
				//	//math::Matrix mat = math::Matrix::Compose(f3ScaleOrigin, quatOrigin * quatWorld, f3PosWorld);
				//	//math::Matrix mat = math::Matrix::CreateFromQuaternion(quatWorld * quatBodyOrigin.Inverse() * quatOrigin);
				//	math::Matrix mat = math::Matrix::Compose(f3ScaleOrigin, quatWorld * quatBodyOrigin.Inverse() * quatOrigin, f3PosWorld);
				//
				//	pBodyPart->pBone->SetMotionTransform(mat);

				//	mapWorldMatrix[pBodyPart->pBone] = std::make_pair(quatOrigin, quatWorld);
				//}
				//else
				//{
				//	if (pBodyPart->isChildBone == true)
				//	{
				//		math::float3 f3ScaleOrigin;
				//		math::float3 f3PosOrigin;
				//		math::Quaternion quatOrigin;
				//		pBodyPart->pBone->GetParent()->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				//		math::float3 f3ScaleOrigin2;
				//		math::float3 f3PosOrigin2;
				//		math::Quaternion quatOrigin2;
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

				//		auto CQ = [&](const math::Quaternion& a, const math::Quaternion& b, const math::Quaternion& c)
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

				//		math::Quaternion quatRet;

				//		switch (nFrame2)
				//		{
				//		case 0: quatRet = CQ(quatOrigin, quatWorld, quatBodyOrigin); break;
				//		case 1: quatRet = CQ(quatOrigin, quatBodyOrigin, quatWorld); break;
				//		case 2: quatRet = CQ(quatWorld, quatOrigin, quatBodyOrigin); break;
				//		case 3: quatRet = CQ(quatWorld, quatBodyOrigin, quatOrigin); break;
				//		case 4: quatRet = CQ(quatBodyOrigin, quatOrigin, quatWorld); break;
				//		case 5: quatRet = CQ(quatBodyOrigin, quatWorld, quatOrigin); break;
				//		}*/

				//		math::Matrix mat;
				//		math::Matrix m = math::Matrix::Compose(f3ScaleOrigin, quatOrigin * quatWorld.Inverse() * quatBodyOrigin, f3PosOrigin);

				//		if (pParent != nullptr)
				//		{
				//			//math::Matrix matInv = pParent->pBone->GetGlobalMatrix().Invert();
				//			//math::Matrix matInv = pParent->pRigidBody->GetWorldMatrix();
				//			//mat = matInv * m;
				//			mat = m;
				//		}
				//		else
				//		{
				//			mat = m;
				//		}

				//		pBodyPart->pBone->GetParent()->SetMotionTransform(mat);
				//	}
				//	else
				//	{
				//		math::Quaternion quatRoot;
				//		if (pParent->isRootNode == true)
				//		{
				//			math::float3 f3ScaleOrigin;
				//			math::float3 f3PosOrigin;
				//			pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatRoot, f3PosOrigin);
				//		}

				//		math::float3 f3ScaleOrigin;
				//		math::float3 f3PosOrigin;
				//		math::Quaternion quatOrigin;
				//		pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				//		math::Matrix matTransform = pParent->pRigidBody->GetWorldMatrix().Invert() * pBodyPart->pRigidBody->GetWorldMatrix();

				//		math::float3 f3ScaleOrigin2;
				//		math::float3 f3PosOrigin2;
				//		math::Quaternion quatOrigin2;
				//		//pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);
				//		matTransform.Decompose(f3ScaleOrigin2, quatOrigin2, f3PosOrigin2);

				//		//math::Matrix mat = math::Matrix::Compose(f3ScaleOrigin, quatOrigin, f3PosOrigin);
				//		//math::Matrix mat = math::Matrix::CreateFromQuaternion(mapWorldMatrix[pBodyPart->pBone].first.Inverse() * quatOrigin);
				//		//math::Matrix mat = math::Matrix::CreateFromQuaternion(quatOrigin2 * quatRoot * quatOrigin);
				//		//math::Matrix mat = math::Matrix::CreateFromQuaternion(quatOrigin2.Inverse() * quatRoot * quatOrigin);
				//		//math::Matrix mat = math::Matrix::CreateFromQuaternion(quatWorld);
				//		math::Matrix mat = math::Matrix::CreateFromQuaternion(quatOrigin2 * quatBodyOrigin.Inverse() * quatOrigin);

				//		pBodyPart->pBone->SetMotionTransform(mat);

				//		//math::float3 f3ScaleOrigin;
				//		//math::float3 f3PosOrigin;
				//		//math::Quaternion quatOrigin;
				//		//pBodyPart->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				//		//math::Matrix mat;
				//		////math::Matrix m = math::Matrix::Compose(f3ScaleOrigin, quatWorld, f3PosWorld);
				//		////math::Matrix m = math::Matrix::Compose(f3ScaleOrigin, quatWorld * quatBodyOrigin.Inverse() * quatOrigin, f3PosOrigin);
				//		//math::Matrix m = math::Matrix::Compose(f3ScaleOrigin, quatOrigin * quatWorld.Inverse() * quatBodyOrigin, f3PosOrigin);
				//		////math::Matrix m = math::Matrix::Compose(f3ScaleOrigin, quatBodyOrigin.Inverse() * quatWorld, f3PosOrigin);

				//		//if (pParent != nullptr)
				//		//{
				//		//	math::float3 f3ScaleOrigin2;
				//		//	math::float3 f3PosOrigin2;
				//		//	math::Quaternion quatOrigin2; 
				//		//	pParent->pBone->GetDefaultMotionData().Decompose(f3ScaleOrigin2, quatOrigin2, f3PosOrigin2);
				//		//	//pParent->pRigidBody->GetWorldMatrix().Invert().Decompose(f3ScaleOrigin2, quatOrigin2, f3PosOrigin2);
				//		//	//mapWorldMatrix[pParent->pBone].Decompose(f3ScaleOrigin2, quatOrigin2, f3PosOrigin2);

				//		//	//m = math::Matrix::Compose(f3ScaleOrigin, quatWorld, f3PosOrigin);
				//		//	
				//		//	//mat = math::Matrix::CreateFromQuaternion(quatOrigin2) * m;
				//		//	auto a = mapWorldMatrix[pParent->pBone];
				//		//	mat = math::Matrix::CreateFromQuaternion(a.first * a.second.Inverse()) * m;

				//		//	//mat = mapWorldMatrix[pParent->pBone].Invert() * m;

				//		//	//math::Matrix matInv = pParent->pBone->GetMotionOffsetTransform();
				//		//	//math::Matrix matInv = pParent->pRigidBody->GetWorldMatrix().Invert();
				//		//	//mat = matInv * mat;
				//		//	//mat = m;
				//		//}
				//		//else
				//		//{
				//		//	mat = m;
				//		//}
				//		//
				//		//pBodyPart->pBone->SetMotionTransform(mat);
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
				math::float3 f3ScaleOrigin;
				math::float3 f3PosOrigin;
				math::Quaternion quatOrigin;
				pBodyPart->matBodyOrigin.Decompose(f3ScaleOrigin, quatOrigin, f3PosOrigin);

				math::float3 f3Pos;
				math::Quaternion quat;
				if (pBodyPart->isChildBone == true)
				{
					auto pChildBone = pBodyPart->pBone;
					auto pParentBone = pBodyPart->pBone->GetParent();

					math::float3 f3ScaleChild;
					math::float3 f3PosChild;
					math::Quaternion quatChild;
					pChildBone->GetGlobalMatrix().Decompose(f3ScaleChild, quatChild, f3PosChild);

					math::float3 f3ScaleParent;
					math::float3 f3PosParent;
					math::Quaternion quatParent;
					pParentBone->GetGlobalMatrix().Decompose(f3ScaleParent, quatParent, f3PosParent);

					math::float3 f3Offset = (f3PosChild - f3PosParent) * 0.5f;
					f3Pos = f3PosParent + f3Offset;
					quat = quatOrigin.Inverse() * quatParent;
				}
				else
				{
					// 여기 문제없는지 확인 필요
					math::float3 f3ScaleBone;
					math::Quaternion quatBone;
					pBodyPart->pBone->GetGlobalMatrix().Decompose(f3ScaleBone, quatBone, f3Pos);
				}

				math::Matrix matWorld = math::Matrix::Compose(math::float3::One, quat, f3Pos);
				pBodyPart->pRigidBody->SetWorldMatrix(matWorld);
			}
		}
	}
}