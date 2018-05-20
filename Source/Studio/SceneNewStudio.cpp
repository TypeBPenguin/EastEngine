#include "stdafx.h"
#include "SceneNewStudio.h"

#include "CommonLib/FileUtil.h"
#include "GraphicsInterface/Camera.h"
#include "Input/InputInterface.h"
#include "Model/ModelInterface.h"
#include "Model/ModelLoader.h"

using namespace eastengine;

namespace StrID
{
	RegisterStringID(Studio);
}

const std::string IBL_Type[] =
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

struct Cube
{
	math::Matrix matWorld;
	math::Matrix matRot;
	math::Vector3 f3Position;

	graphics::IModelInstance* pModelInstance{ nullptr };

	~Cube()
	{
		graphics::IModel::DestroyInstance(&pModelInstance);
	}
};

std::unique_ptr<Cube> pCube1;
std::unique_ptr<Cube> pCube2;

bool CreateBox(graphics::IVertexBuffer** ppVertexBuffer, graphics::IIndexBuffer** ppIndexBuffer)
{
	// A box has six faces, each one pointing in a different direction.
	const int FaceCount = 6;

	static const math::Vector3 faceNormals[FaceCount] =
	{
		{ 0, 0, 1 },
		{ 0, 0, -1 },
		{ 1, 0, 0 },
		{ -1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, -1, 0 },
	};

	static const math::Vector2 textureCoordinates[4] =
	{
		{ 1, 0 },
		{ 1, 1 },
		{ 0, 1 },
		{ 0, 0 },
	};

	std::vector<graphics::VertexPosTexNor> vecVertices;
	std::vector<uint32_t> vecIndices;

	// Create each face in turn.
	for (int i = 0; i < FaceCount; ++i)
	{
		math::Vector3 normal = faceNormals[i];

		// Get two vectors perpendicular both to the face normal and to each other.
		math::Vector3 basis = (i >= 4) ? math::Vector3(0.f, 0.f, 1.f) : math::Vector3(0.f, 1.f, 0.f);

		math::Vector3 side1 = normal.Cross(basis);
		math::Vector3 side2 = normal.Cross(side1);

		// Six indices (two triangles) per face.
		uint32_t vbase = static_cast<uint32_t>(vecVertices.size());
		vecIndices.emplace_back(vbase + 0);
		vecIndices.emplace_back(vbase + 1);
		vecIndices.emplace_back(vbase + 2);

		vecIndices.emplace_back(vbase + 0);
		vecIndices.emplace_back(vbase + 2);
		vecIndices.emplace_back(vbase + 3);

		// Four vertices per face.
		vecVertices.push_back(graphics::VertexPosTexNor((normal - side1 - side2) * 0.5f, textureCoordinates[0], normal));
		vecVertices.push_back(graphics::VertexPosTexNor((normal - side1 + side2) * 0.5f, textureCoordinates[1], normal));
		vecVertices.push_back(graphics::VertexPosTexNor((normal + side1 + side2) * 0.5f, textureCoordinates[2], normal));
		vecVertices.push_back(graphics::VertexPosTexNor((normal + side1 - side2) * 0.5f, textureCoordinates[3], normal));
	}

	for (auto it = vecIndices.begin(); it != vecIndices.end(); it += 3)
	{
		std::swap(*it, *(it + 2));
	}

	(*ppVertexBuffer) = graphics::CreateVertexBuffer(reinterpret_cast<const uint8_t*>(vecVertices.data()), sizeof(graphics::VertexPosTexNor) * vecVertices.size(), static_cast<uint32_t>(vecVertices.size()));
	(*ppIndexBuffer) = graphics::CreateIndexBuffer(reinterpret_cast<const uint8_t*>(vecIndices.data()), sizeof(uint32_t) * vecIndices.size(), static_cast<uint32_t>(vecIndices.size()));

	return true;
}

SceneNewStudio::SceneNewStudio()
	: IScene(StrID::Studio)
{
}

SceneNewStudio::~SceneNewStudio()
{
}

