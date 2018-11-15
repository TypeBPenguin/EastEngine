#include "stdafx.h"
#include "SceneWorld.h"

#include "DirectX/Camera.h"
#include "DirectX/Light.h"

using namespace eastengine;

namespace StrID
{
	RegisterStringID(MainLight);
}

SceneWorld::SceneWorld(const eastengine::string::StringID& strSceneName)
	: IScene(strSceneName)
{
}

SceneWorld::~SceneWorld()
{
}

void SceneWorld::Enter()
{
	graphics::Camera::GetInstance()->SetView(math::float3(0.f, 0.f, -10.f), math::float3(0.f, 0.f, 0.f), math::float3(0.f, 1.f, 0.f));

	math::float3 f3LightDirection(math::float3::Zero - math::float3(0.f, 500.f, -500.f));
	f3LightDirection.Normalize();

	graphics::CascadedShadowsConfig config;
	config.nLevel = 3;
	config.nBufferSize = 2048;
	config.fCascadeDistance = 256.f;

	graphics::ILight* pLight = graphics::ILight::CreateDirectionalLight(StrID::MainLight, f3LightDirection, math::Color::White, 10000.f, 0.5f, 0.25f, &config);
	pLight->SetEnableShadow(false);
}

void SceneWorld::Exit()
{
}

void SceneWorld::Update(float fElapsedTime)
{
}