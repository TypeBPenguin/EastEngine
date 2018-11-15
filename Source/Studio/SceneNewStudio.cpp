#include "stdafx.h"
#include "SceneNewStudio.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"
#include "Input/InputInterface.h"
#include "Model/ModelInterface.h"
#include "Model/ModelLoader.h"
#include "Model/MotionLoader.h"
#include "Model/GeometryModel.h"
#include "Model/ModelManager.h"

#include "GameObject/GameObject.h"
#include "GameObject/GameObjectManager.h"
#include "GameObject/ComponentModel.h"
#include "GameObject/ComponentPhysics.h"

#include "SoundSystem/SoundInterface.h"

#include "GraphicsInterface/imguiHelper.h"

using namespace eastengine;

namespace StrID
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

SceneNewStudio::SceneNewStudio()
	: IScene(StrID::Studio)
{
}

SceneNewStudio::~SceneNewStudio()
{
}

void SceneNewStudio::Enter()
{
	const math::uint2& n2ScreenSize = graphics::GetScreenSize();

	const float fAspect = static_cast<float>(n2ScreenSize.x) / static_cast<float>(n2ScreenSize.y);
	graphics::Camera::GetInstance()->SetProjection(n2ScreenSize.x, n2ScreenSize.y, math::PIDIV4, 0.1f, 1000.f, graphics::GetAPI() == graphics::eVulkan);
	graphics::Camera::GetInstance()->SetView({ 0.f, 2.f, -10.f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f });

	{
		math::float3 f3LightPosition(0.f, 500.f, -500.f);
		math::float3 f3LightDirection(math::float3::Zero - f3LightPosition);
		f3LightDirection.Normalize();

		graphics::ILight* pLight = graphics::ILight::CreateDirectionalLight("MainLight", f3LightDirection, math::Color::White, 1.f, 0.5f, 0.25f);
		pLight->SetEnableShadow(false);
	}

	std::string strPath = file::GetPath(file::eTexture);
	{
		strPath = string::Format("%sIBL\\%s\\%s", file::GetPath(file::eTexture), IBL_Type[1], IBL_Type[1]);

		graphics::IImageBasedLight* pImageBasedLight = graphics::GetImageBasedLight();

		std::string strDiffuseHDR = strPath;
		strDiffuseHDR.append("DiffuseHDR.dds");
		graphics::ITexture* pDiffuseHDR = graphics::CreateTextureAsync(strDiffuseHDR.c_str());
		pImageBasedLight->SetDiffuseHDR(pDiffuseHDR);
		graphics::ReleaseResource(&pDiffuseHDR);

		std::string strSpecularHDR = strPath;
		strSpecularHDR.append("SpecularHDR.dds");
		graphics::ITexture* pSpecularHDR = graphics::CreateTextureAsync(strSpecularHDR.c_str());
		pImageBasedLight->SetSpecularHDR(pSpecularHDR);
		graphics::ReleaseResource(&pSpecularHDR);

		std::string strSpecularBRDF = strPath;
		strSpecularBRDF.append("Brdf.dds");
		graphics::ITexture* pSpecularBRDF = graphics::CreateTextureAsync(strSpecularBRDF.c_str());
		pImageBasedLight->SetSpecularBRDF(pSpecularBRDF);
		graphics::ReleaseResource(&pSpecularBRDF);

		std::string strEnvIBLPath = strPath;
		strEnvIBLPath.append("EnvHDR.dds");
		graphics::ITexture* pEnvironmentHDR = graphics::CreateTextureAsync(strEnvIBLPath.c_str());
		pImageBasedLight->SetEnvironmentHDR(pEnvironmentHDR);
		graphics::ReleaseResource(&pEnvironmentHDR);

		std::vector<graphics::VertexPosTexNor> vertices;
		std::vector<uint32_t> indices;

		graphics::geometry::CreateSphere(vertices, indices, 1.f, 32u);

		graphics::IVertexBuffer* pVertexBuffer = graphics::CreateVertexBuffer(reinterpret_cast<const uint8_t*>(vertices.data()), static_cast<uint32_t>(vertices.size()), sizeof(graphics::VertexPosTexNor));
		graphics::IIndexBuffer* pIndexBuffer = graphics::CreateIndexBuffer(reinterpret_cast<const uint8_t*>(indices.data()), static_cast<uint32_t>(indices.size()), sizeof(uint32_t));

		pImageBasedLight->SetEnvironmentSphere(pVertexBuffer, pIndexBuffer);

		graphics::ReleaseResource(&pVertexBuffer);
		graphics::ReleaseResource(&pIndexBuffer);
	}

	{
		auto pActor = gameobject::GameObjectManager::GetInstance()->CreateActor(StrID::Studio_Ground);

		graphics::MaterialInfo material;
		material.strName = StrID::Studio_Ground;

		graphics::ModelLoader loader;
		loader.InitPlane(StrID::Studio_Ground, 1.f, 1.f, 100, 100, &material);

		auto pCompModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));
		pCompModel->Init(&loader);

		auto pModelInst = pCompModel->GetModelInstance();

		//auto pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::EmComponent::ePhysics));
		//
		//physics::RigidBodyProperty prop;
		//prop.fRestitution = 0.75f;
		//prop.strName = StrID::Studio_Ground;
		//prop.fMass = 0.f;
		//prop.nCollisionFlag = physics::EmCollision::eStaticObject;
		//prop.shapeInfo.SetTriangleMesh();
		//pCompPhysics->Init(pModelInst, prop);

		/*gameobject::SectorInitInfo sectorInitInfo;
		sectorInitInfo.fRadius = 10.f;
		for (auto& direction : sectorInitInfo.nSectorsCount)
		{
		direction = 10;
		}

		m_pSectorMgr = CreateSectorMgr(sectorInitInfo);*/
	}

	graphics::IMaterial* pMaterial_override = nullptr;
	for (int j = 0; j < 5; ++j)
	{
		for (int i = 0; i < 50; ++i)
		{
			/*graphics::MaterialInfo materialInfo;
			materialInfo.strName.Format("TestBox%d", (i % 10) + 1);
			materialInfo.strPath = file::GetPath(file::eTexture);

			materialInfo.strTextureNameArray[graphics::EmMaterial::eAlbedo].Format("Pattern\\pattern_%02d\\%s", (i % 10) + 1, "diffus.tga");
			materialInfo.strTextureNameArray[graphics::EmMaterial::eNormal].Format("Pattern\\pattern_%02d\\%s", (i % 10) + 1, "Normal.tga");
			materialInfo.strTextureNameArray[graphics::EmMaterial::eSpecularColor].Format("Pattern\\pattern_%02d\\%s", (i % 10) + 1, "specular.tga");
			*/
			graphics::MaterialInfo materialInfo;
			materialInfo.strName = "TestBox";
			materialInfo.strPath = file::GetPath(file::eTexture);

			materialInfo.strTextureNameArray[graphics::EmMaterial::eAlbedo].Format("Pattern\\pattern_01\\%s", "diffus.tga");
			materialInfo.strTextureNameArray[graphics::EmMaterial::eNormal].Format("Pattern\\pattern_01\\%s", "Normal.tga");
			materialInfo.strTextureNameArray[graphics::EmMaterial::eSpecular].Format("Pattern\\pattern_01\\%s", "specular.tga");

			//materialInfo.f4PaddingRoughMetEmi.y = 0.1f * ((i % 10) + 1);
			//materialInfo.f4PaddingRoughMetEmi.z = 1.f - 0.1f * ((i % 10) + 1);

			materialInfo.f4PaddingRoughMetEmi.y = 0.5f;
			materialInfo.f4PaddingRoughMetEmi.z = 0.5f;

			//materialInfo.rasterizerStateDesc = graphics::GetDevice()->GetRasterizerStateDesc(graphics::EmRasterizerState::eNone);
			//materialInfo.colorAlbedo = math::Color(math::Random(0.f, 1.f), math::Random(0.f, 1.f), math::Random(0.f, 1.f), 1.f);

			gameobject::IActor* pActor = gameobject::GameObjectManager::GetInstance()->CreateActor("TestBox");

			math::float3 f3Pos;
			f3Pos.x = -4.f + (i % 5) * 3.f;
			//f3Pos.y = 100.5f + (j * 3.f);
			f3Pos.y = 0.5f + (j * 3.f);
			f3Pos.z = -4.f + (i / 5) * 3.f;

			pActor->SetPosition(f3Pos);

			gameobject::ComponentModel* pCompModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));

			graphics::ModelLoader loader;
			//loader.InitBox(string::Format("TestBox%d", (i % 10) + 1).c_str(), &materialInfo);
			loader.InitBox("TestBox", &materialInfo);
			pCompModel->Init(&loader);
			auto pModelInst = pCompModel->GetModelInstance();

			if (i % 2 == 0)
			{
				if (pMaterial_override == nullptr)
				{
					graphics::MaterialInfo materialInfo2;
					materialInfo2.strName = "TestBox";
					materialInfo2.strPath = file::GetPath(file::eTexture);

					materialInfo2.strTextureNameArray[graphics::EmMaterial::eAlbedo].Format("Pattern\\pattern_02\\%s", "diffus.tga");
					materialInfo2.strTextureNameArray[graphics::EmMaterial::eNormal].Format("Pattern\\pattern_02\\%s", "Normal.tga");
					materialInfo2.strTextureNameArray[graphics::EmMaterial::eSpecular].Format("Pattern\\pattern_02\\%s", "specular.tga");

					pMaterial_override = graphics::CreateMaterial(&materialInfo2);
					pMaterial_override->DecreaseReference();
				}
				pModelInst->ChangeMaterial("EastEngine_Box", 0, pMaterial_override);
			}

			//gameobject::ComponentPhysics* pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::EmComponent::ePhysics));
			//
			//physics::RigidBodyProperty prop;
			//prop.fRestitution = 0.5f;
			//prop.strName.Format("TestBox_RigidBody%d", i).c_str();
			//
			//prop.shapeInfo.SetBox(math::float3(1.f));
			////prop.shapeInfo.SetCapsule(math::Random(0.5f, 1.f), math::Random(1.f, 2.f));
			//prop.nCollisionFlag = physics::EmCollision::eCharacterObject;
			//prop.f3OriginPos = f3Pos;
			//pCompPhysics->Init(prop);
		}
	}

	if (false)
	{
		gameobject::TerrainProperty terrain;

		terrain.strTexHeightMap = file::GetPath(file::eTexture);
		terrain.strTexHeightMap.append("heightmap.r16");

		terrain.strTexColorMap = file::GetPath(file::eTexture);
		terrain.strTexColorMap.append("ColorMap2.bmp");

		terrain.fHeightScale = 300.f;

		terrain.n2Size = { 1025, 1025 };
		terrain.n2Patches = { 64, 64 };

		terrain.rigidBodyProperty.fRestitution = 0.25f;
		terrain.rigidBodyProperty.fFriction = 0.75f;

		terrain.strTexDetailMap = file::GetPath(file::eTexture);
		terrain.strTexDetailMap.append("dirt01d.tga");

		terrain.strTexDetailNormalMap = file::GetPath(file::eTexture);
		terrain.strTexDetailNormalMap.append("dirt01n.tga");

		terrain.transform.position = { -500.f, 0.f, -500.f };

		gameobject::ITerrain::Create("BaseTerrain", terrain);

		// 백그라운드 로딩은 이렇게 쓰면됨
		//gameobject::ITerrain::CreateAsync("BaseTerrain", terrain);
	}

	auto CreateActor = [](const string::StringID& strActorName, const char* strModelFilePath,
		const math::float3& f3Position,
		graphics::ModelLoader::LoadType emModelType = graphics::ModelLoader::eEast)
	{
		gameobject::IActor* pActor = gameobject::IActor::Create(strActorName);

		pActor->SetPosition(f3Position);

		graphics::ModelLoader loader;

		string::StringID strFileName = file::GetFileName(strModelFilePath).c_str();
		switch (emModelType)
		{
		case graphics::ModelLoader::LoadType::eFbx:
		case graphics::ModelLoader::LoadType::eObj:
			loader.InitFBX(strFileName, strModelFilePath, 0.01f);
			break;
		case graphics::ModelLoader::LoadType::eXps:
			loader.InitXPS(strFileName, strModelFilePath);
			break;
		case graphics::ModelLoader::LoadType::eEast:
			loader.InitEast(strFileName, strModelFilePath);
			break;
		}
		loader.SetEnableThreadLoad(false);

		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));
		pModel->Init(&loader);

		return pActor;
	};

	for (int i = 0; i < 1; ++i)
	{
		string::StringID name;
		name.Format("UnityChan%d", i);

		strPath = file::GetDataPath();
		strPath.append("Actor\\UnityChan\\unitychan.emod");

		math::float3 pos;
		pos.x = -10.f + (2.f * (i % 10));
		pos.z = 0.f + (2.f * (i / 10));

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));
		graphics::IModelInstance* pModelInstance = pModel->GetModelInstance();

		//if (false)
		{
			const std::vector<const char*> vecAnim =
			{
				"Actor\\UnityChan\\Animations\\unitychan_WAIT00.fbx",
				"Actor\\UnityChan\\Animations\\unitychan_WAIT01.fbx",
				"Actor\\UnityChan\\Animations\\unitychan_WAIT02.fbx",
				"Actor\\UnityChan\\Animations\\unitychan_WAIT03.fbx",
				"Actor\\UnityChan\\Animations\\unitychan_WAIT04.fbx",
			};

			std::string strPathMotion(file::GetDataPath());
			//strPathMotion.append("Actor\\UnityChan\\Animations\\unitychan_WAIT00.fbx");
			strPathMotion.append(vecAnim[math::Random(0llu, vecAnim.size() - 1)]);

			string::StringID strMotionName;
			strMotionName.Format("%s", file::GetFileName(strPathMotion).c_str());
			graphics::MotionLoader motionLoader;
			motionLoader.InitFBX(strMotionName, strPathMotion.c_str(), 0.01f);
			graphics::IMotion* pMotion = graphics::IMotion::Create(motionLoader);

			graphics::MotionPlaybackInfo playback;
			playback.fSpeed = math::Random(0.5f, 1.5f);
			playback.nLoopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;
			//playback.fWeight = math::Random(0.1f, 0.5f);
			playback.fWeight = 1.f;
			pModel->PlayMotion(graphics::EmMotion::eLayer1, pMotion, &playback);
		}

		//{
		//	std::vector<const char*> vecAnim =
		//	{
		//		"Actor\\UnityChan\\Animations\\unitychan_RUN00_F.fbx",
		//		"Actor\\UnityChan\\Animations\\unitychan_JUMP00.fbx",
		//		"Actor\\UnityChan\\Animations\\unitychan_LOSE00.fbx",
		//		"Actor\\UnityChan\\Animations\\unitychan_REFLESH00.fbx",
		//		"Actor\\UnityChan\\Animations\\unitychan_SLIDE00.fbx",
		//		"Actor\\UnityChan\\Animations\\unitychan_UMATOBI00.fbx",
		//		"Actor\\UnityChan\\Animations\\unitychan_WIN00.fbx",
		//	};

		//	std::string strPathMotion(file::GetDataPath());
		//	//strPathMotion.append("Actor\\UnityChan\\Animations\\unitychan_ARpose1.fbx");
		//	//strPathMotion.append("Actor\\UnityChan\\Animations\\unitychan_RUN00_F.fbx");
		//	strPathMotion.append(vecAnim[math::Random(0u, vecAnim.size() - 1)]);

		//	string::StringID strMotionName;
		//	strMotionName.Format("%s", file::GetFileName(strPathMotion).c_str());
		//	graphics::MotionLoader motionLoader;
		//	motionLoader.InitFBX(strMotionName, strPathMotion.c_str(), 0.01f);
		//	graphics::IMotion* pMotion = graphics::IMotion::Create(motionLoader);

		//	graphics::MotionPlaybackInfo playback;
		//	playback.fSpeed = math::Random(0.5f, 1.5f);
		//	playback.nLoopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;
		//	playback.fWeight = math::Random(0.7f, 1.f);
		//	pMotionSystem->Play(graphics::EmMotion::eLayer2, pMotion, &playback);
		//}

		//gameobject::ComponentPhysics* pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::EmComponent::ePhysics));

		//math::float3 ragdollPos = pActor->GetPosition();
		//pCompPhysics->m_pRagDoll->BuildBipadRagDoll(pModelInstance->GetSkeleton(), ragdollPos, math::Quaternion::Identity, 0.8f);
		//pCompPhysics->m_pRagDoll->Start();

		if (false)
		{
			strPath = file::GetDataPath();
			strPath.append("Model\\ElementalSwordIce\\LP.emod");

			graphics::IModelInstance* pModelInstance_Attach = nullptr;
			graphics::ModelLoader loader;
			loader.InitEast(file::GetFileName(strPath).c_str(), strPath.c_str());

			pModelInstance_Attach = graphics::IModel::CreateInstance(loader, false);

			math::float3 f3Pos = { 0.08f, 0.225f, -0.02f };
			math::Quaternion quat = math::Quaternion::CreateFromYawPitchRoll(math::ToRadians(90.f), math::ToRadians(180.f), 0.f);

			pModelInstance->Attachment(pModelInstance_Attach, "Character1_LeftHand", math::Matrix::Compose(math::float3::One, quat, f3Pos));
		}
	}

	{
		string::StringID name;
		name.Format("KimJiYoon");

		math::float3 pos;
		pos.x = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\KimJiYoon\\KimJiYoon.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));

		//if (false)
		{
			std::string strPathMotion(file::GetDataPath());
			strPathMotion.append("Model\\KimJiYoon\\AR_Idle_CC.fbx");

			string::StringID strMotionName;
			strMotionName.Format("%s", file::GetFileName(strPathMotion).c_str());
			graphics::MotionLoader motionLoader;
			motionLoader.InitFBX(strMotionName, strPathMotion.c_str(), 0.01f);
			graphics::IMotion* pMotion = graphics::IMotion::Create(motionLoader);

			graphics::MotionPlaybackInfo playback;
			playback.fSpeed = 1.f;
			playback.nLoopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;
			playback.fWeight = 1.f;
			pModel->PlayMotion(graphics::EmMotion::eLayer1, pMotion, &playback);
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		string::StringID name;
		name.Format("2B_NierAutomata_%d", i);

		math::float3 pos;
		pos.x = -2.f + (i * -2.f);

		strPath = file::GetDataPath();
		strPath.append("Model\\2B_NierAutomata\\2B_NierAutomata.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));

		graphics::IModelInstance* pModelInstance = pModel->GetModelInstance();

		if (i == 1)
		{
			graphics::IModelNode* pNode = pModelInstance->GetModel()->GetNode("Generic_Item.mesh");

			auto SetMaterialVisible = [&](const string::StringID& strMaterialName)
			{
				uint32_t nMaterialID = 0;
				graphics::IMaterial* pMaterial = pNode->GetMaterial(strMaterialName, nMaterialID);

				graphics::IMaterial* pMaterialClone = graphics::CloneMaterial(pMaterial);
				pMaterialClone->SetVisible(false);

				pModelInstance->ChangeMaterial("Generic_Item.mesh", nMaterialID, pMaterialClone);
				pMaterialClone->DecreaseReference();
			};

			SetMaterialVisible("Skirt");
			SetMaterialVisible("Eyepatch");
		}
	}

	//if (false)
	{
		string::StringID name;
		name.Format("Delia");

		math::float3 pos;
		pos.x = 4.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Delia\\Delia.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("Misaki");

		math::float3 pos;
		pos.x = 6.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Misaki\\Misaki.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("Naotora");

		math::float3 pos;
		pos.x = 8.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Naotora\\Naotora.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("Naotora_ShirtDress");

		math::float3 pos;
		pos.x = 10.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Naotora_ShirtDress\\Naotora_ShirtDress.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("Bugeikloth");

		math::float3 pos;
		pos.z = 10.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Bugeikloth\\Bugeikloth.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("DarkKnight_Female");

		math::float3 pos;
		pos.x = -4.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Dark Knight_Female\\DarkKnight_Female.emod");

		//gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos, graphics::EmModelLoader::eXps);
		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("DarkKnight_Transformed_Female");

		math::float3 pos;
		pos.x = -2.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Dark Knight Transformed_Female\\DarkKnight_Transformed_Female.emod");

		//gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos, graphics::EmModelLoader::eXps);
		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("Paladin_Female");

		math::float3 pos;
		pos.x = 0.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Paladin_Female\\Paladin_Female.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("Paladin_Transformed_Female");

		math::float3 pos;
		pos.x = 2.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Paladin Transformed_Female\\Paladin_Transformed_Female.emod");

		//gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos, graphics::EmModelLoader::eXps);
		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("Evie_Temptress");

		math::float3 pos;
		pos.x = 4.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Evie_Temptress\\Evie_Temptress.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		string::StringID name;
		name.Format("Lynn_DancingBlade");

		math::float3 pos;
		pos.x = 6.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Lynn_DancingBlade\\Lynn_DancingBlade.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	{
		string::StringID name;
		name.Format("ElementalSwordIce");

		math::float3 pos;
		pos.y = 1.f;
		pos.z -= 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\ElementalSwordIce\\LP.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	// Sound
	{
		strPath = file::GetPath(file::eSound);
		strPath += "Canada, Ho!.mp3";
		sound::Play2D(strPath, 0.1f);
	}
}

void SceneNewStudio::Exit()
{
}

void SceneNewStudio::Update(float fElapsedTime)
{
	const ImGuiIO& io = ImGui::GetIO();

	if (io.WantCaptureMouse == false)
	{
		ProcessInput(fElapsedTime);
	}

	RenderImGui(fElapsedTime);

	static float fTime = 0.f;
	fTime += fElapsedTime;

	static int nFrame = 0;
	++nFrame;

	if (fTime >= 1.f)
	{
		const float fFrame = static_cast<float>(nFrame) / fTime;
		const float fMS = 1.f / fFrame;
		LOG_MESSAGE("%.2f[%f]", fFrame, fMS);

		fTime -= 1.f;
		nFrame = 0;
	}
}

void SceneNewStudio::ProcessInput(float fElapsedTime)
{
	graphics::Camera* pCamera = graphics::Camera::GetInstance();
	if (pCamera == nullptr)
		return;

	float dx = static_cast<float>(input::Mouse::GetMoveX());
	float dy = static_cast<float>(input::Mouse::GetMoveY());
	float dz = static_cast<float>(input::Mouse::GetMoveWheel());
	bool isMoveAxisX = math::IsZero(dx) == false;
	bool isMoveAxisY = math::IsZero(dy) == false;
	if (input::Mouse::IsButtonPressed(input::Mouse::eRight))
	{
		if (isMoveAxisX == true)
		{
			pCamera->RotateAxisY(dx * 0.1f);
		}

		if (isMoveAxisY == true)
		{
			pCamera->RotateAxisX(dy * 0.1f);
		}
	}

	if (input::Mouse::IsButtonPressed(input::Mouse::eMiddle))
	{
		if (isMoveAxisX == true)
		{
			pCamera->MoveSideward(dx * 0.025f);
		}

		if (isMoveAxisY == true)
		{
			pCamera->MoveUpward(-dy * 0.05f);
		}
	}

	if (input::Mouse::IsButtonPressed(input::Mouse::eLeft))
	{
		if (isMoveAxisX == true)
		{
			pCamera->RotateAxisY(dx * 0.025f);
		}

		if (isMoveAxisY == true)
		{
			pCamera->MoveForward(-dy * 0.05f);
		}
	}

	if (dz != 0.f)
	{
		pCamera->MoveForward(dz * 0.01f);
	}

	if (input::Keyboard::IsKeyPressed(input::Keyboard::eW))
	{
		pCamera->MoveForward(1.f);
	}

	if (input::Keyboard::IsKeyPressed(input::Keyboard::eS))
	{
		pCamera->MoveForward(-1.f);
	}

	if (input::Keyboard::IsKeyPressed(input::Keyboard::eA))
	{
		pCamera->MoveSideward(-1.f);
	}

	if (input::Keyboard::IsKeyPressed(input::Keyboard::eD))
	{
		pCamera->MoveSideward(1.f);
	}

	if (input::Keyboard::IsKeyPressed(input::Keyboard::eE))
	{
		pCamera->MoveUpward(1.f);
	}

	if (input::Keyboard::IsKeyPressed(input::Keyboard::eQ))
	{
		pCamera->MoveUpward(-1.f);
	}

	if (input::GamePad::IsConnected() == true)
	{
		auto LogButton = [](const char* strButtonName, const input::GamePad::ButtonState& emButtonState)
		{
			if (emButtonState == input::GamePad::ButtonState::ePressed)
			{
				LOG_MESSAGE("%s Pressed", strButtonName);
			}
			else if (emButtonState == input::GamePad::ButtonState::eUp)
			{
				LOG_MESSAGE("%s Up", strButtonName);
			}
			else if (emButtonState == input::GamePad::ButtonState::eDown)
			{
				LOG_MESSAGE("%s Down", strButtonName);
			}
		};

		LogButton("A", input::GamePad::A());
		LogButton("B", input::GamePad::B());
		LogButton("X", input::GamePad::X());
		LogButton("Y", input::GamePad::Y());

		LogButton("LeftStick", input::GamePad::LeftStick());
		LogButton("RightStick", input::GamePad::RightStick());

		LogButton("LeftShoulder", input::GamePad::LeftShoulder());
		LogButton("RightShoulder", input::GamePad::RightShoulder());

		LogButton("Back", input::GamePad::Back());
		LogButton("Start", input::GamePad::Start());

		LogButton("DPadUp", input::GamePad::DPadUp());
		LogButton("DPadDown", input::GamePad::DPadDown());
		LogButton("DPadLeft", input::GamePad::DPadLeft());
		LogButton("DPadRight", input::GamePad::DPadRight());

		auto LogStick = [](const char* strStickName, float fValue)
		{
			if (math::IsZero(fValue) == false)
			{
				LOG_MESSAGE("%s : %f", strStickName, fValue);
			}
		};

		LogStick("LeftThumbStickX", input::GamePad::LeftThumbStickX());
		LogStick("LeftThumbStickY", input::GamePad::LeftThumbStickY());
		LogStick("RightThumbStickX", input::GamePad::RightThumbStickX());
		LogStick("RightThumbStickY", input::GamePad::RightThumbStickY());
		LogStick("LeftTrigger", input::GamePad::LeftTrigger());
		LogStick("RightTrigger", input::GamePad::RightTrigger());

		//static float fTime = 0.f;
		//if (fTime >= 5.f)
		//{
		//	pPlayer->SetVibration(0.5f, 0.5f, 1.f);
		//	fTime -= 5.f;
		//}
		//fTime += fElapsedTime;
	}
	else
	{
		//static float fTime = 0.f;
		//if (fTime >= 1.f)
		//{
		//	LOG_MESSAGE("DisConnected");
		//	fTime -= 1.f;
		//}
		//
		//fTime += fElapsedTime;
	}
}

void ShowConfig()
{
	static bool isShowMainMenu = true;

	ImGui::SetNextWindowSize(ImVec2(400, 800), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Config", &isShowMainMenu);

	graphics::Options& graphicsOptions = graphics::GetOptions();

	if (ImGui::CollapsingHeader("Debug") == true)
	{
		/*if (ImGui::TreeNode("CollisionMesh"))
		{
			ImGui::PushID("CollisionMesh");

			bool isApplyVisibleCollisionMesh = Config::IsEnable("VisibleCollisionMesh"_s);
			if (ImGui::Checkbox("Visible", &isApplyVisibleCollisionMesh) == true)
			{
				Config::SetEnable("VisibleCollisionMesh"_s, isApplyVisibleCollisionMesh);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}*/

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

			ImGui::Checkbox("Apply", &graphicsOptions.OnOcclusionCulling);

			if (ImGui::Button("SaveBuffer") == true)
			{
				char path[512]{};
				OPENFILENAME ofn;
				Memory::Clear(&ofn, sizeof(ofn));

				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = graphics::GetHwnd();
				ofn.lpstrFilter = "Bmp(*.bmp)\0*.bmp\0";
				ofn.lpstrFile = path;
				ofn.nMaxFile = 255;
				if (GetSaveFileName(&ofn) != 0)
				{
					graphics::OcclusionCullingWriteBMP(path);
				}
			}

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("VSync"))
		{
			ImGui::PushID("VSync");

			ImGui::Checkbox("Apply", &graphicsOptions.OnVSync);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Shadow"))
		{
			ImGui::PushID("Shadow");

			ImGui::Checkbox("Apply", &graphicsOptions.OnShadow);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("FXAA"))
		{
			ImGui::PushID("FXAA");

			ImGui::Checkbox("Apply", &graphicsOptions.OnFXAA);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Motion Blur"))
		{
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

			ImGui::DragFloat("Threshold", &bloomFilterConfig.fThreshold, 0.001f, 0.f, 10.f);
			ImGui::DragFloat("StrengthMultiplier", &bloomFilterConfig.fStrengthMultiplier, 0.001f, 0.f, 10.f);
			ImGui::Checkbox("IsEnableLuminance", &bloomFilterConfig.isEnableLuminance);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("SSS"))
		{
			ImGui::PushID("SSS");

			ImGui::Checkbox("Apply", &graphicsOptions.OnSSS);

			ImGui::DragFloat("Width", &graphicsOptions.sssConfig.fWidth, 0.001f, 0.f, 100.f);

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

			const float fNear = graphics::Camera::GetInstance()->GetNearClip();
			const float fFar = graphics::Camera::GetInstance()->GetFarClip();

			graphics::Options::DepthOfFieldConfig& depthOfFieldConfig = graphicsOptions.depthOfFieldConfig;

			ImGui::PushItemWidth(100);
			ImGui::DragFloat("FocalDistnace", &depthOfFieldConfig.FocalDistnace, 0.01f, fNear, fFar);
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

		if (ImGui::TreeNode("Tessellation"))
		{
			ImGui::PushID("Tessellation");

			ImGui::Checkbox("Apply", &graphicsOptions.OnTessellation);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Wireframe"))
		{
			ImGui::PushID("Wireframe");

			ImGui::Checkbox("Apply", &graphicsOptions.OnWireframe);

			ImGui::PopID();

			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Skybox") == true)
	{
		static int nSelectedIndex = 0;
		if (ImGui::Combo("Env", &nSelectedIndex, IBL_Type, _countof(IBL_Type)) == true)
		{
			const std::string strPath = string::Format("%sIBL\\%s\\%s", file::GetPath(file::eTexture), IBL_Type[nSelectedIndex], IBL_Type[nSelectedIndex]);

			graphics::IImageBasedLight* pImageBasedLight = graphics::GetImageBasedLight();

			std::string strDiffuseHDR = strPath;
			strDiffuseHDR.append("DiffuseHDR.dds");
			graphics::ITexture* pDiffuseHDR = graphics::CreateTextureAsync(strDiffuseHDR.c_str());
			pImageBasedLight->SetDiffuseHDR(pDiffuseHDR);
			graphics::ReleaseResource(&pDiffuseHDR);

			std::string strSpecularHDR = strPath;
			strSpecularHDR.append("SpecularHDR.dds");
			graphics::ITexture* pSpecularHDR = graphics::CreateTextureAsync(strSpecularHDR.c_str());
			pImageBasedLight->SetSpecularHDR(pSpecularHDR);
			graphics::ReleaseResource(&pSpecularHDR);

			std::string strSpecularBRDF = strPath;
			strSpecularBRDF.append("Brdf.dds");
			graphics::ITexture* pSpecularBRDF = graphics::CreateTextureAsync(strSpecularBRDF.c_str());
			pImageBasedLight->SetSpecularBRDF(pSpecularBRDF);
			graphics::ReleaseResource(&pSpecularBRDF);

			std::string strEnvIBLPath = strPath;
			strEnvIBLPath.append("EnvHDR.dds");
			graphics::ITexture* pEnvironmentHDR = graphics::CreateTextureAsync(strEnvIBLPath.c_str());
			pImageBasedLight->SetEnvironmentHDR(pEnvironmentHDR);
			graphics::ReleaseResource(&pEnvironmentHDR);
		}
	}

	if (ImGui::CollapsingHeader("Light") == true)
	{
		/*gameobject::Sun* pActor = static_cast<gameobject::Sun*>(gameobject::ActorManager::GetInstance()->GetActor(StrID::EastEngine_Sun));
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
		graphics::Camera* pCamera = graphics::Camera::GetInstance();

		math::float3 f3CameraPos = pCamera->GetPosition();
		if (ImGui::DragFloat3("Camera Position", reinterpret_cast<float*>(&f3CameraPos.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			pCamera->SetPosition(f3CameraPos);
		}

		math::float3 f3CameraLookat = pCamera->GetLookat();
		if (ImGui::DragFloat3("Camera Lookat", reinterpret_cast<float*>(&f3CameraLookat.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			pCamera->SetLookat(f3CameraLookat);
		}

		math::float3 f3Up = pCamera->GetUp();
		if (ImGui::DragFloat3("Camera Up", reinterpret_cast<float*>(&f3Up.x), 0.01f, -1.f, 1.f) == true)
		{
			pCamera->SetUp(f3Up);
		}
	}

	ImGui::End();
}

void ShowModelList()
{
}

void ShowMotion(bool& isShowMotionMenu, gameobject::ComponentModel* pCompModel)
{
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Motion System", &isShowMotionMenu);

	ImGui::PushID("MotionSystem");

	if (ImGui::Button("Load Motion") == true)
	{
		char path[512]{};

		OPENFILENAME ofn;
		Memory::Clear(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = graphics::GetHwnd();
		ofn.lpstrFilter = "Motion File(*.fbx;*.emot)\0*.fbx;*.emot\0FBX File(*.fbx)\0*.fbx\0EastMotion File(*.emot)\0*.emot\0";
		ofn.lpstrFile = path;
		ofn.nMaxFile = 256;
		if (GetOpenFileName(&ofn) != 0)
		{
			const std::string strFileExtension = file::GetFileExtension(ofn.lpstrFile);
			if (string::IsEqualsNoCase(strFileExtension.c_str(), ".fbx") == true)
			{
				graphics::MotionLoader loader;
				loader.InitFBX(file::GetFileName(ofn.lpstrFile).c_str(), ofn.lpstrFile, 0.01f);
				graphics::IMotion::Create(loader);
			}
			else if (string::IsEqualsNoCase(strFileExtension.c_str(), ".emot") == true)
			{
				graphics::MotionLoader loader;
				loader.InitEast(file::GetFileName(ofn.lpstrFile).c_str(), ofn.lpstrFile);
				graphics::IMotion::Create(loader);
			}
		}
	}

	const size_t nMotionCount = graphics::ModelManager::GetInstance()->GetMotionCount();

	std::vector<const char*> vecMotionNames;
	vecMotionNames.reserve(nMotionCount);

	for (size_t i = 0; i < nMotionCount; ++i)
	{
		graphics::IMotion* pMotion = graphics::ModelManager::GetInstance()->GetMotion(i);
		vecMotionNames.emplace_back(pMotion->GetName().c_str());
	}

	static int nSelectedIndex = 0;
	if (vecMotionNames.empty() == false)
	{
		ImGui::ListBox("Motion List", &nSelectedIndex, &vecMotionNames.front(), static_cast<int>(vecMotionNames.size()), 6);
	}

	graphics::IMotion* pMotion = nullptr;
	if (0 <= nSelectedIndex && nSelectedIndex < static_cast<int>(vecMotionNames.size()))
	{
		pMotion = graphics::ModelManager::GetInstance()->GetMotion(nSelectedIndex);
	}

	if (pMotion != nullptr)
	{
		const std::array<char*, graphics::EmMotion::eLayerCount> layers = { "Layer1", "Layer2", "Layer3", "Layer4", };

		static graphics::EmMotion::Layers emLayer = graphics::EmMotion::eLayer1;
		ImGui::Combo("Layer", reinterpret_cast<int*>(&emLayer), layers.data(), static_cast<int>(layers.size()));

		static float fMotionSpeed = 1.f;
		ImGui::DragFloat("Speed", &fMotionSpeed, 0.001f, 0.001f, 10.f);

		static float fMotionWeight = 1.f;
		ImGui::DragFloat("Weight", &fMotionWeight, 0.001f, 0.f, 1.f);

		static float fMotionBlendTime = 0.f;
		ImGui::DragFloat("BlendTime", &fMotionBlendTime, 0.001f, 0.f, pMotion->GetEndTime());

		static bool isMotionLoop = false;
		ImGui::Checkbox("Loop", &isMotionLoop);

		ImGui::SameLine();

		static bool isMotionInverse = false;
		ImGui::Checkbox("Invert", &isMotionInverse);

		if (ImGui::Button("Play") == true)
		{
			graphics::MotionPlaybackInfo playback;
			playback.fSpeed = fMotionSpeed;
			playback.fWeight = fMotionWeight;
			playback.fBlendTime = fMotionBlendTime;
			playback.nLoopCount = isMotionLoop == true ? graphics::MotionPlaybackInfo::eMaxLoopCount : 1;
			playback.isInverse = isMotionInverse;

			pCompModel->PlayMotion(emLayer, pMotion, &playback);
		}

		ImGui::Separator();
	}

	graphics::IMotionSystem* pMotionSystem = pCompModel->GetModelInstance()->GetMotionSystem();
	if (pMotionSystem != nullptr)
	{
		for (int i = 0; i < graphics::EmMotion::eLayerCount; ++i)
		{
			graphics::EmMotion::Layers emLayer = static_cast<graphics::EmMotion::Layers>(i);
			graphics::IMotionPlayer* pPlayer = pMotionSystem->GetPlayer(emLayer);

			std::string strLayer = string::Format("Layer%d", i);

			ImGui::PushID(strLayer.c_str());

			ImGui::Text(strLayer.c_str());

			graphics::IMotion* pMotionPlaying = pPlayer->GetMotion();
			if (pMotionPlaying != nullptr)
			{
				char buf[128]{};
				string::Copy(buf, sizeof(buf), pMotionPlaying->GetName().c_str());
				ImGui::InputText("Name", buf, sizeof(buf), ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);

				if (ImGui::Button("Stop") == true)
				{
					pCompModel->StopMotion(emLayer, 0.3f);
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

void ShowMaterial(bool& isShowMaterial, graphics::IMaterial* pMaterial, int nIndex)
{
	ImGui::SetNextWindowSize(ImVec2(400, 800), ImGuiSetCond_FirstUseEver);
	ImGui::Begin(string::Format("%d. %s Info", nIndex, pMaterial->GetName().c_str()).c_str(), &isShowMaterial);

	static char bufName[128]{};
	string::Copy(bufName, pMaterial->GetName().c_str());

	ImGui::PushID(bufName);

	if (ImGui::InputText("Name", bufName, sizeof(bufName), ImGuiInputTextFlags_EnterReturnsTrue) == true)
	{
		pMaterial->SetName(bufName);
	}

	char buf[1024]{};
	string::Copy(buf, pMaterial->GetPath().c_str());
	ImGui::InputText("Path", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

	float fTessellationFactor = pMaterial->GetTessellationFactor();
	if (ImGui::DragFloat("TessellationFactor", &fTessellationFactor, 0.01f, 1.f, 512.f) == true)
	{
		pMaterial->SetTessellationFactor(fTessellationFactor);
	}

	float fStippleTransparencyFactor = pMaterial->GetStippleTransparencyFactor();
	if (ImGui::DragFloat("StippleTransparencyFactor", &fStippleTransparencyFactor, 0.01f, 0.f, 1.f) == true)
	{
		pMaterial->SetStippleTransparencyFactor(fStippleTransparencyFactor);
	}

	const char* strSamplerState[graphics::EmSamplerState::TypeCount] =
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
	if (ImGui::Combo("SamplerState", &nSamplerState, strSamplerState, graphics::EmSamplerState::TypeCount) == true)
	{
		pMaterial->SetSamplerState(static_cast<graphics::EmSamplerState::Type>(nSamplerState));
	}

	const char* strBlendState[graphics::EmBlendState::TypeCount] =
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
	if (ImGui::Combo("BlendState", &nBlendState, strBlendState, graphics::EmBlendState::TypeCount) == true)
	{
		pMaterial->SetBlendState(static_cast<graphics::EmBlendState::Type>(nBlendState));
	}

	const char* strRasterizerState[graphics::EmRasterizerState::TypeCount] =
	{
		"SolidCCW",
		"SolidCW",
		"SolidCullNone",
		"WireframeCCW",
		"WireframeCW",
		"WireframeCullNone",
	};
	int nRasterizerState = pMaterial->GetRasterizerState();
	if (ImGui::Combo("RasterizerState", reinterpret_cast<int*>(&nRasterizerState), strRasterizerState, graphics::EmRasterizerState::TypeCount) == true)
	{
		pMaterial->SetRasterizerState(static_cast<graphics::EmRasterizerState::Type>(nRasterizerState));
	}

	const char* strDepthStencilState[graphics::EmDepthStencilState::TypeCount] =
	{
		"Read_Write_On",
		"Read_Write_Off",
		"Read_On_Write_Off",
		"Read_Off_Write_On",
	};
	int nDepthStencilState = pMaterial->GetDepthStencilState();
	if (ImGui::Combo("DepthStencilState", &nDepthStencilState, strDepthStencilState, graphics::EmDepthStencilState::TypeCount) == true)
	{
		pMaterial->SetDepthStencilState(static_cast<graphics::EmDepthStencilState::Type>(nDepthStencilState));
	}

	bool isVisible = pMaterial->IsVisible();
	if (ImGui::Checkbox("Vibisle", &isVisible) == true)
	{
		pMaterial->SetVisible(isVisible);
	}

	auto TextureInfo = [&](graphics::EmMaterial::Type emType, int nIndex)
	{
		if (ImGui::Button(string::Format("%d.Texture", nIndex).c_str()) == true)
		{
			char path[512]{};

			OPENFILENAME ofn;
			Memory::Clear(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = graphics::GetHwnd();
			ofn.lpstrFilter = "Every File(*.*)\0*.*\0Text File\0*.txt;*.doc\0";
			ofn.lpstrFile = path;
			ofn.nMaxFile = 256;
			if (GetOpenFileName(&ofn) != 0)
			{
				graphics::ITexture* pTexture = graphics::CreateTextureAsync(ofn.lpstrFile);
				pMaterial->SetTextureName(emType, file::GetFileName(ofn.lpstrFile).c_str());
				pMaterial->SetTexture(emType, pTexture);
			}
		}

		const string::StringID& strName = pMaterial->GetTextureName(emType);
		char buf[1024]{};
		string::Copy(buf, strName.c_str());
		ImGui::SameLine();
		ImGui::InputText("", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

		graphics::ITexture* pTexture = pMaterial->GetTexture(emType);
		if (pTexture != nullptr)
		{
			if (ImGui::Button(string::Format("%d.Clear", nIndex).c_str()) == true)
			{
				pMaterial->SetTextureName(emType, "");
				pMaterial->SetTexture(emType, nullptr);
			}

			ImGui::SameLine();
			if (pTexture->GetState() == graphics::IResource::eComplete)
			{
				static std::unordered_map<string::StringID, bool> umapIsShowBigTexture;

				bool& isShow = umapIsShowBigTexture[pTexture->GetName()];

				ImTextureID textureID = imguiHelper::GetTextureID(pTexture);
				if (ImGui::ImageButton(textureID, ImVec2(64.f, 64.f)) == true)
				{
					isShow = !isShow;
				}

				if (isShow == true)
				{
					ImVec2 f2Size(static_cast<float>(pTexture->GetSize().x), static_cast<float>(pTexture->GetSize().y));
					f2Size.x = std::min(f2Size.x, 512.f);
					f2Size.y = std::min(f2Size.y, 512.f);

					ImGui::SetNextWindowSize(f2Size, ImGuiSetCond_FirstUseEver);
					ImGui::Begin(pTexture->GetName().c_str(), &isShow, ImGuiWindowFlags_AlwaysAutoResize);
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

			LOG_MESSAGE("%.2f, %.2f, %.2f", pMaterial->GetAlbedoColor().r, pMaterial->GetAlbedoColor().g, pMaterial->GetAlbedoColor().b);
		}
		TextureInfo(graphics::EmMaterial::eAlbedo, 1);
		bool isAlbedoAlphaChannelMaskMap = pMaterial->IsAlbedoAlphaChannelMaskMap();
		if (ImGui::Checkbox("Is Albedo Alpha Channel a Mask Map ?", &isAlbedoAlphaChannelMaskMap) == true)
		{
			pMaterial->SetAlbedoAlphaChannelMaskMap(isAlbedoAlphaChannelMaskMap);
		}
	}

	ImGui::Separator();
	{
		ImGui::Text("Normal");
		TextureInfo(graphics::EmMaterial::eNormal, 2);
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

		if (pMaterial->GetTexture(graphics::EmMaterial::eRoughness) == nullptr)
		{
			float fRoughness = pMaterial->GetRoughness();
			if (ImGui::DragFloat("Roughness", &fRoughness, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetRoughness(fRoughness);
			}
		}
		TextureInfo(graphics::EmMaterial::eRoughness, 4);
	}

	ImGui::Separator();
	{
		ImGui::Text("Metallic");

		if (pMaterial->GetTexture(graphics::EmMaterial::eMetallic) == nullptr)
		{
			float fMetallic = pMaterial->GetMetallic();
			if (ImGui::DragFloat("Metallic", &fMetallic, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetMetallic(fMetallic);
			}
		}

		TextureInfo(graphics::EmMaterial::eMetallic, 5);
	}

	ImGui::Separator();
	{
		ImGui::Text("Subsurface");

		if (pMaterial->GetTexture(graphics::EmMaterial::eSubsurface) == nullptr)
		{
			float fSubsurface = pMaterial->GetSubsurface();
			if (ImGui::DragFloat("Subsurface", &fSubsurface, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetSubsurface(fSubsurface);
			}
		}

		TextureInfo(graphics::EmMaterial::eSubsurface, 6);
	}

	ImGui::Separator();
	{
		ImGui::Text("Anisotropic");

		if (pMaterial->GetTexture(graphics::EmMaterial::eAnisotropic) == nullptr)
		{
			float fAnisotropic = pMaterial->GetAnisotropic();
			if (ImGui::DragFloat("Anisotropic", &fAnisotropic, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetAnisotropic(fAnisotropic);
			}
		}

		TextureInfo(graphics::EmMaterial::eAnisotropic, 7);
	}

	ImGui::Separator();
	{
		ImGui::Text("Sheen");

		if (pMaterial->GetTexture(graphics::EmMaterial::eSheen) == nullptr)
		{
			float fSheen = pMaterial->GetSheen();
			if (ImGui::DragFloat("Sheen", &fSheen, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetSheen(fSheen);
			}
		}

		TextureInfo(graphics::EmMaterial::eSheen, 8);
	}

	ImGui::Separator();
	{
		ImGui::Text("SheenTint");

		if (pMaterial->GetTexture(graphics::EmMaterial::eSheenTint) == nullptr)
		{
			float fSheenTint = pMaterial->GetSheenTint();
			if (ImGui::DragFloat("SheenTint", &fSheenTint, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetSheenTint(fSheenTint);
			}
		}

		TextureInfo(graphics::EmMaterial::eSheenTint, 9);
	}

	ImGui::Separator();
	{
		ImGui::Text("Clearcoat");

		if (pMaterial->GetTexture(graphics::EmMaterial::eClearcoat) == nullptr)
		{
			float fClearcoat = pMaterial->GetClearcoat();
			if (ImGui::DragFloat("Clearcoat", &fClearcoat, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetClearcoat(fClearcoat);
			}
		}

		TextureInfo(graphics::EmMaterial::eClearcoat, 10);
	}

	ImGui::Separator();
	{
		ImGui::Text("ClearcoatGloss");

		if (pMaterial->GetTexture(graphics::EmMaterial::eClearcoatGloss) == nullptr)
		{
			float fClearcoatGloss = pMaterial->GetClearcoatGloss();
			if (ImGui::DragFloat("ClearcoatGloss", &fClearcoatGloss, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetClearcoatGloss(fClearcoatGloss);
			}
		}

		TextureInfo(graphics::EmMaterial::eClearcoatGloss, 11);
	}

	ImGui::Separator();
	{
		ImGui::Text("Emissive");
		ImVec4 color = ImColor(*reinterpret_cast<const ImVec4*>(&pMaterial->GetEmissiveColor()));
		if (ImGui::ColorEdit3("Emissive Color", reinterpret_cast<float*>(&color)) == true)
		{
			pMaterial->SetEmissiveColor(*reinterpret_cast<math::Color*>(&color));
		}
		TextureInfo(graphics::EmMaterial::eEmissiveColor, 12);

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

void ShowSoundWindow(bool& isShowSoundMenu)
{
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Sound System", &isShowSoundMenu);

	ImGui::PushID("SoundSystem");

	static std::vector<std::string> vecFiles;
	if (vecFiles.empty() == true)
	{
		const std::string strSoundPath = file::GetPath(file::eSound);
		file::GetFiles(strSoundPath, ".*", vecFiles);
	}

	static std::vector<const char*> vecFilePaths;
	if (vecFilePaths.empty() == true)
	{
		vecFilePaths.reserve(vecFiles.size());

		for (auto& filePath : vecFiles)
		{
			vecFilePaths.emplace_back(filePath.c_str());
		}
	}

	static int nSelectedIndex = 0;
	if (vecFiles.empty() == false)
	{
		if (ImGui::ListBox("Sound List", &nSelectedIndex, &vecFilePaths.front(), static_cast<int>(vecFilePaths.size()), 6) == true)
		{
			sound::Play2D(vecFilePaths[nSelectedIndex]);
		}
	}

	ImGui::PopID();

	ImGui::End();
}

void SceneNewStudio::RenderImGui(float fElapsedTime)
{
	ShowConfig();

	static bool isShowDebug = true;
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
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
					char path[512]{};
					OPENFILENAME ofn;
					Memory::Clear(&ofn, sizeof(ofn));

					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = graphics::GetHwnd();
					ofn.lpstrFilter = "Json(*.json)\0*.json\0";
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

		ImGui::End();
	}

	static bool isShowSoundWindow = true;
	ShowSoundWindow(isShowSoundWindow);

	static bool isShowActorWindow = true;
	if (isShowActorWindow == true)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Actor", &isShowActorWindow);

		ImGui::PushID("Actor");

		const size_t nActorCount = gameobject::GameObjectManager::GetInstance()->GetActorCount();

		std::vector<const char*> vecActorName;
		vecActorName.reserve(nActorCount);

		gameobject::GameObjectManager::GetInstance()->ExecuteFunction([&](const gameobject::IActor* pActor)
		{
			vecActorName.emplace_back(pActor->GetName().c_str());
		});

		static int nSelectedIndex = 0;
		if (vecActorName.empty() == false)
		{
			if (nSelectedIndex >= vecActorName.size())
			{
				nSelectedIndex = 0;
			}

			ImGui::ListBox("List", &nSelectedIndex, &vecActorName.front(), static_cast<int>(vecActorName.size()), 4);
		}

		gameobject::IActor* pActor = nullptr;
		if (0 <= nSelectedIndex && nSelectedIndex < static_cast<int>(vecActorName.size()))
		{
			pActor = gameobject::GameObjectManager::GetInstance()->GetActor(nSelectedIndex);
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
					gameobject::IActor::Create(buf);
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
				gameobject::IActor::Destroy(&pActor);
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

		if (ImGui::Button("Load") == true)
		{
			char path[512]{};
			OPENFILENAME ofn;
			Memory::Clear(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = graphics::GetHwnd();
			ofn.lpstrFilter = "EastActor(*.eact)\0*.eact\0";
			ofn.lpstrFile = path;
			ofn.nMaxFile = 255;
			if (GetOpenFileName(&ofn) != 0)
			{
				if (gameobject::IActor::CreateByFile(ofn.lpstrFile) == nullptr)
				{
					LOG_ERROR("로드 실패 : %s", path);
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Save") == true && pActor != nullptr)
		{
			char path[512]{};
			OPENFILENAME ofn;
			Memory::Clear(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = graphics::GetHwnd();
			ofn.lpstrFilter = "EastActor(*.eact)\0*.eact\0";
			ofn.lpstrFile = path;
			ofn.nMaxFile = 255;
			if (GetSaveFileName(&ofn) != 0)
			{
				const std::string strFileExtension = file::GetFileExtension(path);
				if (strFileExtension.empty() == true)
				{
					string::Concat(path, sizeof(path), ".eact");
				}

				if (string::IsEqualsNoCase(file::GetFileExtension(path).c_str(), ".eact") == true)
				{
					gameobject::IActor::SaveToFile(pActor, path);
				}
			}
		}

		if (ImGui::CollapsingHeader("Common"))
		{
			if (pActor != nullptr)
			{
				static char buf[128]{};
				string::Copy(buf, pActor->GetName().c_str());

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

				std::vector<const char*> vecComponents;
				for (int i = 0; i < gameobject::EmComponent::TypeCount; ++i)
				{
					gameobject::EmComponent::Type emType = static_cast<gameobject::EmComponent::Type>(i);
					if (pActor->GetComponent(emType) == nullptr)
					{
						const char* strComponentName = gameobject::EmComponent::ToString(emType);
						if (strComponentName != nullptr)
						{
							vecComponents.emplace_back(strComponentName);
						}
					}
				}

				static int nCurItem = 0;
				if (vecComponents.empty() == false)
				{
					ImGui::PushID("Component");

					static gameobject::EmComponent::Type emType = gameobject::EmComponent::TypeCount;
					nCurItem = std::min(nCurItem, static_cast<int>(vecComponents.size() - 1));
					if (ImGui::Combo("Add Component", &nCurItem, &vecComponents.front(), static_cast<int>(vecComponents.size())) == true)
					{
						emType = static_cast<gameobject::EmComponent::Type>(nCurItem);
						if (emType != gameobject::EmComponent::TypeCount)
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
			for (int i = 0; i < gameobject::EmComponent::TypeCount; ++i)
			{
				gameobject::IComponent* pComponent = pActor->GetComponent(static_cast<gameobject::EmComponent::Type>(i));
				if (pComponent == nullptr)
					continue;

				switch (i)
				{
				case gameobject::EmComponent::eActionState:
				{
					if (ImGui::CollapsingHeader("ActionState"))
					{
					}
				}
				break;
				case gameobject::EmComponent::eTimer:
				{
					if (ImGui::CollapsingHeader("Timer"))
					{
					}
				}
				break;
				case gameobject::EmComponent::ePhysics:
				{
					if (ImGui::CollapsingHeader("Physics"))
					{
					}
				}
				break;
				case gameobject::EmComponent::eModel:
				{
					if (ImGui::CollapsingHeader("Model"))
					{
						ImGui::PushID("Model");

						gameobject::ComponentModel* pCompModel = static_cast<gameobject::ComponentModel*>(pComponent);
						if (pCompModel->GetModel() != nullptr)
						{
							if (pCompModel->IsLoadComplete() == true)
							{
								graphics::IModel* pModel = pCompModel->GetModel();

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
										char path[512]{};
										OPENFILENAME ofn;
										Memory::Clear(&ofn, sizeof(ofn));

										ofn.lStructSize = sizeof(OPENFILENAME);
										ofn.hwndOwner = graphics::GetHwnd();
										ofn.lpstrFilter = "EastModel(*.emod)\0*.emod\0";
										ofn.lpstrFile = path;
										ofn.nMaxFile = 255;
										if (GetSaveFileName(&ofn) != 0)
										{
											const std::string strFileExtension = file::GetFileExtension(path);
											if (strFileExtension.empty() == true)
											{
												string::Concat(path, sizeof(path), ".emod");
											}

											if (string::IsEqualsNoCase(file::GetFileExtension(path).c_str(), ".emod") == true)
											{
												if (graphics::IModel::SaveToFile(pModel, path) == false)
												{
													LOG_ERROR("저장 실패 : %s", path);
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

									static char bufName[128]{};
									string::Copy(bufName, pModel->GetName().c_str());

									ImGui::PushID(bufName);

									if (ImGui::InputText("Name", bufName, sizeof(bufName), ImGuiInputTextFlags_EnterReturnsTrue) == true)
									{
										pModel->ChangeName(bufName);
									}

									static char bufPath[512]{};
									string::Copy(bufPath, pModel->GetFilePath().c_str());
									ImGui::InputText("Path", bufPath, sizeof(bufPath), ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);

									int nModelNode = 0;
									int nMaterialIndex = 0;
									const uint32_t nNodeCount = pModel->GetNodeCount();
									for (uint32_t j = 0; j < nNodeCount; ++j)
									{
										graphics::IModelNode* pModelNode = pModel->GetNode(j);

										if (ImGui::TreeNode(pModelNode->GetName().c_str()))
										{
											uint32_t nVertexCount = 0;
											if (pModelNode->GetVertexBuffer() != nullptr)
											{
												nVertexCount = pModelNode->GetVertexBuffer()->GetVertexCount();
											}
											ImGui::Text("VertexCount : %u", nVertexCount);

											uint32_t nIndexCount = 0;
											if (pModelNode->GetIndexBuffer() != nullptr)
											{
												nIndexCount = pModelNode->GetIndexBuffer()->GetIndexCount();
											}
											ImGui::Text("IndexCount : %u", nIndexCount);

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
												graphics::IMaterial* pMaterial = pModelNode->GetMaterial(k);

												std::string strMtrlName = string::Format("%d. %s", nMaterialIndex, pMaterial->GetName().c_str());

												static std::unordered_map<std::string, bool> umapIsShowMaterial;
												bool& isShow = umapIsShowMaterial[strMtrlName];

												if (ImGui::Button(strMtrlName.c_str()) == true)
												{
													isShow = !isShow;
												}

												if (isShow == true)
												{
													ShowMaterial(isShow, pMaterial, nMaterialIndex);
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
							static float fScaleFactor = 1.f;

							if (ImGui::Button("Load Model") == true)
							{
								char path[512]{};

								OPENFILENAME ofn;
								Memory::Clear(&ofn, sizeof(ofn));

								ofn.lStructSize = sizeof(OPENFILENAME);
								ofn.hwndOwner = graphics::GetHwnd();
								ofn.lpstrFilter = "Model File(*.fbx;*.obj;*.emod)\0*.fbx;*.obj;*.emod\0FBX File(*.fbx)\0*.fbx\0Obj File(*.obj)\0*.obj\0EastModel File(*.emod)\0*.emod\0";
								ofn.lpstrFile = path;
								ofn.nMaxFile = 256;
								if (GetOpenFileName(&ofn) != 0)
								{
									const std::string strFileExtension = file::GetFileExtension(ofn.lpstrFile);
									if (string::IsEqualsNoCase(strFileExtension.c_str(), ".fbx") == true)
									{
										string::StringID strName;
										strName.Format("%s(%f)", file::GetFileName(ofn.lpstrFile).c_str(), fScaleFactor);

										graphics::ModelLoader loader;
										loader.InitFBX(strName, ofn.lpstrFile, fScaleFactor);
										loader.SetEnableThreadLoad(true);

										pCompModel->Init(&loader);
									}
									else if (string::IsEqualsNoCase(strFileExtension.c_str(), ".obj") == true)
									{
										string::StringID strName;
										strName.Format("%s(%f)", file::GetFileName(ofn.lpstrFile).c_str(), fScaleFactor);

										graphics::ModelLoader loader;
										loader.InitObj(strName, ofn.lpstrFile, fScaleFactor);
										loader.SetEnableThreadLoad(true);

										pCompModel->Init(&loader);
									}
									else if (string::IsEqualsNoCase(strFileExtension.c_str(), ".emod") == true)
									{
										graphics::ModelLoader loader;
										loader.InitEast(file::GetFileName(ofn.lpstrFile).c_str(), ofn.lpstrFile);
										loader.SetEnableThreadLoad(true);

										pCompModel->Init(&loader);
									}
								}
							}

							ImGui::SameLine();

							ImGui::DragFloat("ScaleFactor", &fScaleFactor, 0.0001f, 0.0001f, 10000.f, "%.4f");

							if (ImGui::Button("Geometry Model") == true)
							{
								ImGui::OpenPopup("Load Geometry Model");
							}

							if (ImGui::BeginPopupModal("Load Geometry Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize) == true)
							{
								ImGui::PushID("Geometry");

								static std::vector<const char*> vecGeometryModel;
								if (vecGeometryModel.empty() == true)
								{
									for (int j = graphics::ModelLoader::eCube; j < graphics::ModelLoader::eCapsule; ++j)
									{
										const char* strName = graphics::ModelLoader::GetGeometryTypeToString(static_cast<graphics::ModelLoader::GeometryType>(j));
										vecGeometryModel.emplace_back(strName);
									}
								}

								static int nSelectedGeometryIndex = 0;
								ImGui::Combo("Type", &nSelectedGeometryIndex, &vecGeometryModel.front(), static_cast<int>(vecGeometryModel.size()));

								graphics::ModelLoader::GeometryType emType = graphics::ModelLoader::GetGeometryType(vecGeometryModel[nSelectedGeometryIndex]);
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

										pCompModel->Init(&loader);

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

									static math::float3 f3Size(math::float3::One);
									ImGui::DragFloat3("Box Size", reinterpret_cast<float*>(&f3Size.x), 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										graphics::ModelLoader loader;
										loader.InitBox(buf, nullptr, f3Size);

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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

										pCompModel->Init(&loader);

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
				case gameobject::EmComponent::eCamera:
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