void SceneNewStudio::Enter()
{
	const math::UInt2& n2ScreenSize = graphics::GetScreenSize();

	const float fAspect = static_cast<float>(n2ScreenSize.x) / static_cast<float>(n2ScreenSize.y);
	graphics::Camera::GetInstance()->SetProjection(n2ScreenSize.x, n2ScreenSize.y, math::PIDIV4, 0.1f, 1000.f, graphics::GetAPI() == graphics::eVulkan);
	graphics::Camera::GetInstance()->SetView({ 0.f, 2.f, -10.f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f });

	{
		math::Vector3 f3LightPosition(0.f, 500.f, -500.f);
		math::Vector3 f3LightDirection(math::Vector3::Zero - f3LightPosition);
		f3LightDirection.Normalize();

		graphics::ILight* pLight = graphics::ILight::CreateDirectionalLight("MainLight", f3LightDirection, math::Color::White, 1.f, 0.5f, 0.25f);
		pLight->SetEnableShadow(false);
	}

	std::string strPath = file::GetPath(file::eTexture);
	{
		strPath = String::Format("%sIBL\\%s\\%s", file::GetPath(file::eTexture), IBL_Type[1].c_str(), IBL_Type[1].c_str());

		graphics::ImageBasedLight* pImageBasedLight = graphics::GetImageBasedLight();

		std::string strDiffuseHDR = strPath;
		strDiffuseHDR.append("DiffuseHDR.dds");
		graphics::ITexture* pDiffuseHDR = graphics::CreateTextureAsync(strDiffuseHDR.c_str());
		pImageBasedLight->SetDiffuseHDR(pDiffuseHDR);

		std::string strSpecularHDR = strPath;
		strSpecularHDR.append("SpecularHDR.dds");
		graphics::ITexture* pSpecularHDR = graphics::CreateTextureAsync(strSpecularHDR.c_str());
		pImageBasedLight->SetSpecularHDR(pSpecularHDR);

		std::string strSpecularBRDF = strPath;
		strSpecularBRDF.append("Brdf.dds");
		graphics::ITexture* pSpecularBRDF = graphics::CreateTextureAsync(strSpecularBRDF.c_str());
		pImageBasedLight->SetSpecularBRDF(pSpecularBRDF);
	}

	graphics::MaterialInfo materialInfo;
	materialInfo.strPath = file::GetDataPath();
	materialInfo.strPath.append("Model\\MetalPlates\\Texture\\");

	materialInfo.strTextureNameArray[graphics::EmMaterial::eAlbedo] = "MetalPlates_Diffuse.tga";
	materialInfo.strTextureNameArray[graphics::EmMaterial::eNormal].Format("MetalPlates_Normal_%02d.tga", 1);
	materialInfo.strTextureNameArray[graphics::EmMaterial::eMetallic] = "MetalPlates_Metalness.tga";
	materialInfo.f4PaddingRoughMetEmi.y = 0.5f;
	materialInfo.f4PaddingRoughMetEmi.z = 0.5f;
	materialInfo.f4SurSpecTintAniso.y = 1.f;
	materialInfo.f4SurSpecTintAniso.z = 1.f;
	materialInfo.f4SheenTintClearcoatGloss.z = 1.f;
	materialInfo.f4SheenTintClearcoatGloss.w = 1.f;

	graphics::ModelLoader modelLoader;
	modelLoader.InitCube("TestCube", &materialInfo);

	pCube1 = std::make_unique<Cube>();
	pCube1->pModelInstance = graphics::IModel::CreateInstance(modelLoader, false);

	pCube2 = std::make_unique<Cube>();
	pCube2->pModelInstance = graphics::IModel::CreateInstance(modelLoader, false);

	/*CreateBox(&pCube1->pVertexBuffer, &pCube1->pIndexBuffer);
	pCube2->pVertexBuffer = pCube1->pVertexBuffer;
	pCube2->pIndexBuffer = pCube1->pIndexBuffer;

	pCube1->pMaterial = graphics::CreateMaterial(&materialInfo);
	pCube2->pMaterial = pCube1->pMaterial;
	pCube2->pMaterial->IncreaseReference();*/
	//pCube2->pMaterial = graphics::CreateMaterial(&materialInfo);

	//pCube1->pTexture = graphics::CreateTextureAsync("D:\\Projects\\Repos\\EastEngine\\Bin\\Data\\Texture\\gi_flag.tga");
	//pCube2->pTexture = graphics::CreateTextureAsync("D:\\Projects\\Repos\\EastEngine\\Bin\\Data\\Texture\\uv_checker.png");

	pCube2->f3Position = { 1.5f, 0.f, 0.f };
	pCube2->matWorld = math::Matrix::CreateTranslation(pCube2->f3Position + pCube1->f3Position);
}

void SceneNewStudio::Exit()
{
	pCube1.reset();
	pCube2.reset();
}

void SceneNewStudio::Update(float fElapsedTime)
{
	ProcessInput(fElapsedTime);

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

	math::Matrix rotXMat = math::Matrix::CreateRotationX(1.f * fElapsedTime);
	math::Matrix rotYMat = math::Matrix::CreateRotationY(2.f * fElapsedTime);
	math::Matrix rotZMat = math::Matrix::CreateRotationZ(3.f * fElapsedTime);

	pCube1->matRot = rotZMat * (pCube1->matRot * (rotXMat * rotYMat));

	math::Matrix translationMat;
	translationMat.Translation(pCube1->f3Position);

	//pCube1->matWorld = pCube1->matRot * translationMat;

	pCube1->pModelInstance->Update(fElapsedTime, pCube1->matWorld);

	//graphics::RenderJobStatic renderJob1(pCube1.get(), pCube1->pVertexBuffer, pCube1->pIndexBuffer, pCube1->pMaterial, pCube1->matWorld, 0, pCube1->pIndexBuffer->GetIndexCount(), 0.f, {});
	//graphics::PushRenderJob(renderJob1);
	//graphics::PushRenderJob(pCube1->pVertexBuffer, pCube1->pIndexBuffer, pCube1->pTexture, pCube1->matWorld);

	rotXMat = math::Matrix::CreateRotationX(3.f * fElapsedTime);
	rotYMat = math::Matrix::CreateRotationY(2.f * fElapsedTime);
	rotZMat = math::Matrix::CreateRotationZ(1.f * fElapsedTime);

	pCube2->matRot = rotZMat * (pCube2->matRot * (rotXMat * rotYMat));

	math::Matrix translationOffsetMat;
	translationOffsetMat.Translation(pCube2->f3Position);

	math::Matrix scaleMat = math::Matrix::CreateScale(0.5f, 0.5f, 0.5f);

	pCube2->matWorld = scaleMat * translationOffsetMat * pCube2->matRot * translationMat;

	pCube2->pModelInstance->Update(fElapsedTime, pCube2->matWorld);

	//graphics::RenderJobStatic renderJob2(pCube1.get(), pCube2->pVertexBuffer, pCube2->pIndexBuffer, pCube2->pMaterial, pCube2->matWorld, 0, pCube2->pIndexBuffer->GetIndexCount(), 0.f, {});
	//graphics::PushRenderJob(renderJob2);
	//graphics::PushRenderJob(pCube2->pVertexBuffer, pCube2->pIndexBuffer, pCube2->pTexture, pCube2->matWorld);
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