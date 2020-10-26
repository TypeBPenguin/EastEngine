#include "stdafx.h"
#include "SkeletonController.h"

#include "Physics/RigidBody.h"

#include "DirectX/Camera.h"

#include "Renderer/RendererManager.h"
#include "Model/GeometryModel.h"
#include "Model/ModelManager.h"

#include "Input/InputInterface.h"

#include "GameObject/ActorManager.h"
#include "GameObject/ComponentModel.h"

#include "imgui.h"
#include "imguiConvertor.h"

using namespace est;

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

			physics::RigidBodyProperty rigidBodyProperty;
			rigidBodyProperty.strName.Format("%s_%d_%d", "SkeletonController", i, j);
			rigidBodyProperty.nCollisionFlag = physics::CollisionFlag::eKinematicObject;

			switch (i)
			{
			case Type::ePosition:
			{
				switch (j)
				{
				case Axis::eX:
					controller.matTransform = math::Matrix::CreateFromYawPitchRoll(0.f, 0.f, math::ToRadians(-90.f));
					break;
				case Axis::eY:
					break;
				case Axis::eZ:
					controller.matTransform = math::Matrix::CreateFromYawPitchRoll(0.f, math::ToRadians(90.f), 0.f);
					break;
				}

				std::vector<graphics::VertexPosTexNor> vertices;
				std::vector<uint32_t> indices;

				graphics::geometry::CreateCone(vertices, indices, 0.01f, 0.05f);

				const size_t vertexCount = vertices.size();

				std::vector<graphics::VertexPos> rawVertices;
				rawVertices.resize(vertexCount);
				for (size_t k = 0; k < vertexCount; ++k)
				{
					rawVertices[i].pos = vertices[i].pos;
				}

				rigidBodyProperty.shapeInfo.SetTriangleMesh(reinterpret_cast<const math::float3*>(rawVertices.data()), rawVertices.size(), indices.data(), indices.size());
				controller.pRigidBody = physics::RigidBody::Create(rigidBodyProperty);
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
					controller.matTransform = math::Matrix::CreateFromYawPitchRoll(0.f, 0.f, math::ToRadians(-90.f));
					break;
				case Axis::eY:
					break;
				case Axis::eZ:
					controller.matTransform = math::Matrix::CreateFromYawPitchRoll(0.f, math::ToRadians(90.f), 0.f);
					break;
				}

				graphics::geometry::CreateTorus(&controller.pVertexBuffer, &controller.pIndexBuffer, 0.2f, 0.003f);

				rigidBodyProperty.shapeInfo.SetTriangleMesh(reinterpret_cast<const math::float3*>(controller.pVertexBuffer->GetVertexPosPtr()), controller.pVertexBuffer->GetVertexNum(), controller.pIndexBuffer->GetRawValuePtr(), controller.pIndexBuffer->GetIndexNum());
				controller.pRigidBody = physics::RigidBody::Create(rigidBodyProperty);
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

bool SkeletonController::Process(float elapsedTime)
{
	graphics::Camera* pCamera = graphics::Camera::GetInstance();
	if (pCamera == nullptr)
		return false;

	int nThreadID = graphics::GetThreadID(graphics::eUpdate);
	const math::int2 n2MousePoint(input::mouse::GetX(), input::mouse::GetY());
	const math::uint2 n2ScreenSize = graphics::GetDevice()->GetScreenSize();
	const math::Matrix& matView = pCamera->GetViewMatrix(nThreadID);
	const math::Matrix& matProjection = pCamera->GetProjectionMatrix(nThreadID);

	const collision::Ray rayScreen = collision::Ray::CreateFromScreenCoordinates(n2MousePoint, n2ScreenSize, matView, matProjection);

	const bool isEnableRayTest = input::mouse::IsButtonDown(input::mouse::eLeft);
	float fNearDistance = std::numeric_limits<float>::max();
	graphics::ISkeletonInstance::IBone* pNearBone = nullptr;
	bool isExistsCollisionBone = false;

	const size_t nActorCount = gameobject::ActorManager::GetInstance()->GetActorCount();
	for (size_t i = 0; i < nActorCount; ++i)
	{
		gameobject::IActor* pActor = gameobject::ActorManager::GetInstance()->GetActor(i);
		gameobject::ComponentModel* pCompModel = static_cast<gameobject::ComponentModel*>(pActor->GetComponent(gameobject::IComponent::eModel));
		if (pCompModel != nullptr && pCompModel->IsLoadComplete() == true)
		{
			graphics::IModelInstance* pModelInstance = pCompModel->GetModelInstance();
			graphics::ISkeletonInstance* pSkeletonInstance = pModelInstance->GetSkeleton();
			if (pSkeletonInstance != nullptr)
			{
				const size_t nBoneCount = pSkeletonInstance->GetBoneCount();
				for (size_t j = 0; j < nBoneCount; ++j)
				{
					graphics::ISkeletonInstance::IBone* pBone = pSkeletonInstance->GetBone(j);
					if (pBone != nullptr)
					{
						math::float3 f3Scale;
						math::float3 f3Pos;
						math::Quaternion quatRot;
						pBone->GetGlobalMatrix().Decompose(f3Scale, quatRot, f3Pos);

						const collision::OBB obb(f3Pos, math::float3(0.02f), quatRot);

						if (isEnableRayTest == true)
						{
							float fDist = 0.f;
							if (rayScreen.Intersects(obb, fDist) == true)
							{
								if (fDist > 0.5f)
								{
									if (fNearDistance > fDist)
									{
										pNearBone = pBone;
										fNearDistance = fDist;
									}
									isExistsCollisionBone = true;
								}
							}
						}

						if (Config::IsEnable("VisibleSkeleton"_s))
						{
							graphics::RenderSubsetVertex aabb;
							aabb.matWorld = math::Matrix::CreateScale(0.02f) * pBone->GetGlobalMatrix();
							aabb.isWireframe = true;
							aabb.isIgnoreDepth = true;
							graphics::geometry::GetDebugModel(graphics::geometry::EmDebugModel::eBox, &aabb.pVertexBuffer, &aabb.pIndexBuffer);
							graphics::RendererManager::GetInstance()->AddRender(aabb);

							if (pBone->GetParent() != nullptr)
							{
								const math::float3* pStartPos = reinterpret_cast<const math::float3*>(&pBone->GetParent()->GetGlobalMatrix()._41);
								const math::float3* pEndPos = reinterpret_cast<const math::float3*>(&pBone->GetGlobalMatrix()._41);
								graphics::RenderSubsetLineSegment line(*pStartPos, math::Color::Blue, *pEndPos, math::Color::Blue, true);
								graphics::RendererManager::GetInstance()->AddRender(line);
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
		if (m_pSelectedBone != nullptr && input::mouse::IsButtonDown(input::mouse::eMiddle) == true)
		{
			math::float3 f3Scale;
			math::float3 f3Pos;
			math::Quaternion quatRot;
			m_pSelectedBone->GetGlobalMatrix().Decompose(f3Scale, quatRot, f3Pos);

			const collision::OBB obb(f3Pos, math::float3(0.02f), quatRot);

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
		else if (input::mouse::IsButtonUp(input::mouse::eMiddle) == true)
		{
			m_isBoneMoveMode = false;
		}

		if (m_isBoneMoveMode == true)
		{
			math::Matrix matInvView = matView;
			math::float3 sideward = matInvView.Right() * fDist * 0.001f * static_cast<float>(input::mouse::GetMoveX());
			math::float3 upward = matInvView.Up() * fDist * 0.001f * -static_cast<float>(input::mouse::GetMoveY());

			//math::float3 f3Position = m_pSelectedBone->GetUserOffsetPosition() + sideward + upward;
			//m_pSelectedBone->SetUserOffsetPosition(f3Position);

			isExistsCollisionBone = true;
		}
	}

	if (m_pSelectedBone != nullptr)
	{
		graphics::RenderSubsetVertex aabb;
		aabb.matWorld = math::Matrix::CreateScale(0.02f) * m_pSelectedBone->GetGlobalMatrix();
		aabb.isWireframe = true;
		aabb.isIgnoreDepth = true;
		aabb.color = math::Color::LightGreen;
		graphics::geometry::GetDebugModel(graphics::geometry::EmDebugModel::eBox, &aabb.pVertexBuffer, &aabb.pIndexBuffer);
		graphics::RendererManager::GetInstance()->AddRender(aabb);

		for (int i = 0; i < Type::TypeCount; ++i)
		{
			for (int j = 0; j < Axis::AxisCount; ++j)
			{
				Controller& controller = m_controllers[j][i];

				graphics::RenderSubsetVertex renderSubsetController;
				renderSubsetController.isIgnoreDepth = true;
				renderSubsetController.pVertexBuffer = controller.pVertexBuffer;
				renderSubsetController.pIndexBuffer = controller.pIndexBuffer;

				const math::Color ControllerColor[] = 
				{
					math::Color::Red,
					math::Color::Green,
					math::Color::Blue,
				};

				const math::Color SelectedControllerColor[] =
				{
					math::Color::Orange,
					math::Color::LightGreen,
					math::Color::LightBlue,
				};
				renderSubsetController.color = ControllerColor[j];

				switch (i)
				{
				case Type::ePosition:
				{
					math::float3 f3StartPos = m_pSelectedBone->GetGlobalMatrix().Translation();
					math::float3 f3EndPos(0.f, 0.08f, 0.f);
					f3EndPos = math::float3::TransformNormal(f3EndPos, controller.matTransform * m_pSelectedBone->GetGlobalMatrix());

					math::Matrix matWorld = controller.matTransform * m_pSelectedBone->GetGlobalMatrix() * math::Matrix::CreateTranslation(f3EndPos);
					if (controller.pRigidBody != nullptr)
					{
						controller.pRigidBody->SetWorldMatrix(matWorld);
						if (controller.pRigidBody->RayTest(rayScreen.position, rayScreen.direction * 100.f))
						{
							renderSubsetController.color = SelectedControllerColor[j];
						}
					}

					renderSubsetController.matWorld = matWorld;
					graphics::RendererManager::GetInstance()->AddRender(renderSubsetController);

					f3EndPos += f3StartPos;

					graphics::RenderSubsetLineSegment line(f3StartPos, renderSubsetController.color, f3EndPos, renderSubsetController.color, true);
					graphics::RendererManager::GetInstance()->AddRender(line);
				}
				break;
				case Type::eScale:
				{
				}
				break;
				case Type::eRotation:
				{
					math::Matrix matWorld = controller.matTransform * math::Matrix::CreateTranslation(m_pSelectedBone->GetGlobalMatrix().Translation());
					if (controller.pRigidBody != nullptr)
					{
						controller.pRigidBody->SetWorldMatrix(matWorld);
						if (controller.pRigidBody->RayTest(rayScreen.position, rayScreen.direction * 100.f))
						{
							renderSubsetController.color = SelectedControllerColor[j];
						}
					}

					renderSubsetController.matWorld = matWorld;
					graphics::RendererManager::GetInstance()->AddRender(renderSubsetController);
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
			//math::float3 f3Scale = m_pSelectedBone->GetUserOffsetScale();
			//math::float3 f3Rotation = m_pSelectedBone->GetUserOffsetRotation();
			//math::float3 f3Position = m_pSelectedBone->GetUserOffsetPosition();

			//f3Rotation.x = math::ToDegrees(f3Rotation.x);
			//f3Rotation.y = math::ToDegrees(f3Rotation.y);
			//f3Rotation.z = math::ToDegrees(f3Rotation.z);

			//static float values[7] = { 0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f };
			//ImGui::PushID("SelectedBone");

			//ImGui::Text("%s", m_pSelectedBone->GetName().c_str());

			//ImGui::BeginGroup();
			//ImGui::PushID("Scale");
			//ImGui::Text("Scale");
			//ImGui::DragFloat("x", &f3Scale.x, 0.01f, 0.f, 100.f);
			//ImGui::DragFloat("y", &f3Scale.y, 0.01f, 0.f, 100.f);
			//ImGui::DragFloat("z", &f3Scale.z, 0.01f, 0.f, 100.f);
			//ImGui::PopID();
			//ImGui::EndGroup();

			//ImGui::BeginGroup();
			//ImGui::PushID("Position");
			//ImGui::Text("Position");
			//ImGui::DragFloat("x", &f3Position.x, 0.01f, -1000.f, 1000.f);
			//ImGui::DragFloat("y", &f3Position.y, 0.01f, -1000.f, 1000.f);
			//ImGui::DragFloat("z", &f3Position.z, 0.01f, -1000.f, 1000.f);
			//ImGui::PopID();
			//ImGui::EndGroup();

			//ImGui::BeginGroup();
			//ImGui::PushID("Rotation");
			//ImGui::Text("Rotation");
			////ImGui::DragFloat("x", &f3Rotation.x, 0.001f, -math::PI, math::PI, "%.3f");
			////ImGui::DragFloat("y", &f3Rotation.y, 0.001f, -math::PI, math::PI, "%.3f");
			////ImGui::DragFloat("z", &f3Rotation.z, 0.001f, -math::PI, math::PI, "%.3f");
			//ImGui::DragFloat("x", &f3Rotation.x, 0.1f, -360.f, 360.f, "%.3f");
			//ImGui::DragFloat("y", &f3Rotation.y, 0.1f, -360.f, 360.f, "%.3f");
			//ImGui::DragFloat("z", &f3Rotation.z, 0.1f, -360.f, 360.f, "%.3f");
			//ImGui::PopID();
			//ImGui::EndGroup();

			//ImGui::PopID();

			//bool isReset = ImGui::Button("Reset");

			//ImGui::End();

			//if (isReset == true)
			//{
			//	m_pSelectedBone->SetUserOffsetScale(math::float3::One);
			//	m_pSelectedBone->SetUserOffsetRotation(math::float3::Zero);
			//	m_pSelectedBone->SetUserOffsetPosition(math::float3::Zero);
			//}
			//else
			//{
			//	f3Rotation.x = math::ToRadians(f3Rotation.x);
			//	f3Rotation.y = math::ToRadians(f3Rotation.y);
			//	f3Rotation.z = math::ToRadians(f3Rotation.z);

			//	m_pSelectedBone->SetUserOffsetScale(f3Scale);
			//	m_pSelectedBone->SetUserOffsetRotation(f3Rotation);
			//	m_pSelectedBone->SetUserOffsetPosition(f3Position);
			//}
		}
		break;
		case 1:
		{
			//math::Transform transform = m_pSelectedBone->GetMotionMatrix();
		}
		break;
		}
	}
}