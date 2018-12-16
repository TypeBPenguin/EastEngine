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
	//_CrtSetBreakAlloc(752014);
#endif

	LOG_ERROR("Studio Start..");

	try
	{
		eastengine::MainSystem::Initializer initializer;
		initializer.emAPI = eastengine::graphics::eDX11;
		initializer.width = 1600;
		initializer.height = 900;
		initializer.isFullScreen = false;
		initializer.applicationTitle = StrID::EastEngine;
		initializer.applicationName = StrID::EastEngine;

		if (eastengine::MainSystem::GetInstance()->Initialize(initializer) == true)
		{
			eastengine::IScene* pScenes[] = { new SceneNewStudio };
			eastengine::MainSystem::GetInstance()->Run(pScenes, _countof(pScenes), 0);
		}
	}
	catch (const std::exception& e)
	{
		OutputDebugString(e.what());
		LOG_ERROR(e.what());
		system("pause");
	}

	LOG_MESSAGE("게임 종료");
	LOG_MESSAGE("3..");
	Sleep(1000);
	LOG_WARNING("2..");
	Sleep(1000);
	LOG_ERROR("1..");
	Sleep(1000);

	eastengine::MainSystem::DestroyInstance();

    return 0;
}

