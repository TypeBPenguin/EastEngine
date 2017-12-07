#include "stdafx.h"
#include "SceneWorld.h"

#include "DirectX/CameraManager.h"
#include "DirectX/Light.h"

using namespace EastEngine;

namespace StrID
{
	RegisterStringID(MainCamera);
	RegisterStringID(MainLight);
}

SceneWorld::SceneWorld(const EastEngine::String::StringID& strSceneName)
	: SceneInterface(strSceneName)
{
}

SceneWorld::~SceneWorld()
{
}

void SceneWorld::Enter()
{
	Graphics::CameraManager::GetInstance()->CreateCamera(StrID::MainCamera, Math::Vector3(0.f, 0.f, -10.f), Math::Vector3(0.f, 0.f, 0.f), Math::Vector3(0.f, 1.f, 0.f));

	Math::Vector3 f3LightDirection(Math::Vector3::Zero - Math::Vector3(0.f, 500.f, -500.f));
	f3LightDirection.Normalize();

	Graphics::CascadedShadowsConfig config;
	config.nLevel = 3;
	config.nBufferSize = 2048;
	config.fCascadeDistance = 256.f;

	Graphics::ILight* pLight = Graphics::ILight::CreateDirectionalLight(StrID::MainLight, f3LightDirection, Math::Color::White, 10000.f, 0.5f, 0.25f, &config);
	pLight->SetEnableShadow(false);
}

void SceneWorld::Exit()
{
}

void SceneWorld::Update(float fElapsedTime)
{
}