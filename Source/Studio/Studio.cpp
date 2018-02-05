#include "stdafx.h"

#include "SceneStudio.h"

namespace StrID
{
	RegisterStringID(Studio);
	RegisterStringID(EastEngine);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdLine, int iCmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(1654);
#endif

	LOG_ERROR("Studio Start..");

	if (EastEngine::MainSystem::GetInstance()->Initialize(StrID::EastEngine, 1600, 900, false, true) == true)
	{
		EastEngine::SceneManager::GetInstance()->AddScene(new SceneStudio);
		EastEngine::SceneManager::GetInstance()->ChangeScene(StrID::Studio);

		EastEngine::MainSystem::GetInstance()->Run();
	}

	LOG_MESSAGE("게임 종료");
	LOG_MESSAGE("3..");
	Sleep(1000);
	LOG_WARNING("2..");
	Sleep(1000);
	LOG_ERROR("1..");
	Sleep(1000);

	EastEngine::MainSystem::DestroyInstance();

    return 0;
}

