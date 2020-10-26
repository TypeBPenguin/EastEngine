#include "stdafx.h"
#include "SceneCellularAutomata.h"

#include "Graphics/Interface/imguiHelper.h"

using namespace est;

namespace sid
{
	RegisterStringID(CellularAutomata);
}

SceneCellularAutomata::SceneCellularAutomata()
	: IScene(sid::CellularAutomata)
{
}

SceneCellularAutomata::~SceneCellularAutomata()
{
}

void SceneCellularAutomata::Enter(const std::queue<est::gameobject::ActorPtr>& savedPrevSceneActors)
{
	graphics::Camera* pCamera = graphics::GetCamera();

	graphics::Camera::DescView cameraView;
	cameraView.position = { 0.f, 10.f, -10.f };
	cameraView.lookat = { 0.f, 0.f, 0.f };
	cameraView.up = math::float3::Up;
	pCamera->SetView(cameraView);
}

void SceneCellularAutomata::Exit(std::queue<est::gameobject::ActorPtr>& saveSceneActors_out)
{
}

void SceneCellularAutomata::Update(float elapsedTime)
{
	const ImGuiIO& io = ImGui::GetIO();

	static bool isVisible = false;
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Cellular Automata", &isVisible);

		ImGui::End();
	}
}