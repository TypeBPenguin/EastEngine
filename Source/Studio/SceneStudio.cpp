#include "stdafx.h"
#include "SceneStudio.h"

#include "CommonLib/BehaviorTree.h"
#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"
#include "Graphics/Interface/imguiHelper.h"

#include "Graphics/Model/ModelInterface.h"
#include "Graphics/Model/GeometryModel.h"
#include "Graphics/Model/ModelManager.h"

#include "Input/InputInterface.h"

#include "GameObject/ComponentModel.h"
#include "GameObject/ComponentPhysics.h"

#include "SoundSystem/SoundInterface.h"

#include "Minion.h"
#include "NodeGraphEditor.h"

using namespace est;

namespace sid
{
	RegisterStringID(Studio);
	RegisterStringID(Studio_Ground);
}

const char* IBL_Type[] =
{
	"IceLake",
	"Milkyway",
	"SnowMachine",
	"TropicalBeach",
	"WinterForest",
	"Alexs_Apartment",
	"Chiricahua_Plaza",
	"GrandCanyon_C_YumaPoint",
	"Popcorn_Lobby",
	"Stadium_Center",
	"Summi_Pool",
	"Tokyo_BigSight",
};

SceneStudio::SceneStudio()
	: IScene(sid::Studio)
{
}

SceneStudio::~SceneStudio()
{
}

const est::string::StringID SceneStudio::Name{ sid::Studio };

void TestBehaviorTree()
{
	struct Bear
	{
		int money{ 0 };
		int hungry{ 0 };
		int sleepy{ 0 };
		int food{ 0 };

		void Work() { ++hungry; ++sleepy; money += 2; }
		void Sleep() { hungry += 1; sleepy -= 2; }
		void Eat() { --food; hungry -= 5; }

		bool IsWakeup() const { return sleepy <= 0; }
		bool IsSleepy() const { return sleepy > 10; }
		bool IsHungry() const { return hungry > 5; }
		bool IsHasFood() const { return food > 0; }

		bool CanIBuyFood(int needMoney) const { return needMoney < money; }
		void BuyFood(int needMoney) { money -= needMoney; ++food; }
	};

	Bear bear;

	BehaviorTree bTree;
	BehaviorTree::IDecoratorNode* pRoot = bTree.GetRoot();
	auto pCoolDown = pRoot->SetCooldown(L"CoolDown", 3.f);
	auto pMain = pCoolDown->SetSequence(L"Main");

	auto pRandomSelector = pMain->AddRandomSelector(L"RandomSelector");
	pMain->AddAction(L"Day", [&]()
	{
		LOG_ERROR(L"Day Result : nmoney[%d], hungry[%d], sleepy[%d], food[%d]", bear.money, bear.hungry, bear.sleepy, bear.food);
		return true;
	});

	{
		auto pEatSomeFood = pRandomSelector->AddSequence(L"EatSomeFood");
		pEatSomeFood->AddAction(L"CheckHungry", [&]()
		{
			if (bear.IsHungry() == true)
			{
				LOG_MESSAGE(L"곰은 배가 고파요");
				return true;
			}
			else
			{
				LOG_WARNING(L"곰은 배고프지 않아요");
				return false;
			}
		});

		auto pCheckFood = pEatSomeFood->AddSelector(L"CheckFood");
		pCheckFood->AddAction(L"CheckFood", [&]()
		{
			if (bear.IsHasFood() == true)
			{
				LOG_MESSAGE(L"곰은 음식을 가지고 있어요");
				return true;
			}
			else
			{
				LOG_WARNING(L"곰은 음식이 없어요");
				return false;
			}
		});

		auto pBuyFood = pCheckFood->AddSequence(L"BuyFood");
		pBuyFood->AddAction(L"CheckMoney", [&]()
		{
			if (bear.CanIBuyFood(5) == true)
			{
				LOG_MESSAGE(L"곰은 돈이 있어요");
				return true;
			}
			else
			{
				LOG_WARNING(L"곰은 돈이 없어요");
				return false;
			}
		});
		pBuyFood->AddAction(L"BuyFood", [&]()
		{
			LOG_MESSAGE(L"곰은 음식을 샀어요");
			bear.BuyFood(5);
			return true;
		});

		pEatSomeFood->AddAction(L"EatFood", [&]()
		{
			LOG_MESSAGE(L"곰은 음식을 먹었어요");
			bear.Eat();
			return true;
		});
	}
	{
		auto pSleep = pRandomSelector->AddSequence(L"Sleep");
		pSleep->AddAction(L"IsSleep", [&]()
		{
			if (bear.IsSleepy() == true)
			{
				LOG_MESSAGE(L"곰은 졸려요");
				return true;
			}
			else
			{
				LOG_WARNING(L"곰은 졸리지 않아요");
				return false;
			}
		});

		auto pSleeping = pSleep->AddConditionalLoop(L"SleepingLoop");
		pSleeping->SetAction(L"Sleeping", [&]()
		{
			if (bear.IsWakeup() == false)
			{
				LOG_MESSAGE(L"곰은 잠자고 있어요");
				bear.Sleep();
				return true;
			}
			else
			{
				LOG_WARNING(L"곰은 잠에서 깨어났어요");
				return false;
			}
		});
	}
	{
		auto pWork = pRandomSelector->AddSequence(L"Work");
		pWork->AddAction(L"CanIWork", [&]()
		{
			if (bear.IsSleepy() == false)
			{
				LOG_MESSAGE(L"곰은 일 할수 있어요");
				return true;
			}
			else
			{
				LOG_MESSAGE(L"곰은 졸려서 일 할수 없어요");
				return false;
			}
		});

		pWork->AddAction(L"Work", [&]()
		{
			LOG_MESSAGE(L"곰은 일을 했어요");
			bear.Work();
			return true;
		});
	}

	while (1)
	{
		const float elapsedTime = 0.016f;

		bTree.Run(elapsedTime);

		Sleep(16);
	}
}

