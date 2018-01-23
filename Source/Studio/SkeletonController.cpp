#include "stdafx.h"
#include "SkeletonController.h"

#include "CommonLib/Config.h"

#include "Physics/RigidBody.h"

#include "DirectX/CameraManager.h"

#include "Renderer/RendererManager.h"
#include "Model/GeometryModel.h"
#include "Model/ModelManager.h"

#include "Input/InputInterface.h"

#include "GameObject/ActorManager.h"
#include "GameObject/ComponentModel.h"

#include "imgui.h"
#include "imguiConvertor.h"

using namespace EastEngine;

SkeletonController::SkeletonController()
	: m_pSelectedBone(nullptr)
	, m_isBoneMoveMode(false)
	, m_isShowControllerUI(false)
{
	for (int i = 0; i < Type::TypeCount; ++i)
	{
		for (int j = 0; j < Axis::AxisCount; ++j)
		{
			Controller& controller = m_controllers[j][i];

			Physics::RigidBodyProperty rigidBodyProperty;
			rigidBodyProperty.strName.Format("%s_%d_%d", "SkeletonController", i, j);
			rigidBodyProperty.nCollisionFlag = Physics::EmCollision::eKinematicObject;

			switch (i)
			{
			case Type::ePosition:
			{
				switch (j)
				{
				case Axis::eX:
					controller.matTransform = Math::Matrix::CreateFromYawPitchRoll(0.f, 0.f, Math::ToRadians(-90.f));
					break;
				case Axis::eY:
					break;
				case Axis::eZ:
					controller.matTransform = Math::Matrix::CreateFromYawPitchRoll(0.f, Math::ToRadians(90.f), 0.f);
					break;
				}

				Graphics::GeometryModel::CreateCone(&controller.pVertexBuffer, &controller.pIndexBuffer, 0.01f, 0.05f);

				rigidBodyProperty.shapeInfo.SetTriangleMesh(reinterpret_cast<const Math::Vector3*>(controller.pVertexBuffer->GetVertexPosPtr()), controller.pVertexBuffer->GetVertexNum(), controller.pIndexBuffer->GetRawValuePtr(), controller.pIndexBuffer->GetIndexNum());
				controller.pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);
			}
			break;
			case Type::eScale:
			{
			}
			break;
			case Type::eRotation:
			{
				switch (j)
				{
				case Axis::eX:
					controller.matTransform = Math::Matrix::CreateFromYawPitchRoll(0.f, 0.f, Math::ToRadians(-90.f));
					break;
				case Axis::eY:
					break;
				case Axis::eZ:
					controller.matTransform = Math::Matrix::CreateFromYawPitchRoll(0.f, Math::ToRadians(90.f), 0.f);
					break;
				}

				Graphics::GeometryModel::CreateTorus(&controller.pVertexBuffer, &controller.pIndexBuffer, 0.2f, 0.003f);

				rigidBodyProperty.shapeInfo.SetTriangleMesh(reinterpret_cast<const Math::Vector3*>(controller.pVertexBuffer->GetVertexPosPtr()), controller.pVertexBuffer->GetVertexNum(), controller.pIndexBuffer->GetRawValuePtr(), controller.pIndexBuffer->GetIndexNum());
				controller.pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);
			}
			break;
			}
		}
	}
}

SkeletonController::~SkeletonController()
{
	for (int i = 0; i < Type::TypeCount; ++i)
	{
		for (int j = 0; j < Axis::AxisCount; ++j)
		{
			Controller& controller = m_controllers[j][i];
			SafeDelete(controller.pVertexBuffer);
			SafeDelete(controller.pIndexBuffer);
			SafeDelete(controller.pRigidBody);
		}
	}
}

