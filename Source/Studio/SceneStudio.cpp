#include "stdafx.h"
#include "SceneStudio.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"
#include "CommonLib/Performance.h"

#include "DirectX/Camera.h"
#include "DirectX/Light.h"
#include "DirectX/MaterialNode.h"
#include "DirectX/OcclusionCulling.h"

#include "Renderer/ASSAO.h"
#include "Renderer/DepthOfField.h"

#include "Model/ModelManager.h"
#include "Model/MotionManager.h"

#include "Particle/ParticleInterface.h"

#include "Windows/Windows.h"

#include "Input/InputInterface.h"

#include "Renderer/DepthOfField.h"
#include "Renderer/HDRFilter.h"
#include "Renderer/SSS.h"
#include "Renderer/ColorGrading.h"
#include "Renderer/BloomFilter.h"

#include "GameObject/GameObject.h"
#include "GameObject/ActorManager.h"
#include "GameObject/ComponentModel.h"
#include "GameObject/ComponentPhysics.h"
#include "GameObject/ComponentTimer.h"

#include "Contents/Sun.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"

#include "SkeletonController.h"
#include "MaterialNodeManager.h"

using namespace eastengine;

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool HandleMsg(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	static std::string strPrevUniCode;

	auto MsgProc = ImGui_ImplDX11_WndProcHandler;

	switch (nMsg)
	{
	case WM_IME_STARTCOMPOSITION:
		return true;
	case WM_IME_ENDCOMPOSITION:
		return false;
	case WM_IME_COMPOSITION:
	{
		HIMC himc = ImmGetContext(hWnd);
		if (himc != nullptr)
		{
			long lRet = 0;
			wchar_t szCompStr[256] = { 0 };

			// 문자열 조합 완성
			// imgui에서 아직 유니코드 문자열이 입력되는 걸 완벽하게 지원하지 않는다.
			// 그래서 조합 중인 문자열을 imgui 에 넣는 건 힘들다.
			// 일단은 완성된 문자열만 imgui 에 추가하도록 처리.
			if (lParam & GCS_RESULTSTR)
			{
				lRet = ImmGetCompositionStringW(himc, GCS_RESULTSTR, szCompStr, 256) / sizeof(wchar_t);
				szCompStr[lRet] = 0;

				strPrevUniCode = String::WideToMulti(szCompStr);

				if (lRet > 0)
				{
					MsgProc(hWnd, WM_CHAR, static_cast<WPARAM>(szCompStr[0]), lParam);
				}
			}
		}
		ImmReleaseContext(hWnd, himc);
		return true;
	}
	default:
	{
		if (nMsg == WM_CHAR && strPrevUniCode.empty() == false)
		{
			static int nIdx = 0;
			static WPARAM temp = 0;
			if (strPrevUniCode[nIdx] == static_cast<char>(wParam))
			{
				if (nIdx == 0)
				{
					temp = wParam;
					++nIdx;
				}
				else
				{
					strPrevUniCode.clear();
					nIdx = 0;
				}
			}
			else
			{
				strPrevUniCode.clear();

				if (nIdx != 0)
				{
					MsgProc(hWnd, WM_CHAR, temp, lParam);

					temp = 0;
					nIdx = 0;
				}

				return SUCCEEDED(MsgProc(hWnd, nMsg, wParam, lParam));
			}
		}
		else
		{
			return SUCCEEDED(MsgProc(hWnd, nMsg, wParam, lParam));
		}
		break;
	}
	}

	return false;
}

namespace StrID
{
	RegisterStringID(Studio);
	RegisterStringID(Studio_Ground);

	RegisterStringID(None);
	RegisterStringID(EastEngine_Sun);
}

const std::array<const char*, 12> IBL_Type =
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
	: IScene(StrID::Studio)
	, m_pSkeletonController(nullptr)
	, m_pMaterialNodeManager(nullptr)
	, m_pSectorMgr(nullptr)
{
}

SceneStudio::~SceneStudio()
{
	SafeDelete(m_pSkeletonController);
	SafeDelete(m_pMaterialNodeManager);
}

