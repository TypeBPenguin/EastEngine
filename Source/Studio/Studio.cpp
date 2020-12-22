#include "stdafx.h"

#include "SceneStudio.h"

namespace sid
{
	RegisterStringID(Studio);
}

int main()
{
#if defined(_CRTDBG_MAP_ALLOC) && (defined(DEBUG) || defined(_DEBUG))
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(110106);
#endif

	LOG_ERROR(L"Studio Start..");

	try
	{
		est::MainSystem::Initializer initializer;
		initializer.emAPI = est::graphics::eDX11;
		initializer.width = 1600;
		initializer.height = 900;
		initializer.isFullScreen = false;
		initializer.isVSync = false;
		initializer.applicationTitle = sid::Studio;
		initializer.applicationName = sid::Studio;
		initializer.limitElapsedTime = 1.0;

		if (est::MainSystem::GetInstance()->Initialize(initializer) == true)
		{
			est::graphics::SetDefaultImageBaseLight();
			{
				std::vector<std::unique_ptr<est::IScene>> pScenes;
				pScenes.emplace_back(std::make_unique<SceneStudio>());

				est::MainSystem::GetInstance()->Run(std::move(pScenes), SceneStudio::Name);
			}
		}
	}
	catch (const std::exception& e)
	{
		const std::wstring what = est::string::MultiToWide(e.what());
		OutputDebugString(what.c_str());
		LOG_ERROR(what.c_str());
		system("pause");
	}

	LOG_MESSAGE(L"게임 종료");
	LOG_MESSAGE(L"3..");
	Sleep(1000);
	LOG_WARNING(L"2..");
	Sleep(1000);
	LOG_ERROR(L"1..");
	Sleep(1000);

	est::MainSystem::DestroyInstance();

    return 0;
}

