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

	if (EastEngine::MainSystem::GetInstance()->Init(StrID::EastEngine, 1600, 900, false, false) == true)
	{
		EastEngine::SSceneMgr::GetInstance()->AddScene(new SceneStudio);
		EastEngine::SSceneMgr::GetInstance()->ChangeScene(StrID::Studio);

		EastEngine::MainSystem::GetInstance()->Run();
	}

	PRINT_LOG("게임 종료");
	PRINT_LOG("3..");
	Sleep(1000);
	PRINT_LOG("2..");
	Sleep(1000);
	PRINT_LOG("1..");
	Sleep(1000);

	EastEngine::MainSystem::GetInstance()->Release();
	EastEngine::MainSystem::DestroyInstance();

    return 0;
}