void SceneStudio::Enter()
{
	Windows::WindowsManager::GetInstance()->AddMessageHandler(HandleMsg);

	HWND hWnd = Windows::GetHwnd();
	ID3D11Device* pd3dDevice = graphics::GetDevice()->GetInterface();
	ID3D11DeviceContext* pd3dDeviceContext = graphics::GetImmediateContext()->GetInterface();
	ImGui_ImplDX11_Init(hWnd, pd3dDevice, pd3dDeviceContext);

	graphics::GraphicsSystem::GetInstance()->AddFuncAfterRender([&]()
	{
		RenderUI();
	});

	std::string strFontPath = file::GetPath(file::eFont);
	strFontPath.append("ArialUni.ttf");
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(strFontPath.c_str(), 16.f, nullptr, io.Fonts->GetGlyphRangesKorean());

	graphics::Camera::GetInstance()->SetView(math::Vector3(0.f, 1.f, -10.f), math::Vector3(0.f, 50.f, 0.f), math::Vector3(0.f, 1.f, 0.f));

	{
		math::Vector3 f3LightPosition(0.f, 500.f, -500.f);
		math::Vector3 f3LightDirection(math::Vector3::Zero - f3LightPosition);
		f3LightDirection.Normalize();

		graphics::CascadedShadowsConfig config;
		config.nLevel = 3;
		config.nBufferSize = 2048;
		config.fCascadeDistance = 256.f;

		graphics::ILight* pLight = graphics::ILight::CreateDirectionalLight("MainLight", f3LightDirection, math::Color::White, 1.f, 0.5f, 0.25f, &config);
		pLight->SetEnableShadow(false);
	}
	//{
	//	math::Vector3 f3LightPosition(500.f, 500.f, 0.f);
	//	math::Vector3 f3LightDirection(math::Vector3::Zero - f3LightPosition);
	//	f3LightDirection.Normalize();
	//
	//	graphics::CascadedShadowsConfig config;
	//	config.nLevel = 3;
	//	config.nBufferSize = 2048;
	//	config.fCascadeDistance = 256.f;
	//
	//	graphics::ILight::CreateDirectionalLight("MainLight2", f3LightDirection, math::Color::White, 10000.f, 0.5f, 0.25f, &config);
	//}
	
	std::string strPath = file::GetPath(file::eTexture);
	{
		strPath = String::Format("%sIBL\\%s\\%s", file::GetPath(file::eTexture), IBL_Type[0], IBL_Type[0]);

		graphics::IImageBasedLight* pIBL = graphics::GetImageBasedLight();

		std::string strDiffuseHDR = strPath;
		strDiffuseHDR.append("DiffuseHDR.dds");
		std::shared_ptr<graphics::ITexture> pDiffuseHDR = graphics::ITexture::Create(strDiffuseHDR);
		pIBL->SetDiffuseHDR(pDiffuseHDR);

		std::string strSpecularHDR = strPath;
		strSpecularHDR.append("SpecularHDR.dds");
		std::shared_ptr<graphics::ITexture> pSpecularHDR = graphics::ITexture::Create(strSpecularHDR);
		pIBL->SetSpecularHDR(pSpecularHDR);

		std::string strSpecularBRDF = strPath;
		strSpecularBRDF.append("Brdf.dds");
		std::shared_ptr<graphics::ITexture> pSpecularBRDF = graphics::ITexture::Create(strSpecularBRDF);
		pIBL->SetSpecularBRDF(pSpecularBRDF);

		gameobject::SkyboxProperty sky;

		sky.strTexSky = strPath;
		sky.strTexSky.append("EnvHDR.dds");

		sky.fBoxSize = 1.f;

		m_pSkybox = gameobject::ISkybox::Create("BaseSkybox", sky);
	}

	{
		auto pActor = gameobject::ActorManager::GetInstance()->CreateActor(StrID::Studio_Ground);
	
		graphics::MaterialInfo material;
		material.strName = StrID::Studio_Ground;

		graphics::ModelLoader loader;
		loader.InitPlane(StrID::Studio_Ground, 1.f, 1.f, 100, 100, &material);
	
		auto pCompModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));
		pCompModel->Init(&loader);
	
		auto pModelInst = pCompModel->GetModelInstance();
	
		auto pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::EmComponent::ePhysics));
	
		Physics::RigidBodyProperty prop;
		prop.fRestitution = 0.75f;
		prop.strName = StrID::Studio_Ground;
		prop.fMass = 0.f;
		prop.nCollisionFlag = Physics::EmCollision::eStaticObject;
		prop.shapeInfo.SetTriangleMesh();
		pCompPhysics->Init(pModelInst, prop);
	
		/*gameobject::SectorInitInfo sectorInitInfo;
		sectorInitInfo.fRadius = 10.f;
		for (auto& direction : sectorInitInfo.nSectorsCount)
		{
			direction = 10;
		}
	
		m_pSectorMgr = CreateSectorMgr(sectorInitInfo);*/
	}

	if (false)
	{
		graphics::MaterialInfo materialInfo;
		materialInfo.strName = "TestDecal";
		materialInfo.strPath = file::GetPath(file::eTexture);

		materialInfo.strTextureNameArray[graphics::EmMaterial::eAlbedo] = "Albedo.tga";
		materialInfo.strTextureNameArray[graphics::EmMaterial::eNormal] = "Normal.tga";
		//materialInfo.strTextureNameArray[graphics::EmMaterial::eSpecularColor] = "Specular.tga";
		materialInfo.strTextureNameArray[graphics::EmMaterial::eRoughness] = "Roughness.tga";
		materialInfo.strTextureNameArray[graphics::EmMaterial::eMetallic] = "Metallic.tga";

		materialInfo.f4PaddingRoughMetEmi.y = 0.5f;
		materialInfo.f4PaddingRoughMetEmi.z = 0.5f;

		materialInfo.emDepthStencilState = graphics::EmDepthStencilState::eRead_Write_Off;

		graphics::ParticleDecalAttributes attributes;
		attributes.strDecalName = "TestDecal";
		attributes.f3Scale = math::Vector3(2.f, 1.f, 2.f);
		attributes.pMaterialInfo = &materialInfo;

		graphics::IParticleDecal::Create(attributes);;
	}

	graphics::IMaterial* pMaterial_override = nullptr;
	//for (int j = 0; j < 5; ++j)
	//{
	//	for (int i = 0; i < 50; ++i)
	//	{
	//		/*graphics::MaterialInfo materialInfo;
	//		materialInfo.strName.Format("TestBox%d", (i % 10) + 1);
	//		materialInfo.strPath = file::GetPath(file::eTexture);

	//		materialInfo.strTextureNameArray[graphics::EmMaterial::eAlbedo].Format("Pattern\\pattern_%02d\\%s", (i % 10) + 1, "diffus.tga");
	//		materialInfo.strTextureNameArray[graphics::EmMaterial::eNormal].Format("Pattern\\pattern_%02d\\%s", (i % 10) + 1, "Normal.tga");
	//		materialInfo.strTextureNameArray[graphics::EmMaterial::eSpecularColor].Format("Pattern\\pattern_%02d\\%s", (i % 10) + 1, "specular.tga");
	//		*/
	//		graphics::MaterialInfo materialInfo;
	//		materialInfo.strName = "TestBox";
	//		materialInfo.strPath = file::GetPath(file::eTexture);

	//		materialInfo.strTextureNameArray[graphics::EmMaterial::eAlbedo].Format("Pattern\\pattern_01\\%s", "diffus.tga");
	//		materialInfo.strTextureNameArray[graphics::EmMaterial::eNormal].Format("Pattern\\pattern_01\\%s", "Normal.tga");
	//		materialInfo.strTextureNameArray[graphics::EmMaterial::eSpecularColor].Format("Pattern\\pattern_01\\%s", "specular.tga");

	//		//materialInfo.f4PaddingRoughMetEmi.y = 0.1f * ((i % 10) + 1);
	//		//materialInfo.f4PaddingRoughMetEmi.z = 1.f - 0.1f * ((i % 10) + 1);

	//		materialInfo.f4PaddingRoughMetEmi.y = 0.5f;
	//		materialInfo.f4PaddingRoughMetEmi.z = 0.5f;

	//		//materialInfo.rasterizerStateDesc = graphics::GetDevice()->GetRasterizerStateDesc(graphics::EmRasterizerState::eNone);
	//		//materialInfo.colorAlbedo = math::Color(math::Random(0.f, 1.f), math::Random(0.f, 1.f), math::Random(0.f, 1.f), 1.f);

	//		gameobject::IActor* pActor = gameobject::ActorManager::GetInstance()->CreateActor("TestBox");

	//		math::Vector3 f3Pos;
	//		f3Pos.x = -4.f + (i % 5) * 3.f;
	//		f3Pos.y = 100.5f + (j * 3.f);
	//		f3Pos.z = -4.f + (i / 5) * 3.f;

	//		pActor->SetPosition(f3Pos);

	//		gameobject::ComponentModel* pCompModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));

	//		graphics::ModelLoader loader;
	//		//loader.InitBox(String::Format("TestBox%d", (i % 10) + 1).c_str(), &materialInfo);
	//		loader.InitBox("TestBox", &materialInfo);
	//		pCompModel->Init(&loader);
	//		auto pModelInst = pCompModel->GetModelInstance();

	//		if (i % 2 == 0)
	//		{
	//			if (pMaterial_override == nullptr)
	//			{
	//				graphics::MaterialInfo materialInfo2;
	//				materialInfo2.strName = "TestBox";
	//				materialInfo2.strPath = file::GetPath(file::eTexture);

	//				materialInfo2.strTextureNameArray[graphics::EmMaterial::eAlbedo].Format("Pattern\\pattern_02\\%s", "diffus.tga");
	//				materialInfo2.strTextureNameArray[graphics::EmMaterial::eNormal].Format("Pattern\\pattern_02\\%s", "Normal.tga");
	//				materialInfo2.strTextureNameArray[graphics::EmMaterial::eSpecularColor].Format("Pattern\\pattern_02\\%s", "specular.tga");

	//				pMaterial_override = graphics::IMaterial::Create(&materialInfo2);
	//			}
	//			pModelInst->ChangeMaterial("EastEngine_Box", 0, pMaterial_override);
	//		}

	//		gameobject::ComponentPhysics* pCompPhysics = static_cast<gameobject::ComponentPhysics*>(pActor->CreateComponent(gameobject::EmComponent::ePhysics));

	//		Physics::RigidBodyProperty prop;
	//		prop.fRestitution = 0.5f;
	//		prop.strName.Format("TestBox_RigidBody%d", i).c_str();

	//		prop.shapeInfo.SetBox(math::Vector3(1.f));
	//		//prop.shapeInfo.SetCapsule(math::Random(0.5f, 1.f), math::Random(1.f, 2.f));
	//		prop.nCollisionFlag = Physics::EmCollision::eCharacterObject;
	//		prop.f3OriginPos = f3Pos;
	//		pCompPhysics->Init(prop);
	//	}
	//}

	if (false)
	{
		for (int i = 0; i < 10; ++i)
		{
			String::StringID strName;
			strName.Format("SunLight_%d", i);
	
			math::Color lightColor(math::Random(0.f, 1.f), math::Random(0.f, 1.f), math::Random(0.f, 1.f), 1.f);
	
			math::Vector3 f3Position(-50.f + 10.f * i, 5.f, 0.f);
			//math::Vector3 f3Position(0.f, 5.f, 0.f);
			math::Vector3 f3Direction = math::Vector3(0.f, 0.f, 0.f) - math::Vector3(0.f, 1.f, math::Random(-2.f, 2.f));
			//math::Vector3 f3Direction = math::Vector3(0.f, 0.f, 0.f) - math::Vector3(0.f, 1.f, -2.f);
			f3Direction.Normalize();
	
			Contents::Sun* pSun = new Contents::Sun;

			graphics::ShadowConfig shadowConfig;
			shadowConfig.nBufferSize = 1024;

			graphics::ILight* pSpotLight = graphics::ILight::CreateSpotLight(strName, f3Position, f3Direction, math::Random(20.f, 60.f), lightColor, 100.f * (i + 1), 0.1f, 0.2f, &shadowConfig);
			//graphics::ILight* pSpotLight = graphics::ILight::CreateSpotLight(strName, f3Position, f3Direction, 90.f, lightColor, 100.f * (i + 1), 0.1f, 0.2f, &shadowConfig);
			pSpotLight->SetEnableShadow(false);

			//graphics::ILight* pPointLight = graphics::ILight::CreatePointLight(strName, f3Position, lightColor, 100.f * (i + 1), 0.1f, 0.2f, &shadowConfig);

			if (pSun->Init(StrID::EastEngine_Sun, pSpotLight, 1.f, math::Vector3::Zero, math::Vector3(0.f, 0.f, 0.f), math::Vector3(0.f, 9500.f, 0.f)) == true)
			{
				pSun->GetActor()->SetPosition(f3Position);
				m_vecSuns.push_back(pSun);
			}
			else
			{
				SafeDelete(pSun);
			}
		}
	}

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

		terrain.f3Position = { -500.f, 0.f, -500.f };

		//gameobject::ITerrain::Create("BaseTerrain", terrain);

		// 백그라운드 로딩은 이렇게 쓰면됨
		//gameobject::ITerrain::CreateAsync("BaseTerrain", terrain);
	}

	auto CreateActor = [](const String::StringID& strActorName, const char* strModelFilePath, 
		const math::Vector3& f3Position,
		graphics::EmModelLoader::LoadType emModelType = graphics::EmModelLoader::eEast)
	{
		gameobject::IActor* pActor = gameobject::IActor::Create(strActorName);

		pActor->SetPosition(f3Position);

		graphics::ModelLoader loader;

		String::StringID strFileName = file::GetFileName(strModelFilePath).c_str();
		switch (emModelType)
		{
		case graphics::EmModelLoader::LoadType::eFbx:
		case graphics::EmModelLoader::LoadType::eObj:
			loader.InitFBX(strFileName, strModelFilePath, 0.01f);
			break;
		case graphics::EmModelLoader::LoadType::eXps:
			loader.InitXPS(strFileName, strModelFilePath);
			break;
		case graphics::EmModelLoader::LoadType::eEast:
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
		String::StringID name;
		name.Format("UnityChan%d", i);

		strPath = file::GetDataPath();
		strPath.append("Actor\\UnityChan\\unitychan.emod");

		math::Vector3 pos;
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
			strPathMotion.append(vecAnim[math::Random(0u, vecAnim.size() - 1)]);

			String::StringID strMotionName;
			strMotionName.Format("%s", file::GetFileName(strPathMotion).c_str());
			graphics::MotionLoader motionLoader;
			motionLoader.InitFBX(strMotionName, strPathMotion.c_str(), 0.01f);
			graphics::IMotion* pMotion = graphics::IMotion::Create(motionLoader);

			graphics::MotionPlaybackInfo playback;
			//playback.fSpeed = math::Random(0.5f, 1.5f);
			playback.fSpeed = 1.f;
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

		//	String::StringID strMotionName;
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
		
		//math::Vector3 ragdollPos = pActor->GetPosition();
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

			math::Vector3 f3Pos = { 0.08f, 0.225f, -0.02f };
			math::Quaternion quat = math::Quaternion::CreateFromYawPitchRoll(math::ToRadians(90.f), math::ToRadians(180.f), 0.f);

			pModelInstance->Attachment(pModelInstance_Attach, "Character1_LeftHand", math::Matrix::Compose(math::Vector3::One, quat, f3Pos));
		}
	}

	{
		String::StringID name;
		name.Format("KimJiYoon");

		math::Vector3 pos;
		pos.x = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\KimJiYoon\\KimJiYoon.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));

		//if (false)
		{
			std::string strPathMotion(file::GetDataPath());
			strPathMotion.append("Model\\KimJiYoon\\AR_Idle_CC.fbx");

			String::StringID strMotionName;
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
		String::StringID name;
		name.Format("2B_NierAutomata_%d", i);

		math::Vector3 pos;
		pos.x = -2.f + (i * -2.f);

		strPath = file::GetDataPath();
		strPath.append("Model\\2B_NierAutomata\\2B_NierAutomata.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));

		graphics::IModelInstance* pModelInstance = pModel->GetModelInstance();

		if (i == 1)
		{
			graphics::IModelNode* pNode = pModelInstance->GetModel()->GetNode("Generic_Item.mesh");

			auto SetMaterialVisible = [&](const String::StringID& strMaterialName)
			{
				uint32_t nMaterialID = 0;
				graphics::IMaterial* pMaterial = pNode->GetMaterial(strMaterialName, nMaterialID);

				graphics::IMaterial* pMaterialClone = graphics::IMaterial::Clone(pMaterial);
				pMaterialClone->SetVisible(false);

				pModelInstance->ChangeMaterial("Generic_Item.mesh", nMaterialID, pMaterialClone);
			};

			//SetMaterialVisible("Skirt");
			SetMaterialVisible("Eyepatch");
		}
	}

	//if (false)
	{
		String::StringID name;
		name.Format("Delia");

		math::Vector3 pos;
		pos.x = 4.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Delia\\Delia.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("Misaki");

		math::Vector3 pos;
		pos.x = 6.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Misaki\\Misaki.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("Naotora");

		math::Vector3 pos;
		pos.x = 8.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Naotora\\Naotora.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("Naotora_ShirtDress");

		math::Vector3 pos;
		pos.x = 10.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Naotora_ShirtDress\\Naotora_ShirtDress.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("Bugeikloth");

		math::Vector3 pos;
		pos.z = 10.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Bugeikloth\\Bugeikloth.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("DarkKnight_Female");

		math::Vector3 pos;
		pos.x = -4.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Dark Knight_Female\\DarkKnight_Female.emod");

		//gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos, graphics::EmModelLoader::eXps);
		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("DarkKnight_Transformed_Female");

		math::Vector3 pos;
		pos.x = -2.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Dark Knight Transformed_Female\\DarkKnight_Transformed_Female.emod");

		//gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos, graphics::EmModelLoader::eXps);
		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("Paladin_Female");

		math::Vector3 pos;
		pos.x = 0.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Paladin_Female\\Paladin_Female.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("Paladin_Transformed_Female");

		math::Vector3 pos;
		pos.x = 2.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Paladin Transformed_Female\\Paladin_Transformed_Female.emod");

		//gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos, graphics::EmModelLoader::eXps);
		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("Evie_Temptress");

		math::Vector3 pos;
		pos.x = 4.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Evie_Temptress\\Evie_Temptress.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	//if (false)
	{
		String::StringID name;
		name.Format("Lynn_DancingBlade");

		math::Vector3 pos;
		pos.x = 6.f;
		pos.z = 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\Lynn_DancingBlade\\Lynn_DancingBlade.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

	{
		String::StringID name;
		name.Format("ElementalSwordIce");

		math::Vector3 pos;
		pos.y = 1.f;
		pos.z -= 2.f;

		strPath = file::GetDataPath();
		strPath.append("Model\\ElementalSwordIce\\LP.emod");

		gameobject::IActor* pActor = CreateActor(name, strPath.c_str(), pos);
	}

#ifndef _DEBUG
	for (int i = 0; i < 110; ++i)
	{
		const int x = i % 11;
		const int z = i / 11;

		const float fValue = static_cast<float>(x) / 10.f;

		enum BrdfProperty
		{
			eRoughness = 0,
			eMetallic,
			eSpecular,
			eSpecularTint,
			eSubserface,
			eAnisotropic,
			eSheen,
			eSheenTint,
			eClearcoat,
			eClearcoatGloss,

			BrdfPropertyCount,
		};

		const std::array<const math::Color, BrdfPropertyCount> color =
		{
			math::Color::White,
			math::Color::Gold,
			math::Color::Red,
			math::Color::Red,
			math::Color::Violet,
			math::Color::Purple,
			math::Color::Brown,
			math::Color::Brown,
			math::Color::BlueViolet,
			math::Color::BlueViolet,
		};

		const std::array<const std::string, BrdfPropertyCount> propertyName =
		{
			"Roughness",
			"Metallic",
			"Specular",
			"SpecularTint",
			"Subserface",
			"Anisotropic",
			"Sheen",
			"SheenTint",
			"Clearcoat",
			"ClearcoatGloss",
		};

		String::StringID name;
		name.Format("Standard_Sphere_%s_%.1f", propertyName[z].c_str(), fValue);

		gameobject::IActor* pActor = gameobject::IActor::Create(name);
		pActor->SetPosition({ -2.5f + (x * 0.6f), 0.25f, -4.f - (z * 0.75f) });

		graphics::MaterialInfo materialInfo;
		materialInfo.colorAlbedo = color[z];

		switch (z)
		{
		case BrdfProperty::eRoughness:
			materialInfo.f4PaddingRoughMetEmi.y = fValue;
			break;
		case BrdfProperty::eMetallic:
			materialInfo.f4PaddingRoughMetEmi.z = fValue;
			break;
		case BrdfProperty::eSpecular:
			materialInfo.f4PaddingRoughMetEmi.y = 0.3f;
			materialInfo.f4SurSpecTintAniso.y = fValue;
			break;
		case BrdfProperty::eSpecularTint:
			materialInfo.f4PaddingRoughMetEmi.y = 0.5f;
			materialInfo.f4SurSpecTintAniso.y = 1.f;
			materialInfo.f4SurSpecTintAniso.z = fValue;
			break;
		case BrdfProperty::eSubserface:
			materialInfo.f4SurSpecTintAniso.x = fValue;
			break;
		case BrdfProperty::eAnisotropic:
			materialInfo.f4PaddingRoughMetEmi.y = 0.5f;
			materialInfo.f4PaddingRoughMetEmi.z = 0.5f;
			materialInfo.f4SurSpecTintAniso.w = fValue;
			break;
		case BrdfProperty::eSheen:
			materialInfo.f4SheenTintClearcoatGloss.x = fValue;
			break;
		case BrdfProperty::eSheenTint:
			materialInfo.f4SheenTintClearcoatGloss.x = 1.f;
			materialInfo.f4SheenTintClearcoatGloss.y = fValue;
			break;
		case BrdfProperty::eClearcoat:
			materialInfo.f4SheenTintClearcoatGloss.z = fValue;
			break;
		case BrdfProperty::eClearcoatGloss:
			materialInfo.f4SheenTintClearcoatGloss.z = 1.f;
			materialInfo.f4SheenTintClearcoatGloss.w = fValue;
			break;
		}

		graphics::ModelLoader loader;
		loader.InitSphere(name, &materialInfo, 0.5f, 32u);
		loader.SetEnableThreadLoad(false);

		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));
		pModel->Init(&loader);
	}

	for (int i = 0; i < 36; ++i)
	{
		const int x = i % 6;
		const int z = i / 6;

		String::StringID name;
		name.Format("PBR_MetalPlates_%d", i);

		gameobject::IActor* pActor = gameobject::IActor::Create(name);
		pActor->SetPosition({ -2.5f + (x * 0.6f), 0.25f, -13.f - (z * 0.75f) });

		graphics::MaterialInfo materialInfo;
		materialInfo.strPath = file::GetDataPath();
		materialInfo.strPath.append("Model\\MetalPlates\\");
		materialInfo.strTextureNameArray[graphics::EmMaterial::eAlbedo] = "MetalPlates_Diffuse.tga";
		materialInfo.strTextureNameArray[graphics::EmMaterial::eNormal].Format("MetalPlates_Normal_%02d.tga", i + 1);
		materialInfo.strTextureNameArray[graphics::EmMaterial::eMetallic] = "MetalPlates_Metalness.tga";
		materialInfo.f4PaddingRoughMetEmi.y = 0.25f;
		materialInfo.f4PaddingRoughMetEmi.z = 1.f;
		materialInfo.f4SurSpecTintAniso.y = 1.f;
		materialInfo.f4SurSpecTintAniso.z = 1.f;
		materialInfo.f4SheenTintClearcoatGloss.z = 1.f;
		materialInfo.f4SheenTintClearcoatGloss.w = 1.f;

		graphics::ModelLoader loader;
		loader.InitSphere(name, &materialInfo, 0.5f, 32u);
		loader.SetEnableThreadLoad(false);

		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(pActor->CreateComponent(gameobject::EmComponent::eModel));
		pModel->Init(&loader);
	}
#endif

	m_pSkeletonController = new SkeletonController;
	m_pMaterialNodeManager = new MaterialNodeManager;
}