void SceneStudio::Enter(const std::queue<gameobject::ActorPtr>& savedPrevSceneActors)
{
	graphics::Camera& camera = graphics::GetCamera();

	graphics::Camera::DescView cameraView;
	cameraView.position = { 0.f, 10.f, -10.f };
	cameraView.lookat = { 0.f, 0.f, 0.f };
	cameraView.up = math::float3::Up;
	camera.SetView(cameraView);

	graphics::FirstPersonCameraMan::DescMove descMove;
	camera.SetFirstPersonCameraMan(descMove);

	//graphics::ThirdPersonCameraMan::DescMove descMove;
	//camera.SetThirdPersonCameraMan(descMove);

	{
		const math::float3 f3LightPosition(0.f, 500.f, -500.f);

		graphics::DirectionalLightData lightData;
		lightData.direction = math::float3::Zero - f3LightPosition;
		lightData.direction.Normalize();
		graphics::DirectionalLightPtr pDirectionalLight = graphics::CreateDirectionalLight(L"MainLight", false, lightData);
		pDirectionalLight->SetEnableShadow(true);
		m_pLights.emplace_back(pDirectionalLight);

		graphics::PointLightData pointLightData;
		pointLightData.lightIntensity = 1000.f;
		pointLightData.position.y = 10.f;
		pointLightData.color = math::float3(1.f, 0.f, 0.f);
		m_pLights.emplace_back(graphics::CreatePointLight(L"PointLight", false, pointLightData));

		graphics::SpotLightData spotLightData;
		spotLightData.lightIntensity = 1000.f;
		spotLightData.position.x = -10.f;
		spotLightData.position.y = 10.f;
		spotLightData.direction = math::float3(0.f, -1.f, 0.f);
		spotLightData.angle = 30.f;
		spotLightData.color = math::float3(0.f, 1.f, 0.f);
		m_pLights.emplace_back(graphics::CreateSpotLight(L"SpotLight", false, spotLightData));
	}

	std::wstring path;
	//{
	//	path = string::Format(L"%sTexture\\IBL\\%s\\%s", file::GetEngineDataPath(), IBL_Type[1], IBL_Type[1]);
	//
	//	graphics::IImageBasedLight* pImageBasedLight = graphics::GetImageBasedLight();
	//
	//	std::wstring diffuseHDR = path;
	//	diffuseHDR.append(L"DiffuseHDR.dds");
	//	graphics::ITexture* pDiffuseHDR = graphics::CreateTextureAsync(diffuseHDR.c_str());
	//	pImageBasedLight->SetDiffuseHDR(pDiffuseHDR);
	//	graphics::ReleaseResource(&pDiffuseHDR);
	//
	//	std::wstring specularHDR = path;
	//	specularHDR.append(L"SpecularHDR.dds");
	//	graphics::ITexture* pSpecularHDR = graphics::CreateTextureAsync(specularHDR.c_str());
	//	pImageBasedLight->SetSpecularHDR(pSpecularHDR);
	//	graphics::ReleaseResource(&pSpecularHDR);
	//
	//	std::wstring specularBRDF = path;
	//	specularBRDF.append(L"Brdf.dds");
	//	graphics::ITexture* pSpecularBRDF = graphics::CreateTextureAsync(specularBRDF.c_str());
	//	pImageBasedLight->SetSpecularBRDF(pSpecularBRDF);
	//	graphics::ReleaseResource(&pSpecularBRDF);
	//
	//	std::wstring envIBLPath = path;
	//	strEnvIBLPath.append(L"EnvHDR.dds");
	//	graphics::ITexture* pEnvironmentHDR = graphics::CreateTextureAsync(strEnvIBLPath.c_str());
	//	pImageBasedLight->SetEnvironmentHDR(pEnvironmentHDR);
	//	graphics::ReleaseResource(&pEnvironmentHDR);
	//
	//	std::vector<graphics::VertexPosTexNor> vertices;
	//	std::vector<uint32_t> indices;
	//
	//	graphics::geometry::CreateSphere(vertices, indices, 1.f, 32u);
	//
	//	graphics::IVertexBuffer* pVertexBuffer = graphics::CreateVertexBuffer(reinterpret_cast<const uint8_t*>(vertices.data()), static_cast<uint32_t>(vertices.size()), sizeof(graphics::VertexPosTexNor), false);
	//	graphics::IIndexBuffer* pIndexBuffer = graphics::CreateIndexBuffer(reinterpret_cast<const uint8_t*>(indices.data()), static_cast<uint32_t>(indices.size()), sizeof(uint32_t), false);
	//
	//	pImageBasedLight->SetEnvironmentSphere(pVertexBuffer, pIndexBuffer);
	//
	//	graphics::ReleaseResource(&pVertexBuffer);
	//	graphics::ReleaseResource(&pIndexBuffer);
	//}

	{
		gameobject::ActorPtr pActor = gameobject::CreateActor(sid::Studio_Ground);

		graphics::IMaterial::Data material;
		material.name = sid::Studio_Ground;

		graphics::ModelLoader loader;
		loader.InitPlane(sid::Studio_Ground, 1.f, 1.f, 100, 100, &material);
		loader.SetEnableThreadLoad(false);

		auto pCompModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::IComponent::eModel));
		pCompModel->Add(0, loader);
		graphics::IModelInstance* pModelInstance = pCompModel->GetModelInstance(0);

		const graphics::VertexPos* pVertices = nullptr;
		size_t numVertices = 0;
		pModelInstance->GetModel()->GetNode(0)->GetRawVertices(&pVertices, numVertices);

		const uint32_t* pIndices = nullptr;
		size_t numIndices = 0;
		pModelInstance->GetModel()->GetNode(0)->GetRawIndices(&pIndices, numIndices);

		auto pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::IComponent::ePhysics));
		physics::RigidActorProperty rigidProperty;
		rigidProperty.material.restitution = 0.75f;
		rigidProperty.material.staticFriction = 0.25f;
		rigidProperty.material.dynamicFriction = 0.25f;
		rigidProperty.shape.SetTriangleMeshGeometry(math::float3::One, math::Quaternion::Identity, reinterpret_cast<const math::float3*>(pVertices), static_cast<uint32_t>(numVertices), pIndices, static_cast<uint32_t>(numIndices), physics::IGeometry::MeshFlag::eNone);
		rigidProperty.shape.flags = physics::IShape::eSimulationShape | physics::IShape::eSceneQueryShape;
		rigidProperty.rigidAcotr.name = "Plane";
		rigidProperty.rigidAcotr.type = physics::IActor::eRigidStatic;
		pCompPhysics->CreateRigidActor(rigidProperty);

		//auto pModelInst = pCompModel->GetModelInstance();

		//auto pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::IComponent::ePhysics));
		//
		//physics::RigidBodyProperty prop;
		//prop.fRestitution = 0.75f;
		//prop.name = sid::Studio_Ground;
		//prop.fMass = 0.f;
		//prop.nCollisionFlag = physics::CollisionFlag::eStaticObject;
		//prop.shapeInfo.SetTriangleMesh();
		//pCompPhysics->Initialize(pModelInst, prop);

		/*gameobject::SectorInitInfo sectorInitInfo;
		sectorInitInfo.fRadius = 10.f;
		for (auto& direction : sectorInitInfo.nSectorsCount)
		{
		direction = 10;
		}

		m_pSectorMgr = CreateSectorMgr(sectorInitInfo);*/

		m_actors.emplace_back(std::move(pActor));
	}

	if (false)
	{
		gameobject::TerrainProperty terrain;

		terrain.texHeightMap = file::GetEngineDataPath();
		terrain.texHeightMap.append(L"Texture\\heightmap.r16");

		terrain.texColorMap = file::GetEngineDataPath();
		terrain.texColorMap.append(L"Texture\\ColorMap2.bmp");

		terrain.heightScale = 300.f;

		terrain.size = { 1025, 1025 };
		terrain.patches = { 64, 64 };

		terrain.rigidActorProperty.material.restitution = 0.25f;
		terrain.rigidActorProperty.material.staticFriction = 0.75f;
		terrain.rigidActorProperty.material.dynamicFriction = 0.75f;

		terrain.texDetailMap = file::GetEngineDataPath();
		terrain.texDetailMap.append(L"Texture\\dirt01d.dds");

		terrain.texDetailNormalMap = file::GetEngineDataPath();
		terrain.texDetailNormalMap.append(L"Texture\\dirt01n.dds");

		terrain.transform.position = { -500.f, 0.f, -500.f };

		gameobject::TerrainPtr pTerrain = gameobject::CreateTerrain(L"BaseTerrain", terrain);

		// 백그라운드 로딩은 이렇게 쓰면됨
		//gameobject::TerrainPtr pTerrain = gameobject::CreateTerrainAsync(L"BaseTerrainAsync", terrain);
		m_terrains.emplace_back(std::move(pTerrain));
	}

	auto CreateActor = [&](const string::StringID& strActorName, const wchar_t* modelFilePath,
		const math::float3& f3Position,
		graphics::ModelLoader::LoadType emModelType = graphics::ModelLoader::eEast)
	{
		gameobject::ActorPtr pActor = gameobject::CreateActor(strActorName);

		pActor->SetPosition(f3Position);

		graphics::ModelLoader loader;

		string::StringID strFileName = file::GetFileName(modelFilePath).c_str();
		switch (emModelType)
		{
		case graphics::ModelLoader::LoadType::eFbx:
		case graphics::ModelLoader::LoadType::eObj:
			loader.InitFBX(strFileName, modelFilePath, 0.01f);
			break;
		case graphics::ModelLoader::LoadType::eXps:
			loader.InitXPS(strFileName, modelFilePath);
			break;
		case graphics::ModelLoader::LoadType::eEast:
			loader.InitEast(strFileName, modelFilePath);
			break;
		}
		loader.SetEnableThreadLoad(false);

		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::IComponent::eModel));
		pModel->Add(0, loader);

		return m_actors.emplace_back(std::move(pActor)).get();
	};

	for (int i = 0; i < 100; ++i)
	{
		const string::StringID name = string::Format(L"UnityChan%d", i);

		path = file::GetEngineDataPath();
		path.append(L"Model\\UnityChan\\unitychan.emod");

		math::float3 pos;
		pos.x = -10.f + (2.f * (i % 10));
		pos.z = 0.f + (2.f * (i / 10));

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::IComponent::eModel));
		graphics::IModelInstance* pModelInstance = pModel->GetModelInstance(0);

		//if (false)
		{
			const std::vector<const wchar_t*> vecAnim =
			{
				L"Model\\UnityChan\\Animations\\unitychan_WAIT00.fbx",
				L"Model\\UnityChan\\Animations\\unitychan_WAIT01.fbx",
				L"Model\\UnityChan\\Animations\\unitychan_WAIT02.fbx",
				L"Model\\UnityChan\\Animations\\unitychan_WAIT03.fbx",
				L"Model\\UnityChan\\Animations\\unitychan_WAIT04.fbx",
			};

			std::wstring pathMotion(file::GetEngineDataPath());
			//pathMotion.append(L"Actor\\UnityChan\\Animations\\unitychan_WAIT00.fbx");
			pathMotion.append(vecAnim[math::Random(0llu, vecAnim.size() - 1)]);

			const string::StringID strMotionName = string::Format(L"%s", file::GetFileName(pathMotion).c_str());
			graphics::MotionLoader motionLoader;
			motionLoader.InitFBX(strMotionName, pathMotion.c_str(), 0.01f);
			graphics::MotionPtr pMotion = graphics::CreateMotion(motionLoader);

			graphics::MotionPlaybackInfo playback;
			playback.speed = math::RandomReal(0.5f, 1.5f);
			playback.loopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;
			//playback.weight = math::Random(0.1f, 0.5f);
			playback.weight = 1.f;
			pModel->PlayMotion(0, graphics::MotionLayers::eLayer1, pMotion, &playback);
		}

		//{
		//	std::vector<const wchar_t*> vecAnim =
		//	{
		//		L"Actor\\UnityChan\\Animations\\unitychan_RUN00_F.fbx",
		//		L"Actor\\UnityChan\\Animations\\unitychan_JUMP00.fbx",
		//		L"Actor\\UnityChan\\Animations\\unitychan_LOSE00.fbx",
		//		L"Actor\\UnityChan\\Animations\\unitychan_REFLESH00.fbx",
		//		L"Actor\\UnityChan\\Animations\\unitychan_SLIDE00.fbx",
		//		L"Actor\\UnityChan\\Animations\\unitychan_UMATOBI00.fbx",
		//		L"Actor\\UnityChan\\Animations\\unitychan_WIN00.fbx",
		//	};

		//	std::wstring pathMotion(file::GetEngineDataPath());
		//	//pathMotion.append(L"Actor\\UnityChan\\Animations\\unitychan_ARpose1.fbx");
		//	//pathMotion.append(L"Actor\\UnityChan\\Animations\\unitychan_RUN00_F.fbx");
		//	pathMotion.append(vecAnim[math::Random(0u, vecAnim.size() - 1)]);

		//	string::StringID strMotionName;
		//	strMotionName.Format(L"%s", file::GetFileName(pathMotion).c_str());
		//	graphics::MotionLoader motionLoader;
		//	motionLoader.InitFBX(strMotionName, pathMotion.c_str(), 0.01f);
		//	graphics::IMotion* pMotion = graphics::CreateMotion(motionLoader);

		//	graphics::MotionPlaybackInfo playback;
		//	playback.speed = math::Random(0.5f, 1.5f);
		//	playback.loopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;
		//	playback.weight = math::Random(0.7f, 1.f);
		//	pMotionSystem->Play(graphics::MotionLayers::eLayer2, pMotion, &playback);
		//}

		//gameobject::ComponentPhysics* pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::IComponent::ePhysics));

		//math::float3 ragdollPos = pActor->GetPosition();
		//pCompPhysics->m_pRagDoll->BuildBipadRagDoll(pModelInstance->GetSkeleton(), ragdollPos, math::Quaternion::Identity, 0.8f);
		//pCompPhysics->m_pRagDoll->Start();

		//if (false)
		{
			path = file::GetEngineDataPath();
			path.append(L"Model\\ElementalSwordIce\\LP.emod");

			graphics::ModelLoader loader;
			loader.InitEast(file::GetFileName(path).c_str(), path.c_str());

			graphics::ModelInstancePtr pModelInstance_Attach = graphics::CreateModelInstance(loader, false);

			math::float3 f3Pos = { 0.08f, 0.225f, -0.02f };
			math::Quaternion quat = math::Quaternion::CreateFromYawPitchRoll(math::ToRadians(90.f), math::ToRadians(180.f), 0.f);

			pModelInstance->Attachment(0, std::move(pModelInstance_Attach), "Character1_LeftHand", math::Matrix::Compose(math::float3::One, quat, f3Pos));
		}
	}

	{
		const string::StringID name = string::Format(L"KimJiYoon");

		math::float3 pos;
		pos.x = 2.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\KimJiYoon\\KimJiYoon.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::IComponent::eModel));

		//if (false)
		{
			std::wstring pathMotion(file::GetEngineDataPath());
			pathMotion.append(L"Model\\KimJiYoon\\AR_Idle_CC.fbx");

			const string::StringID strMotionName = string::Format(L"%s", file::GetFileName(pathMotion).c_str());
			graphics::MotionLoader motionLoader;
			motionLoader.InitFBX(strMotionName, pathMotion.c_str(), 0.01f);
			graphics::MotionPtr pMotion = graphics::CreateMotion(motionLoader);

			graphics::MotionPlaybackInfo playback;
			playback.speed = 1.f;
			playback.loopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;
			playback.weight = 1.f;
			pModel->PlayMotion(0, graphics::MotionLayers::eLayer1, pMotion, &playback);
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		const string::StringID name = string::Format(L"2B_NierAutomata_%d", i);

		math::float3 pos;
		pos.x = -2.f + (i * -2.f);

		path = file::GetEngineDataPath();
		path.append(L"Model\\2B_NierAutomata\\2B_NierAutomata.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::IComponent::eModel));

		graphics::IModelInstance* pModelInstance = pModel->GetModelInstance(0);

		if (i == 1)
		{
			graphics::IModelNode* pNode = pModelInstance->GetModel()->GetNode(L"Generic_Item.mesh");

			auto SetMaterialVisible = [&](const string::StringID& strMaterialName)
			{
				uint32_t materialID = 0;
				graphics::MaterialPtr pMaterial = pNode->GetMaterial(strMaterialName, materialID);

				graphics::MaterialPtr pMaterialClone = graphics::CloneMaterial(pMaterial.get());
				pMaterialClone->SetVisible(false);

				pModelInstance->ChangeMaterial(L"Generic_Item.mesh", materialID, pMaterialClone);
			};

			SetMaterialVisible(L"Skirt");
			SetMaterialVisible(L"Eyepatch");
		}
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"Delia");

		math::float3 pos;
		pos.x = 4.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Delia\\Delia.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"Misaki");

		math::float3 pos;
		pos.x = 6.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Misaki\\Misaki.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"Naotora");

		math::float3 pos;
		pos.x = 8.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Naotora\\Naotora.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"Naotora_ShirtDress");

		math::float3 pos;
		pos.x = 10.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Naotora_ShirtDress\\Naotora_ShirtDress.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"Bugeikloth");

		math::float3 pos;
		pos.z = 10.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Bugeikloth\\Bugeikloth.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"DarkKnight_Female");

		math::float3 pos;
		pos.x = -4.f;
		pos.z = 2.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Dark Knight_Female\\DarkKnight_Female.emod");

		//gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos, graphics::EmModelLoader::eXps);
		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"DarkKnight_Transformed_Female");

		math::float3 pos;
		pos.x = -2.f;
		pos.z = 2.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Dark Knight Transformed_Female\\DarkKnight_Transformed_Female.emod");

		//gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos, graphics::EmModelLoader::eXps);
		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"Paladin_Female");

		math::float3 pos;
		pos.x = 0.f;
		pos.z = 2.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Paladin_Female\\Paladin_Female.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"Paladin_Transformed_Female");

		math::float3 pos;
		pos.x = 2.f;
		pos.z = 2.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Paladin Transformed_Female\\Paladin_Transformed_Female.emod");

		//gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos, graphics::EmModelLoader::eXps);
		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"Evie_Temptress");

		math::float3 pos;
		pos.x = 4.f;
		pos.z = 2.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Evie_Temptress\\Evie_Temptress.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	//if (false)
	{
		const string::StringID name = string::Format(L"Lynn_DancingBlade");

		math::float3 pos;
		pos.x = 6.f;
		pos.z = 2.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\Lynn_DancingBlade\\Lynn_DancingBlade.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	{
		const string::StringID name = string::Format(L"ElementalSwordIce");

		math::float3 pos;
		pos.y = 1.f;
		pos.z -= 2.f;

		path = file::GetEngineDataPath();
		path.append(L"Model\\ElementalSwordIce\\LP.emod");

		gameobject::IActor* pActor = CreateActor(name, path.c_str(), pos);
	}

	std::shared_ptr<graphics::IMaterial> pMaterial_override;
	for (int j = 0; j < 5; ++j)
	{
		for (int i = 0; i < 5; ++i)
		{
			/*graphics::IMaterial::Data materialData;
			materialData.name.Format(L"TestBox%d", (i % 10) + 1);
			materialData.path = file::GetPath(file::eTexture);

			materialData.textureNameArray[graphics::IMaterial::eAlbedo].Format(L"Pattern\\pattern_%02d\\%s", (i % 10) + 1, "diffus.tga");
			materialData.textureNameArray[graphics::IMaterial::eNormal].Format(L"Pattern\\pattern_%02d\\%s", (i % 10) + 1, "Normal.tga");
			materialData.textureNameArray[graphics::IMaterial::eSpecularColor].Format(L"Pattern\\pattern_%02d\\%s", (i % 10) + 1, "specular.tga");
			*/
			graphics::IMaterial::Data materialData;
			materialData.name = "TestBox";
			//materialData.path = file::GetEngineDataPath();

			//materialData.textureNameArray[graphics::IMaterial::eAlbedo].Format(L"Texture\\Pattern\\pattern_01\\%s", L"diffus.tga");
			//materialData.textureNameArray[graphics::IMaterial::eNormal].Format(L"Texture\\Pattern\\pattern_01\\%s", L"Normal.tga");
			//materialData.textureNameArray[graphics::IMaterial::eSpecular].Format(L"Texture\\Pattern\\pattern_01\\%s", L"specular.tga");

			//materialData.paddingRoughMetEmi.y = 0.1f * ((i % 10) + 1);
			//materialData.paddingRoughMetEmi.z = 1.f - 0.1f * ((i % 10) + 1);

			materialData.paddingRoughMetEmi.y = 0.75f;
			materialData.paddingRoughMetEmi.z = 0.2f;

			//materialData.rasterizerStateDesc = graphics::GetDevice()->GetRasterizerStateDesc(graphics::RasterizerState::eNone);
			//materialData.colorAlbedo = math::Color(math::RandomReal(0.f, 1.f), math::RandomReal(0.f, 1.f), math::RandomReal(0.f, 1.f), 1.f);
			materialData.colorAlbedo = math::Color(1.f, 0.f, 0.f, 1.f);

			gameobject::ActorPtr pActor = gameobject::CreateActor(L"TestBox");

			math::float3 position;
			position.x = -4.f + (i % 5) * 3.f;
			position.y = 100.5f + (j * 3.f);
			//position.y = 0.5f + (j * 3.f);
			position.z = -4.f + (i / 5) * 3.f;

			pActor->SetPosition(position);

			gameobject::ComponentModel* pCompModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::IComponent::eModel));

			const math::float3 halfExtents(math::float3::One);

			graphics::ModelLoader loader;
			loader.InitBox(L"TestBox", &materialData, halfExtents);
			loader.SetEnableThreadLoad(false);
			pCompModel->Add(0, loader);

			auto pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::IComponent::ePhysics));
			physics::RigidActorProperty rigidProperty;
			rigidProperty.material.restitution = 0.75f;
			rigidProperty.material.staticFriction = 0.25f;
			rigidProperty.material.dynamicFriction = 0.25f;
			//rigidProperty.shape.SetTriangleMeshGeometry(math::float3::One, math::Quaternion::Identity, reinterpret_cast<const math::float3*>(pVertices), numVertices, pIndices, numIndices, physics::IGeometry::MeshFlag::eNone);
			rigidProperty.shape.SetBoxGeometry(halfExtents);
			rigidProperty.shape.flags = physics::IShape::eSimulationShape | physics::IShape::eSceneQueryShape;
			rigidProperty.rigidAcotr.name = string::Format(L"TestBox%d", (i % 10) + 1);
			rigidProperty.rigidAcotr.type = physics::IActor::eRigidDynamic;
			rigidProperty.rigidAcotr.globalTransform.position = position;
			physics::IRigidDynamic* pRigidDynamic = static_cast<physics::IRigidDynamic*>(pCompPhysics->CreateRigidActor(rigidProperty));

			//if (i % 2 == 0)
			//{
			//	if (pMaterial_override == nullptr)
			//	{
			//		graphics::IMaterial::Data materialData2;
			//		materialData2.name = "TestBox";
			//		materialData2.path = file::GetEngineDataPath();
			//
			//		materialData2.textureNameArray[graphics::IMaterial::eAlbedo].Format(L"Texture\\Pattern\\pattern_02\\%s", L"diffus.tga");
			//		materialData2.textureNameArray[graphics::IMaterial::eNormal].Format(L"Texture\\Pattern\\pattern_02\\%s", L"Normal.tga");
			//		materialData2.textureNameArray[graphics::IMaterial::eSpecular].Format(L"Texture\\Pattern\\pattern_02\\%s", L"specular.tga");
			//
			//		pMaterial_override = graphics::CreateMaterial(&materialData2);
			//	}
			//	pModelInstance->ChangeMaterial(L"est_Box", 0, pMaterial_override);
			//}

			//gameobject::ComponentPhysics* pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::IComponent::ePhysics));
			//
			//physics::RigidBodyProperty prop;
			//prop.fRestitution = 0.5f;
			//prop.name.Format(L"TestBox_RigidBody%d", i).c_str();
			//
			//prop.shapeInfo.SetBox(math::float3(1.f));
			////prop.shapeInfo.SetCapsule(math::Random(0.5f, 1.f), math::Random(1.f, 2.f));
			//prop.nCollisionFlag = physics::CollisionFlag::eCharacterObject;
			//prop.f3OriginPos = f3Pos;
			//pCompPhysics->Initialize(prop);

			m_actors.emplace_back(std::move(pActor));
		}
	}

	{
		m_pMinion = std::make_unique<Minion>();
	}

	// Sound
	{
		path = file::GetEngineDataPath();
		path += L"Sound\\Canada, Ho!.mp3";
		sound::Play2D(path, 0.1f);
	}
}

void SceneStudio::Exit(std::queue<gameobject::ActorPtr>& saveSceneActors_out)
{
	m_pMinion.reset();
	m_pLights.clear();
}

void SceneStudio::Update(float elapsedTime)
{
	const ImGuiIO& io = ImGui::GetIO();

	if (io.WantCaptureMouse == false)
	{
		ProcessInput(elapsedTime);
	}

	RenderImGui(elapsedTime);

	static float time = 0.f;
	time += elapsedTime;

	static int nFrame = 0;
	++nFrame;

	if (time >= 1.f)
	{
		const float fFrame = static_cast<float>(nFrame) / time;
		const float fMS = 1.f / fFrame;
		LOG_MESSAGE(L"%.2f[%f]", fFrame, fMS);

		time -= 1.f;
		nFrame = 0;
	}
}