bool SkeletonController::Process(float fElapsedTime)
{
	Graphics::Camera* pCamera = Graphics::CameraManager::GetInstance()->GetMainCamera();
	if (pCamera == nullptr)
		return false;

	const Math::Int2 n2MousePoint(Input::Mouse::GetX(), Input::Mouse::GetY());
	const Math::UInt2 n2ScreenSize = Graphics::GetDevice()->GetScreenSize();
	const Math::Matrix& matView = pCamera->GetViewMatrix();
	const Math::Matrix& matProjection = pCamera->GetProjMatrix();

	const Collision::Ray rayScreen = Collision::Ray::CreateFromScreenCoordinates(n2MousePoint, n2ScreenSize, matView, matProjection);

	const bool isEnableRayTest = Input::Mouse::IsButtonDown(Input::Mouse::eLeft);
	float fNearDistance = std::numeric_limits<float>::max();
	Graphics::ISkeletonInstance::IBone* pNearBone = nullptr;
	bool isExistsCollisionBone = false;

	const size_t nActorCount = GameObject::ActorManager::GetInstance()->GetActorCount();
	for (size_t i = 0; i < nActorCount; ++i)
	{
		GameObject::IActor* pActor = GameObject::ActorManager::GetInstance()->GetActor(i);
		GameObject::ComponentModel* pCompModel = static_cast<GameObject::ComponentModel*>(pActor->GetComponent(GameObject::EmComponent::eModel));
		if (pCompModel != nullptr && pCompModel->IsLoadComplete() == true)
		{
			Graphics::IModelInstance* pModelInstance = pCompModel->GetModelInstance();
			Graphics::ISkeletonInstance* pSkeletonInstance = pModelInstance->GetSkeleton();
			if (pSkeletonInstance != nullptr)
			{
				const size_t nBoneCount = pSkeletonInstance->GetBoneCount();
				for (size_t j = 0; j < nBoneCount; ++j)
				{
					Graphics::ISkeletonInstance::IBone* pBone = pSkeletonInstance->GetBone(j);
					if (pBone != nullptr)
					{
						Math::Vector3 f3Scale;
						Math::Vector3 f3Pos;
						Math::Quaternion quatRot;
						pBone->GetGlobalMatrix().Decompose(f3Scale, quatRot, f3Pos);

						const Collision::OBB obb(f3Pos, Math::Vector3(0.02f), quatRot);

						if (isEnableRayTest == true)
						{
							float fDist = 0.f;
							if (rayScreen.Intersects(obb, fDist) == true)
							{
								if (fNearDistance > fDist)
								{
									pNearBone = pBone;
									fNearDistance = fDist;
								}
								isExistsCollisionBone = true;
							}
						}

						if (Config::IsEnable("VisibleSkeleton"_s))
						{
							Graphics::RenderSubsetVertex aabb;
							aabb.matWorld = Math::Matrix::CreateScale(0.02f) * pBone->GetGlobalMatrix();
							aabb.isWireframe = true;
							aabb.isIgnoreDepth = true;
							Graphics::GeometryModel::GetDebugModel(Graphics::GeometryModel::EmDebugModel::eBox, &aabb.pVertexBuffer, &aabb.pIndexBuffer);
							Graphics::RendererManager::GetInstance()->AddRender(aabb);

							if (pBone->GetParent()->IsRootBone() == false)
							{
								const Math::Vector3* pStartPos = reinterpret_cast<const Math::Vector3*>(&pBone->GetParent()->GetGlobalMatrix()._41);
								const Math::Vector3* pEndPos = reinterpret_cast<const Math::Vector3*>(&pBone->GetGlobalMatrix()._41);
								Graphics::RenderSubsetLineSegment line(*pStartPos, Math::Color::Blue, *pEndPos, Math::Color::Blue, true);
								Graphics::RendererManager::GetInstance()->AddRender(line);
							}
						}
					}
				}
			}
		}
	}

	if (isEnableRayTest == true && pNearBone != nullptr)
	{
		m_isShowControllerUI = true;
		m_pSelectedBone = pNearBone;
	}
	else
	{
		static float fDist = 0.f;
		if (m_pSelectedBone != nullptr && Input::Mouse::IsButtonDown(Input::Mouse::eMiddle) == true)
		{
			Math::Vector3 f3Scale;
			Math::Vector3 f3Pos;
			Math::Quaternion quatRot;
			m_pSelectedBone->GetGlobalMatrix().Decompose(f3Scale, quatRot, f3Pos);

			const Collision::OBB obb(f3Pos, Math::Vector3(0.02f), quatRot);

			fDist = 0.f;
			if (rayScreen.Intersects(obb, fDist) == true)
			{
				m_isBoneMoveMode = true;
			}
			else
			{
				m_isBoneMoveMode = false;
			}
		}
		else if (Input::Mouse::IsButtonUp(Input::Mouse::eMiddle) == true)
		{
			m_isBoneMoveMode = false;
		}

		if (m_isBoneMoveMode == true)
		{
			Math::Matrix matInvView = matView;
			Math::Vector3 sideward = matInvView.Right() * fDist * 0.001f * static_cast<float>(Input::Mouse::GetMoveX());
			Math::Vector3 upward = matInvView.Up() * fDist * 0.001f * -static_cast<float>(Input::Mouse::GetMoveY());

			Math::Vector3 f3Position = m_pSelectedBone->GetUserOffsetPosition() + sideward + upward;
			m_pSelectedBone->SetUserOffsetPosition(f3Position);

			isExistsCollisionBone = true;
		}
	}

	if (m_pSelectedBone != nullptr)
	{
		Graphics::RenderSubsetVertex aabb;
		aabb.matWorld = Math::Matrix::CreateScale(0.02f) * m_pSelectedBone->GetGlobalMatrix();
		aabb.isWireframe = true;
		aabb.isIgnoreDepth = true;
		aabb.color = Math::Color::LightGreen;
		Graphics::GeometryModel::GetDebugModel(Graphics::GeometryModel::EmDebugModel::eBox, &aabb.pVertexBuffer, &aabb.pIndexBuffer);
		Graphics::RendererManager::GetInstance()->AddRender(aabb);

		for (int i = 0; i < Type::TypeCount; ++i)
		{
			for (int j = 0; j < Axis::AxisCount; ++j)
			{
				Controller& controller = m_controllers[j][i];

				Graphics::RenderSubsetVertex renderSubsetController;
				renderSubsetController.isIgnoreDepth = true;
				renderSubsetController.pVertexBuffer = controller.pVertexBuffer;
				renderSubsetController.pIndexBuffer = controller.pIndexBuffer;

				const Math::Color ControllerColor[] = 
				{
					Math::Color::Red,
					Math::Color::Green,
					Math::Color::Blue,
				};

				const Math::Color SelectedControllerColor[] =
				{
					Math::Color::Orange,
					Math::Color::LightGreen,
					Math::Color::LightBlue,
				};
				renderSubsetController.color = ControllerColor[j];

				switch (i)
				{
				case Type::ePosition:
				{
					Math::Vector3 f3StartPos = m_pSelectedBone->GetGlobalMatrix().Translation();
					Math::Vector3 f3EndPos(0.f, 0.08f, 0.f);
					f3EndPos = Math::Vector3::TransformNormal(f3EndPos, controller.matTransform * m_pSelectedBone->GetGlobalMatrix());

					Math::Matrix matWorld = controller.matTransform * m_pSelectedBone->GetGlobalMatrix() * Math::Matrix::CreateTranslation(f3EndPos);
					if (controller.pRigidBody != nullptr)
					{
						controller.pRigidBody->SetWorldMatrix(matWorld);
						if (controller.pRigidBody->RayTest(rayScreen.position, rayScreen.direction * 100.f))
						{
							renderSubsetController.color = SelectedControllerColor[j];
						}
					}

					renderSubsetController.matWorld = matWorld;
					Graphics::RendererManager::GetInstance()->AddRender(renderSubsetController);

					f3EndPos += f3StartPos;

					Graphics::RenderSubsetLineSegment line(f3StartPos, renderSubsetController.color, f3EndPos, renderSubsetController.color, true);
					Graphics::RendererManager::GetInstance()->AddRender(line);
				}
				break;
				case Type::eScale:
				{
				}
				break;
				case Type::eRotation:
				{
					Math::Matrix matWorld = controller.matTransform * Math::Matrix::CreateTranslation(m_pSelectedBone->GetGlobalMatrix().Translation());
					if (controller.pRigidBody != nullptr)
					{
						controller.pRigidBody->SetWorldMatrix(matWorld);
						if (controller.pRigidBody->RayTest(rayScreen.position, rayScreen.direction * 100.f))
						{
							renderSubsetController.color = SelectedControllerColor[j];
						}
					}

					renderSubsetController.matWorld = matWorld;
					Graphics::RendererManager::GetInstance()->AddRender(renderSubsetController);
				}
				break;
				}
			}
		}
	}

	return isExistsCollisionBone;
}

