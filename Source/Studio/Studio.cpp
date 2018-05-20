#include "stdafx.h"

#include "SceneNewStudio.h"

namespace StrID
{
	RegisterStringID(Studio);
	RegisterStringID(EastEngine);
}

int main()
{
#if defined(DEBUG) || defined(_DEBUG)
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(329250);
#endif

	LOG_ERROR("Studio Start..");

	try
	{
		const eastengine::graphics::APIs emAPI = eastengine::graphics::eDX12;
		if (eastengine::MainSystem::GetInstance()->Initialize(emAPI, 1600, 900, false, StrID::EastEngine, StrID::EastEngine) == true)
		{
			eastengine::IScene* pScenes[] = { new SceneNewStudio };
			eastengine::MainSystem::GetInstance()->Run(pScenes, _countof(pScenes), 0);
		}

		LOG_MESSAGE("게임 종료");
		LOG_MESSAGE("3..");
		Sleep(1000);
		LOG_WARNING("2..");
		Sleep(1000);
		LOG_ERROR("1..");
		Sleep(1000);

		eastengine::MainSystem::DestroyInstance();
	}
	catch (const std::exception& e)
	{
		OutputDebugString(e.what());
		LOG_ERROR(e.what());
		system("pause");
	}

    return 0;
}