void SceneStudio::ProcessInput(float elapsedTime)
{
	graphics::Camera& camera = graphics::GetCamera();
	if (camera.GetCameraMan() == nullptr)
		return;

	float dx = static_cast<float>(input::mouse::GetMoveX());
	float dy = static_cast<float>(input::mouse::GetMoveY());
	float dz = static_cast<float>(input::mouse::GetMoveWheel());
	bool isMoveAxisX = math::IsZero(dx) == false;
	bool isMoveAxisY = math::IsZero(dy) == false;

	if (dynamic_cast<graphics::FirstPersonCameraMan*>(camera.GetCameraMan()) != nullptr)
	{
		graphics::FirstPersonCameraMan* pCameraMan = static_cast<graphics::FirstPersonCameraMan*>(camera.GetCameraMan());
		if (input::mouse::IsButtonPressed(input::mouse::eRight))
		{
			if (isMoveAxisX == true)
			{
				pCameraMan->RotateAxisY(dx * 0.1f);
			}

			if (isMoveAxisY == true)
			{
				pCameraMan->RotateAxisX(dy * 0.1f);
			}
		}

		if (input::mouse::IsButtonPressed(input::mouse::eMiddle))
		{
			if (isMoveAxisX == true)
			{
				pCameraMan->MoveSideward(dx * 0.025f);
			}

			if (isMoveAxisY == true)
			{
				pCameraMan->MoveUpward(-dy * 0.05f);
			}
		}

		if (input::mouse::IsButtonPressed(input::mouse::eLeft))
		{
			if (isMoveAxisX == true)
			{
				pCameraMan->RotateAxisY(dx * 0.025f);
			}

			if (isMoveAxisY == true)
			{
				pCameraMan->MoveForward(-dy * 0.05f);
			}
		}

		if (dz != 0.f)
		{
			pCameraMan->MoveForward(dz * 0.01f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eW))
		{
			pCameraMan->MoveForward(1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eS))
		{
			pCameraMan->MoveForward(-1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eA))
		{
			pCameraMan->MoveSideward(-1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eD))
		{
			pCameraMan->MoveSideward(1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eE))
		{
			pCameraMan->MoveUpward(1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eQ))
		{
			pCameraMan->MoveUpward(-1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eUp))
		{
			pCameraMan->MoveAxisZ(1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eDown))
		{
			pCameraMan->MoveAxisZ(-1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eLeft))
		{
			pCameraMan->MoveAxisX(-1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eRight))
		{
			pCameraMan->MoveAxisX(1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eLeftBracket))
		{
			pCameraMan->MoveAxisY(-1.f);
		}

		if (input::keyboard::IsKeyPressed(input::keyboard::eRightBracket))
		{
			pCameraMan->MoveAxisY(1.f);
		}
	}
	else if (dynamic_cast<graphics::ThirdPersonCameraMan*>(camera.GetCameraMan()))
	{
		graphics::ThirdPersonCameraMan* pCameraMan = static_cast<graphics::ThirdPersonCameraMan*>(camera.GetCameraMan());
		if (input::mouse::IsButtonPressed(input::mouse::eRight))
		{
			if (isMoveAxisX == true)
			{
				pCameraMan->RotateAxisY(dx * 0.1f);
			}

			if (isMoveAxisY == true)
			{
				pCameraMan->RotateAxisX(dy * 0.1f);
			}
		}

		if (dz != 0.f)
		{
			pCameraMan->MoveDistance(-dz * 0.01f);
		}
	}

	if (input::gamepad::IsConnected() == true)
	{
		auto LogButton = [](const wchar_t* buttonName, const input::gamepad::ButtonState& emButtonState)
		{
			if (emButtonState == input::gamepad::ButtonState::ePressed)
			{
				LOG_MESSAGE(L"%s Pressed", buttonName);
			}
			else if (emButtonState == input::gamepad::ButtonState::eUp)
			{
				LOG_MESSAGE(L"%s Up", buttonName);
			}
			else if (emButtonState == input::gamepad::ButtonState::eDown)
			{
				LOG_MESSAGE(L"%s Down", buttonName);
			}
		};

		LogButton(L"A", input::gamepad::A());
		LogButton(L"B", input::gamepad::B());
		LogButton(L"X", input::gamepad::X());
		LogButton(L"Y", input::gamepad::Y());

		LogButton(L"LeftStick", input::gamepad::LeftStick());
		LogButton(L"RightStick", input::gamepad::RightStick());

		LogButton(L"LeftShoulder", input::gamepad::LeftShoulder());
		LogButton(L"RightShoulder", input::gamepad::RightShoulder());

		LogButton(L"Back", input::gamepad::Back());
		LogButton(L"Start", input::gamepad::Start());

		LogButton(L"DPadUp", input::gamepad::DPadUp());
		LogButton(L"DPadDown", input::gamepad::DPadDown());
		LogButton(L"DPadLeft", input::gamepad::DPadLeft());
		LogButton(L"DPadRight", input::gamepad::DPadRight());

		auto LogStick = [](const wchar_t* stickName, float fValue)
		{
			if (math::IsZero(fValue) == false)
			{
				LOG_MESSAGE(L"%s : %f", stickName, fValue);
			}
		};

		LogStick(L"LeftThumbStickX", input::gamepad::LeftThumbStickX());
		LogStick(L"LeftThumbStickY", input::gamepad::LeftThumbStickY());
		LogStick(L"RightThumbStickX", input::gamepad::RightThumbStickX());
		LogStick(L"RightThumbStickY", input::gamepad::RightThumbStickY());
		LogStick(L"LeftTrigger", input::gamepad::LeftTrigger());
		LogStick(L"RightTrigger", input::gamepad::RightTrigger());

		//static float time = 0.f;
		//if (time >= 5.f)
		//{
		//	pPlayer->SetVibration(0.5f, 0.5f, 1.f);
		//	time -= 5.f;
		//}
		//time += elapsedTime;
	}
	else
	{
		//static float time = 0.f;
		//if (time >= 1.f)
		//{
		//	LOG_MESSAGE(L"DisConnected");
		//	time -= 1.f;
		//}
		//
		//time += elapsedTime;
	}

	if (input::keyboard::IsKeyPressed(input::keyboard::eLeftControl) == true && input::mouse::IsButtonDown(input::mouse::eLeft) == true)
	{
		m_selectedActor = gameobject::IGameObject::eInvalidHandle;

		const math::uint2 screenSize = graphics::GetScreenSize();
		const int mouseX = input::mouse::GetX();
		const int mouseY = input::mouse::GetY();

		const collision::Ray ray(mouseX, mouseY, screenSize, camera.GetViewMatrix(), camera.GetProjectionMatrix());

		physics::HitActorShape hitActorShape;
		if (physics::scene::Raycast(ray.position, ray.direction, camera.GetProjection().farClip, nullptr, &hitActorShape, {}, {}) == true)
		{
			if (hitActorShape.pActor != nullptr)
			{
				gameobject::IGameObject* pGameObject = static_cast<gameobject::IGameObject*>(hitActorShape.pActor->GetUserData());
				if (pGameObject != nullptr && pGameObject->GetType() == gameobject::ObjectType::eActor)
				{
					m_selectedActor = pGameObject->GetHandle();
				}
			}
		}
	}
}

void SceneStudio::RenderImGui(float elapsedTime)
{
	ShowConfig();

	static bool isShowDebug = true;
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
		ImGui::Begin("Debug Info", &isShowDebug);

		float fps = MainSystem::GetInstance()->GetFPS();
		float ms = 1.f / fps * 1000.f;
		ImGui::Text("FPS : %.2f (%.2f ms)", fps, ms);

		if (ImGui::CollapsingHeader("Tracer") == true)
		{
			const bool isTracing = performance::tracer::IsTracing();
			ImGui::Text("State : %s", isTracing == true ? "Tracing" : "Idle");

			if (isTracing == true)
			{
				ImGui::Text("Time : %.2f", performance::tracer::TracingTime());

				if (ImGui::Button("End") == true)
				{
					wchar_t path[512]{};
					OPENFILENAME ofn;
					memory::Clear(&ofn, sizeof(ofn));

					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = graphics::GetHwnd();
					ofn.lpstrFilter = L"Json(*.json)\0*.json\0";
					ofn.lpstrFile = path;
					ofn.nMaxFile = 255;
					if (GetSaveFileName(&ofn) != 0)
					{
						TRACER_END(path);
					}
				}
			}
			else
			{
				if (ImGui::Button("Start") == true)
				{
					TRACER_START();
				}
			}
		}

		if (ImGui::CollapsingHeader("Detail") == true)
		{
			graphics::DebugInfo& debugInfo = graphics::GetDebugInfo();
			ImGui::Checkbox("Collection", &debugInfo.isEnableCollection);

			const graphics::DebugInfo& prevDebugInfo = graphics::GetPrevDebugInfo();

			if (ImGui::TreeNode("OcclusionCulling"))
			{
				ImGui::PushID("OcclusionCulling");

				const graphics::DebugInfo::OcclusionCulling& occlusionCulling = prevDebugInfo.occlusionCulling;
				ImGui::Text("RenderTryCount : %u", occlusionCulling.renderTryCount.load());
				ImGui::Text("RenderCompleteCount : %u", occlusionCulling.renderCompleteCount.load());
				ImGui::Text("VisibleCount : %u", occlusionCulling.visibleCount.load());
				ImGui::Text("OccludedCount : %u", occlusionCulling.occludedCount.load());
				ImGui::Text("ViewCulledCount : %u", occlusionCulling.viewCulledCount.load());

				ImGui::PopID();

				ImGui::TreePop();
			}
		}

		if (ImGui::Button("ScreenShot") == true)
		{
			std::wstring path = file::GetBinPath();
			path += L"ScreenShot\\";

			CreateDirectory(path.c_str(), nullptr);

			const std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			std::tm now{};
			localtime_s(&now, &time);

			path += string::Format(L"%d%2d%2d_%2d%2d%2d", now.tm_year + 1900, now.tm_mon, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);

			graphics::ScreenShot(graphics::ScreenShotFormat::eJPEG, path, {});
		}

		ImGui::End();
	}

	static bool isShowSoundWindow = true;
	ShowSoundWindow(isShowSoundWindow);

	static bool isShowActorMenu = true;
	ShowActorWindow(isShowActorMenu);

	ShowGizmo();
	ShowNodeEditer();
}