void SceneStudio::Exit()
{
	std::for_each(m_vecSuns.begin(), m_vecSuns.end(), DeleteSTLObject());
	m_vecSuns.clear();

	gameobject::ISkybox::Destroy(&m_pSkybox);
	SafeDelete(m_pSkeletonController);
	SafeDelete(m_pMaterialNodeManager);
	
	ImGui_ImplDX11_Shutdown();
}

void SceneStudio::Update(float fElapsedTime)
{
	TRACER_EVENT("SceneStudio::Update");

	ImGuiIO& io = ImGui::GetIO();

	TRACER_BEGINEVENT("SceneStudio::Update", "SkeletonController");
	bool isProcessedMouseInput = m_pSkeletonController->Process(fElapsedTime);
	TRACER_ENDEVENT();

	if (io.WantCaptureMouse == false)
	{
		if (isProcessedMouseInput == false)
		{
			TRACER_BEGINEVENT("SceneStudio::Update", "SkeletonController");
			ProcessInput(fElapsedTime);
			TRACER_ENDEVENT();
		}
	}

	if (m_pSectorMgr != nullptr)
	{
		TRACER_BEGINEVENT("SceneStudio::Update", "SkeletonController");
		m_pSectorMgr->Update(fElapsedTime);
		TRACER_ENDEVENT();
	}
}

