#include "stdafx.h"
#include "SceneStudio.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/CameraManager.h"
#include "DirectX/Light.h"
#include "DirectX/MaterialNode.h"

#include "Renderer/ASSAO.h"
#include "Renderer/DepthOfField.h"

#include "Model/ModelManager.h"
#include "Model/MotionManager.h"

#include "Particle/ParticleInterface.h"

#include "Windows/Windows.h"

#include "Input/InputInterface.h"

#include "Renderer/DepthOfField.h"
#include "Renderer/HDRFilter.h"

#include "GameObject/GameObject.h"
#include "GameObject/ActorManager.h"
#include "GameObject/ComponentModel.h"
#include "GameObject/ComponentPhysics.h"
#include "GameObject/ComponentTimer.h"

#include "Contents/Sun.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"

#include "MaterialNodeManager.h"

using namespace EastEngine;

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

SceneStudio::SceneStudio()
	: SceneInterface(StrID::Studio)
	, m_pMaterialNodeManager(nullptr)
	, m_pSectorMgr(nullptr)
{
}

SceneStudio::~SceneStudio()
{
	SafeDelete(m_pMaterialNodeManager);
}

void SceneStudio::Enter()
{
	Windows::WindowsManager::GetInstance()->AddMessageHandler(HandleMsg);

	HWND hWnd = Windows::GetHwnd();
	ID3D11Device* pd3dDevice = Graphics::GetDevice()->GetInterface();
	ID3D11DeviceContext* pd3dDeviceContext = Graphics::GetDeviceContext()->GetInterface();
	ImGui_ImplDX11_Init(hWnd, pd3dDevice, pd3dDeviceContext);

	Graphics::GraphicsSystem::GetInstance()->AddFuncAfterRender([&]()
	{
		RenderUI();
	});

	std::string strFontPath = File::GetPath(File::eFont);
	strFontPath.append("ArialUni.ttf");
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(strFontPath.c_str(), 16.f, nullptr, io.Fonts->GetGlyphRangesKorean());

	//Graphics::CameraManager::GetInstance()->CreateCamera("MainCamera", Math::Vector3(0.f, 50.f, -100.f), Math::Vector3(0.f, 50.f, 0.f), Math::Vector3(0.f, 1.f, 0.f));
	Graphics::CameraManager::GetInstance()->CreateCamera("MainCamera", Math::Vector3(0.f, 1.f, -10.f), Math::Vector3(0.f, 50.f, 0.f), Math::Vector3(0.f, 1.f, 0.f));

	{
		Math::Vector3 f3LightPosition(0.f, 500.f, -500.f);
		Math::Vector3 f3LightDirection(Math::Vector3::Zero - f3LightPosition);
		f3LightDirection.Normalize();

		Graphics::CascadedShadowsConfig config;
		config.nLevel = 3;
		config.nBufferSize = 2048;
		config.fCascadeDistance = 256.f;

		Graphics::ILight* pLight = Graphics::ILight::CreateDirectionalLight("MainLight", f3LightDirection, Math::Color::White, 20000.f, 0.5f, 0.25f, &config);
		pLight->SetEnableShadow(false);
	}
	//{
	//	Math::Vector3 f3LightPosition(500.f, 500.f, 0.f);
	//	Math::Vector3 f3LightDirection(Math::Vector3::Zero - f3LightPosition);
	//	f3LightDirection.Normalize();
	//
	//	Graphics::CascadedShadowsConfig config;
	//	config.nLevel = 3;
	//	config.nBufferSize = 2048;
	//	config.fCascadeDistance = 256.f;
	//
	//	Graphics::ILight::CreateDirectionalLight("MainLight2", f3LightDirection, Math::Color::White, 10000.f, 0.5f, 0.25f, &config);
	//}
	
	std::string strPath = File::GetPath(File::eTexture);

	Graphics::IImageBasedLight* pIBL = Graphics::GetImageBasedLight();

	std::string strCubeMapPath = strPath;
	strCubeMapPath.append("CubeMap.dds");
	std::shared_ptr<Graphics::ITexture> pTexture = Graphics::ITexture::Create("CubeMap.dds", strCubeMapPath);
	pIBL->SetCubeMap(pTexture);

	std::string strIrradiancePath = strPath;
	strIrradiancePath.append("Irradiance.dds");
	pTexture = Graphics::ITexture::Create("Irradiance.dds", strIrradiancePath);
	pIBL->SetIrradianceMap(pTexture);

	{
		auto pActor = GameObject::ActorManager::GetInstance()->CreateActor(StrID::Studio_Ground);
	
		Graphics::MaterialInfo material;
		material.strName = StrID::Studio_Ground;
		Graphics::ModelLoader loader;
		loader.InitPlane(StrID::Studio_Ground, 1.f, 1.f, 100, 100, &material);
	
		auto pCompModel = static_cast<GameObject::ComponentModel*>(pActor->CreateComponent(GameObject::EmComponent::eModel));
		pCompModel->Init(&loader);
	
		auto pModelInst = pCompModel->GetModelInstance();
	
		auto pCompPhysics = static_cast<GameObject::ComponentPhysics*>(pActor->CreateComponent(GameObject::EmComponent::ePhysics));
	
		Physics::RigidBodyProperty prop;
		prop.fRestitution = 0.75f;
		prop.strName = StrID::Studio_Ground;
		prop.fMass = 0.f;
		prop.nCollisionFlag = Physics::EmCollision::eStaticObject;
		prop.shapeInfo.SetTriangleMesh();
		pCompPhysics->Init(pModelInst, prop);
	
		/*GameObject::SectorInitInfo sectorInitInfo;
		sectorInitInfo.fRadius = 10.f;
		for (auto& direction : sectorInitInfo.nSectorsCount)
		{
			direction = 10;
		}
	
		m_pSectorMgr = CreateSectorMgr(sectorInitInfo);*/
	}

	{
		Graphics::MaterialInfo materialInfo;
		materialInfo.strName = "TestDecal";
		materialInfo.strPath = File::GetPath(File::eTexture);

		materialInfo.strTextureNameArray[Graphics::EmMaterial::eAlbedo] = "Albedo.tga";
		materialInfo.strTextureNameArray[Graphics::EmMaterial::eNormal] = "Normal.tga";
		//materialInfo.strTextureNameArray[Graphics::EmMaterial::eSpecularColor] = "Specular.tga";
		materialInfo.strTextureNameArray[Graphics::EmMaterial::eRoughness] = "Roughness.tga";
		materialInfo.strTextureNameArray[Graphics::EmMaterial::eMetallic] = "Metallic.tga";

		materialInfo.f4DisRoughMetEmi.y = 0.5f;
		materialInfo.f4DisRoughMetEmi.z = 0.5f;

		materialInfo.emDepthStencilState = Graphics::EmDepthStencilState::eOff;

		Graphics::ParticleDecalAttributes attributes;
		attributes.strDecalName = "TestDecal";
		attributes.f3Scale = Math::Vector3(2.f, 1.f, 2.f);
		attributes.pMaterialInfo = &materialInfo;

		Graphics::IParticleDecal::Create(attributes);;
	}

	Graphics::IMaterial* pMaterial_override = nullptr;
	//for (int j = 0; j < 5; ++j)
	//{
	//	for (int i = 0; i < 50; ++i)
	//	{
	//		/*Graphics::MaterialInfo materialInfo;
	//		materialInfo.strName.Format("TestBox%d", (i % 10) + 1);
	//		materialInfo.strPath = File::GetPath(File::eTexture);

	//		materialInfo.strTextureNameArray[Graphics::EmMaterial::eAlbedo].Format("Pattern\\pattern_%02d\\%s", (i % 10) + 1, "diffus.tga");
	//		materialInfo.strTextureNameArray[Graphics::EmMaterial::eNormal].Format("Pattern\\pattern_%02d\\%s", (i % 10) + 1, "Normal.tga");
	//		materialInfo.strTextureNameArray[Graphics::EmMaterial::eSpecularColor].Format("Pattern\\pattern_%02d\\%s", (i % 10) + 1, "specular.tga");
	//		*/
	//		Graphics::MaterialInfo materialInfo;
	//		materialInfo.strName = "TestBox";
	//		materialInfo.strPath = File::GetPath(File::eTexture);

	//		materialInfo.strTextureNameArray[Graphics::EmMaterial::eAlbedo].Format("Pattern\\pattern_01\\%s", "diffus.tga");
	//		materialInfo.strTextureNameArray[Graphics::EmMaterial::eNormal].Format("Pattern\\pattern_01\\%s", "Normal.tga");
	//		materialInfo.strTextureNameArray[Graphics::EmMaterial::eSpecularColor].Format("Pattern\\pattern_01\\%s", "specular.tga");

	//		//materialInfo.f4DisRoughMetEmi.y = 0.1f * ((i % 10) + 1);
	//		//materialInfo.f4DisRoughMetEmi.z = 1.f - 0.1f * ((i % 10) + 1);

	//		materialInfo.f4DisRoughMetEmi.y = 0.5f;
	//		materialInfo.f4DisRoughMetEmi.z = 0.5f;

	//		//materialInfo.rasterizerStateDesc = Graphics::GetDevice()->GetRasterizerStateDesc(Graphics::EmRasterizerState::eNone);
	//		//materialInfo.colorAlbedo = Math::Color(Math::Random(0.f, 1.f), Math::Random(0.f, 1.f), Math::Random(0.f, 1.f), 1.f);

	//		GameObject::IActor* pActor = GameObject::ActorManager::GetInstance()->CreateActor("TestBox");

	//		Math::Vector3 f3Pos;
	//		f3Pos.x = -4.f + (i % 5) * 3.f;
	//		f3Pos.y = 100.5f + (j * 3.f);
	//		f3Pos.z = -4.f + (i / 5) * 3.f;

	//		pActor->SetPosition(f3Pos);

	//		GameObject::ComponentModel* pCompModel = static_cast<GameObject::ComponentModel*>(pActor->CreateComponent(GameObject::EmComponent::eModel));

	//		Graphics::ModelLoader loader;
	//		//loader.InitBox(String::Format("TestBox%d", (i % 10) + 1).c_str(), &materialInfo);
	//		loader.InitBox("TestBox", &materialInfo);
	//		pCompModel->Init(&loader);
	//		auto pModelInst = pCompModel->GetModelInstance();

	//		if (i % 2 == 0)
	//		{
	//			if (pMaterial_override == nullptr)
	//			{
	//				Graphics::MaterialInfo materialInfo2;
	//				materialInfo2.strName = "TestBox";
	//				materialInfo2.strPath = File::GetPath(File::eTexture);

	//				materialInfo2.strTextureNameArray[Graphics::EmMaterial::eAlbedo].Format("Pattern\\pattern_02\\%s", "diffus.tga");
	//				materialInfo2.strTextureNameArray[Graphics::EmMaterial::eNormal].Format("Pattern\\pattern_02\\%s", "Normal.tga");
	//				materialInfo2.strTextureNameArray[Graphics::EmMaterial::eSpecularColor].Format("Pattern\\pattern_02\\%s", "specular.tga");

	//				pMaterial_override = Graphics::IMaterial::Create(&materialInfo2);
	//			}
	//			pModelInst->ChangeMaterial("EastEngine_Box", 0, pMaterial_override);
	//		}

	//		GameObject::ComponentPhysics* pCompPhysics = static_cast<GameObject::ComponentPhysics*>(pActor->CreateComponent(GameObject::EmComponent::ePhysics));

	//		Physics::RigidBodyProperty prop;
	//		prop.fRestitution = 0.5f;
	//		prop.strName.Format("TestBox_RigidBody%d", i).c_str();

	//		prop.shapeInfo.SetBox(Math::Vector3(1.f));
	//		//prop.shapeInfo.SetCapsule(Math::Random(0.5f, 1.f), Math::Random(1.f, 2.f));
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
	
			Math::Color lightColor(Math::Random(0.f, 1.f), Math::Random(0.f, 1.f), Math::Random(0.f, 1.f), 1.f);
	
			Math::Vector3 f3Position(-50.f + 10.f * i, 5.f, 0.f);
			//Math::Vector3 f3Position(0.f, 5.f, 0.f);
			Math::Vector3 f3Direction = Math::Vector3(0.f, 0.f, 0.f) - Math::Vector3(0.f, 1.f, Math::Random(-2.f, 2.f));
			//Math::Vector3 f3Direction = Math::Vector3(0.f, 0.f, 0.f) - Math::Vector3(0.f, 1.f, -2.f);
			f3Direction.Normalize();
	
			Contents::Sun* pSun = new Contents::Sun;

			Graphics::ShadowConfig shadowConfig;
			shadowConfig.nBufferSize = 1024;

			Graphics::ILight* pSpotLight = Graphics::ILight::CreateSpotLight(strName, f3Position, f3Direction, Math::Random(20.f, 60.f), lightColor, 100.f * (i + 1), 0.1f, 0.2f, &shadowConfig);
			//Graphics::ILight* pSpotLight = Graphics::ILight::CreateSpotLight(strName, f3Position, f3Direction, 90.f, lightColor, 100.f * (i + 1), 0.1f, 0.2f, &shadowConfig);
			pSpotLight->SetEnableShadow(false);

			//Graphics::ILight* pPointLight = Graphics::ILight::CreatePointLight(strName, f3Position, lightColor, 100.f * (i + 1), 0.1f, 0.2f, &shadowConfig);

			if (pSun->Init(StrID::EastEngine_Sun, pSpotLight, 1.f, Math::Vector3::Zero, Math::Vector3(0.f, 0.f, 0.f), Math::Vector3(0.f, 9500.f, 0.f)) == true)
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
		GameObject::TerrainProperty terrain;

		terrain.strTexHeightMap = File::GetPath(File::eTexture);
		terrain.strTexHeightMap.append("heightmap.r16");

		terrain.strTexColorMap = File::GetPath(File::eTexture);
		terrain.strTexColorMap.append("ColorMap2.bmp");

		terrain.fHeightScale = 300.f;

		terrain.n2Size = { 1025, 1025 };
		terrain.n2Patches = { 64, 64 };

		terrain.rigidBodyProperty.fRestitution = 0.25f;
		terrain.rigidBodyProperty.fFriction = 0.75f;

		terrain.strTexDetailMap = File::GetPath(File::eTexture);
		terrain.strTexDetailMap.append("dirt01d.tga");

		terrain.strTexDetailNormalMap = File::GetPath(File::eTexture);
		terrain.strTexDetailNormalMap.append("dirt01n.tga");

		terrain.f3Position = { -500.f, 0.f, -500.f };

		//GameObject::ITerrain::Create("BaseTerrain", terrain);

		// 백그라운드 로딩은 이렇게 쓰면됨
		//GameObject::ITerrain::CreateAsync("BaseTerrain", terrain);
	}

	{
		GameObject::SkyboxProperty sky;

		sky.strTexSky = File::GetPath(File::eTexture);
		sky.strTexSky.append("grasscube1024.dds");

		sky.fBoxSize = 5000.f;

		GameObject::ISkybox::Create("BaseSkybox", sky);
	}

	for (int i = 0; i < 1; ++i)
	{
		String::StringID name;
		name.Format("UnityChan%d", i);
		GameObject::IActor* pActor = GameObject::IActor::Create(name);

		Math::Vector3 pos;
		//pos.x = -10.f + (2.f * (i % 10));
		pos.y = 0.5f;
		//pos.z = 0.f + (2.f * (i / 10));
		pActor->SetPosition(pos);

		strPath = File::GetDataPath();
		strPath.append("Actor\\UnityChan\\unitychan.emod");
		
		Graphics::ModelLoader loader;

		String::StringID strFileName = File::GetFileName(strPath).c_str();
		loader.InitEast(strFileName, strPath.c_str());
		loader.SetEnableThreadLoad(false);
		
		GameObject::ComponentModel* pModel = static_cast<GameObject::ComponentModel*>(pActor->CreateComponent(GameObject::EmComponent::eModel));
		pModel->Init(&loader);

		Graphics::IModelInstance* pModelInstance = pModel->GetModelInstance();

		if (false)
		{
			const std::vector<const char*> vecAnim =
			{
				"Actor\\UnityChan\\Animations\\unitychan_WAIT00.fbx",
				"Actor\\UnityChan\\Animations\\unitychan_WAIT01.fbx",
				"Actor\\UnityChan\\Animations\\unitychan_WAIT02.fbx",
				"Actor\\UnityChan\\Animations\\unitychan_WAIT03.fbx",
				"Actor\\UnityChan\\Animations\\unitychan_WAIT04.fbx",
			};

			std::string strPathMotion(File::GetDataPath());
			//strPathMotion.append("Actor\\UnityChan\\Animations\\unitychan_WAIT00.fbx");
			strPathMotion.append(vecAnim[Math::Random(0u, vecAnim.size() - 1)]);

			String::StringID strMotionName;
			strMotionName.Format("%s", File::GetFileName(strPathMotion).c_str());
			Graphics::MotionLoader motionLoader;
			motionLoader.InitFBX(strMotionName, strPathMotion.c_str(), 0.01f);
			Graphics::IMotion* pMotion = Graphics::IMotion::Create(motionLoader);

			Graphics::MotionPlaybackInfo playback;
			//playback.fSpeed = Math::Random(0.5f, 1.5f);
			playback.fSpeed = 1.f;
			playback.nLoopCount = Graphics::MotionPlaybackInfo::eMaxLoopCount;
			//playback.fWeight = Math::Random(0.1f, 0.5f);
			playback.fWeight = 1.f;
			pModel->PlayMotion(Graphics::EmMotion::eLayer1, pMotion, &playback);
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

		//	std::string strPathMotion(File::GetDataPath());
		//	//strPathMotion.append("Actor\\UnityChan\\Animations\\unitychan_ARpose1.fbx");
		//	//strPathMotion.append("Actor\\UnityChan\\Animations\\unitychan_RUN00_F.fbx");
		//	strPathMotion.append(vecAnim[Math::Random(0u, vecAnim.size() - 1)]);

		//	String::StringID strMotionName;
		//	strMotionName.Format("%s", File::GetFileName(strPathMotion).c_str());
		//	Graphics::MotionLoader motionLoader;
		//	motionLoader.InitFBX(strMotionName, strPathMotion.c_str(), 0.01f);
		//	Graphics::IMotion* pMotion = Graphics::IMotion::Create(motionLoader);

		//	Graphics::MotionPlaybackInfo playback;
		//	playback.fSpeed = Math::Random(0.5f, 1.5f);
		//	playback.nLoopCount = Graphics::MotionPlaybackInfo::eMaxLoopCount;
		//	playback.fWeight = Math::Random(0.7f, 1.f);
		//	pMotionSystem->Play(Graphics::EmMotion::eLayer2, pMotion, &playback);
		//}

		GameObject::ComponentPhysics* pCompPhysics = static_cast<GameObject::ComponentPhysics*>(pActor->CreateComponent(GameObject::EmComponent::ePhysics));
		
		//Math::Vector3 ragdollPos = pActor->GetPosition();
		//pCompPhysics->m_pRagDoll->BuildBipadRagDoll(pModelInstance->GetSkeleton(), ragdollPos, Math::Quaternion::Identity, 0.8f);
		//pCompPhysics->m_pRagDoll->Start();

		if (false)
		{
			strPath = File::GetDataPath();
			strPath.append("Model\\ElementalSwordIce\\LP.emod");

			Graphics::IModelInstance* pModelInstance_Attach = nullptr;
			loader.InitEast(File::GetFileName(strPath).c_str(), strPath.c_str());

			pModelInstance_Attach = Graphics::IModel::CreateInstance(loader, false);

			Math::Vector3 f3Pos = { 0.08f, 0.225f, -0.02f };
			Math::Quaternion quat = Math::Quaternion::CreateFromYawPitchRoll(Math::ToRadians(90.f), Math::ToRadians(180.f), 0.f);

			pModelInstance->Attachment(pModelInstance_Attach, "Character1_LeftHand", Math::Matrix::Compose(Math::Vector3::One, quat, f3Pos));
		}
	}

	{
		String::StringID name;
		name.Format("KimJiYoon");
		GameObject::IActor* pActor = GameObject::IActor::Create(name);

		Math::Vector3 pos;
		pos.x = 2.f;
		pActor->SetPosition(pos);

		strPath = File::GetDataPath();
		strPath.append("Model\\KimJiYoon\\KimJiYoon.emod");

		Graphics::ModelLoader loader;

		String::StringID strFileName = File::GetFileName(strPath).c_str();
		loader.InitEast(strFileName, strPath.c_str());
		loader.SetEnableThreadLoad(false);

		GameObject::ComponentModel* pModel = static_cast<GameObject::ComponentModel*>(pActor->CreateComponent(GameObject::EmComponent::eModel));
		pModel->Init(&loader);

		Graphics::IModelInstance* pModelInstance = pModel->GetModelInstance();
		Graphics::IMotionSystem* pMotionSystem = pModelInstance->GetMotionSystem();

		//if (false)
		{
			std::string strPathMotion(File::GetDataPath());
			strPathMotion.append("Model\\KimJiYoon\\AR_Idle_CC.fbx");

			String::StringID strMotionName;
			strMotionName.Format("%s", File::GetFileName(strPathMotion).c_str());
			Graphics::MotionLoader motionLoader;
			motionLoader.InitFBX(strMotionName, strPathMotion.c_str(), 0.01f);
			Graphics::IMotion* pMotion = Graphics::IMotion::Create(motionLoader);

			Graphics::MotionPlaybackInfo playback;
			playback.fSpeed = 1.f;
			playback.nLoopCount = Graphics::MotionPlaybackInfo::eMaxLoopCount;
			playback.fWeight = 1.f;
			pMotionSystem->Play(Graphics::EmMotion::eLayer1, pMotion, &playback);
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		String::StringID name;
		name.Format("2B_NierAutomata_%d", i);
		GameObject::IActor* pActor = GameObject::IActor::Create(name);

		Math::Vector3 pos;
		pos.x = -2.f + (i * -2.f);
		pActor->SetPosition(pos);

		strPath = File::GetDataPath();
		//strPath.append("Model\\2B_NierAutomata\\Generic_Item.mesh");
		strPath.append("Model\\2B_NierAutomata\\2B_NierAutomata.emod");

		Graphics::ModelLoader loader;

		String::StringID strFileName = File::GetFileName(strPath).c_str();
		//loader.InitXPS(strFileName, strPath.c_str());
		//loader.AddDevideByKeywordByXPS("skirt");
		//loader.AddDevideByKeywordByXPS("eyepatch");
		//loader.AddDevideByKeywordByXPS("white");
		loader.InitEast(strFileName, strPath.c_str());
		loader.SetEnableThreadLoad(false);

		GameObject::ComponentModel* pModel = static_cast<GameObject::ComponentModel*>(pActor->CreateComponent(GameObject::EmComponent::eModel));
		pModel->Init(&loader);

		Graphics::IModelInstance* pModelInstance = pModel->GetModelInstance();
		Graphics::IMotionSystem* pMotionSystem = pModelInstance->GetMotionSystem();

		if (i == 1)
		{
			Graphics::IModelNode* pNode = pModelInstance->GetModel()->GetNode("Generic_Item.mesh");

			auto SetMaterialVisible = [&](const String::StringID& strMaterialName)
			{
				uint32_t nMaterialID = 0;
				Graphics::IMaterial* pMaterial = pNode->GetMaterial(strMaterialName, nMaterialID);

				Graphics::IMaterial* pMaterialClone = Graphics::IMaterial::Clone(pMaterial);
				pMaterialClone->SetVisible(false);

				pModelInstance->ChangeMaterial("Generic_Item.mesh", nMaterialID, pMaterialClone);
			};

			//SetMaterialVisible("Skirt");
			SetMaterialVisible("Eyepatch");
		}

		{
			std::string strPathMotion(File::GetDataPath());
			strPathMotion.append("Model\\2B_NierAutomata\\default.pose");

			String::StringID strMotionName;
			strMotionName.Format("%s", File::GetFileName(strPathMotion).c_str());
			Graphics::MotionLoader motionLoader;
			motionLoader.InitXPS(strMotionName, strPathMotion.c_str());
			Graphics::IMotion* pMotion = Graphics::IMotion::Create(motionLoader);

			Graphics::MotionPlaybackInfo playback;
			playback.fSpeed = 1.f;
			playback.nLoopCount = Graphics::MotionPlaybackInfo::eMaxLoopCount;
			playback.fWeight = 1.f;
			pMotionSystem->Play(Graphics::EmMotion::eLayer1, pMotion, &playback);
		}
	}

	{
		String::StringID name;
		name.Format("ElementalSwordIce");
		GameObject::IActor* pActor = GameObject::IActor::Create(name);

		Math::Vector3 pos;
		pos.y = 1.f;
		pos.z -= 2.f;
		pActor->SetPosition(pos);

		strPath = File::GetDataPath();
		strPath.append("Model\\ElementalSwordIce\\LP.emod");

		Graphics::ModelLoader loader;

		String::StringID strFileName = File::GetFileName(strPath).c_str();
		loader.InitEast(strFileName, strPath.c_str());
		loader.SetEnableThreadLoad(false);

		GameObject::ComponentModel* pModel = static_cast<GameObject::ComponentModel*>(pActor->CreateComponent(GameObject::EmComponent::eModel));
		pModel->Init(&loader);
	}

	m_pMaterialNodeManager = new MaterialNodeManager;
}

void SceneStudio::Exit()
{
	std::for_each(m_vecSuns.begin(), m_vecSuns.end(), DeleteSTLObject());
	m_vecSuns.clear();

	SafeDelete(m_pMaterialNodeManager);
	
	ImGui_ImplDX11_Shutdown();
}

void SceneStudio::Update(float fElapsedTime)
{
	ImGuiIO& io = ImGui::GetIO();

	if (io.WantCaptureMouse == false)
	{
		ProcessInput(fElapsedTime);
	}

	if (m_pSectorMgr != nullptr)
	{
		m_pSectorMgr->Update(fElapsedTime);
	}
}

void SceneStudio::ProcessInput(float fElapsedTime)
{
	Graphics::Camera* pCamera = Graphics::CameraManager::GetInstance()->GetMainCamera();
	if (pCamera == nullptr)
		return;

	float dx = static_cast<float>(Input::Mouse::GetMoveX());
	float dy = static_cast<float>(Input::Mouse::GetMoveY());
	float dz = static_cast<float>(Input::Mouse::GetMoveWheel());
	bool isMoveAxisX = Math::IsZero(dx) == false;
	bool isMoveAxisY = Math::IsZero(dy) == false;
	if (Input::Mouse::IsButtonPressed(Input::Mouse::eRight))
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

	if (Input::Mouse::IsButtonPressed(Input::Mouse::eMiddle))
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

	if (Input::Mouse::IsButtonPressed(Input::Mouse::eLeft))
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

	if (Input::Keyboard::IsKeyPressed(Input::Keyboard::eW))
	{
		pCamera->MoveForward(1.f);
	}

	if (Input::Keyboard::IsKeyPressed(Input::Keyboard::eS))
	{
		pCamera->MoveForward(-1.f);
	}

	if (Input::Keyboard::IsKeyPressed(Input::Keyboard::eA))
	{
		pCamera->MoveSideward(-1.f);
	}

	if (Input::Keyboard::IsKeyPressed(Input::Keyboard::eD))
	{
		pCamera->MoveSideward(1.f);
	}

	if (Input::Keyboard::IsKeyPressed(Input::Keyboard::eE))
	{
		pCamera->MoveUpward(1.f);
	}

	if (Input::Keyboard::IsKeyPressed(Input::Keyboard::eQ))
	{
		pCamera->MoveUpward(-1.f);
	}

	if (Input::GamePad::IsConnected() == true)
	{
		auto LogButton = [](const char* strButtonName, const Input::GamePad::ButtonState& emButtonState)
		{
			if (emButtonState == Input::GamePad::ButtonState::ePressed)
			{
				LOG_MESSAGE("%s Pressed", strButtonName);
			}
			else if (emButtonState == Input::GamePad::ButtonState::eUp)
			{
				LOG_MESSAGE("%s Up", strButtonName);
			}
			else if (emButtonState == Input::GamePad::ButtonState::eDown)
			{
				LOG_MESSAGE("%s Down", strButtonName);
			}
		};

		LogButton("A", Input::GamePad::A());
		LogButton("B", Input::GamePad::B());
		LogButton("X", Input::GamePad::X());
		LogButton("Y", Input::GamePad::Y());

		LogButton("LeftStick", Input::GamePad::LeftStick());
		LogButton("RightStick", Input::GamePad::RightStick());

		LogButton("LeftShoulder", Input::GamePad::LeftShoulder());
		LogButton("RightShoulder", Input::GamePad::RightShoulder());

		LogButton("Back", Input::GamePad::Back());
		LogButton("Start", Input::GamePad::Start());

		LogButton("DPadUp", Input::GamePad::DPadUp());
		LogButton("DPadDown", Input::GamePad::DPadDown());
		LogButton("DPadLeft", Input::GamePad::DPadLeft());
		LogButton("DPadRight", Input::GamePad::DPadRight());

		auto LogStick = [](const char* strStickName, float fValue)
		{
			if (Math::IsZero(fValue) == false)
			{
				LOG_MESSAGE("%s : %f", strStickName, fValue);
			}
		};

		LogStick("LeftThumbStickX", Input::GamePad::LeftThumbStickX());
		LogStick("LeftThumbStickY", Input::GamePad::LeftThumbStickY());
		LogStick("RightThumbStickX", Input::GamePad::RightThumbStickX());
		LogStick("RightThumbStickY", Input::GamePad::RightThumbStickY());
		LogStick("LeftTrigger", Input::GamePad::LeftTrigger());
		LogStick("RightTrigger", Input::GamePad::RightTrigger());

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

void ShowConfig()
{
	static bool isShowMainMenu = true;

	ImGui::SetNextWindowSize(ImVec2(400, 800), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Config", &isShowMainMenu);

	if (ImGui::CollapsingHeader("Debug") == true)
	{
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

		if (ImGui::TreeNode("ASSAO"))
		{
			ImGui::PushID("ASSAO");

			bool isApplyASSAO = Config::IsEnable("SSAO"_s);
			if (ImGui::Checkbox("Apply", &isApplyASSAO) == true)
			{
				Config::SetEnable("SSAO"_s, isApplyASSAO);
			}

			ImGui::PushItemWidth(100);

			auto& settings = Graphics::ASSAO::GetInstance()->GetSettings();
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
			ImGui::DragFloat("TemporalSupersamplingAngleOffset", &settings.TemporalSupersamplingAngleOffset, 0.01f, 0.f, 2.f);
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

			float fNear = Graphics::CameraManager::GetInstance()->GetNear();
			float fFar = Graphics::CameraManager::GetInstance()->GetFar();

			auto& setting = Graphics::DepthOfField::GetInstance()->GetSetting();

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

			Graphics::HDRFilter* pHDRFilter = Graphics::HDRFilter::GetInstance();

			bool isApplyLensFlare = pHDRFilter->IsEnableLensFlare();
			if (ImGui::Checkbox("LensFlare Apply", &isApplyLensFlare) == true)
			{
				pHDRFilter->SetEnableLensFlare(isApplyLensFlare);
			}

			ImGui::PushItemWidth(100);
			float fBloomThreshold = pHDRFilter->GetBloomThreshold();
			if (ImGui::DragFloat("Bloom Threshold", &fBloomThreshold, 0.01f, 0.f, 10.f) == true)
			{
				pHDRFilter->SetBloomThreshold(fBloomThreshold);
			}

			float fBloomMultiplier = pHDRFilter->GetBloomMultiplier();
			if (ImGui::DragFloat("Bloom Multiplier", &fBloomMultiplier, 0.01f, 0.f, 16.f) == true)
			{
				pHDRFilter->SetBloomMultiplier(fBloomMultiplier);
			}

			float fToneMapKey = pHDRFilter->GetToneMapKey();
			if (ImGui::DragFloat("ToneMapKey", &fToneMapKey, 0.01f, 0.f, 10.f) == true)
			{
				pHDRFilter->SetToneMapKey(fToneMapKey);
			}

			float fMaxLuminance = pHDRFilter->GetMaxLuminance();
			if (ImGui::DragFloat("MaxLuminance", &fMaxLuminance, 0.1f, 0.f, 1024.f) == true)
			{
				pHDRFilter->SetMaxLuminance(fMaxLuminance);
			}

			float fBlurSigma = pHDRFilter->GetBlurSigma();
			if (ImGui::DragFloat("fBlurSigma", &fBlurSigma, 0.01f, 0.f, 32.f) == true)
			{
				pHDRFilter->SetBlurSigma(fBlurSigma);
			}

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

	if (ImGui::CollapsingHeader("Light") == true)
	{
		/*GameObject::Sun* pActor = static_cast<GameObject::Sun*>(GameObject::ActorManager::GetInstance()->GetActor(StrID::EastEngine_Sun));
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
				Math::Color changeColor = *reinterpret_cast<Math::Color*>(&color);
				pLight->SetColor(changeColor);
			}
		}

		Math::Vector3 f3LightPos = pActor->GetPosition();
		if (ImGui::DragFloat3("Light Position", reinterpret_cast<float*>(&f3LightPos.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			pActor->SetPosition(f3LightPos);
		}*/
	}

	if (ImGui::CollapsingHeader("Camera") == true)
	{
		Graphics::Camera* pCamera = Graphics::CameraManager::GetInstance()->GetMainCamera();

		Math::Vector3 f3CameraPos = pCamera->GetPosition();
		if (ImGui::DragFloat3("Camera Position", reinterpret_cast<float*>(&f3CameraPos.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			pCamera->SetPosition(f3CameraPos);
		}

		Math::Vector3 f3CameraLookat = pCamera->GetLookat();
		if (ImGui::DragFloat3("Camera Lookat", reinterpret_cast<float*>(&f3CameraLookat.x), 0.01f, -1000000.f, 1000000.f) == true)
		{
			pCamera->SetLookat(f3CameraLookat);
		}

		Math::Vector3 f3Up = pCamera->GetUp();
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

void ShowMotion(bool& isShowMotionMenu, GameObject::ComponentModel* pCompModel)
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
			std::string strFileExtension = File::GetFileExtension(ofn.lpstrFile);
			if (String::IsEqualsNoCase(strFileExtension.c_str(), "fbx") == true)
			{
				Graphics::MotionLoader loader;
				loader.InitFBX(File::GetFileName(ofn.lpstrFile).c_str(), ofn.lpstrFile, 0.01f);
				Graphics::IMotion::Create(loader);
			}
			else if (String::IsEqualsNoCase(strFileExtension.c_str(), "emot") == true)
			{
				Graphics::MotionLoader loader;
				loader.InitEast(File::GetFileName(ofn.lpstrFile).c_str(), ofn.lpstrFile);
				Graphics::IMotion::Create(loader);
			}
		}
	}

	auto& umapMotions = Graphics::MotionManager::GetInstance()->GetMotions();

	std::vector<const char*> vecMotionNames;
	vecMotionNames.reserve(umapMotions.size());

	for (auto iter : umapMotions)
	{
		vecMotionNames.emplace_back(iter.first.c_str());
	}

	static int nSelectedIndex = 0;
	if (vecMotionNames.empty() == false)
	{
		ImGui::ListBox("Motion List", &nSelectedIndex, &vecMotionNames.front(), vecMotionNames.size(), 6);
	}

	Graphics::IMotion* pMotion = nullptr;
	if (0 <= nSelectedIndex && nSelectedIndex < static_cast<int>(vecMotionNames.size()))
	{
		pMotion = Graphics::MotionManager::GetInstance()->GetMotion(vecMotionNames[nSelectedIndex]);
	}

	if (pMotion != nullptr)
	{
		const std::array<char*, Graphics::EmMotion::eLayerCount> layers = { "Layer1", "Layer2", "Layer3", "Layer4", };

		static Graphics::EmMotion::Layers emLayer = Graphics::EmMotion::eLayer1;
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
			Graphics::MotionPlaybackInfo playback;
			playback.fSpeed = fMotionSpeed;
			playback.fWeight = fMotionWeight;
			playback.fBlendTime = fMotionBlendTime;
			playback.nLoopCount = isMotionLoop == true ? Graphics::MotionPlaybackInfo::eMaxLoopCount : 1;
			playback.isInverse = isMotionInverse;

			pCompModel->PlayMotion(emLayer, pMotion, &playback);
		}

		ImGui::Separator();
	}

	Graphics::IMotionSystem* pMotionSystem = pCompModel->GetModelInstance()->GetMotionSystem();
	if (pMotionSystem != nullptr)
	{
		for (int i = 0; i < Graphics::EmMotion::eLayerCount; ++i)
		{
			Graphics::EmMotion::Layers emLayer = static_cast<Graphics::EmMotion::Layers>(i);
			Graphics::IMotionPlayer* pPlayer = pMotionSystem->GetPlayer(emLayer);

			std::string strLayer = String::Format("Layer%d", i);

			ImGui::PushID(strLayer.c_str());

			ImGui::Text(strLayer.c_str());

			Graphics::IMotion* pMotionPlaying = pPlayer->GetMotion();
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

void ShowMaterial(bool& isShowMaterial, Graphics::IMaterial* pMaterial, int nIndex)
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

	const char* strSamplerState[Graphics::EmSamplerState::TypeCount] =
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
	Graphics::EmSamplerState::Type emSamplerState = pMaterial->GetSamplerState();
	if (ImGui::Combo("SamplerState", reinterpret_cast<int*>(&emSamplerState), strSamplerState, Graphics::EmSamplerState::TypeCount) == true)
	{
		pMaterial->SetSamplerState(emSamplerState);
	}

	const char* strBlendState[Graphics::EmBlendState::TypeCount] =
	{
		"Off",
		"Linear",
		"Additive",
		"SubTractive",
		"Multiplicative",
		"Squared",
		"Negative",
		"Opacity",
	};
	Graphics::EmBlendState::Type emBlendState = pMaterial->GetBlendState();
	if (ImGui::Combo("BlendState", reinterpret_cast<int*>(&emBlendState), strBlendState, Graphics::EmBlendState::TypeCount) == true)
	{
		pMaterial->SetBlendState(emBlendState);
	}

	const char* strRasterizerState[Graphics::EmRasterizerState::TypeCount] =
	{
		"SolidCCW",
		"SolidCW",
		"SolidCullNone",
		"WireframeCCW",
		"WireframeCW",
		"WireframeCullNone",
	};
	Graphics::EmRasterizerState::Type emRasterizerState = pMaterial->GetRasterizerState();
	if (ImGui::Combo("RasterizerState", reinterpret_cast<int*>(&emRasterizerState), strRasterizerState, Graphics::EmRasterizerState::TypeCount) == true)
	{
		pMaterial->SetRasterizerState(emRasterizerState);
	}

	const char* strDepthStencilState[Graphics::EmDepthStencilState::TypeCount] =
	{
		"eOn",
		"eOff",
	};
	Graphics::EmDepthStencilState::Type emDepthStencilState = pMaterial->GetDepthStencilState();
	if (ImGui::Combo("DepthStencilState", reinterpret_cast<int*>(&emDepthStencilState), strDepthStencilState, Graphics::EmDepthStencilState::TypeCount) == true)
	{
		pMaterial->SetDepthStencilState(emDepthStencilState);
	}

	auto TextureInfo = [&](Graphics::EmMaterial::Type emType, int nIndex)
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
				std::shared_ptr<Graphics::ITexture> pTexture = Graphics::ITexture::Create(ofn.lpstrFile, ofn.lpstrFile);
				pMaterial->SetTextureName(emType, File::GetFileName(ofn.lpstrFile).c_str());
				pMaterial->SetTexture(emType, pTexture);
			}
		}

		const String::StringID& strName = pMaterial->GetTextureName(emType);
		char buf[1024] = { 0 };
		String::Copy(buf, strName.c_str());
		ImGui::SameLine();
		ImGui::InputText("", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

		std::shared_ptr<Graphics::ITexture> pTexture = pMaterial->GetTexture(emType);
		if (pTexture != nullptr)
		{
			if (ImGui::Button(String::Format("%d.Clear", nIndex).c_str()) == true)
			{
				pMaterial->SetTextureName(emType, "");
				pMaterial->SetTexture(emType, nullptr);
			}

			ImGui::SameLine();
			if (pTexture->GetLoadState() == Graphics::EmLoadState::eComplete)
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
					f2Size.x = Math::Min(f2Size.x, 512.f);
					f2Size.y = Math::Min(f2Size.y, 512.f);

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
			pMaterial->SetAlbedoColor(*reinterpret_cast<Math::Color*>(&color));
		}
		TextureInfo(Graphics::EmMaterial::eAlbedo, 1);
		bool isAlbedoAlphaChannelMaskMap = pMaterial->IsAlbedoAlphaChannelMaskMap();
		if (ImGui::Checkbox("Is Albedo Alpha Channel a Mask Map ?", &isAlbedoAlphaChannelMaskMap) == true)
		{
			pMaterial->SetAlbedoAlphaChannelMaskMap(isAlbedoAlphaChannelMaskMap);
		}
	}

	ImGui::Separator();
	{
		ImGui::Text("Normal");
		TextureInfo(Graphics::EmMaterial::eNormal, 2);
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

		ImGui::Text("Specular Color");
		TextureInfo(Graphics::EmMaterial::eSpecularColor, 3);
	}

	ImGui::Separator();
	{
		ImGui::Text("Roughness");

		if (pMaterial->GetTexture(Graphics::EmMaterial::eRoughness) == nullptr)
		{
			float fRoughness = pMaterial->GetRoughness();
			if (ImGui::DragFloat("Roughness", &fRoughness, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetRoughness(fRoughness);
			}
		}
		TextureInfo(Graphics::EmMaterial::eRoughness, 4);
	}

	ImGui::Separator();
	{
		ImGui::Text("Metallic");

		if (pMaterial->GetTexture(Graphics::EmMaterial::eMetallic) == nullptr)
		{
			float fMetallic = pMaterial->GetMetallic();
			if (ImGui::DragFloat("Metallic", &fMetallic, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetMetallic(fMetallic);
			}
		}

		TextureInfo(Graphics::EmMaterial::eMetallic, 5);
	}

	ImGui::Separator();
	{
		ImGui::Text("Surface");

		if (pMaterial->GetTexture(Graphics::EmMaterial::eSurface) == nullptr)
		{
			float fSurface = pMaterial->GetSurface();
			if (ImGui::DragFloat("Surface", &fSurface, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetSurface(fSurface);
			}
		}

		TextureInfo(Graphics::EmMaterial::eSurface, 6);
	}

	ImGui::Separator();
	{
		ImGui::Text("Anisotropic");

		if (pMaterial->GetTexture(Graphics::EmMaterial::eAnisotropic) == nullptr)
		{
			float fAnisotropic = pMaterial->GetAnisotropic();
			if (ImGui::DragFloat("Anisotropic", &fAnisotropic, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetAnisotropic(fAnisotropic);
			}
		}

		TextureInfo(Graphics::EmMaterial::eAnisotropic, 7);
	}

	ImGui::Separator();
	{
		ImGui::Text("Sheen");

		if (pMaterial->GetTexture(Graphics::EmMaterial::eSheen) == nullptr)
		{
			float fSheen = pMaterial->GetSheen();
			if (ImGui::DragFloat("Sheen", &fSheen, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetSheen(fSheen);
			}
		}

		TextureInfo(Graphics::EmMaterial::eSheen, 8);
	}

	ImGui::Separator();
	{
		ImGui::Text("SheenTint");

		if (pMaterial->GetTexture(Graphics::EmMaterial::eSheenTint) == nullptr)
		{
			float fSheenTint = pMaterial->GetSheenTint();
			if (ImGui::DragFloat("SheenTint", &fSheenTint, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetSheenTint(fSheenTint);
			}
		}

		TextureInfo(Graphics::EmMaterial::eSheenTint, 9);
	}

	ImGui::Separator();
	{
		ImGui::Text("Clearcoat");

		if (pMaterial->GetTexture(Graphics::EmMaterial::eClearcoat) == nullptr)
		{
			float fClearcoat = pMaterial->GetClearcoat();
			if (ImGui::DragFloat("Clearcoat", &fClearcoat, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetClearcoat(fClearcoat);
			}
		}

		TextureInfo(Graphics::EmMaterial::eClearcoat, 10);
	}

	ImGui::Separator();
	{
		ImGui::Text("ClearcoatGloss");

		if (pMaterial->GetTexture(Graphics::EmMaterial::eClearcoatGloss) == nullptr)
		{
			float fClearcoatGloss = pMaterial->GetClearcoatGloss();
			if (ImGui::DragFloat("ClearcoatGloss", &fClearcoatGloss, 0.001f, 0.f, 1.f) == true)
			{
				pMaterial->SetClearcoatGloss(fClearcoatGloss);
			}
		}

		TextureInfo(Graphics::EmMaterial::eClearcoatGloss, 11);
	}

	ImGui::Separator();
	{
		ImGui::Text("Emissive");
		ImVec4 color = ImColor(*reinterpret_cast<const ImVec4*>(&pMaterial->GetEmissiveColor()));
		if (ImGui::ColorEdit3("Emissive Color", reinterpret_cast<float*>(&color)) == true)
		{
			pMaterial->SetEmissiveColor(*reinterpret_cast<Math::Color*>(&color));
		}
		TextureInfo(Graphics::EmMaterial::eEmissiveColor, 12);

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
	ImGui_ImplDX11_NewFrame();

	ShowConfig();

	//static bool isShowMaterialEditor = false;
	//m_pMaterialNodeManager->Update(isShowMaterialEditor);

	static bool isShowDebug = true;
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Debug Info", &isShowDebug);

		float fps = MainSystem::GetInstance()->GetFPS();
		float ms = 1.f / fps * 1000.f;
		ImGui::Text("Frame %.2f(%.2f)", fps, ms);

		ImGui::End();
	}

	static bool isShowActorWindow = true;
	if (isShowActorWindow == true)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Actor", &isShowActorWindow);

		ImGui::PushID("Actor");

		uint32_t nActorCount = GameObject::ActorManager::GetInstance()->GetActorCount();

		std::vector<const char*> vecActorName;
		vecActorName.reserve(nActorCount);
		for (uint32_t i = 0; i < nActorCount; ++i)
		{
			GameObject::IActor* pActor = GameObject::ActorManager::GetInstance()->GetActor(i);
			vecActorName.emplace_back(pActor->GetName().c_str());
		}

		static int nSelectedIndex = 0;
		if (vecActorName.empty() == false)
		{
			ImGui::ListBox("List", &nSelectedIndex, &vecActorName.front(), vecActorName.size(), 4);
		}

		GameObject::IActor* pActor = nullptr;
		if (0 <= nSelectedIndex && nSelectedIndex < static_cast<int>(vecActorName.size()))
		{
			pActor = GameObject::ActorManager::GetInstance()->GetActor(nSelectedIndex);
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
					GameObject::Actor::Create(buf);
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
				GameObject::IActor::Destroy(&pActor);
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
				if (GameObject::Actor::CreateByFile(ofn.lpstrFile) == nullptr)
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
				std::string strFileExtension = File::GetFileExtension(path);
				if (strFileExtension.empty() == true)
				{
					String::Concat(path, sizeof(path), ".eact");
				}

				if (String::IsEqualsNoCase(File::GetFileExtension(path).c_str(), "eact") == true)
				{
					GameObject::Actor::SaveToFile(pActor, path);
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

				Math::Vector3 f3Scale = pActor->GetScale();
				if (ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&f3Scale.x), 0.01f, 0.01f, 100.f) == true)
				{
					pActor->SetScale(f3Scale);
				}

				Math::Vector3 f3Pos = pActor->GetPosition();
				if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&f3Pos.x), 0.01f, -1000000.f, 1000000.f) == true)
				{
					pActor->SetPosition(f3Pos);
				}

				static std::unordered_map<GameObject::IActor*, Math::Vector3> umapActorRotation;
				Math::Vector3& f3Rotation = umapActorRotation[pActor];

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

					Math::Quaternion quatRotation = Math::Quaternion::CreateFromYawPitchRoll(Math::ToRadians(f3Rotation.y), Math::ToRadians(f3Rotation.x), Math::ToRadians(f3Rotation.z));

					pActor->SetRotation(quatRotation);
				}

				ImGui::Separator();

				std::vector<const char*> vecComponents;
				for (int i = 0; i < GameObject::EmComponent::TypeCount; ++i)
				{
					GameObject::EmComponent::Type emType = static_cast<GameObject::EmComponent::Type>(i);
					if (pActor->GetComponent(emType) == nullptr)
					{
						const char* strComponentName = GameObject::EmComponent::ToString(emType);
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

					static GameObject::EmComponent::Type emType = GameObject::EmComponent::TypeCount;
					nCurItem = Math::Min(nCurItem, static_cast<int>(vecComponents.size() - 1));
					if (ImGui::Combo("Add Component", &nCurItem, &vecComponents.front(), vecComponents.size()) == true)
					{
						emType = static_cast<GameObject::EmComponent::Type>(nCurItem);
						if (emType != GameObject::EmComponent::TypeCount)
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
			for (int i = 0; i < GameObject::EmComponent::TypeCount; ++i)
			{
				GameObject::IComponent* pComponent = pActor->GetComponent(static_cast<GameObject::EmComponent::Type>(i));
				if (pComponent == nullptr)
					continue;

				switch (i)
				{
				case GameObject::EmComponent::eActionState:
				{
					if (ImGui::CollapsingHeader("ActionState"))
					{
					}
				}
				break;
				case GameObject::EmComponent::eTimer:
				{
					if (ImGui::CollapsingHeader("Timer"))
					{
					}
				}
				break;
				case GameObject::EmComponent::ePhysics:
				{
					if (ImGui::CollapsingHeader("Physics"))
					{
					}
				}
				break;
				case GameObject::EmComponent::eModel:
				{
					if (ImGui::CollapsingHeader("Model"))
					{
						ImGui::PushID("Model");

						GameObject::ComponentModel* pCompModel = static_cast<GameObject::ComponentModel*>(pComponent);
						if (pCompModel->GetModel() != nullptr)
						{
							if (pCompModel->IsLoadComplete() == true)
							{
								Graphics::IModel* pModel = pCompModel->GetModel();

								bool isVisibleModel = pModel->IsVisible();
								if (ImGui::Checkbox("Visible", &isVisibleModel) == true)
								{
									pModel->SetVisible(isVisibleModel);
								}

								Math::Vector3 f3Scale = pModel->GetLocalScale();
								if (ImGui::DragFloat3("Local Scale", reinterpret_cast<float*>(&f3Scale), 0.01f, 0.01f, 100.f) == true)
								{
									pModel->SetLocalScale(f3Scale);
								}

								Math::Vector3 f3Pos = pModel->GetLocalPosition();
								if (ImGui::DragFloat3("Local Position", reinterpret_cast<float*>(&f3Pos), 0.01f, -1000000.f, 1000000.f) == true)
								{
									pModel->SetLocalPosition(f3Pos);
								}

								static std::unordered_map<Graphics::IModel*, Math::Vector3> umapMotionRotation;
								Math::Vector3& f3Rotation = umapMotionRotation[pModel];
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

									Math::Quaternion quatRotation = Math::Quaternion::CreateFromYawPitchRoll(Math::ToRadians(f3Rotation.y), Math::ToRadians(f3Rotation.x), Math::ToRadians(f3Rotation.z));

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
											std::string strFileExtension = File::GetFileExtension(path);
											if (strFileExtension.empty() == true)
											{
												String::Concat(path, sizeof(path), ".emod");
											}

											if (String::IsEqualsNoCase(File::GetFileExtension(path).c_str(), "emod") == true)
											{
												if (Graphics::IModel::SaveToFile(pModel, path) == false)
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
										Graphics::IModelNode* pModelNode = pModel->GetNode(j);

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
												Graphics::IMaterial* pMaterial = pModelNode->GetMaterial(k);

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
									std::string strFileExtension = File::GetFileExtension(ofn.lpstrFile);
									if (String::IsEqualsNoCase(strFileExtension.c_str(), "fbx") == true)
									{
										String::StringID strName;
										strName.Format("%s(%f)", File::GetFileName(ofn.lpstrFile).c_str(), fScaleFactor);

										Graphics::ModelLoader loader;
										loader.InitFBX(strName, ofn.lpstrFile, fScaleFactor);
										loader.SetEnableThreadLoad(true);

										pCompModel->Init(&loader);
									}
									else if (String::IsEqualsNoCase(strFileExtension.c_str(), "obj") == true)
									{
										String::StringID strName;
										strName.Format("%s(%f)", File::GetFileName(ofn.lpstrFile).c_str(), fScaleFactor);

										Graphics::ModelLoader loader;
										loader.InitObj(strName, ofn.lpstrFile, fScaleFactor);
										loader.SetEnableThreadLoad(true);

										pCompModel->Init(&loader);
									}
									else if (String::IsEqualsNoCase(strFileExtension.c_str(), "emod") == true)
									{
										Graphics::ModelLoader loader;
										loader.InitEast(File::GetFileName(ofn.lpstrFile).c_str(), ofn.lpstrFile);
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
									for (int j = Graphics::EmModelLoader::eCube; j < Graphics::EmModelLoader::eCapsule; ++j)
									{
										const char* strName = Graphics::EmModelLoader::GetGeometryTypeToSTring(static_cast<Graphics::EmModelLoader::GeometryType>(j));
										vecGeometryModel.emplace_back(strName);
									}
								}

								static int nSelectedGeometryIndex = 0;
								ImGui::Combo("Type", &nSelectedGeometryIndex, &vecGeometryModel.front(), vecGeometryModel.size());

								Graphics::EmModelLoader::GeometryType emType = Graphics::EmModelLoader::GetGeometryType(vecGeometryModel[nSelectedGeometryIndex]);
								switch (emType)
								{
								case Graphics::EmModelLoader::eCube:
								{
									ImGui::PushID("Cube");

									static char buf[128] = "Cube";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Cube Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										Graphics::ModelLoader loader;
										loader.InitCube(buf, nullptr, fSize);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eBox:
								{
									ImGui::PushID("Box");

									static char buf[128] = "Box";
									ImGui::InputText("Name", buf, sizeof(buf));

									static Math::Vector3 f3Size(Math::Vector3::One);
									ImGui::DragFloat3("Box Size", reinterpret_cast<float*>(&f3Size.x), 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										Graphics::ModelLoader loader;
										loader.InitBox(buf, nullptr, f3Size);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eSphere:
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
										Graphics::ModelLoader loader;
										loader.InitSphere(buf, nullptr, fDiameter, nTessellation);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eGeoSphere:
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
										Graphics::ModelLoader loader;
										loader.InitGeoSphere(buf, nullptr, fDiameter, nTessellation);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eCylinder:
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
										Graphics::ModelLoader loader;
										loader.InitCylinder(buf, nullptr, fHeight, fDiameter, nTessellation);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eCone:
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
										Graphics::ModelLoader loader;
										loader.InitCone(buf, nullptr, fDiameter, fHeight, nTessellation);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eTorus:
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
										Graphics::ModelLoader loader;
										loader.InitTorus(buf, nullptr, fDiameter, fThickness, nTessellation);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eTetrahedron:
								{
									ImGui::PushID("Tetrahedron");

									static char buf[128] = "Tetrahedron";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										Graphics::ModelLoader loader;
										loader.InitTetrahedron(buf, nullptr, fSize);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eOctahedron:
								{
									ImGui::PushID("Octahedron");

									static char buf[128] = "Octahedron";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										Graphics::ModelLoader loader;
										loader.InitOctahedron(buf, nullptr, fSize);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eDodecahedron:
								{
									ImGui::PushID("Dodecahedron");

									static char buf[128] = "Dodecahedron";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										Graphics::ModelLoader loader;
										loader.InitDodecahedron(buf, nullptr, fSize);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eIcosahedron:
								{
									ImGui::PushID("Icosahedron");

									static char buf[128] = "Icosahedron";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fSize = 1.f;
									ImGui::DragFloat("Size", &fSize, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										Graphics::ModelLoader loader;
										loader.InitIcosahedron(buf, nullptr, fSize);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eTeapot:
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
										Graphics::ModelLoader loader;
										loader.InitTeapot(buf, nullptr, fSize, nTessellation);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eHexagon:
								{
									ImGui::PushID("Hexagon");

									static char buf[128] = "Hexagon";
									ImGui::InputText("Name", buf, sizeof(buf));

									static float fRadius = 1.f;
									ImGui::DragFloat("Radius", &fRadius, 0.01f, 0.01f, 1000000.f);

									if (ImGui::Button("Confirm") == true)
									{
										Graphics::ModelLoader loader;
										loader.InitHexagon(buf, nullptr, fRadius);

										pCompModel->Init(&loader);

										ImGui::CloseCurrentPopup();
									}

									ImGui::PopID();
								}
								break;
								case Graphics::EmModelLoader::eCapsule:
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
										Graphics::ModelLoader loader;
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
				case GameObject::EmComponent::eCamera:
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