void SceneStudio::ShowConfig()
{
	static bool isShowMainMenu = true;

	ImGui::SetNextWindowSize(ImVec2(400, 800), ImGuiCond_FirstUseEver);
	ImGui::Begin("Config", &isShowMainMenu);

	graphics::Options& graphicsOptions = graphics::GetOptions();

	if (ImGui::CollapsingHeader("Display") == true)
	{
		bool isFullScreen = est::graphics::IsFullScreen();
		if (ImGui::Checkbox("FullScreen", &isFullScreen) == true)
		{
			graphics::SetFullScreen(isFullScreen, [](bool isSuccess)
				{
					if (isSuccess == true)
					{
						const math::uint2& screenSize = graphics::GetScreenSize();
						graphics::Camera& camera = est::graphics::GetCamera();
						graphics::Camera::DescProjection projectionDesc = camera.GetProjection();
						projectionDesc.width = screenSize.x;
						projectionDesc.height = screenSize.y;
						camera.SetProjection(projectionDesc);
					}
				});
		}

		ImGui::SameLine();

		ImGui::Checkbox("VSync", &graphicsOptions.OnVSync);

		if (isFullScreen == false)
		{
			const std::vector<est::graphics::DisplayModeDesc>& displayModeDescs = est::graphics::GetSupportedDisplayModeDesc();
			int resolutionIndex = static_cast<int>(est::graphics::GetSelectedDisplayModeIndex());

			std::vector<std::string> names;
			names.reserve(displayModeDescs.size());
			for (auto& desc : displayModeDescs)
			{
				names.emplace_back(string::WideToMulti(string::Format(L"%u x %u", desc.width, desc.height)));
			}

			std::vector<const char*> _names;
			_names.reserve(names.size());
			for (auto& name : names)
			{
				_names.emplace_back(name.c_str());
			}

			if (ImGui::Combo("Resolution", &resolutionIndex, _names.data(), static_cast<int>(_names.size())) == true)
			{
				est::graphics::ChangeDisplayMode(resolutionIndex, [](bool isSuccess)
					{
						if (isSuccess == true)
						{
							const math::uint2& screenSize = graphics::GetScreenSize();
							graphics::Camera& camera = est::graphics::GetCamera();
							graphics::Camera::DescProjection projectionDesc = camera.GetProjection();
							projectionDesc.width = screenSize.x;
							projectionDesc.height = screenSize.y;
							camera.SetProjection(projectionDesc);
						}
					});
			}
		}

		float fov = math::ToDegrees(graphics::GetCamera().GetProjection().fov);
		if (ImGui::DragFloat("FOV", &fov, 0.1f, 0.f, 180.f) == true)
		{
			graphics::Camera& camera = est::graphics::GetCamera();
			graphics::Camera::DescProjection projectionDesc = camera.GetProjection();
			projectionDesc.fov = math::ToRadians(fov);
			camera.SetProjection(projectionDesc);
		}
	}

	if (ImGui::CollapsingHeader("Debug") == true)
	{
		if (ImGui::TreeNode("CollisionVisible"))
		{
			ImGui::PushID("CollisionVisible");

			ImGui::Checkbox("Visible", &graphicsOptions.OnCollisionVisible);

			ImGui::PopID();

			ImGui::TreePop();
		}

		/*if (ImGui::TreeNode("Skeleton"))
		{
			ImGui::PushID("Skeleton");

			bool isApplyVisibleCollisionMesh = Config::IsEnable("VisibleSkeleton"_s);
			if (ImGui::Checkbox("Visible", &isApplyVisibleCollisionMesh) == true)
			{
				Config::SetEnable("VisibleSkeleton"_s, isApplyVisibleCollisionMesh);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}*/
	}

	if (ImGui::CollapsingHeader("Graphics") == true)
	{
		if (ImGui::TreeNode("OcclusionCulling"))
		{
			ImGui::PushID("OcclusionCulling");

			//ImGui::Checkbox("Apply", &graphicsOptions.OnOcclusionCulling);
			//
			//if (ImGui::Button("SaveBuffer") == true)
			//{
			//	wchar_t path[512]{};
			//	OPENFILENAME ofn;
			//	memory::Clear(&ofn, sizeof(ofn));
			//
			//	ofn.lStructSize = sizeof(OPENFILENAME);
			//	ofn.hwndOwner = graphics::GetHwnd();
			//	ofn.lpstrFilter = L"Bmp(*.bmp)\0*.bmp\0";
			//	ofn.lpstrFile = path;
			//	ofn.nMaxFile = 255;
			//	if (GetSaveFileName(&ofn) != 0)
			//	{
			//		graphics::OcclusionCullingWriteBMP(path);
			//	}
			//}

			ImGui::PopID();

			ImGui::TreePop();
		}

		//if (ImGui::TreeNode("Shadow"))
		//{
		//	ImGui::PushID("Shadow");
		//
		//	ImGui::Checkbox("Apply", &graphicsOptions.OnShadow);
		//
		//	ImGui::PopID();
		//
		//	ImGui::TreePop();
		//}

		if (ImGui::TreeNode("FXAA"))
		{
			ImGui::PushID("FXAA");

			ImGui::Checkbox("Apply", &graphicsOptions.OnFXAA);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("ColorGrading"))
		{
			ImGui::PushID("ColorGrading");

			ImGui::Checkbox("Apply", &graphicsOptions.OnColorGrading);

			ImGui::ColorEdit3("Guide", &graphicsOptions.colorGradingConfig.colorGuide.x);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("BloomFilter"))
		{
			ImGui::PushID("BloomFilter");

			ImGui::Checkbox("Apply", &graphicsOptions.OnBloomFilter);

			const std::array<const char*, graphics::Options::BloomFilterConfig::PresetCount> presets = { "Wide", "Focussed", "Small", "SuperWide", "Cheap", "One", };

			graphics::Options::BloomFilterConfig& bloomFilterConfig = graphicsOptions.bloomFilterConfig;

			ImGui::Combo("Presets", reinterpret_cast<int*>(&bloomFilterConfig.emPreset), presets.data(), static_cast<int>(presets.size()));

			ImGui::DragFloat("Threshold", &bloomFilterConfig.threshold, 0.001f, 0.f, 10.f);
			ImGui::DragFloat("StrengthMultiplier", &bloomFilterConfig.strengthMultiplier, 0.001f, 0.f, 10.f);
			ImGui::Checkbox("IsEnableLuminance", &bloomFilterConfig.isEnableLuminance);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("SSS"))
		{
			ImGui::PushID("SSS");

			ImGui::Checkbox("Apply", &graphicsOptions.OnSSS);

			ImGui::DragFloat("Width", &graphicsOptions.sssConfig.width, 0.001f, 0.f, 100.f);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("SSR"))
		{
			ImGui::PushID("SSR");

			ImGui::Checkbox("Apply", &graphicsOptions.OnSSR);

			ImGui::DragFloat("BlurSigma", &graphicsOptions.ssrConfig.blurSigma, 0.001f, 0.5f, 10.f);
			ImGui::DragInt("SampleCount", &graphicsOptions.ssrConfig.sampleCount, 0.1f, 4, 128);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("ASSAO"))
		{
			ImGui::PushID("ASSAO");

			ImGui::Checkbox("Apply", &graphicsOptions.OnASSAO);

			ImGui::PushItemWidth(100);

			graphics::Options::AssaoConfig& assaoConfig = graphicsOptions.assaoConfig;

			ImGui::DragFloat("Radius", &assaoConfig.Radius, 0.01f, 0.f, 100.f);
			ImGui::DragFloat("ShadowMultiplier", &assaoConfig.ShadowMultiplier, 0.01f, 0.f, 5.f);
			ImGui::DragFloat("ShadowPower", &assaoConfig.ShadowPower, 0.01f, 0.5f, 5.f);
			ImGui::DragFloat("ShadowClamp", &assaoConfig.ShadowClamp, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("HorizonAngleThreshold", &assaoConfig.HorizonAngleThreshold, 0.001f, 0.f, 0.2f);
			ImGui::DragFloat("FadeOutFrom", &assaoConfig.FadeOutFrom, 0.1f, 0.f, 1000000.f);
			ImGui::DragFloat("FadeOutTo", &assaoConfig.FadeOutTo, 0.1f, 0.f, 1000000.f);
			ImGui::DragInt("QualityLevel", &assaoConfig.QualityLevel, 0.01f, -1, 3);
			ImGui::DragFloat("AdaptiveQualityLimit", &assaoConfig.AdaptiveQualityLimit, 0.01f, 0.f, 1.f);
			ImGui::DragInt("BlurPassCount", &assaoConfig.BlurPassCount, 0.01f, 0, 6);
			ImGui::DragFloat("Sharpness", &assaoConfig.Sharpness, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("TemporalSupersamplingAngleOffset", &assaoConfig.TemporalSupersamplingAngleOffset, 0.01f, 0.f, math::PI);
			ImGui::DragFloat("TemporalSupersamplingRadiusOffset", &assaoConfig.TemporalSupersamplingRadiusOffset, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("DetailShadowStrength", &assaoConfig.DetailShadowStrength, 0.01f, 0.f, 5.f);

			ImGui::PopItemWidth();

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("DepthOfField"))
		{
			ImGui::PushID("DepthOfField");

			ImGui::Checkbox("Apply", &graphicsOptions.OnDOF);

			const float nearClip = graphics::GetCamera().GetProjection().nearClip;
			const float farClip = graphics::GetCamera().GetProjection().farClip;

			graphics::Options::DepthOfFieldConfig& depthOfFieldConfig = graphicsOptions.depthOfFieldConfig;

			ImGui::PushItemWidth(100);
			ImGui::DragFloat("FocalDistnace", &depthOfFieldConfig.FocalDistnace, 0.01f, nearClip, farClip);
			ImGui::DragFloat("FocalWidth", &depthOfFieldConfig.FocalWidth, 0.01f, 1.f, 10000.f);

			ImGui::PopItemWidth();

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("HDRFilter"))
		{
			ImGui::PushID("HDRFilter");

			ImGui::Checkbox("Apply", &graphicsOptions.OnHDR);

			graphics::Options::HDRConfig& hdrConfig = graphicsOptions.hdrConfig;
			if (ImGui::Button("Reset") == true)
			{
				hdrConfig = graphics::Options::HDRConfig();
			}

			ImGui::PushItemWidth(150);

			const std::array<const char*, graphics::Options::HDRConfig::NumToneMappingTypes> ToneMappingTypes =
			{
				"None",
				"Logarithmic",
				"Exponential",
				"DragoLogarithmic",
				"Reinhard",
				"ReinhardModified",
				"FilmicALU",
				"FilmicUncharted",
				"ACES",
			};

			const std::array<const char*, graphics::Options::HDRConfig::NumAutoExposureTypes> AutoExposureTypes =
			{
				"Manual",
				"GeometricMean",
				"GeometricMeanAutoKey",
			};

			ImGui::Combo("ToneMappingTypes", reinterpret_cast<int*>(&hdrConfig.emToneMappingType), ToneMappingTypes.data(), static_cast<int>(ToneMappingTypes.size()));
			ImGui::Combo("AutoExposureTypes", reinterpret_cast<int*>(&hdrConfig.emAutoExposureType), AutoExposureTypes.data(), static_cast<int>(AutoExposureTypes.size()));

			ImGui::DragFloat("BloomThreshold", &hdrConfig.BloomThreshold, 0.01f, 0.f, 10.f);
			ImGui::DragFloat("BloomMagnitude", &hdrConfig.BloomMagnitude, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("BloomBlurSigma", &hdrConfig.BloomBlurSigma, 0.01f, 0.5f, 1.5f);
			ImGui::DragFloat("Tau", &hdrConfig.Tau, 0.01f, 0.f, 4.f);
			ImGui::DragFloat("Exposure", &hdrConfig.Exposure, 0.01f, -10.f, 10.f);
			ImGui::DragFloat("KeyValue", &hdrConfig.KeyValue, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("WhiteLevel", &hdrConfig.WhiteLevel, 0.01f, 0.f, 5.f);
			ImGui::DragFloat("ShoulderStrength", &hdrConfig.ShoulderStrength, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("LinearStrength", &hdrConfig.LinearStrength, 0.01f, 0.f, 5.f);
			ImGui::DragFloat("LinearAngle", &hdrConfig.LinearAngle, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("ToeStrength", &hdrConfig.ToeStrength, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("ToeNumerator", &hdrConfig.ToeNumerator, 0.01f, 0.f, 0.5f);
			ImGui::DragFloat("ToeDenominator", &hdrConfig.ToeDenominator, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("LinearWhite", &hdrConfig.LinearWhite, 0.01f, 0.f, 20.f);
			ImGui::DragFloat("LuminanceSaturation", &hdrConfig.LuminanceSaturation, 0.01f, 0.f, 4.f);

			ImGui::DragInt("LumMapMipLevel", &hdrConfig.LumMapMipLevel, 0.1f, 0, 10);
			ImGui::DragFloat("Bias", &hdrConfig.Bias, 0.01f, 0.f, 1.f);

			ImGui::PopItemWidth();

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("MotionBlur"))
		{
			ImGui::PushID("MotionBlur");

			ImGui::Checkbox("Apply", &graphicsOptions.OnMotionBlur);

			const std::array<const char*, graphics::Options::MotionBlurConfig::ModeCount> MotionBlurModes =
			{
				"DepthBuffer_4Samples",
				"DepthBuffer_8Samples",
				"DepthBuffer_12Samples",
				"VelocityBuffer_4Samples",
				"VelocityBuffer_8Samples",
				"VelocityBuffer_12Samples",
				"DualVelocityBuffer_4Samples",
				"DualVelocityBuffer_8Samples",
				"DualVelocityBuffer_12Samples",
			};

			graphics::Options::MotionBlurConfig& motionBlurConfig = graphicsOptions.motionBlurConfig;
			ImGui::Combo("Modes", reinterpret_cast<int*>(&motionBlurConfig.emMode), MotionBlurModes.data(), static_cast<int>(MotionBlurModes.size()));

			ImGui::DragFloat("Amount", &motionBlurConfig.blurAmount, 0.01f, 0.1f, 5.f, "%.2f");

			ImGui::PopID();

			ImGui::TreePop();
		}

		//if (ImGui::TreeNode("Tessellation"))
		//{
		//	ImGui::PushID("Tessellation");
		//
		//	ImGui::Checkbox("Apply", &graphicsOptions.OnTessellation);
		//
		//	ImGui::PopID();
		//
		//	ImGui::TreePop();
		//}
		//
		//if (ImGui::TreeNode("Wireframe"))
		//{
		//	ImGui::PushID("Wireframe");
		//
		//	ImGui::Checkbox("Apply", &graphicsOptions.OnWireframe);
		//
		//	ImGui::PopID();
		//
		//	ImGui::TreePop();
		//}
	}

	if (ImGui::CollapsingHeader("Skybox") == true)
	{
		static int selectedIndex = 0;
		if (ImGui::Combo("Env", &selectedIndex, IBL_Type, _countof(IBL_Type)) == true)
		{
			const std::wstring iblTypeString = string::MultiToWide(IBL_Type[selectedIndex]);
			const std::wstring path = string::Format(L"%sTexture\\IBL\\%s\\%s", file::GetEngineDataPath(), iblTypeString.c_str(), iblTypeString.c_str());

			graphics::IImageBasedLight* pImageBasedLight = graphics::GetImageBasedLight();

			std::wstring diffuseHDR = path;
			diffuseHDR.append(L"DiffuseHDR.dds");
			graphics::TexturePtr pDiffuseHDR = graphics::CreateTextureAsync(diffuseHDR.c_str());
			pImageBasedLight->SetDiffuseHDR(pDiffuseHDR);

			std::wstring specularHDR = path;
			specularHDR.append(L"SpecularHDR.dds");
			graphics::TexturePtr pSpecularHDR = graphics::CreateTextureAsync(specularHDR.c_str());
			pImageBasedLight->SetSpecularHDR(pSpecularHDR);

			std::wstring specularBRDF = path;
			specularBRDF.append(L"Brdf.dds");
			graphics::TexturePtr pSpecularBRDF = graphics::CreateTextureAsync(specularBRDF.c_str());
			pImageBasedLight->SetSpecularBRDF(pSpecularBRDF);

			std::wstring envIBLPath = path;
			envIBLPath.append(L"EnvHDR.dds");
			graphics::TexturePtr pEnvironmentHDR = graphics::CreateTextureAsync(envIBLPath.c_str());
			pImageBasedLight->SetEnvironmentHDR(pEnvironmentHDR);
		}
	}

	if (ImGui::CollapsingHeader("Light") == true)
	{
		if (ImGui::TreeNode("Directional"))
		{
			const size_t directionalLightCount = graphics::GetLightCount(graphics::ILight::Type::eDirectional);
			for (size_t i = 0; i < directionalLightCount; ++i)
			{
				graphics::LightPtr pLight = graphics::GetLight(graphics::ILight::Type::eDirectional, i);
				if (pLight == nullptr)
					continue;

				graphics::IDirectionalLight* pDirectionalLight = static_cast<graphics::IDirectionalLight*>(pLight.get());

				const std::string lightName = string::WideToMulti(pDirectionalLight->GetName().c_str());
				if (ImGui::TreeNode(lightName.c_str()))
				{
					ImGui::PushID(lightName.c_str());

					float intensity = pDirectionalLight->GetIntensity();
					if (ImGui::DragFloat("Intensity", &intensity, 0.01f) == true)
					{
						pDirectionalLight->SetIntensity(intensity);
					}

					float ambientIntensity = pDirectionalLight->GetAmbientIntensity();
					if (ImGui::DragFloat("AmbientIntensity", &ambientIntensity, 0.01f) == true)
					{
						pDirectionalLight->SetAmbientIntensity(ambientIntensity);
					}

					float reflectionIntensity = pDirectionalLight->GetReflectionIntensity();
					if (ImGui::DragFloat("ReflectionIntensity", &reflectionIntensity, 0.01f) == true)
					{
						pDirectionalLight->SetReflectionIntensity(reflectionIntensity);
					}

					math::float3 color = pDirectionalLight->GetColor();
					if (ImGui::ColorEdit3("Diffuse", &color.x) == true)
					{
						pDirectionalLight->SetColor(color);
					}

					math::float3 direction = pDirectionalLight->GetDirection();
					if (ImGui::DragFloat3("Direction", &direction.x, 0.01f) == true)
					{
						direction.Normalize();
						pDirectionalLight->SetDirection(direction);
					}

					bool isEnableShadow = pDirectionalLight->IsEnableShadow();
					if (ImGui::Checkbox("IsEnableShadow", &isEnableShadow) == true)
					{
						pDirectionalLight->SetEnableShadow(isEnableShadow);
					}

					graphics::CascadedShadows& cascadedShadows = pDirectionalLight->GetCascadedShadows();
					graphics::CascadedShadowsConfig config = cascadedShadows.GetConfig();

					ImGui::DragInt("Cascades Count", reinterpret_cast<int*>(&config.numCascades), 0.01f, 1, graphics::CascadedShadowsConfig::eMaxCascades);

					ImGui::InputInt("Resolution", reinterpret_cast<int*>(&config.resolution));

					int pcfBlurSize = static_cast<int>(config.pcfBlurSize);
					if (ImGui::InputInt("PCF Blur Size", &pcfBlurSize, 2) == true)
					{
						config.pcfBlurSize = static_cast<uint32_t>(std::max(pcfBlurSize, 1));
					}

					ImGui::DragFloat("Cascade Distance", &config.cascadeDistance, 0.1f, 32.f, 2048.f);

					ImGui::DragFloat("Depth Bias", &config.depthBias, 0.0000001f, 0.f, 1.f, "%.6f");

					cascadedShadows.SetConfig(config);

					ImGui::PopID();

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}
		/*gameobject::Sun* pActor = static_cast<gameobject::Sun*>(gameobject::ActorManager::GetInstance()->GetActor(sid::est_Sun));
		auto pLight = pActor->GetLight();
		{
		static float f = pLight->GetIntensity();
		if (ImGui::DragFloat("Light Intensity", &f, 0.01f, 0.f, 20000.f) == true)
		{
		pLight->SetIntensity(f);
		}
		}

		{
		static float f = pLight->GetAmbientIntensity();
		if (ImGui::DragFloat("Light Ambient Intensity", &f, 0.01f, 0.f, 100.f) == true)
		{
		pLight->SetAmbientIntensity(f);
		}
		}

		{
		static float f = pLight->GetReflectionIntensity();
		if (ImGui::DragFloat("Light Reflection Intensity", &f, 0.01f, 0.f, 100.f) == true)
		{
		pLight->SetReflectionIntensity(f);
		}
		}

		{
		static ImVec4 color = ImColor(*reinterpret_cast<const ImVec4*>(&pLight->GetColor()));
		if (ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&color)) == true)
		{
		math::Color changeColor = *reinterpret_cast<math::Color*>(&color);
		pLight->SetColor(changeColor);
		}
		}

		math::float3 f3LightPos = pActor->GetPosition();
		if (ImGui::DragFloat3("Light Position", reinterpret_cast<float*>(&f3LightPos.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
		pActor->SetPosition(f3LightPos);
		}*/
	}

	if (ImGui::CollapsingHeader("Camera") == true)
	{
		graphics::Camera& camera = graphics::GetCamera();

		math::float3 cameraPosition = camera.GetPosition();
		if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&cameraPosition.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			camera.SetPosition(cameraPosition);
		}

		math::float3 cameraLookat = camera.GetLookat();
		if (ImGui::DragFloat3("Lookat", reinterpret_cast<float*>(&cameraLookat.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			camera.SetLookat(cameraLookat);
		}

		math::float3 cameraUp = camera.GetUp();
		if (ImGui::DragFloat3("Up", reinterpret_cast<float*>(&cameraUp.x), 0.01f, -1.f, 1.f) == true)
		{
			camera.SetUp(cameraUp);
		}

		if (ImGui::TreeNode("CameraMan"))
		{
			ImGui::PushID("CameraMan");

			enum CameraMan
			{
				eNone = 0,
				eFirstPerson,
				eThirdPerson,
			};

			static int selectedIndex = -1;
			if (selectedIndex == -1)
			{
				graphics::ICameraMan* pCameraMan = camera.GetCameraMan();
				if (pCameraMan == nullptr)
				{
					selectedIndex = CameraMan::eNone;
				}
				else if (dynamic_cast<graphics::FirstPersonCameraMan*>(pCameraMan) != nullptr)
				{
					selectedIndex = CameraMan::eFirstPerson;
				}
				else if (dynamic_cast<graphics::ThirdPersonCameraMan*>(pCameraMan) != nullptr)
				{
					selectedIndex = CameraMan::eThirdPerson;
				}
				else
				{
					throw_line("unknown camera man");
				}
			}

			if (ImGui::RadioButton("None", &selectedIndex, CameraMan::eNone) == true)
			{
				camera.SetCameraMan(nullptr);
			}

			if (ImGui::RadioButton("FirstPerson", &selectedIndex, CameraMan::eFirstPerson) == true)
			{
				camera.SetFirstPersonCameraMan();
				graphics::FirstPersonCameraMan* pCameraMan = static_cast<graphics::FirstPersonCameraMan*>(camera.GetCameraMan());
				pCameraMan->SetRotation({});
			}

			if (ImGui::RadioButton("ThirdPerson", &selectedIndex, CameraMan::eThirdPerson) == true)
			{
				camera.SetThirdPersonCameraMan();
				graphics::ThirdPersonCameraMan* pCameraMan = static_cast<graphics::ThirdPersonCameraMan*>(camera.GetCameraMan());
				pCameraMan->SetRotation({});
			}

			switch (selectedIndex)
			{
			case CameraMan::eNone:
				break;
			case CameraMan::eFirstPerson:
			{
				graphics::FirstPersonCameraMan* pCameraMan = static_cast<graphics::FirstPersonCameraMan*>(camera.GetCameraMan());
				graphics::FirstPersonCameraMan::DescMove descMove = pCameraMan->GetDescMove();

				const bool isChanged1 = ImGui::DragFloat("MoveSpeed", &descMove.moveSpeed, 0.01f, 1.f, 100.f);
				const bool isChanged2 = ImGui::DragFloat("RotateSpeed", &descMove.rotateSpeed, 0.01f, 1.f, 100.f);

				if (isChanged1 == true || isChanged2 == true)
				{
					pCameraMan->SetDescMove(descMove);
				}

				math::float3 MoveByDirection = pCameraMan->GetMoveByDirection();
				if (ImGui::DragFloat3("MoveByDirection", reinterpret_cast<float*>(&MoveByDirection.x), 0.01f, 0, 10000.f) == true)
				{
					const math::float3 diff = MoveByDirection - pCameraMan->GetMoveByDirection();
					pCameraMan->MoveForward(diff.z);
					pCameraMan->MoveSideward(diff.x);
					pCameraMan->MoveUpward(diff.y);
				}

				math::float3 MoveByAxis = pCameraMan->GetMoveByAxis();
				if (ImGui::DragFloat3("MoveByAxis", reinterpret_cast<float*>(&MoveByAxis.x), 0.01f, 0, 10000.f) == true)
				{
					const math::float3 diff = MoveByAxis - pCameraMan->GetMoveByAxis();
					pCameraMan->MoveAxisX(diff.x);
					pCameraMan->MoveAxisY(diff.y);
					pCameraMan->MoveAxisZ(diff.z);
				}

				math::float3 RotateByAxis = pCameraMan->GetRotateByAxis();
				if (ImGui::DragFloat3("RotateByAxis", reinterpret_cast<float*>(&RotateByAxis.x), 0.f) == true)
				{
					const math::float3 diff = RotateByAxis - pCameraMan->GetRotateByAxis();
					pCameraMan->RotateAxisX(diff.x);
					pCameraMan->RotateAxisY(diff.y);
					pCameraMan->RotateAxisZ(diff.z);
				}

				math::float3 Rotation = pCameraMan->GetRotation();
				if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&Rotation.x), 0.f) == true)
				{
					pCameraMan->SetRotation(Rotation);
				}
			}
			break;
			case CameraMan::eThirdPerson:
			{
				graphics::ThirdPersonCameraMan* pCameraMan = static_cast<graphics::ThirdPersonCameraMan*>(camera.GetCameraMan());
				graphics::ThirdPersonCameraMan::DescMove descMove = pCameraMan->GetDescMove();

				const bool isChanged1 = ImGui::DragFloat("MoveSpeed", &descMove.moveSpeed, 0.01f, 1.f, 100.f);
				const bool isChanged2 = ImGui::DragFloat("RotateSpeed", &descMove.rotateSpeed, 0.01f, 1.f, 100.f);
				const bool isChanged3 = ImGui::DragFloatRange2("ClampDistance", &descMove.minDistance, &descMove.maxDistance, 0.01f, 1.f, 10000.f);

				if (isChanged1 == true || isChanged2 == true || isChanged3 == true)
				{
					pCameraMan->SetDescMove(descMove);
				}

				math::float3 TargetPosition = pCameraMan->GetTargetPosition();
				if (ImGui::DragFloat3("TargetPosition", reinterpret_cast<float*>(&TargetPosition.x), 0.01f, -10000.f, 10000.f) == true)
				{
					pCameraMan->SetTargetPosition(TargetPosition);
				}

				float distance = pCameraMan->GetDistance();
				ImGui::DragFloat("Distance", &distance, 0.f, descMove.minDistance, descMove.maxDistance);

				float moveDistance = pCameraMan->GetMoveDistance();
				if (ImGui::DragFloat("MoveDistance", &moveDistance, 0.01f, -1000.f, 1000.f) == true)
				{
					const float diff = moveDistance - pCameraMan->GetMoveDistance();
					pCameraMan->MoveDistance(diff);
				}

				math::float3 RotateByAxis = pCameraMan->GetRotateByAxis();
				if (ImGui::DragFloat3("RotateByAxis", reinterpret_cast<float*>(&RotateByAxis.x), 0.f) == true)
				{
					const math::float3 diff = RotateByAxis - pCameraMan->GetRotateByAxis();
					pCameraMan->RotateAxisX(diff.x);
					pCameraMan->RotateAxisY(diff.y);
					pCameraMan->RotateAxisZ(diff.z);
				}

				math::float3 Rotation = pCameraMan->GetRotation();
				ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&Rotation.x), 0.f);
			}
			break;
			}

			ImGui::PopID();

			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Physics") == true)
	{
		static bool isEnableShootObject = false;
		ImGui::Checkbox("ShootObject", &isEnableShootObject);

		if (isEnableShootObject == true)
		{
			static physics::RigidActorProperty rigidProperty;
			static bool isFirst = false;
			if (isFirst == false)
			{
				rigidProperty.material.dynamicFriction = 0.25f;
				rigidProperty.material.staticFriction = 0.25f;
				rigidProperty.material.restitution = 0.5f;

				rigidProperty.shape.flags = physics::IShape::eSimulationShape | physics::IShape::eSceneQueryShape;
				rigidProperty.shape.SetSphereGeometry(0.5f);
				rigidProperty.rigidAcotr.type = physics::IActor::eRigidDynamic;

				isFirst = true;
			}

			enum ShootingObjectGeometryType
			{
				eST_eSphere = 0,
				eST_eBox,
				eST_Capsule,
				eST_Random,
				eST_Count,
			};

			static ShootingObjectGeometryType emShootingObjectGeometryType = eST_eSphere;
			const std::array<const char*, eST_Count> ShootingObjectGeometryTypeName = { "Sphere", "Box", "Capsule", "Random" };

			if (ImGui::TreeNode("Property"))
			{
				ImGui::PushID("Property");

				if (ImGui::TreeNode("Material"))
				{
					ImGui::DragFloat("DynamicFriction", &rigidProperty.material.dynamicFriction, 0.01f, 0.f, std::numeric_limits<float>::max());
					ImGui::DragFloat("StaticFriction", &rigidProperty.material.staticFriction, 0.01f, 0.f, std::numeric_limits<float>::max());
					ImGui::DragFloat("Restitution", &rigidProperty.material.restitution, 0.01f, 0.01f, 1.f);

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Shape"))
				{
					ImGui::DragFloat("ContactOffset", &rigidProperty.shape.contactOffset, 0.01f, std::min(0.f, rigidProperty.shape.restOffset), std::numeric_limits<float>::max());
					ImGui::DragFloat("RestOffset", &rigidProperty.shape.restOffset, 0.01f, -std::numeric_limits<float>::max(), rigidProperty.shape.contactOffset);

					static float radius = 0.5;
					static math::float3 halfExtents = math::float3::One;
					static float halfHeight = 0.5f;

					const std::array<const char*, graphics::MotionLayers::eLayerCount> layers = { "Layer1", "Layer2", "Layer3", "Layer4", };
					if (ImGui::Combo("GeometryType", reinterpret_cast<int*>(&emShootingObjectGeometryType), ShootingObjectGeometryTypeName.data(), static_cast<int>(ShootingObjectGeometryTypeName.size())) == true)
					{
						switch (emShootingObjectGeometryType)
						{
						case ShootingObjectGeometryType::eST_eSphere:
						{
							radius = 0.5f;
							rigidProperty.shape.SetSphereGeometry(radius);
						}
						break;
						case ShootingObjectGeometryType::eST_eBox:
						{
							halfExtents = math::float3::One;
							rigidProperty.shape.SetBoxGeometry(halfExtents);
						}
						break;
						case ShootingObjectGeometryType::eST_Capsule:
						{
							radius = 0.5f;
							halfHeight = 0.5f;
							rigidProperty.shape.SetCapsuleGeometry(radius, halfHeight);
						}
						break;
						}
					}

					switch (emShootingObjectGeometryType)
					{
					case ShootingObjectGeometryType::eST_eSphere:
					{
						if (ImGui::DragFloat("Radius", &radius, 0.01f, 0.01f, 9999.f) == true)
						{
							rigidProperty.shape.SetSphereGeometry(radius);
						}
					}
					break;
					case ShootingObjectGeometryType::eST_eBox:
					{
						if (ImGui::DragFloat3("HalfExtents", reinterpret_cast<float*>(&halfExtents), 0.01f, 0.01f, 9999.f) == true)
						{
							rigidProperty.shape.SetBoxGeometry(halfExtents);
						}
					}
					break;
					case ShootingObjectGeometryType::eST_Capsule:
					{
						bool isChanged = false;
						isChanged |= ImGui::DragFloat("Radius", &radius, 0.01f, 0.01f, 9999.f) == true;
						isChanged |= ImGui::DragFloat("HalfHeight", &halfHeight, 0.01f, 0.01f, 9999.f);

						if (isChanged == true)
						{
							rigidProperty.shape.SetCapsuleGeometry(radius, halfHeight);
						}
					}
					break;
					}

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Rigid"))
				{
					if (ImGui::TreeNode("CenterMassLocalPose"))
					{
						ImGui::DragFloat4("Rotation", reinterpret_cast<float*>(&rigidProperty.rigidAcotr.dynamicProperty.centerMassLocalPose.rotation), 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
						ImGui::DragFloat3("Position", reinterpret_cast<float*>(&rigidProperty.rigidAcotr.dynamicProperty.centerMassLocalPose.position), 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
					}
					ImGui::DragFloat("Mass", &rigidProperty.rigidAcotr.dynamicProperty.mass, 0.01f, 0.f, std::numeric_limits<float>::max());
					ImGui::DragFloat3("MassSpaceInertiaTensor", reinterpret_cast<float*>(&rigidProperty.rigidAcotr.dynamicProperty.massSpaceInertiaTensor), 0.01f, 0.f, std::numeric_limits<float>::max());

					ImGui::DragFloat("LinearDamping", &rigidProperty.rigidAcotr.dynamicProperty.linearDamping, 0.01f, 0.f, std::numeric_limits<float>::max());
					ImGui::DragFloat("AngularDamping", &rigidProperty.rigidAcotr.dynamicProperty.angularDamping, 0.01f, 0.f, std::numeric_limits<float>::max());
					ImGui::DragFloat("MaxAngularVelocity", &rigidProperty.rigidAcotr.dynamicProperty.maxAngularVelocity, 0.01f, 0.f, std::numeric_limits<float>::max());
					ImGui::DragFloat("SleepThreshold", &rigidProperty.rigidAcotr.dynamicProperty.sleepThreshold, 0.01f, 0.f, std::numeric_limits<float>::max());
					ImGui::DragFloat("StabilizationThreshold", &rigidProperty.rigidAcotr.dynamicProperty.stabilizationThreshold, 0.01f, 0.f, std::numeric_limits<float>::max());
					ImGui::DragFloat("ContactReportThreshold", &rigidProperty.rigidAcotr.dynamicProperty.contactReportThreshold, 0.01f, 0.f, std::numeric_limits<float>::max());

					ImGui::TreePop();
				}

				ImGui::PopID();

				ImGui::TreePop();
			}

			ImGui::Text("Shoot : Spacebar");

			static float shootingSpeed = 50.f;
			ImGui::DragFloat("ShootingSpeed", &shootingSpeed, 0.01f, 0.01f, 9999.f);

			if (input::keyboard::IsKeyDown(input::keyboard::eSpace) == true)
			{
				static size_t shootingObjectID = 0;
				const string::StringID shootingObjectName = string::Format(L"ShootingModel_%d", shootingObjectID++);

				graphics::Camera& camera = graphics::GetCamera();

				const math::float3& position = camera.GetPosition();

				math::float3 velocity = camera.GetDirection();
				velocity.Normalize();
				velocity *= shootingSpeed;

				gameobject::ActorPtr pShootingObject = gameobject::CreateActor(shootingObjectName);
				gameobject::ComponentModel* pCompModel = static_cast<gameobject::ComponentModel*>(pShootingObject->CreateComponent(gameobject::IComponent::eModel));

				graphics::IMaterial::Data materialData;
				materialData.name = shootingObjectName;

				materialData.paddingRoughMetEmi.y = math::RandomReal(0.f, 1.f);
				materialData.paddingRoughMetEmi.z = math::RandomReal(0.f, 1.f);
				materialData.colorAlbedo = math::Color(math::RandomReal(0.f, 1.f), math::RandomReal(0.f, 1.f), math::RandomReal(0.f, 1.f), 1.f);

				graphics::ModelLoader loader;
				switch (emShootingObjectGeometryType)
				{
				case ShootingObjectGeometryType::eST_eSphere:
				{
					const physics::SphereGeometry* pSphereGeometry = static_cast<const physics::SphereGeometry*>(rigidProperty.shape.pGeometry.get());
					loader.InitSphere(shootingObjectName, &materialData, pSphereGeometry->radius * 2.f);
				}
				break;
				case ShootingObjectGeometryType::eST_eBox:
				{
					const physics::BoxGeometry* pBoxGeometry = static_cast<const physics::BoxGeometry*>(rigidProperty.shape.pGeometry.get());
					loader.InitBox(shootingObjectName, &materialData, pBoxGeometry->halfExtents);
				}
				break;
				case ShootingObjectGeometryType::eST_Capsule:
				{
					const physics::CapsuleGeometry* pCapsuleGeometry = static_cast<const physics::CapsuleGeometry*>(rigidProperty.shape.pGeometry.get());
					loader.InitCapsule(shootingObjectName, &materialData, pCapsuleGeometry->radius, pCapsuleGeometry->halfHeight * 2.f);
				}
				break;
				case ShootingObjectGeometryType::eST_Random:
				{
					const ShootingObjectGeometryType emSelectedGeometryType = static_cast<ShootingObjectGeometryType>(math::Random(static_cast<int>(eST_eSphere), static_cast<int>(eST_Capsule)));
					switch (emSelectedGeometryType)
					{
					case ShootingObjectGeometryType::eST_eSphere:
						loader.InitSphere(shootingObjectName, &materialData, 1.f);
						rigidProperty.shape.SetSphereGeometry(0.5f);
						break;
					case ShootingObjectGeometryType::eST_eBox:
						loader.InitBox(shootingObjectName, &materialData, math::float3::One);
						rigidProperty.shape.SetBoxGeometry(math::float3::One);
						break;
					case ShootingObjectGeometryType::eST_Capsule:
						loader.InitCapsule(shootingObjectName, &materialData, 0.5f, 1.f);
						rigidProperty.shape.SetCapsuleGeometry(0.5f, 0.5f);
						break;
					}
				}
				break;
				}

				loader.SetEnableThreadLoad(false);
				pCompModel->Add(0, loader);

				auto pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pShootingObject->CreateComponent(gameobject::IComponent::ePhysics));
				rigidProperty.rigidAcotr.name = shootingObjectName;
				rigidProperty.rigidAcotr.globalTransform.position = position;

				physics::IRigidDynamic* pRigidDynamic = static_cast<physics::IRigidDynamic*>(pCompPhysics->CreateRigidActor(rigidProperty));
				pCompPhysics->SetLinearVelocity(velocity);

				m_actors.emplace_back(std::move(pShootingObject));
			}
		}
	}

	ImGui::End();
}

void SceneStudio::ShowMotion(bool& isShowMotionMenu, gameobject::ComponentModel* pCompModel)
{
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Motion System", &isShowMotionMenu);

	ImGui::PushID("MotionSystem");

	if (ImGui::Button("Load Motion") == true)
	{
		const size_t bufferSize = 8192;
		wchar_t path[bufferSize]{};

		OPENFILENAME ofn;
		memory::Clear(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = graphics::GetHwnd();
		ofn.lpstrFilter = L"Motion File(*.fbx;*.emot)\0*.fbx;*.emot\0FBX File(*.fbx)\0*.fbx\0EastMotion File(*.emot)\0*.emot\0";
		ofn.lpstrFile = path;
		ofn.nMaxFile = bufferSize;
		ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;
		if (GetOpenFileName(&ofn) != 0)
		{
			wchar_t* ptr = path;
			ptr[ofn.nFileOffset - 1] = 0;

			const std::wstring directory = ptr;
			ptr += ofn.nFileOffset;

			std::vector<std::wstring> files;
			while (*ptr)
			{
				files.emplace_back(directory + L"\\" + ptr);
				ptr += (lstrlen(ptr) + 1);
			}

			for (auto& filePath : files)
			{
				const std::wstring strFileExtension = file::GetFileExtension(filePath);
				if (string::IsEqualsNoCase(strFileExtension.c_str(), L".fbx") == true)
				{
					graphics::MotionLoader loader;
					loader.InitFBX(file::GetFileName(filePath).c_str(), filePath.c_str());
					graphics::CreateMotion(loader);
				}
				else if (string::IsEqualsNoCase(strFileExtension.c_str(), L".emot") == true)
				{
					graphics::MotionLoader loader;
					loader.InitEast(file::GetFileName(filePath).c_str(), filePath.c_str());
					graphics::CreateMotion(loader);
				}
			}
		}
	}

	const size_t motionCount = graphics::ModelManager::GetInstance()->GetMotionCount();

	std::vector<std::string> motionNamesMulti;
	motionNamesMulti.reserve(motionCount);

	for (size_t i = 0; i < motionCount; ++i)
	{
		graphics::MotionPtr pMotion = graphics::ModelManager::GetInstance()->GetMotion(i);
		motionNamesMulti.emplace_back(string::WideToMulti(pMotion->GetName().c_str()));
	}

	std::vector<const char*> motionNames;
	motionNames.reserve(motionCount);
	for (auto& name : motionNamesMulti)
	{
		motionNames.emplace_back(name.c_str());
	}

	{
		static graphics::MotionLayers emLayer = graphics::MotionLayers::eLayer1;
		static float fMotionSpeed = 1.f;
		static float fMotionWeight = 1.f;
		static float fMotionBlendTime = 0.f;
		static bool isMotionLoop = false;
		static bool isMotionInverse = false;
		static bool isFreezeAtLastFrame = false;

		static int selectedIndex = 0;
		if (motionNames.empty() == false)
		{
			int prevSelectedIndex = -1;
			if (ImGui::ListBox("Motion List", &selectedIndex, &motionNames.front(), static_cast<int>(motionNames.size()), 6) == true)
			{
				if (prevSelectedIndex == selectedIndex)
				{
					graphics::MotionPlaybackInfo playback;
					playback.speed = fMotionSpeed;
					playback.weight = fMotionWeight;
					playback.blendTime = fMotionBlendTime;
					playback.loopCount = isMotionLoop == true ? graphics::MotionPlaybackInfo::eMaxLoopCount : 1;
					playback.isInverse = isMotionInverse;
					playback.isFreezeAtLastFrame = isFreezeAtLastFrame;

					graphics::MotionPtr pMotion;
					if (0 <= selectedIndex && selectedIndex < static_cast<int>(motionNames.size()))
					{
						pMotion = graphics::ModelManager::GetInstance()->GetMotion(selectedIndex);
					}

					if (pMotion != nullptr)
					{
						pCompModel->PlayMotion(0, emLayer, pMotion, &playback);
					}
				}

				prevSelectedIndex = selectedIndex;
			}
		}

		graphics::MotionPtr pMotion = nullptr;
		if (0 <= selectedIndex && selectedIndex < static_cast<int>(motionNames.size()))
		{
			pMotion = graphics::ModelManager::GetInstance()->GetMotion(selectedIndex);
		}

		if (pMotion != nullptr)
		{
			const std::array<const char*, graphics::MotionLayers::eLayerCount> layers = { "Layer1", "Layer2", "Layer3", "Layer4", };

			ImGui::Combo("Layer", reinterpret_cast<int*>(&emLayer), layers.data(), static_cast<int>(layers.size()));

			ImGui::DragFloat("Speed", &fMotionSpeed, 0.001f, 0.001f, 10.f);
			ImGui::DragFloat("Weight", &fMotionWeight, 0.001f, 0.f, 1.f);
			ImGui::DragFloat("BlendTime", &fMotionBlendTime, 0.001f, 0.f, pMotion->GetEndTime());
			ImGui::Checkbox("Loop", &isMotionLoop);

			ImGui::SameLine();

			ImGui::Checkbox("Invert", &isMotionInverse);
			ImGui::Checkbox("FreezeAtLastFrame", &isFreezeAtLastFrame);

			if (ImGui::Button("Play") == true)
			{
				graphics::MotionPlaybackInfo playback;
				playback.speed = fMotionSpeed;
				playback.weight = fMotionWeight;
				playback.blendTime = fMotionBlendTime;
				playback.loopCount = isMotionLoop == true ? graphics::MotionPlaybackInfo::eMaxLoopCount : 1;
				playback.isInverse = isMotionInverse;
				playback.isFreezeAtLastFrame = isFreezeAtLastFrame;

				pCompModel->PlayMotion(0, emLayer, pMotion, &playback);
			}

			ImGui::Separator();
		}
	}

	graphics::IMotionSystem* pMotionSystem = pCompModel->GetModelInstance(0)->GetMotionSystem();
	if (pMotionSystem != nullptr)
	{
		for (int i = 0; i < graphics::MotionLayers::eLayerCount; ++i)
		{
			const graphics::MotionLayers emLayer = static_cast<graphics::MotionLayers>(i);
			graphics::IMotionPlayer* pPlayer = pMotionSystem->GetPlayer(emLayer);

			std::string strLayer = string::Format("Layer%d", i);

			ImGui::PushID(strLayer.c_str());

			ImGui::Text(strLayer.c_str());

			graphics::MotionPtr pMotionPlaying = pPlayer->GetMotion();
			if (pMotionPlaying != nullptr)
			{
				const std::string motionName = string::WideToMulti(pMotionPlaying->GetName().c_str());

				char buf[128]{};
				string::Copy(buf, sizeof(buf), motionName.c_str());
				ImGui::InputText("Name", buf, sizeof(buf), ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);

				if (ImGui::Button("Stop") == true)
				{
					pCompModel->StopMotion(0, emLayer, 0.3f);
				}

				ImGui::SameLine();

				bool isMotionPause = pPlayer->IsPause();
				ImGui::Checkbox("Pause", &isMotionPause);
				pPlayer->SetPause(isMotionPause);

				float fMotionSpeed = pPlayer->GetSpeed();
				ImGui::DragFloat("Speed", &fMotionSpeed, 0.001f, 0.001f, 10.f);
				pPlayer->SetSpeed(fMotionSpeed);

				float fMotionWeight = pPlayer->GetWeight();
				ImGui::DragFloat("Weight", &fMotionWeight, 0.001f, 0.f, 1.f);
				pPlayer->SetWeight(fMotionWeight);

				const std::string strBuf = string::Format("%.2f/%.2f", pPlayer->GetBlendWeight(), pPlayer->GetWeight());
				ImGui::ProgressBar(pPlayer->GetBlendWeight() / pPlayer->GetWeight(), ImVec2(0.0f, 0.0f), strBuf.c_str());
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("BlendWeight");

				float fMotionPlayTime = pPlayer->GetPlayTime();
				float fEndTime = pMotionPlaying->GetEndTime();
				if (pPlayer->IsInverse() == true)
				{
					const std::string strBuf2 = string::Format("%.2f/%.2f", fEndTime - fMotionPlayTime, fEndTime);
					ImGui::ProgressBar((fEndTime - fMotionPlayTime) / fEndTime, ImVec2(0.0f, 0.0f), strBuf2.c_str());
				}
				else
				{
					const std::string strBuf2 = string::Format("%.2f/%.2f", fMotionPlayTime, fEndTime);
					ImGui::ProgressBar(fMotionPlayTime / fEndTime, ImVec2(0.0f, 0.0f), strBuf2.c_str());
				}
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("Play Time");
			}
			else
			{
				ImGui::Text("Empty");
			}

			ImGui::Separator();

			ImGui::PopID();
		}
	}

	ImGui::PopID();

	ImGui::End();
}

void SceneStudio::ShowMaterial(bool& isShowMaterial, graphics::IMaterial* pMaterial, int index)
{
	ImGui::SetNextWindowSize(ImVec2(400, 800), ImGuiCond_FirstUseEver);
	ImGui::Begin(string::Format("%d. %s Info", index, pMaterial->GetName().c_str()).c_str(), &isShowMaterial);

	const std::string materialName = string::WideToMulti(pMaterial->GetName().c_str());

	static char bufName[128]{};
	string::Copy(bufName, materialName.c_str());

	ImGui::PushID(bufName);

	if (ImGui::InputText("Name", bufName, sizeof(bufName), ImGuiInputTextFlags_EnterReturnsTrue) == true)
	{
		pMaterial->SetName(bufName);
	}

	const std::string materialPath = string::WideToMulti(pMaterial->GetName().c_str());

	char buf[1024]{};
	string::Copy(buf, materialPath.c_str());
	ImGui::InputText("Path", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

	float tessellationFactor = pMaterial->GetTessellationFactor();
	if (ImGui::DragFloat("TessellationFactor", &tessellationFactor, 0.01f, 1.f, 512.f) == true)
	{
		pMaterial->SetTessellationFactor(tessellationFactor);
	}

	float stippleTransparencyFactor = pMaterial->GetStippleTransparencyFactor();
	if (ImGui::DragFloat("StippleTransparencyFactor", &stippleTransparencyFactor, 0.01f, 0.f, 1.f) == true)
	{
		pMaterial->SetStippleTransparencyFactor(stippleTransparencyFactor);
	}

	const char* strSamplerState[graphics::SamplerState::TypeCount] =
	{
		"MinMagMipLinearWrap",
		"MinMagMipLinearClamp",
		"MinMagMipLinearBorder",
		"MinMagMipLinearMirror",
		"MinMagMipLinearMirrorOnce",
		"MinMagLinearMipPointWrap",
		"MinMagLinearMipPointClamp",
		"MinMagLinearMipPointBorder",
		"MinMagLinearMipPointMirror",
		"MinMagLinearMipPointMirrorOnce",
		"AnisotropicWrap",
		"AnisotropicClamp",
		"AnisotropicBorder",
		"AnisotropicMirror",
		"AnisotropicMirrorOnce",
		"MinMagMipPointWrap",
		"MinMagMipPointClamp",
		"MinMagMipPointBorder",
		"MinMagMipPointMirror",
		"MinMagMipPointMirrorOnce",
	};
	int nSamplerState = pMaterial->GetSamplerState();
	if (ImGui::Combo("SamplerState", &nSamplerState, strSamplerState, graphics::SamplerState::TypeCount) == true)
	{
		pMaterial->SetSamplerState(static_cast<graphics::SamplerState::Type>(nSamplerState));
	}

	const char* strBlendState[graphics::BlendState::TypeCount] =
	{
		"Off",
		"Linear",
		"Additive",
		"SubTractive",
		"Multiplicative",
		"Squared",
		"Negative",
		"Opacity",
		"AlphaBlend",
	};
	int nBlendState = pMaterial->GetBlendState();
	if (ImGui::Combo("BlendState", &nBlendState, strBlendState, graphics::BlendState::TypeCount) == true)
	{
		pMaterial->SetBlendState(static_cast<graphics::BlendState::Type>(nBlendState));
	}

	const char* strRasterizerState[graphics::RasterizerState::TypeCount] =
	{
		"SolidCCW",
		"SolidCW",
		"SolidCullNone",
		"WireframeCCW",
		"WireframeCW",
		"WireframeCullNone",
	};
	int nRasterizerState = pMaterial->GetRasterizerState();
	if (ImGui::Combo("RasterizerState", reinterpret_cast<int*>(&nRasterizerState), strRasterizerState, graphics::RasterizerState::TypeCount) == true)
	{
		pMaterial->SetRasterizerState(static_cast<graphics::RasterizerState::Type>(nRasterizerState));
	}

	const char* strDepthStencilState[graphics::DepthStencilState::TypeCount] =
	{
		"Read_Write_On",
		"Read_Write_Off",
		"Read_On_Write_Off",
		"Read_Off_Write_On",
	};
	int nDepthStencilState = pMaterial->GetDepthStencilState();
	if (ImGui::Combo("DepthStencilState", &nDepthStencilState, strDepthStencilState, graphics::DepthStencilState::TypeCount) == true)
	{
		pMaterial->SetDepthStencilState(static_cast<graphics::DepthStencilState::Type>(nDepthStencilState));
	}

	bool isVisible = pMaterial->IsVisible();
	if (ImGui::Checkbox("Vibisle", &isVisible) == true)
	{
		pMaterial->SetVisible(isVisible);
	}

	auto TextureInfo = [&](graphics::IMaterial::Type emType, int index)
	{
		if (ImGui::Button(string::Format("%d.Texture", index).c_str()) == true)
		{
			wchar_t path[512]{};

			OPENFILENAME ofn;
			memory::Clear(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = graphics::GetHwnd();
			ofn.lpstrFilter = L"Every File(*.*)\0*.*\0Text File\0*.txt;*.doc\0";
			ofn.lpstrFile = path;
			ofn.nMaxFile = 256;
			if (GetOpenFileName(&ofn) != 0)
			{
				graphics::TexturePtr pTexture = graphics::CreateTextureAsync(ofn.lpstrFile);
				pMaterial->SetTextureName(emType, file::GetFileName(ofn.lpstrFile).c_str());
				pMaterial->SetTexture(emType, pTexture);
			}
		}

		const std::string name = string::WideToMulti(pMaterial->GetTextureName(emType).c_str());

		char buf[1024]{};
		string::Copy(buf, name.c_str());
		ImGui::SameLine();
		ImGui::InputText("", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

		graphics::TexturePtr pTexture = pMaterial->GetTexture(emType);
		if (pTexture != nullptr)
		{
			if (ImGui::Button(string::Format("%d.Clear", index).c_str()) == true)
			{
				pMaterial->SetTextureName(emType, "");
				pMaterial->SetTexture(emType, nullptr);
			}

			ImGui::SameLine();
			if (pTexture->GetState() == graphics::IResource::eComplete)
			{
				static std::unordered_map<string::StringID, bool> umapIsShowBigTexture;

				bool& isShow = umapIsShowBigTexture[pTexture->GetName()];

				ImTextureID textureID = imguiHelper::GetTextureID(pTexture.get());
				if (ImGui::ImageButton(textureID, ImVec2(64.f, 64.f)) == true)
				{
					isShow = !isShow;
				}

				if (isShow == true)
				{
					ImVec2 f2Size(static_cast<float>(pTexture->GetSize().x), static_cast<float>(pTexture->GetSize().y));
					f2Size.x = std::min(f2Size.x, 512.f);
					f2Size.y = std::min(f2Size.y, 512.f);

					const std::string textureName = string::WideToMulti(pTexture->GetName().c_str());

					ImGui::SetNextWindowSize(f2Size, ImGuiCond_FirstUseEver);
					ImGui::Begin(textureName.c_str(), &isShow, ImGuiWindowFlags_AlwaysAutoResize);
					ImGui::Image(textureID, f2Size);
					ImGui::End();
				}
			}
			else
			{
				ImGui::Text("Loading...");
			}
		}
	};

	ImGui::Separator();
	{
		ImGui::Text("Albedo");
		ImVec4 color = ImColor(*reinterpret_cast<const ImVec4*>(&pMaterial->GetAlbedoColor()));
		if (ImGui::ColorEdit3("Albedo", reinterpret_cast<float*>(&color)) == true)
		{
			pMaterial->SetAlbedoColor(*reinterpret_cast<math::Color*>(&color));

			LOG_MESSAGE(L"%.2f, %.2f, %.2f", pMaterial->GetAlbedoColor().r, pMaterial->GetAlbedoColor().g, pMaterial->GetAlbedoColor().b);
		}
		TextureInfo(graphics::IMaterial::eAlbedo, 1);
		bool isAlbedoAlphaChannelMaskMap = pMaterial->IsAlbedoAlphaChannelMaskMap();
		if (ImGui::Checkbox("Is Albedo Alpha Channel a Mask Map ?", &isAlbedoAlphaChannelMaskMap) == true)
		{
			pMaterial->SetAlbedoAlphaChannelMaskMap(isAlbedoAlphaChannelMaskMap);
		}
	}

	ImGui::Separator();
	{
		ImGui::Text("Normal");
		TextureInfo(graphics::IMaterial::eNormal, 2);
	}

	ImGui::Separator();
	{
		ImGui::Text("Specular");
		float fSpecular = pMaterial->GetSpecular();
		if (ImGui::DragFloat("Specular", &fSpecular, 0.001f, 0.f, 1.f) == true)
		{
			pMaterial->SetSpecular(fSpecular);
		}

		float fSpecularTint = pMaterial->GetSpecularTint();
		if (ImGui::DragFloat("SpecularTint", &fSpecularTint, 0.001f, 0.f, 1.f) == true)
		{
			pMaterial->SetSpecularTint(fSpecularTint);
		}
	}

	ImGui::Separator();
	{
		ImGui::Text("Roughness");

		if (pMaterial->GetTexture(graphics::IMaterial::eRoughness) == nullptr)
		{
			float fRoughness = pMaterial->GetRoughness();
			if (ImGui::DragFloat("Roughness", &fRoughness, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetRoughness(fRoughness);
			}
		}
		TextureInfo(graphics::IMaterial::eRoughness, 4);
	}

	ImGui::Separator();
	{
		ImGui::Text("Metallic");

		if (pMaterial->GetTexture(graphics::IMaterial::eMetallic) == nullptr)
		{
			float fMetallic = pMaterial->GetMetallic();
			if (ImGui::DragFloat("Metallic", &fMetallic, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetMetallic(fMetallic);
			}
		}

		TextureInfo(graphics::IMaterial::eMetallic, 5);
	}

	ImGui::Separator();
	{
		ImGui::Text("Subsurface");

		if (pMaterial->GetTexture(graphics::IMaterial::eSubsurface) == nullptr)
		{
			float fSubsurface = pMaterial->GetSubsurface();
			if (ImGui::DragFloat("Subsurface", &fSubsurface, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetSubsurface(fSubsurface);
			}
		}

		TextureInfo(graphics::IMaterial::eSubsurface, 6);
	}

	ImGui::Separator();
	{
		ImGui::Text("Anisotropic");

		if (pMaterial->GetTexture(graphics::IMaterial::eAnisotropic) == nullptr)
		{
			float fAnisotropic = pMaterial->GetAnisotropic();
			if (ImGui::DragFloat("Anisotropic", &fAnisotropic, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetAnisotropic(fAnisotropic);
			}
		}

		TextureInfo(graphics::IMaterial::eAnisotropic, 7);
	}

	ImGui::Separator();
	{
		ImGui::Text("Sheen");

		if (pMaterial->GetTexture(graphics::IMaterial::eSheen) == nullptr)
		{
			float fSheen = pMaterial->GetSheen();
			if (ImGui::DragFloat("Sheen", &fSheen, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetSheen(fSheen);
			}
		}

		TextureInfo(graphics::IMaterial::eSheen, 8);
	}

	ImGui::Separator();
	{
		ImGui::Text("SheenTint");

		if (pMaterial->GetTexture(graphics::IMaterial::eSheenTint) == nullptr)
		{
			float fSheenTint = pMaterial->GetSheenTint();
			if (ImGui::DragFloat("SheenTint", &fSheenTint, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetSheenTint(fSheenTint);
			}
		}

		TextureInfo(graphics::IMaterial::eSheenTint, 9);
	}

	ImGui::Separator();
	{
		ImGui::Text("Clearcoat");

		if (pMaterial->GetTexture(graphics::IMaterial::eClearcoat) == nullptr)
		{
			float fClearcoat = pMaterial->GetClearcoat();
			if (ImGui::DragFloat("Clearcoat", &fClearcoat, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetClearcoat(fClearcoat);
			}
		}

		TextureInfo(graphics::IMaterial::eClearcoat, 10);
	}

	ImGui::Separator();
	{
		ImGui::Text("ClearcoatGloss");

		if (pMaterial->GetTexture(graphics::IMaterial::eClearcoatGloss) == nullptr)
		{
			float fClearcoatGloss = pMaterial->GetClearcoatGloss();
			if (ImGui::DragFloat("ClearcoatGloss", &fClearcoatGloss, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetClearcoatGloss(fClearcoatGloss);
			}
		}

		TextureInfo(graphics::IMaterial::eClearcoatGloss, 11);
	}

	ImGui::Separator();
	{
		ImGui::Text("Emissive");
		ImVec4 color = ImColor(*reinterpret_cast<const ImVec4*>(&pMaterial->GetEmissiveColor()));
		if (ImGui::ColorEdit3("Emissive Color", reinterpret_cast<float*>(&color)) == true)
		{
			pMaterial->SetEmissiveColor(*reinterpret_cast<math::Color*>(&color));
		}
		TextureInfo(graphics::IMaterial::eEmissiveColor, 12);

		float fEmissiveIntensity = pMaterial->GetEmissive();
		if (ImGui::DragFloat("Emissive Intensity", &fEmissiveIntensity, 0.01f, 0.f, 10000.f) == true)
		{
			pMaterial->SetEmissive(fEmissiveIntensity);
		}
	}

	ImGui::Separator();

	ImGui::PopID();

	ImGui::End();
}

void SceneStudio::ShowSoundWindow(bool& isShowSoundMenu)
{
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Sound System", &isShowSoundMenu);

	ImGui::PushID("SoundSystem");

	static std::vector<std::wstring> files;
	if (files.empty() == true)
	{
		std::wstring soundPath = file::GetEngineDataPath();
		soundPath += L"Sound\\";
		file::GetFiles(soundPath, L".*", files);
	}

	static std::vector<std::string> soundFilePaths;
	std::vector<std::string> fileNames;
	std::vector<const char*> names;
	if (soundFilePaths.empty() == true)
	{
		soundFilePaths.reserve(files.size());

		for (auto& filePath : files)
		{
			soundFilePaths.emplace_back(string::WideToMulti(filePath).c_str());
		}
	}

	for (auto& filePath : soundFilePaths)
	{
		fileNames.emplace_back(file::GetFileName(filePath));
	}

	for (auto& fileName : fileNames)
	{
		names.emplace_back(fileName.c_str());
	}

	static int selectedIndex = 0;
	if (names.empty() == false)
	{
		static sound::ChannelID channelID(sound::ChannelID::Default());
		if (ImGui::ListBox("Sound List", &selectedIndex, &names.front(), static_cast<int>(names.size()), 6) == true)
		{
			sound::Stop(channelID, 1.f);
			channelID = sound::Play2D(files[selectedIndex], 0.5f);
		}
	}

	ImGui::PopID();

	ImGui::End();
}

void SceneStudio::ShowActorWindow(bool& isShowActorMenu)
{
	if (isShowActorMenu == true)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);

		ImGui::Begin("Actor", &isShowActorMenu);

		ImGui::PushID("Actor");

		const size_t actorCount = m_actors.size();

		std::vector<std::string> actorNames;
		actorNames.reserve(actorCount);

		for (auto& pActor : m_actors)
		{
			actorNames.emplace_back(string::WideToMulti(pActor->GetName().c_str()));
		}

		static int selectedIndex = 0;
		if (actorNames.empty() == false)
		{
			if (selectedIndex >= actorNames.size())
			{
				selectedIndex = 0;
			}

			std::vector<const char*> names;
			for (auto& name : actorNames)
			{
				names.emplace_back(name.c_str());
			}

			ImGui::ListBox("List", &selectedIndex, &names.front(), static_cast<int>(names.size()), 4);
		}

		gameobject::IActor* pActor = nullptr;
		if (0 <= selectedIndex && selectedIndex < static_cast<int>(actorNames.size()))
		{
			pActor = m_actors[selectedIndex].get();
		}

		if (ImGui::Button("Add") == true)
		{
			ImGui::OpenPopup("Input Actor Name");
		}

		ImGui::SameLine();

		if (ImGui::Button("Remove") == true)
		{
			if (pActor == nullptr)
			{
				ImGui::OpenPopup("Select Actor");
			}
			else
			{
				ImGui::OpenPopup("Is Remove Actor?");
			}
		}

		if (ImGui::BeginPopupModal("Input Actor Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize) == true)
		{
			static char buf[128] = "Actor";
			ImGui::InputText("Input Name", buf, sizeof(buf));

			if (ImGui::Button("Confirm") == true)
			{
				if (string::Length(buf) > 1)
				{
					gameobject::CreateActor(buf);
				}
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel") == true)
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Is Remove Actor?", nullptr, ImGuiWindowFlags_AlwaysAutoResize) == true)
		{
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				if (pActor != nullptr)
				{
					auto iter = std::find_if(m_actors.begin(), m_actors.end(), [pActor](const gameobject::ActorPtr& pFindActor)
						{
							return pFindActor.get() == pActor;
						});

					if (iter != m_actors.end())
					{
						m_actors.erase(iter);
					}
				}
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Select Actor", nullptr, ImGuiWindowFlags_AlwaysAutoResize) == true)
		{
			ImGui::Text("Please Select Actor");

			ImGui::EndPopup();
		}

		if (ImGui::CollapsingHeader("Common"))
		{
			if (pActor != nullptr)
			{
				const std::string actorName = string::WideToMulti(pActor->GetName().c_str());

				static char buf[128]{};
				string::Copy(buf, actorName.c_str());

				ImGui::PushID(buf);

				if (ImGui::InputText("Name", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue) == true)
				{
					pActor->SetName(buf);
				}

				math::float3 f3Scale = pActor->GetScale();
				if (ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&f3Scale.x), 0.01f, 0.01f, 100.f) == true)
				{
					pActor->SetScale(f3Scale);
				}

				math::float3 f3Pos = pActor->GetPosition();
				if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&f3Pos.x), 0.01f, -1000000.f, 1000000.f) == true)
				{
					pActor->SetPosition(f3Pos);
				}

				static std::unordered_map<gameobject::IActor*, math::float3> umapActorRotation;
				math::float3& f3Rotation = umapActorRotation[pActor];

				if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&f3Rotation.x), 0.1f, -360.f, 360.f) == true)
				{
					auto ResetRot = [](float& fRot)
					{
						if (fRot >= 360.f)
						{
							fRot -= 360.f;
						}
						else if (fRot <= -360.f)
						{
							fRot += 360.f;
						}
					};

					ResetRot(f3Rotation.x);
					ResetRot(f3Rotation.y);
					ResetRot(f3Rotation.z);

					math::Quaternion quatRotation = math::Quaternion::CreateFromYawPitchRoll(math::ToRadians(f3Rotation.y), math::ToRadians(f3Rotation.x), math::ToRadians(f3Rotation.z));

					pActor->SetRotation(quatRotation);
				}

				ImGui::Separator();

				std::vector<std::string> componentNamesMulti;
				for (int i = 0; i < gameobject::IComponent::TypeCount; ++i)
				{
					gameobject::IComponent::Type emType = static_cast<gameobject::IComponent::Type>(i);
					if (pActor->GetComponent(emType) == nullptr)
					{
						const wchar_t* componentName = gameobject::IComponent::ToString(emType);
						if (componentName != nullptr)
						{
							componentNamesMulti.emplace_back(string::WideToMulti(componentName));
						}
					}
				}

				static int nCurItem = 0;
				if (componentNamesMulti.empty() == false)
				{
					std::vector<const char*> componentNames;
					for (auto& name : componentNamesMulti)
					{
						componentNames.emplace_back(name.c_str());
					}

					ImGui::PushID("Component");

					static gameobject::IComponent::Type emType = gameobject::IComponent::TypeCount;
					nCurItem = std::min(nCurItem, static_cast<int>(componentNames.size() - 1));
					if (ImGui::Combo("Add Component", &nCurItem, &componentNames.front(), static_cast<int>(componentNames.size())) == true)
					{
						emType = static_cast<gameobject::IComponent::Type>(nCurItem);
						if (emType != gameobject::IComponent::TypeCount)
						{
							ImGui::OpenPopup("Component Add Confirm");
						}
					}

					if (ImGui::BeginPopupModal("Component Add Confirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize) == true)
					{
						if (ImGui::Button("Yes") == true)
						{
							pActor->CreateComponent(emType);
							ImGui::CloseCurrentPopup();
						}

						if (ImGui::Button("No") == true)
						{
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					ImGui::PopID();
				}

				ImGui::PopID();
			}
		}

		if (pActor != nullptr)
		{
			for (int i = 0; i < gameobject::IComponent::TypeCount; ++i)
			{
				gameobject::IComponent* pComponent = pActor->GetComponent(static_cast<gameobject::IComponent::Type>(i));
				if (pComponent == nullptr)
					continue;

				switch (i)
				{
				case gameobject::IComponent::eBehaviorTree:
				{
					if (ImGui::CollapsingHeader("ActionState"))
					{
					}
				}
				break;
				case gameobject::IComponent::eTimer:
				{
					if (ImGui::CollapsingHeader("Timer"))
					{
					}
				}
				break;
				case gameobject::IComponent::ePhysics:
				{
					if (ImGui::CollapsingHeader("Physics"))
					{
					}
				}
				break;
				case gameobject::IComponent::eModel:
				{
					if (ImGui::CollapsingHeader("Model"))
					{
						ImGui::PushID("Model");

						gameobject::ComponentModel* pCompModel = static_cast<gameobject::ComponentModel*>(pComponent);
						if (pCompModel->GetModelInstance(0) != nullptr)
						{
							if (pCompModel->IsLoadComplete(0) == true)
							{
								graphics::IModel* pModel = pCompModel->GetModelInstance(0)->GetModel();

								bool isVisibleModel = pModel->IsVisible();
								if (ImGui::Checkbox("Visible", &isVisibleModel) == true)
								{
									pModel->SetVisible(isVisibleModel);
								}

								math::float3 f3Scale = pModel->GetLocalScale();
								if (ImGui::DragFloat3("Local Scale", reinterpret_cast<float*>(&f3Scale), 0.01f, 0.01f, 100.f) == true)
								{
									pModel->SetLocalScale(f3Scale);
								}

								math::float3 f3Pos = pModel->GetLocalPosition();
								if (ImGui::DragFloat3("Local Position", reinterpret_cast<float*>(&f3Pos), 0.01f, -1000000.f, 1000000.f) == true)
								{
									pModel->SetLocalPosition(f3Pos);
								}

								static std::unordered_map<graphics::IModel*, math::float3> umapMotionRotation;
								math::float3& f3Rotation = umapMotionRotation[pModel];
								if (ImGui::DragFloat3("Local Rotation", reinterpret_cast<float*>(&f3Rotation), 0.1f, -360.f, 360.f) == true)
								{
									auto ResetRot = [](float& fRot)
									{
										if (fRot >= 360.f)
										{
											fRot -= 360.f;
										}
										else if (fRot <= -360.f)
										{
											fRot += 360.f;
										}
									};

									ResetRot(f3Rotation.x);
									ResetRot(f3Rotation.y);
									ResetRot(f3Rotation.z);

									math::Quaternion quatRotation = math::Quaternion::CreateFromYawPitchRoll(math::ToRadians(f3Rotation.y), math::ToRadians(f3Rotation.x), math::ToRadians(f3Rotation.z));

									pModel->SetLocalRotation(quatRotation);
								}

								if (pModel != nullptr)
								{
									if (ImGui::Button("Save") == true)
									{
										wchar_t path[512]{};
										OPENFILENAME ofn;
										memory::Clear(&ofn, sizeof(ofn));

										ofn.lStructSize = sizeof(OPENFILENAME);
										ofn.hwndOwner = graphics::GetHwnd();
										ofn.lpstrFilter = L"EastModel(*.emod)\0*.emod\0";
										ofn.lpstrFile = path;
										ofn.nMaxFile = 255;
										if (GetSaveFileName(&ofn) != 0)
										{
											const std::wstring strFileExtension = file::GetFileExtension(path);
											if (strFileExtension.empty() == true)
											{
												string::Concat(path, sizeof(path), L".emod");
											}

											if (string::IsEqualsNoCase(file::GetFileExtension(path).c_str(), L".emod") == true)
											{
												if (graphics::SaveFile(pModel, path) == false)
												{
													LOG_ERROR(L"저장 실패 : %s", path);
												}
											}
										}
									}

									if (pModel->GetSkeleton() != nullptr)
									{
										static bool isShowMotionSystem = false;
										if (ImGui::Button("Show Motion System") == true)
										{
											isShowMotionSystem = !isShowMotionSystem;
										}

										if (isShowMotionSystem == true)
										{
											ShowMotion(isShowMotionSystem, pCompModel);
										}
									}

									const std::string modelName = string::WideToMulti(pModel->GetName().c_str());

									static char bufName[128]{};
									string::Copy(bufName, modelName.c_str());

									ImGui::PushID(bufName);

									if (ImGui::InputText("Name", bufName, sizeof(bufName), ImGuiInputTextFlags_EnterReturnsTrue) == true)
									{
										pModel->ChangeName(bufName);
									}

									const std::string modelPath = string::WideToMulti(pModel->GetFilePath().c_str());

									static char bufPath[512]{};
									string::Copy(bufPath, modelPath.c_str());
									ImGui::InputText("Path", bufPath, sizeof(bufPath), ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);

									int nModelNode = 0;
									int nMaterialIndex = 0;
									const uint32_t nNodeCount = pModel->GetNodeCount();
									for (uint32_t j = 0; j < nNodeCount; ++j)
									{
										graphics::IModelNode* pModelNode = pModel->GetNode(j);

										const std::string modelNodeName = string::WideToMulti(pModelNode->GetName().c_str());
										if (ImGui::TreeNode(modelNodeName.c_str()))
										{
											uint32_t nVertexCount = 0;
											if (pModelNode->GetVertexBuffer() != nullptr)
											{
												nVertexCount = pModelNode->GetVertexBuffer()->GetVertexCount();
											}
											ImGui::Text("VertexCount : %u", nVertexCount);

											uint32_t indexCount = 0;
											if (pModelNode->GetIndexBuffer() != nullptr)
											{
												indexCount = pModelNode->GetIndexBuffer()->GetIndexCount();
											}
											ImGui::Text("IndexCount : %u", indexCount);

											bool isVisibleModelNode = pModelNode->IsVisible();
											if (ImGui::Checkbox(string::Format("%d. Model Visible", nModelNode).c_str(), &isVisibleModelNode) == true)
											{
												pModelNode->SetVisible(isVisibleModelNode);
											}
											++nModelNode;

											ImGui::Separator();
											ImGui::Text("Material");

											uint32_t nMaterialCount = pModelNode->GetMaterialCount();
											for (uint32_t k = 0; k < nMaterialCount; ++k)
											{
												graphics::MaterialPtr pMaterial = pModelNode->GetMaterial(k);

												const std::string materialName = string::WideToMulti(pMaterial->GetName().c_str());
												std::string mtrlName = string::Format("%d. %s", nMaterialIndex, materialName.c_str());

												static std::unordered_map<std::string, bool> umapIsShowMaterial;
												bool& isShow = umapIsShowMaterial[mtrlName];

												if (ImGui::Button(mtrlName.c_str()) == true)
												{
													isShow = !isShow;
												}

												if (isShow == true)
												{
													ShowMaterial(isShow, pMaterial.get(), nMaterialIndex);
												}

												++nMaterialIndex;
											}

											ImGui::Separator();

											ImGui::TreePop();
										}
									}

									ImGui::PopID();
								}
								else
								{
									ImGui::Text("Invalid Model");
								}
							}
							else
							{
								ImGui::Text("Loading...");
							}
						}
						else
						{
							static float scaleFactor = 1.f;

							if (ImGui::Button("Load Model") == true)
							{
								wchar_t path[512]{};

								OPENFILENAME ofn;
								memory::Clear(&ofn, sizeof(ofn));

								ofn.lStructSize = sizeof(OPENFILENAME);
								ofn.hwndOwner = graphics::GetHwnd();
								ofn.lpstrFilter = L"Model File(*.fbx;*.obj;*.emod)\0*.fbx;*.obj;*.emod\0FBX File(*.fbx)\0*.fbx\0Obj File(*.obj)\0*.obj\0EastModel File(*.emod)\0*.emod\0";
								ofn.lpstrFile = path;
								ofn.nMaxFile = 256;
								if (GetOpenFileName(&ofn) != 0)
								{
									const std::wstring strFileExtension = file::GetFileExtension(ofn.lpstrFile);
									if (string::IsEqualsNoCase(strFileExtension.c_str(), L".fbx") == true)
									{
										const string::StringID name = string::Format(L"%s(%f)", file::GetFileName(ofn.lpstrFile).c_str(), scaleFactor);

										graphics::ModelLoader loader;
										loader.InitFBX(name, ofn.lpstrFile, scaleFactor);
										loader.SetEnableThreadLoad(true);

										pCompModel->Add(0, loader);
									}
									else if (string::IsEqualsNoCase(strFileExtension.c_str(), L".obj") == true)
									{
										const string::StringID name = string::Format(L"%s(%f)", file::GetFileName(ofn.lpstrFile).c_str(), scaleFactor);

										graphics::ModelLoader loader;
										loader.InitObj(name, ofn.lpstrFile, scaleFactor);
										loader.SetEnableThreadLoad(true);

										pCompModel->Add(0, loader);
									}
									else if (string::IsEqualsNoCase(strFileExtension.c_str(), L".emod") == true)
									{
										graphics::ModelLoader loader;
										loader.InitEast(file::GetFileName(ofn.lpstrFile).c_str(), ofn.lpstrFile);
										loader.SetEnableThreadLoad(true);

										pCompModel->Add(0, loader);
									}
								}
							}

							ImGui::SameLine();

							ImGui::DragFloat("ScaleFactor", &scaleFactor, 0.0001f, 0.0001f, 10000.f, "%.4f");

							if (ImGui::Button("Geometry Model") == true)
							{
								ImGui::OpenPopup("Load Geometry Model");
							}

							if (ImGui::BeginPopupModal("Load Geometry Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize) == true)
							{
								ImGui::PushID("Geometry");

								static std::vector<std::string> geometryModels;
								if (geometryModels.empty() == true)
								{
									for (int j = graphics::ModelLoader::eCube; j < graphics::ModelLoader::eCapsule; ++j)
									{
										const wchar_t* name = graphics::ModelLoader::GetGeometryTypeToString(static_cast<graphics::ModelLoader::GeometryType>(j));
										geometryModels.emplace_back(string::WideToMulti(name));
									}
								}

								std::vector<const char*> geometryModelNames;
								for (auto& name : geometryModels)
								{
									geometryModelNames.emplace_back(name.c_str());
								}

								static int selectedGeometryIndex = 0;
								ImGui::Combo("Type", &selectedGeometryIndex, &geometryModelNames.front(), static_cast<int>(geometryModelNames.size()));

								const std::wstring modelName = string::MultiToWide(geometryModelNames[selectedGeometryIndex]);
								graphics::ModelLoader::GeometryType emType = graphics::ModelLoader::GetGeometryType(modelName.c_str());
								switch (emType)
								{
								case graphics::ModelLoader::eCube:
								{
									ImGui::PushID("Cube");

									static char buf[128] = "Cube";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Cube Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitCube(buf, nullptr, fSize);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eBox:
								{
									ImGui::PushID("Box");

									static char buf[128] = "Box";
									ImGui::InputText("Name", buf, sizeof(buf));

									static math::float3 halfExtents(math::float3::One);
									ImGui::DragFloat3("Box Size", reinterpret_cast<float*>(&halfExtents.x), 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitBox(buf, nullptr, halfExtents);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eSphere:
								{
									ImGui::PushID("Sphere");

									static char buf[128] = "Sphere";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fDiameter = 1.f;
									ImGui::DragFloat("Diameter", &fDiameter, 0.01f, 0.01f, 1000000.f);

									static int nTessellation = 16;
									ImGui::DragInt("Tessellation", &nTessellation, 0.01f, 16, 128);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitSphere(buf, nullptr, fDiameter, nTessellation);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eGeoSphere:
								{
									ImGui::PushID("GeoSphere");

									static char buf[128] = "GeoSphere";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fDiameter = 1.f;
									ImGui::DragFloat("Diameter", &fDiameter, 0.01f, 0.01f, 1000000.f);

									static int nTessellation = 3;
									ImGui::DragInt("Tessellation", &nTessellation, 0.01f, 3, 16);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitGeoSphere(buf, nullptr, fDiameter, nTessellation);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eCylinder:
								{
									ImGui::PushID("Cylinder");

									static char buf[128] = "Cylinder";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fHeight = 1.f;
									ImGui::DragFloat("Height", &fHeight, 0.01f, 0.01f, 1000000.f);

									static float fDiameter = 1.f;
									ImGui::DragFloat("Diameter", &fDiameter, 0.01f, 0.01f, 1000000.f);

									static int nTessellation = 32;
									ImGui::DragInt("Tessellation", &nTessellation, 0.01f, 8, 128);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitCylinder(buf, nullptr, fHeight, fDiameter, nTessellation);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eCone:
								{
									ImGui::PushID("Cone");

									static char buf[128] = "Cone";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fDiameter = 1.f;
									ImGui::DragFloat("Diameter", &fDiameter, 0.01f, 0.01f, 1000000.f);

									static float fHeight = 1.f;
									ImGui::DragFloat("Height", &fHeight, 0.01f, 0.01f, 1000000.f);

									static int nTessellation = 32;
									ImGui::DragInt("Tessellation", &nTessellation, 0.01f, 8, 128);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitCone(buf, nullptr, fDiameter, fHeight, nTessellation);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eTorus:
								{
									ImGui::PushID("Torus");

									static char buf[128] = "Torus";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fDiameter = 1.f;
									ImGui::DragFloat("Diameter", &fDiameter, 0.01f, 0.01f, 1000000.f);

									static float fThickness = 0.333f;
									ImGui::DragFloat("Height", &fThickness, 0.01f, 0.01f, 1000000.f);

									static int nTessellation = 32;
									ImGui::DragInt("Tessellation", &nTessellation, 0.01f, 8, 128);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitTorus(buf, nullptr, fDiameter, fThickness, nTessellation);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eTetrahedron:
								{
									ImGui::PushID("Tetrahedron");

									static char buf[128] = "Tetrahedron";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitTetrahedron(buf, nullptr, fSize);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eOctahedron:
								{
									ImGui::PushID("Octahedron");

									static char buf[128] = "Octahedron";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitOctahedron(buf, nullptr, fSize);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eDodecahedron:
								{
									ImGui::PushID("Dodecahedron");

									static char buf[128] = "Dodecahedron";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitDodecahedron(buf, nullptr, fSize);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eIcosahedron:
								{
									ImGui::PushID("Icosahedron");

									static char buf[128] = "Icosahedron";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitIcosahedron(buf, nullptr, fSize);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eTeapot:
								{
									ImGui::PushID("Teapot");

									static char buf[128] = "Teapot";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Size", &fSize, 0.01f, 0.01f, 1000000.f);

									static int nTessellation = 8;
									ImGui::DragInt("Tessellation", &nTessellation, 0.01f, 4, 128);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitTeapot(buf, nullptr, fSize, nTessellation);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eHexagon:
								{
									ImGui::PushID("Hexagon");

									static char buf[128] = "Hexagon";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fRadius = 1.f;
									ImGui::DragFloat("Radius", &fRadius, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitHexagon(buf, nullptr, fRadius);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case graphics::ModelLoader::eCapsule:
								{
									ImGui::PushID("Capsule");

									static char buf[128] = "Capsule";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fRadius = 0.5f;
									ImGui::DragFloat("Radius", &fRadius, 0.01f, 0.01f, 1000000.f);

									static float fHeight = 1.f;
									ImGui::DragFloat("Height", &fHeight, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitCapsule(buf, nullptr, fRadius, fHeight);

										pCompModel->Add(0, loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								}

								if (ImGui::Button("Cancel") == true)
								{
									ImGui::CloseCurrentPopup();
								}

								ImGui::PopID();

								ImGui::EndPopup();
							}
						}

						ImGui::PopID();
					}
				}
				break;
				case gameobject::IComponent::eCamera:
				{
					if (ImGui::CollapsingHeader("Camera"))
					{
					}
				}
				break;
				}
			}
		}

		ImGui::PopID();

		ImGui::End();
	}
}

void SceneStudio::ShowGizmo()
{
	if (m_selectedActor != gameobject::IGameObject::InvalidHandle)
	{
		graphics::Camera& camera = graphics::GetCamera();

		auto iter = std::find_if(m_actors.begin(), m_actors.end(), [&](const est::gameobject::ActorPtr& pActor)
			{
				return pActor->GetHandle() == m_selectedActor;
			});

		if (iter != m_actors.end())
		{
			gameobject::IActor* pActor = iter->get();
			gameobject::ComponentPhysics* pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::IComponent::ePhysics));
			if (pCompPhysics != nullptr)
			{
				ImGuiIO& io = ImGui::GetIO();
				ImGuizmo::BeginFrame();
				ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
				const math::Matrix& viewMatrix = camera.GetViewMatrix();
				const math::Matrix& projectionMatrix = camera.GetProjectionMatrix();

				const ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
				const ImGuizmo::MODE mode = ImGuizmo::MODE::LOCAL;

				math::Matrix matrix = pActor->GetWorldMatrix();
				math::Matrix deltaMatrix;

				bool isEnableSnap = false;

				ImGuizmo::Manipulate(reinterpret_cast<const float*>(&viewMatrix), reinterpret_cast<const float*>(&projectionMatrix),
					operation, mode, reinterpret_cast<float*>(&matrix), reinterpret_cast<float*>(&deltaMatrix), nullptr, nullptr);

				if (deltaMatrix != math::Matrix::Identity)
				{
					const math::Transform transform(matrix);
					switch (operation)
					{
					case ImGuizmo::OPERATION::TRANSLATE:
						pCompPhysics->SetGlobalPosition(transform.position);
						break;
					case ImGuizmo::OPERATION::ROTATE:
						pCompPhysics->SetGlobalRotation(transform.rotation);
						break;
					case ImGuizmo::OPERATION::SCALE:
						pActor->SetScale(transform.scale);
						break;
					}
				}
			}
		}
	}
}

void SceneStudio::ShowNodeEditer()
{
	NodeEditorShow();
}