void SceneStudio::ProcessInput(float fElapsedTime)
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
		static float fTime = 0.f;
		if (fTime >= 1.f)
		{
			LOG_MESSAGE("DisConnected");
			fTime -= 1.f;
		}

		fTime += fElapsedTime;
	}
}

void SceneStudio::ShowConfig()
{
	static bool isShowMainMenu = true;

	ImGui::SetNextWindowSize(ImVec2(400, 800), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Config", &isShowMainMenu);

	if (ImGui::CollapsingHeader("Debug") == true)
	{
		if (ImGui::TreeNode("GBuffer"))
		{
			ImGui::PushID("GBuffer");

			auto ShowGBuffer = [](const String::StringID& strName, std::shared_ptr<graphics::ITexture> pTexture)
			{
				static std::unordered_map<String::StringID, bool> umapIsShowBigTexture;

				bool& isVisible = umapIsShowBigTexture[pTexture->GetName()];

				ImGui::Text(strName.c_str());

				ImTextureID textureID = pTexture->GetShaderResourceView();
				if (ImGui::ImageButton(textureID, ImVec2(128, 128)) == true)
				{
					isVisible = !isVisible;
				}

				if (isVisible == true)
				{
					ImVec2 f2Size(static_cast<float>(pTexture->GetSize().x), static_cast<float>(pTexture->GetSize().y));
					f2Size.x = math::Min(f2Size.x, 512.f);
					f2Size.y = math::Min(f2Size.y, 512.f);

					ImGui::SetNextWindowSize(f2Size, ImGuiSetCond_FirstUseEver);
					ImGui::Begin(pTexture->GetName().c_str(), &isVisible, ImGuiWindowFlags_AlwaysAutoResize);
					ImGui::Image(textureID, f2Size);
					ImGui::End();
				}
			};

			ShowGBuffer("Depth", graphics::GetDevice()->GetMainDepthStencil()->GetTexture());

			//graphics::IGBuffers* pGBuffer = graphics::GetDevice()->GetGBuffers();
			//ShowGBuffer("Normals : rg(normal), ba(tangent)", pGBuffer->GetGBuffer(graphics::EmGBuffer::eNormals)->GetTexture());
			//ShowGBuffer("Colors : r(position), g(none), ba(emissive)", pGBuffer->GetGBuffer(graphics::EmGBuffer::eColors)->GetTexture());
			//ShowGBuffer("DisneyBRDF", pGBuffer->GetGBuffer(graphics::EmGBuffer::eDisneyBRDF)->GetTexture());

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("OcclusionCulling"))
		{
			ImGui::PushID("OcclusionCulling");

			bool isEnableOcclusionCulling = Config::IsEnable("OcclusionCulling"_s);
			if (ImGui::Checkbox("Apply", &isEnableOcclusionCulling) == true)
			{
				Config::SetEnable("OcclusionCulling"_s, isEnableOcclusionCulling);
			}

			if (ImGui::Button("SaveImageFile") == true)
			{
				graphics::OcclusionCulling::GetInstance()->Write("image.bmp");
			}

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("CollisionMesh"))
		{
			ImGui::PushID("CollisionMesh");

			bool isApplyVisibleCollisionMesh = Config::IsEnable("VisibleCollisionMesh"_s);
			if (ImGui::Checkbox("Visible", &isApplyVisibleCollisionMesh) == true)
			{
				Config::SetEnable("VisibleCollisionMesh"_s, isApplyVisibleCollisionMesh);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Skeleton"))
		{
			ImGui::PushID("Skeleton");

			bool isApplyVisibleCollisionMesh = Config::IsEnable("VisibleSkeleton"_s);
			if (ImGui::Checkbox("Visible", &isApplyVisibleCollisionMesh) == true)
			{
				Config::SetEnable("VisibleSkeleton"_s, isApplyVisibleCollisionMesh);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Graphics") == true)
	{
		if (ImGui::TreeNode("VSync"))
		{
			ImGui::PushID("VSync");

			bool isVSync = graphics::GetDevice()->IsVSync();
			if (ImGui::Checkbox("Apply", &isVSync) == true)
			{
				graphics::GetDevice()->SetVSync(isVSync);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Shadow"))
		{
			ImGui::PushID("Shadow");

			bool isApplyShadow = Config::IsEnable("Shadow"_s);
			if (ImGui::Checkbox("Apply", &isApplyShadow) == true)
			{
				Config::SetEnable("Shadow"_s, isApplyShadow);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("FXAA"))
		{
			ImGui::PushID("FXAA");

			bool isApplyFXAA = Config::IsEnable("FXAA"_s);
			if (ImGui::Checkbox("Apply", &isApplyFXAA) == true)
			{
				Config::SetEnable("FXAA"_s, isApplyFXAA);
			}

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

			bool isApplyColorGrading = Config::IsEnable("ColorGrading"_s);
			if (ImGui::Checkbox("Apply", &isApplyColorGrading) == true)
			{
				Config::SetEnable("ColorGrading"_s, isApplyColorGrading);
			}

			math::Vector3 color = graphics::ColorGrading::GetInstance()->GetColorGuide();
			if (ImGui::ColorEdit3("Guide", &color.x) == true)
			{
				graphics::ColorGrading::GetInstance()->SetColorGuide(color);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("BloomFilter"))
		{
			ImGui::PushID("BloomFilter");

			bool isApplyColorGrading = Config::IsEnable("BloomFilter"_s);
			if (ImGui::Checkbox("Apply", &isApplyColorGrading) == true)
			{
				Config::SetEnable("BloomFilter"_s, isApplyColorGrading);
			}

			const std::array<char*, graphics::BloomFilter::PresetCount> presets = { "Wide", "Focussed", "Small", "SuperWide", "Cheap", "One", };

			auto& settings = graphics::BloomFilter::GetInstance()->GetSettings();
			ImGui::Combo("Presets", reinterpret_cast<int*>(&settings.emPreset), presets.data(), presets.size());

			ImGui::DragFloat("Threshold", &settings.fThreshold, 0.001f, 0.f, 10.f);
			ImGui::DragFloat("StrengthMultiplier", &settings.fStrengthMultiplier, 0.001f, 0.f, 10.f);
			ImGui::Checkbox("IsEnableLuminance", &settings.isEnableLuminance);

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("SSS"))
		{
			ImGui::PushID("SSS");

			bool isApplySSS = Config::IsEnable("SSS"_s);
			if (ImGui::Checkbox("Apply", &isApplySSS) == true)
			{
				Config::SetEnable("SSS"_s, isApplySSS);
			}

			float fSSSWidth = graphics::SSS::GetInstance()->GetSSSWidth();
			if (ImGui::DragFloat("Width", &fSSSWidth, 0.001f, 0.f, 100.f) == true)
			{
				graphics::SSS::GetInstance()->SetSSSWidth(fSSSWidth);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("ASSAO"))
		{
			ImGui::PushID("ASSAO");

			bool isApplyASSAO = Config::IsEnable("SSAO"_s);
			if (ImGui::Checkbox("Apply", &isApplyASSAO) == true)
			{
				Config::SetEnable("SSAO"_s, isApplyASSAO);
			}

			ImGui::PushItemWidth(100);

			auto& settings = graphics::ASSAO::GetInstance()->GetSettings();
			ImGui::DragFloat("Radius", &settings.Radius, 0.01f, 0.f, 100.f);
			ImGui::DragFloat("ShadowMultiplier", &settings.ShadowMultiplier, 0.01f, 0.f, 5.f);
			ImGui::DragFloat("ShadowPower", &settings.ShadowPower, 0.01f, 0.5f, 5.f);
			ImGui::DragFloat("ShadowClamp", &settings.ShadowClamp, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("HorizonAngleThreshold", &settings.HorizonAngleThreshold, 0.001f, 0.f, 0.2f);
			ImGui::DragFloat("FadeOutFrom", &settings.FadeOutFrom, 0.1f, 0.f, 1000000.f);
			ImGui::DragFloat("FadeOutTo", &settings.FadeOutTo, 0.1f, 0.f, 1000000.f);
			ImGui::DragInt("QualityLevel", &settings.QualityLevel, 0.01f, -1, 3);
			ImGui::DragFloat("AdaptiveQualityLimit", &settings.AdaptiveQualityLimit, 0.01f, 0.f, 1.f);
			ImGui::DragInt("BlurPassCount", &settings.BlurPassCount, 0.01f, 0, 6);
			ImGui::DragFloat("Sharpness", &settings.Sharpness, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("TemporalSupersamplingAngleOffset", &settings.TemporalSupersamplingAngleOffset, 0.01f, 0.f, math::PI);
			ImGui::DragFloat("TemporalSupersamplingRadiusOffset", &settings.TemporalSupersamplingRadiusOffset, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("DetailShadowStrength", &settings.DetailShadowStrength, 0.01f, 0.f, 5.f);

			ImGui::PopItemWidth();

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("DepthOfField"))
		{
			ImGui::PushID("DepthOfField");
			bool isApplyDepthOfField = Config::IsEnable("DepthOfField"_s);
			if (ImGui::Checkbox("Apply", &isApplyDepthOfField) == true)
			{
				Config::SetEnable("DepthOfField"_s, isApplyDepthOfField);
			}

			float fNear = graphics::Camera::GetInstance()->GetNearClip(graphics::eUpdate);
			float fFar = graphics::Camera::GetInstance()->GetFarClip(graphics::eUpdate);

			auto& setting = graphics::DepthOfField::GetInstance()->GetSetting();

			ImGui::PushItemWidth(100);
			ImGui::DragFloat("FocalDistnace", &setting.fFocalDistnace, 0.01f, fNear, fFar);
			ImGui::DragFloat("FocalWidth", &setting.fFocalWidth, 0.01f, 1.f, 10000.f);

			ImGui::PopItemWidth();

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("HDRFilter"))
		{
			ImGui::PushID("HDRFilter");

			bool isApplyHDRFilter = Config::IsEnable("HDRFilter"_s);
			if (ImGui::Checkbox("Apply", &isApplyHDRFilter) == true)
			{
				Config::SetEnable("HDRFilter"_s, isApplyHDRFilter);
			}

			graphics::HDRFilter::Settings& settings = graphics::HDRFilter::GetInstance()->GetSettings();
			if (ImGui::Button("Reset") == true)
			{
				settings = graphics::HDRFilter::Settings();
				graphics::HDRFilter::GetInstance()->SetToneMappingType(graphics::HDRFilter::eNone);
				graphics::HDRFilter::GetInstance()->SetAutoExposureType(graphics::HDRFilter::eManual);
			}

			ImGui::PushItemWidth(150);

			const std::array<const char*, graphics::HDRFilter::NumToneMappingTypes> ToneMappingTypes = 
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

			const std::array<const char*, graphics::HDRFilter::NumAutoExposureTypes> AutoExposureTypes =
			{
				"Manual",
				"GeometricMean",
				"GeometricMeanAutoKey",
			};

			graphics::HDRFilter::ToneMappingType emToneMappingType = graphics::HDRFilter::GetInstance()->GetToneMappingType();
			if (ImGui::Combo("ToneMappingTypes", reinterpret_cast<int*>(&emToneMappingType), ToneMappingTypes.data(), ToneMappingTypes.size()) == true)
			{
				graphics::HDRFilter::GetInstance()->SetToneMappingType(emToneMappingType);
			}

			graphics::HDRFilter::AutoExposureType emAutoExposureType = graphics::HDRFilter::GetInstance()->GetAutoExposureType();
			if (ImGui::Combo("AutoExposureTypes", reinterpret_cast<int*>(&emAutoExposureType), AutoExposureTypes.data(), AutoExposureTypes.size()) == true)
			{
				graphics::HDRFilter::GetInstance()->SetAutoExposureType(emAutoExposureType);
			}

			ImGui::DragFloat("BloomThreshold", &settings.BloomThreshold, 0.01f, 0.f, 10.f);
			ImGui::DragFloat("BloomMagnitude", &settings.BloomMagnitude, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("BloomBlurSigma", &settings.BloomBlurSigma, 0.01f, 0.5f, 1.5f);
			ImGui::DragFloat("Tau", &settings.Tau, 0.01f, 0.f, 4.f);
			ImGui::DragFloat("Exposure", &settings.Exposure, 0.01f, -10.f, 10.f);
			ImGui::DragFloat("KeyValue", &settings.KeyValue, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("WhiteLevel", &settings.WhiteLevel, 0.01f, 0.f, 5.f);
			ImGui::DragFloat("ShoulderStrength", &settings.ShoulderStrength, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("LinearStrength", &settings.LinearStrength, 0.01f, 0.f, 5.f);
			ImGui::DragFloat("LinearAngle", &settings.LinearAngle, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("ToeStrength", &settings.ToeStrength, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("ToeNumerator", &settings.ToeNumerator, 0.01f, 0.f, 0.5f);
			ImGui::DragFloat("ToeDenominator", &settings.ToeDenominator, 0.01f, 0.f, 2.f);
			ImGui::DragFloat("LinearWhite", &settings.LinearWhite, 0.01f, 0.f, 20.f);
			ImGui::DragFloat("LuminanceSaturation", &settings.LuminanceSaturation, 0.01f, 0.f, 4.f);

			int nLumMapMipLevel = static_cast<int>(settings.LumMapMipLevel);
			if (ImGui::DragInt("LumMapMipLevel", &nLumMapMipLevel, 0.1f, 0, 10) == true)
			{
				settings.LumMapMipLevel = static_cast<float>(nLumMapMipLevel);
			}
			ImGui::DragFloat("Bias", &settings.Bias, 0.01f, 0.f, 1.f);

			ImGui::PopItemWidth();

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Tessellation"))
		{
			ImGui::PushID("Tessellation");

			bool isApplyTessellation = Config::IsEnable("Tessellation"_s);
			if (ImGui::Checkbox("Apply", &isApplyTessellation) == true)
			{
				Config::SetEnable("Tessellation"_s, isApplyTessellation);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Wireframe"))
		{
			ImGui::PushID("Wireframe");

			bool isApplyWireframe = Config::IsEnable("Wireframe"_s);
			if (ImGui::Checkbox("Apply", &isApplyWireframe) == true)
			{
				Config::SetEnable("Wireframe"_s, isApplyWireframe);
			}

			ImGui::PopID();

			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Skybox") == true)
	{
		static int nSelectedIndex = 0;
		if (ImGui::Combo("Env", &nSelectedIndex, IBL_Type.data(), IBL_Type.size()) == true)
		{
			std::string strPath = String::Format("%sIBL\\%s\\%s", file::GetPath(file::eTexture), IBL_Type[nSelectedIndex], IBL_Type[nSelectedIndex]);

			graphics::IImageBasedLight* pIBL = graphics::GetImageBasedLight();

			std::string strDiffuseHDR = strPath;
			strDiffuseHDR.append("DiffuseHDR.dds");
			std::shared_ptr<graphics::ITexture> pDiffuseHDR = graphics::ITexture::Create(strDiffuseHDR);
			pIBL->SetDiffuseHDR(pDiffuseHDR);

			std::string strSpecularHDR = strPath;
			strSpecularHDR.append("SpecularHDR.dds");
			std::shared_ptr<graphics::ITexture> pSpecularHDR = graphics::ITexture::Create(strSpecularHDR);
			pIBL->SetSpecularHDR(pSpecularHDR);

			std::string strSpecularBRDF = strPath;
			strSpecularBRDF.append("Brdf.dds");
			std::shared_ptr<graphics::ITexture> pSpecularBRDF = graphics::ITexture::Create(strSpecularBRDF);
			pIBL->SetSpecularBRDF(pSpecularBRDF);

			std::string strEnvIBLPath = strPath;
			strEnvIBLPath.append("EnvHDR.dds");
			std::shared_ptr<graphics::ITexture> pEnvIBL = graphics::ITexture::Create(strEnvIBLPath);
			m_pSkybox->SetTexture(pEnvIBL);
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

		math::Vector3 f3LightPos = pActor->GetPosition();
		if (ImGui::DragFloat3("Light Position", reinterpret_cast<float*>(&f3LightPos.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			pActor->SetPosition(f3LightPos);
		}*/
	}

	if (ImGui::CollapsingHeader("Camera") == true)
	{
		graphics::Camera* pCamera = graphics::Camera::GetInstance();

		math::Vector3 f3CameraPos = pCamera->GetPosition();
		if (ImGui::DragFloat3("Camera Position", reinterpret_cast<float*>(&f3CameraPos.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			pCamera->SetPosition(f3CameraPos);
		}

		math::Vector3 f3CameraLookat = pCamera->GetLookat();
		if (ImGui::DragFloat3("Camera Lookat", reinterpret_cast<float*>(&f3CameraLookat.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			pCamera->SetLookat(f3CameraLookat);
		}

		math::Vector3 f3Up = pCamera->GetUp();
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
		char path[512] = { 0 };

		OPENFILENAME ofn;
		Memory::Clear(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = Windows::GetHwnd();
		ofn.lpstrFilter = "Motion File(*.fbx;*.emot)\0*.fbx;*.emot\0FBX File(*.fbx)\0*.fbx\0EastMotion File(*.emot)\0*.emot\0";
		ofn.lpstrFile = path;
		ofn.nMaxFile = 256;
		if (GetOpenFileName(&ofn) != 0)
		{
			std::string strFileExtension = file::GetFileExtension(ofn.lpstrFile);
			if (String::IsEqualsNoCase(strFileExtension.c_str(), "fbx") == true)
			{
				graphics::MotionLoader loader;
				loader.InitFBX(file::GetFileName(ofn.lpstrFile).c_str(), ofn.lpstrFile, 0.01f);
				graphics::IMotion::Create(loader);
			}
			else if (String::IsEqualsNoCase(strFileExtension.c_str(), "emot") == true)
			{
				graphics::MotionLoader loader;
				loader.InitEast(file::GetFileName(ofn.lpstrFile).c_str(), ofn.lpstrFile);
				graphics::IMotion::Create(loader);
			}
		}
	}

	const size_t nMotionCount = graphics::MotionManager::GetInstance()->GetMotionCount();

	std::vector<const char*> vecMotionNames;
	vecMotionNames.reserve(nMotionCount);

	for (size_t i = 0; i < nMotionCount; ++i)
	{
		graphics::IMotion* pMotion = graphics::MotionManager::GetInstance()->GetMotion(i);
		vecMotionNames.emplace_back(pMotion->GetName().c_str());
	}

	static int nSelectedIndex = 0;
	if (vecMotionNames.empty() == false)
	{
		ImGui::ListBox("Motion List", &nSelectedIndex, &vecMotionNames.front(), vecMotionNames.size(), 6);
	}

	graphics::IMotion* pMotion = nullptr;
	if (0 <= nSelectedIndex && nSelectedIndex < static_cast<int>(vecMotionNames.size()))
	{
		pMotion = graphics::MotionManager::GetInstance()->GetMotion(nSelectedIndex);
	}

	if (pMotion != nullptr)
	{
		const std::array<char*, graphics::EmMotion::eLayerCount> layers = { "Layer1", "Layer2", "Layer3", "Layer4", };

		static graphics::EmMotion::Layers emLayer = graphics::EmMotion::eLayer1;
		ImGui::Combo("Layer", reinterpret_cast<int*>(&emLayer), layers.data(), layers.size());

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

			std::string strLayer = String::Format("Layer%d", i);

			ImGui::PushID(strLayer.c_str());

			ImGui::Text(strLayer.c_str());

			graphics::IMotion* pMotionPlaying = pPlayer->GetMotion();
			if (pMotionPlaying != nullptr)
			{
				char buf[128] = { 0 };
				String::Copy(buf, sizeof(buf), pMotionPlaying->GetName().c_str());
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

				std::string strBuf = String::Format("%.2f/%.2f", pPlayer->GetBlendWeight(), pPlayer->GetWeight());
				ImGui::ProgressBar(pPlayer->GetBlendWeight() / pPlayer->GetWeight(), ImVec2(0.0f, 0.0f), strBuf.c_str());
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("BlendWeight");

				float fMotionPlayTime = pPlayer->GetPlayTime();
				float fEndTime = pMotionPlaying->GetEndTime();
				if (pPlayer->IsInverse() == true)
				{
					std::string strBuf = String::Format("%.2f/%.2f", fEndTime - fMotionPlayTime, fEndTime);
					ImGui::ProgressBar((fEndTime - fMotionPlayTime) / fEndTime, ImVec2(0.0f, 0.0f), strBuf.c_str());
				}
				else
				{
					std::string strBuf = String::Format("%.2f/%.2f", fMotionPlayTime, fEndTime);
					ImGui::ProgressBar(fMotionPlayTime / fEndTime, ImVec2(0.0f, 0.0f), strBuf.c_str());
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
	ImGui::Begin(String::Format("%d. %s Info", nIndex, pMaterial->GetName().c_str()).c_str(), &isShowMaterial);

	static char bufName[128] = { 0 };
	String::Copy(bufName, pMaterial->GetName().c_str());

	ImGui::PushID(bufName);

	if (ImGui::InputText("Name", bufName, sizeof(bufName), ImGuiInputTextFlags_EnterReturnsTrue) == true)
	{
		pMaterial->SetName(bufName);
	}

	char buf[1024] = { 0 };
	String::Copy(buf, pMaterial->GetPath().c_str());
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
	graphics::EmSamplerState::Type emSamplerState = pMaterial->GetSamplerState();
	if (ImGui::Combo("SamplerState", reinterpret_cast<int*>(&emSamplerState), strSamplerState, graphics::EmSamplerState::TypeCount) == true)
	{
		pMaterial->SetSamplerState(emSamplerState);
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
	graphics::EmBlendState::Type emBlendState = pMaterial->GetBlendState();
	if (ImGui::Combo("BlendState", reinterpret_cast<int*>(&emBlendState), strBlendState, graphics::EmBlendState::TypeCount) == true)
	{
		pMaterial->SetBlendState(emBlendState);
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
	graphics::EmRasterizerState::Type emRasterizerState = pMaterial->GetRasterizerState();
	if (ImGui::Combo("RasterizerState", reinterpret_cast<int*>(&emRasterizerState), strRasterizerState, graphics::EmRasterizerState::TypeCount) == true)
	{
		pMaterial->SetRasterizerState(emRasterizerState);
	}

	const char* strDepthStencilState[graphics::EmDepthStencilState::TypeCount] =
	{
		"Read_Write_On",
		"Read_Write_Off",
		"Read_On_Write_Off",
		"Read_Off_Write_On",
	};
	graphics::EmDepthStencilState::Type emDepthStencilState = pMaterial->GetDepthStencilState();
	if (ImGui::Combo("DepthStencilState", reinterpret_cast<int*>(&emDepthStencilState), strDepthStencilState, graphics::EmDepthStencilState::TypeCount) == true)
	{
		pMaterial->SetDepthStencilState(emDepthStencilState);
	}

	bool isVisible = pMaterial->IsVisible();
	if (ImGui::Checkbox("Vibisle", &isVisible) == true)
	{
		pMaterial->SetVisible(isVisible);
	}

	auto TextureInfo = [&](graphics::EmMaterial::Type emType, int nIndex)
	{
		if (ImGui::Button(String::Format("%d.Texture", nIndex).c_str()) == true)
		{
			char path[512] = { 0 };

			OPENFILENAME ofn;
			Memory::Clear(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = Windows::GetHwnd();
			ofn.lpstrFilter = "Every File(*.*)\0*.*\0Text File\0*.txt;*.doc\0";
			ofn.lpstrFile = path;
			ofn.nMaxFile = 256;
			if (GetOpenFileName(&ofn) != 0)
			{
				std::shared_ptr<graphics::ITexture> pTexture = graphics::ITexture::Create(ofn.lpstrFile);
				pMaterial->SetTextureName(emType, file::GetFileName(ofn.lpstrFile).c_str());
				pMaterial->SetTexture(emType, pTexture);
			}
		}

		const String::StringID& strName = pMaterial->GetTextureName(emType);
		char buf[1024] = { 0 };
		String::Copy(buf, strName.c_str());
		ImGui::SameLine();
		ImGui::InputText("", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

		std::shared_ptr<graphics::ITexture> pTexture = pMaterial->GetTexture(emType);
		if (pTexture != nullptr)
		{
			if (ImGui::Button(String::Format("%d.Clear", nIndex).c_str()) == true)
			{
				pMaterial->SetTextureName(emType, "");
				pMaterial->SetTexture(emType, nullptr);
			}

			ImGui::SameLine();
			if (pTexture->GetState() == graphics::EmLoadState::eComplete)
			{
				static std::unordered_map<String::StringID, bool> umapIsShowBigTexture;

				bool& isShow = umapIsShowBigTexture[pTexture->GetName()];

				ImTextureID textureID = pTexture->GetShaderResourceView();
				if (ImGui::ImageButton(textureID, ImVec2(64.f, 64.f)) == true)
				{
					isShow = !isShow;
				}

				if (isShow == true)
				{
					ImVec2 f2Size(static_cast<float>(pTexture->GetSize().x), static_cast<float>(pTexture->GetSize().y));
					f2Size.x = math::Min(f2Size.x, 512.f);
					f2Size.y = math::Min(f2Size.y, 512.f);

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

void SceneStudio::RenderUI()
{
	TRACER_EVENT("SceneStudio::RenderUI");

	ImGui_ImplDX11_DeviceContextChange(graphics::GetDeferredContext(graphics::eRender)->GetInterface());
	ImGui_ImplDX11_NewFrame();

	ShowConfig();

	//static bool isShowMaterialEditor = false;
	//m_pMaterialNodeManager->Update(isShowMaterialEditor);

	m_pSkeletonController->RenderUI();

	static bool isShowDebug = true;
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Debug Info", &isShowDebug);

		float fps = MainSystem::GetInstance()->GetFPS();
		float ms = 1.f / fps * 1000.f;
		ImGui::Text("Frame %.2f(%.2f)", fps, ms);

		if (ImGui::CollapsingHeader("Tracer") == true)
		{
			const bool isTracing = Performance::Tracer::GetInstance()->IsTracing();
			ImGui::Text("State : %s", isTracing == true ? "Tracing" : "Idle");

			if (isTracing == true)
			{
				ImGui::Text("Time : %.2f", Performance::Tracer::GetInstance()->TracingTime());

				if (ImGui::Button("End") == true)
				{
					char path[512] = { 0 };
					OPENFILENAME ofn;
					Memory::Clear(&ofn, sizeof(ofn));

					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = Windows::GetHwnd();
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

	static bool isShowActorWindow = true;
	if (isShowActorWindow == true)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Actor", &isShowActorWindow);

		ImGui::PushID("Actor");

		uint32_t nActorCount = gameobject::ActorManager::GetInstance()->GetActorCount();

		std::vector<const char*> vecActorName;
		vecActorName.reserve(nActorCount);
		for (uint32_t i = 0; i < nActorCount; ++i)
		{
			gameobject::IActor* pActor = gameobject::ActorManager::GetInstance()->GetActor(i);
			vecActorName.emplace_back(pActor->GetName().c_str());
		}

		static int nSelectedIndex = 0;
		if (vecActorName.empty() == false)
		{
			ImGui::ListBox("List", &nSelectedIndex, &vecActorName.front(), vecActorName.size(), 4);
		}

		gameobject::IActor* pActor = nullptr;
		if (0 <= nSelectedIndex && nSelectedIndex < static_cast<int>(vecActorName.size()))
		{
			pActor = gameobject::ActorManager::GetInstance()->GetActor(nSelectedIndex);
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
				if (String::Length(buf) > 1)
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
			char path[512] = { 0 };
			OPENFILENAME ofn;
			Memory::Clear(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = Windows::GetHwnd();
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
			char path[512] = { 0 };
			OPENFILENAME ofn;
			Memory::Clear(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = Windows::GetHwnd();
			ofn.lpstrFilter = "EastActor(*.eact)\0*.eact\0";
			ofn.lpstrFile = path;
			ofn.nMaxFile = 255;
			if (GetSaveFileName(&ofn) != 0)
			{
				std::string strFileExtension = file::GetFileExtension(path);
				if (strFileExtension.empty() == true)
				{
					String::Concat(path, sizeof(path), ".eact");
				}

				if (String::IsEqualsNoCase(file::GetFileExtension(path).c_str(), "eact") == true)
				{
					gameobject::IActor::SaveToFile(pActor, path);
				}
			}
		}

		if (ImGui::CollapsingHeader("Common"))
		{
			if (pActor != nullptr)
			{
				static char buf[128] = { 0 };
				String::Copy(buf, pActor->GetName().c_str());

				ImGui::PushID(buf);

				if (ImGui::InputText("Name", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue) == true)
				{
					pActor->SetName(buf);
				}

				math::Vector3 f3Scale = pActor->GetScale();
				if (ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&f3Scale.x), 0.01f, 0.01f, 100.f) == true)
				{
					pActor->SetScale(f3Scale);
				}

				math::Vector3 f3Pos = pActor->GetPosition();
				if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&f3Pos.x), 0.01f, -1000000.f, 1000000.f) == true)
				{
					pActor->SetPosition(f3Pos);
				}

				static std::unordered_map<gameobject::IActor*, math::Vector3> umapActorRotation;
				math::Vector3& f3Rotation = umapActorRotation[pActor];

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
					nCurItem = math::Min(nCurItem, static_cast<int>(vecComponents.size() - 1));
					if (ImGui::Combo("Add Component", &nCurItem, &vecComponents.front(), vecComponents.size()) == true)
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

								math::Vector3 f3Scale = pModel->GetLocalScale();
								if (ImGui::DragFloat3("Local Scale", reinterpret_cast<float*>(&f3Scale), 0.01f, 0.01f, 100.f) == true)
								{
									pModel->SetLocalScale(f3Scale);
								}

								math::Vector3 f3Pos = pModel->GetLocalPosition();
								if (ImGui::DragFloat3("Local Position", reinterpret_cast<float*>(&f3Pos), 0.01f, -1000000.f, 1000000.f) == true)
								{
									pModel->SetLocalPosition(f3Pos);
								}

								static std::unordered_map<graphics::IModel*, math::Vector3> umapMotionRotation;
								math::Vector3& f3Rotation = umapMotionRotation[pModel];
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
										char path[512] = { 0 };
										OPENFILENAME ofn;
										Memory::Clear(&ofn, sizeof(ofn));

										ofn.lStructSize = sizeof(OPENFILENAME);
										ofn.hwndOwner = Windows::GetHwnd();
										ofn.lpstrFilter = "EastModel(*.emod)\0*.emod\0";
										ofn.lpstrFile = path;
										ofn.nMaxFile = 255;
										if (GetSaveFileName(&ofn) != 0)
										{
											std::string strFileExtension = file::GetFileExtension(path);
											if (strFileExtension.empty() == true)
											{
												String::Concat(path, sizeof(path), ".emod");
											}

											if (String::IsEqualsNoCase(file::GetFileExtension(path).c_str(), "emod") == true)
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

									static char bufName[128] = { 0 };
									String::Copy(bufName, pModel->GetName().c_str());

									ImGui::PushID(bufName);

									if (ImGui::InputText("Name", bufName, sizeof(bufName), ImGuiInputTextFlags_EnterReturnsTrue) == true)
									{
										pModel->ChangeName(bufName);
									}

									static char bufPath[512] = { 0 };
									String::Copy(bufPath, pModel->GetFilePath().c_str());
									ImGui::InputText("Path", bufPath, sizeof(bufPath), ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);

									int nModelNode = 0;
									int nMaterialIndex = 0;
									uint32_t nNodeCount = pModel->GetNodeCount();
									for (uint32_t j = 0; j < nNodeCount; ++j)
									{
										graphics::IModelNode* pModelNode = pModel->GetNode(j);

										if (ImGui::TreeNode(pModelNode->GetName().c_str()))
										{
											int nVertexCount = 0;
											if (pModelNode->GetVertexBuffer() != nullptr)
											{
												nVertexCount = pModelNode->GetVertexBuffer()->GetVertexNum();
											}
											ImGui::Text("VertexCount : %d", nVertexCount);

											int nIndexCount = 0;
											if (pModelNode->GetIndexBuffer() != nullptr)
											{
												nIndexCount = pModelNode->GetIndexBuffer()->GetIndexNum();
											}
											ImGui::Text("IndexCount : %d", nIndexCount);

											bool isVisibleModelNode = pModelNode->IsVisible();
											if (ImGui::Checkbox(String::Format("%d. Model Visible", nModelNode).c_str(), &isVisibleModelNode) == true)
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

												std::string strMtrlName = String::Format("%d. %s", nMaterialIndex, pMaterial->GetName().c_str());

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
								char path[512] = { 0 };

								OPENFILENAME ofn;
								Memory::Clear(&ofn, sizeof(ofn));

								ofn.lStructSize = sizeof(OPENFILENAME);
								ofn.hwndOwner = Windows::GetHwnd();
								ofn.lpstrFilter = "Model File(*.fbx;*.obj;*.emod)\0*.fbx;*.obj;*.emod\0FBX File(*.fbx)\0*.fbx\0Obj File(*.obj)\0*.obj\0EastModel File(*.emod)\0*.emod\0";
								ofn.lpstrFile = path;
								ofn.nMaxFile = 256;
								if (GetOpenFileName(&ofn) != 0)
								{
									std::string strFileExtension = file::GetFileExtension(ofn.lpstrFile);
									if (String::IsEqualsNoCase(strFileExtension.c_str(), "fbx") == true)
									{
										String::StringID strName;
										strName.Format("%s(%f)", file::GetFileName(ofn.lpstrFile).c_str(), fScaleFactor);

										graphics::ModelLoader loader;
										loader.InitFBX(strName, ofn.lpstrFile, fScaleFactor);
										loader.SetEnableThreadLoad(true);

										pCompModel->Init(&loader);
									}
									else if (String::IsEqualsNoCase(strFileExtension.c_str(), "obj") == true)
									{
										String::StringID strName;
										strName.Format("%s(%f)", file::GetFileName(ofn.lpstrFile).c_str(), fScaleFactor);

										graphics::ModelLoader loader;
										loader.InitObj(strName, ofn.lpstrFile, fScaleFactor);
										loader.SetEnableThreadLoad(true);

										pCompModel->Init(&loader);
									}
									else if (String::IsEqualsNoCase(strFileExtension.c_str(), "emod") == true)
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
									for (int j = graphics::EmModelLoader::eCube; j < graphics::EmModelLoader::eCapsule; ++j)
									{
										const char* strName = graphics::EmModelLoader::GetGeometryTypeToSTring(static_cast<graphics::EmModelLoader::GeometryType>(j));
										vecGeometryModel.emplace_back(strName);
									}
								}

								static int nSelectedGeometryIndex = 0;
								ImGui::Combo("Type", &nSelectedGeometryIndex, &vecGeometryModel.front(), vecGeometryModel.size());

								graphics::EmModelLoader::GeometryType emType = graphics::EmModelLoader::GetGeometryType(vecGeometryModel[nSelectedGeometryIndex]);
								switch (emType)
								{
								case graphics::EmModelLoader::eCube:
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
								case graphics::EmModelLoader::eBox:
								{
									ImGui::PushID("Box");

									static char buf[128] = "Box";
									ImGui::InputText("Name", buf, sizeof(buf));

									static math::Vector3 f3Size(math::Vector3::One);
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
								case graphics::EmModelLoader::eSphere:
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
								case graphics::EmModelLoader::eGeoSphere:
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
								case graphics::EmModelLoader::eCylinder:
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
								case graphics::EmModelLoader::eCone:
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
								case graphics::EmModelLoader::eTorus:
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
								case graphics::EmModelLoader::eTetrahedron:
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
								case graphics::EmModelLoader::eOctahedron:
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
								case graphics::EmModelLoader::eDodecahedron:
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
								case graphics::EmModelLoader::eIcosahedron:
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
								case graphics::EmModelLoader::eTeapot:
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
								case graphics::EmModelLoader::eHexagon:
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
								case graphics::EmModelLoader::eCapsule:
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

	// Rendering
	ImGui::Render();
}