void SkeletonController::RenderUI()
{
	if (m_pSelectedBone != nullptr)
	{
		ImGui::Begin("Skeleton Controller", &m_isShowControllerUI);

		const char* BoneType[] = 
		{
			"UserOffset",
			"MotionData",
		};
		static int nType = 0;
		ImGui::Combo("Type", &nType, BoneType, ARRAYSIZE(BoneType));

		switch (nType)
		{
		case 0:
		{
			Math::Vector3 f3Scale = m_pSelectedBone->GetUserOffsetScale();
			Math::Vector3 f3Rotation = m_pSelectedBone->GetUserOffsetRotation();
			Math::Vector3 f3Position = m_pSelectedBone->GetUserOffsetPosition();

			f3Rotation.x = Math::ToDegrees(f3Rotation.x);
			f3Rotation.y = Math::ToDegrees(f3Rotation.y);
			f3Rotation.z = Math::ToDegrees(f3Rotation.z);

			static float values[7] = { 0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f };
			ImGui::PushID("SelectedBone");

			ImGui::Text("%s", m_pSelectedBone->GetName().c_str());

			ImGui::BeginGroup();
			ImGui::PushID("Scale");
			ImGui::Text("Scale");
			ImGui::DragFloat("x", &f3Scale.x, 0.01f, 0.f, 100.f);
			ImGui::DragFloat("y", &f3Scale.y, 0.01f, 0.f, 100.f);
			ImGui::DragFloat("z", &f3Scale.z, 0.01f, 0.f, 100.f);
			ImGui::PopID();
			ImGui::EndGroup();

			ImGui::BeginGroup();
			ImGui::PushID("Position");
			ImGui::Text("Position");
			ImGui::DragFloat("x", &f3Position.x, 0.01f, -1000.f, 1000.f);
			ImGui::DragFloat("y", &f3Position.y, 0.01f, -1000.f, 1000.f);
			ImGui::DragFloat("z", &f3Position.z, 0.01f, -1000.f, 1000.f);
			ImGui::PopID();
			ImGui::EndGroup();

			ImGui::BeginGroup();
			ImGui::PushID("Rotation");
			ImGui::Text("Rotation");
			//ImGui::DragFloat("x", &f3Rotation.x, 0.001f, -Math::PI, Math::PI, "%.3f");
			//ImGui::DragFloat("y", &f3Rotation.y, 0.001f, -Math::PI, Math::PI, "%.3f");
			//ImGui::DragFloat("z", &f3Rotation.z, 0.001f, -Math::PI, Math::PI, "%.3f");
			ImGui::DragFloat("x", &f3Rotation.x, 0.1f, -360.f, 360.f, "%.3f");
			ImGui::DragFloat("y", &f3Rotation.y, 0.1f, -360.f, 360.f, "%.3f");
			ImGui::DragFloat("z", &f3Rotation.z, 0.1f, -360.f, 360.f, "%.3f");
			ImGui::PopID();
			ImGui::EndGroup();

			ImGui::PopID();

			bool isReset = ImGui::Button("Reset");

			ImGui::End();

			if (isReset == true)
			{
				m_pSelectedBone->SetUserOffsetScale(Math::Vector3::One);
				m_pSelectedBone->SetUserOffsetRotation(Math::Vector3::Zero);
				m_pSelectedBone->SetUserOffsetPosition(Math::Vector3::Zero);
			}
			else
			{
				f3Rotation.x = Math::ToRadians(f3Rotation.x);
				f3Rotation.y = Math::ToRadians(f3Rotation.y);
				f3Rotation.z = Math::ToRadians(f3Rotation.z);

				m_pSelectedBone->SetUserOffsetScale(f3Scale);
				m_pSelectedBone->SetUserOffsetRotation(f3Rotation);
				m_pSelectedBone->SetUserOffsetPosition(f3Position);
			}
		}
		break;
		case 1:
		{
			Math::Transform transform = m_pSelectedBone->GetMotionTransform();
		}
		break;
		}
	}
}