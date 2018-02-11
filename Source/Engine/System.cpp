#include "stdafx.h"
#include "System.h"

#include "FpsChecker.h"
#include "SceneManager.h"

#include "CommonLib/CommandLine.h"
#include "CommonLib/FileUtil.h"
#include "CommonLib/DirectoryMonitor.h"
#include "CommonLib/ThreadPool.h"
#include "CommonLib/Timer.h"
#include "CommonLib/CrashHandler.h"
#include "CommonLib/Performance.h"

#include "CommonLib/PipeStream.h"

#include "DirectX/Device.h"
#include "LuaSystem/LuaSystem.h"
#include "Input/InputDevice.h"
#include "Physics/PhysicsSystem.h"
#include "SoundSystem/SoundSystem.h"
#include "GameObject/ActorManager.h"
#include "GameObject/TerrainManager.h"
#include "GameObject/SkyManager.h"
#include "UI/UIMgr.h"

#include "GameObject/ComponentModel.h"

namespace EastEngine
{
	class MainSystem::Impl
	{
	public:
		Impl();
		~Impl();

	public:
		bool Initialize(const String::StringID& strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync);

	public:
		void Run();

		bool HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

	public:
		float GetFPS() const { return m_pFpsChecker->GetFps(); }

	public:
		const String::StringID& GetApplicationName() const { return m_strApplicationName; }
		const Math::Int2& GetScreenSize() const { return m_n2ScreenSize; }
		bool IsFullScreen() const { return m_isFullScreen; }

	private:
		void Synchronize();
		void Flush(float fElapsedTime);
		void Update(float fElapsedTime);
		void Render();

	private:
		std::unique_ptr<FpsChecker> m_pFpsChecker;
		SceneManager* s_pSceneManager{ nullptr };

		Timer* s_pTimer{ nullptr };

		Performance::Tracer* s_pPerformanceTracer{ nullptr };

		Config::SCommandLine* s_pCommandLine{ nullptr };
		File::DirectoryMonitor* s_pDirectoryMonitor{ nullptr };
		Lua::System* s_pLuaSystem{ nullptr };
		Windows::WindowsManager* s_pWindows{ nullptr };
		Graphics::GraphicsSystem* s_pGraphicsSystem{ nullptr };
		Input::Device* s_pInputDevice{ nullptr };
		Physics::PhysicsSystem* s_pPhysicsSystem{ nullptr };
		Sound::System* s_pSoundSystem{ nullptr };
		UI::UIManager* s_pUIMgr{ nullptr };
		GameObject::ActorManager* s_pActorMgr{ nullptr };
		GameObject::TerrainManager* s_pTerrainManager{ nullptr };
		GameObject::SkyManager* m_pSkyManager{ nullptr };

		String::StringID m_strApplicationName;
		Math::Int2 m_n2ScreenSize;
		bool m_isFullScreen{ false };
		bool m_isVsync{ false };
	};

	MainSystem::Impl::Impl()
		: m_pFpsChecker{ std::make_unique<FpsChecker>() }
	{
	}

	MainSystem::Impl::~Impl()
	{
		Thread::ThreadPool::GetInstance()->Release();
		Thread::ThreadPool::DestroyInstance();

		SceneManager::DestroyInstance();
		s_pSceneManager = nullptr;

		GameObject::ActorManager::DestroyInstance();
		s_pActorMgr = nullptr;

		GameObject::TerrainManager::DestroyInstance();
		s_pTerrainManager = nullptr;

		GameObject::SkyManager::DestroyInstance();
		m_pSkyManager = nullptr;

		SafeRelease(s_pUIMgr);
		UI::UIManager::DestroyInstance();

		Sound::System::DestroyInstance();
		s_pSoundSystem = nullptr;

		SafeRelease(s_pPhysicsSystem);
		Physics::PhysicsSystem::DestroyInstance();

		Input::Device::DestroyInstance();
		s_pInputDevice = nullptr;

		Graphics::GraphicsSystem::DestroyInstance();
		s_pGraphicsSystem = nullptr;

		Lua::System::DestroyInstance();
		s_pLuaSystem = nullptr;

		Windows::WindowsManager::DestroyInstance();
		s_pWindows = nullptr;

		s_pTimer = nullptr;
		Timer::DestroyInstance();

		s_pCommandLine = nullptr;
		Config::SCommandLine::DestroyInstance();

		File::DirectoryMonitor::DestroyInstance();
		s_pDirectoryMonitor = nullptr;

		Performance::Tracer::DestroyInstance();

		String::Release();
		CrashHandler::Release();
	}

	bool MainSystem::Impl::Initialize(const String::StringID& strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync)
	{
		std::string strDumpPath = File::GetBinPath();
		strDumpPath.append("Dump\\");
		if (CrashHandler::Initialize(strDumpPath.c_str()) == false)
			return false;

		s_pPerformanceTracer = Performance::Tracer::GetInstance();

		s_pDirectoryMonitor = File::DirectoryMonitor::GetInstance();

		m_strApplicationName = strApplicationName;
		m_n2ScreenSize = Math::Int2(nScreenWidth, nScreenHeight);
		m_isFullScreen = isFullScreen;;
		m_isVsync = isVsync;;

		s_pCommandLine = Config::SCommandLine::GetInstance();
		if (s_pCommandLine->Init() == false)
			return false;

		Thread::ThreadPool::GetInstance()->Init(std::thread::hardware_concurrency() - 1);

		s_pTimer = Timer::GetInstance();

		s_pWindows = Windows::WindowsManager::GetInstance();

		if (s_pWindows->Initialize(strApplicationName.c_str(), nScreenWidth, nScreenHeight, isFullScreen) == false)
			return false;

		HWND hWnd = s_pWindows->GetHwnd();
		HINSTANCE hInstance = s_pWindows->GetHInstance();

		s_pGraphicsSystem = Graphics::GraphicsSystem::GetInstance();
		if (s_pGraphicsSystem->Initialize(hWnd, m_n2ScreenSize.x, m_n2ScreenSize.y, m_isFullScreen, m_isVsync, 1.f) == false)
			return false;

		s_pWindows->AddMessageHandler([](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) -> bool
		{
			return Graphics::GraphicsSystem::GetInstance()->HandleMessage(hWnd, nMsg, wParam, lParam);
		});

		s_pInputDevice = Input::Device::GetInstance();
		if (s_pInputDevice->Initialize(hInstance, hWnd) == false)
			return false;

		s_pWindows->AddMessageHandler([](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) -> bool
		{
			return Input::Device::GetInstance()->HandleMessage(hWnd, nMsg, wParam, lParam);
		});

		s_pPhysicsSystem = Physics::PhysicsSystem::GetInstance();
		if (s_pPhysicsSystem->Init() == false)
			return false;

		s_pSoundSystem = Sound::System::GetInstance();

		s_pLuaSystem = Lua::System::GetInstance();
		if (s_pLuaSystem->Initialize(false) == false)
			return false;

		s_pSceneManager = SceneManager::GetInstance();

		s_pUIMgr = UI::UIManager::GetInstance();
		if (s_pUIMgr->Init(s_pWindows->GetHwnd()) == false)
			return false;

		s_pWindows->AddMessageHandler([](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) -> bool
		{
			return UI::UIManager::GetInstance()->HandleMsg(hWnd, nMsg, wParam, lParam);
		});

		s_pActorMgr = GameObject::ActorManager::GetInstance();
		s_pTerrainManager = GameObject::TerrainManager::GetInstance();
		m_pSkyManager = GameObject::SkyManager::GetInstance();

		return true;
	}

	void MainSystem::Impl::Run()
	{
		MSG msg;
		Memory::Clear(&msg, sizeof(msg));

		// 유저로부터 종료 메세지를 받을 때까지 루프
		while (true)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// 윈도우에서 어플리케이션의 종료를 요청하는 경우 종료
			if (msg.message == WM_QUIT)
				break;
			
			s_pPerformanceTracer->RefreshState();

			PERF_TRACER_EVENT("System", "Frame");

			s_pTimer->Tick();
			float fElapsedTime = s_pTimer->GetElapsedTime();

			m_pFpsChecker->Update(fElapsedTime);

			s_pWindows->ProcessMessages();

			Flush(fElapsedTime);
			Synchronize();
			
			Concurrency::parallel_invoke
			(
				[&] { Update(fElapsedTime); },
				[&] { Render(); }
			);
		}
	}

	bool MainSystem::Impl::HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
	{
		return false;
	}

	void MainSystem::Impl::Synchronize()
	{
		PERF_TRACER_EVENT("System::Synchronize", "");
		s_pGraphicsSystem->Synchronize();
	}

	void MainSystem::Impl::Flush(float fElapsedTime)
	{
		PERF_TRACER_EVENT("System::Flush", "");
		s_pSceneManager->Flush();
		s_pGraphicsSystem->Flush(fElapsedTime);
	}

	void MainSystem::Impl::Update(float fElapsedTime)
	{
		PERF_TRACER_EVENT("System::Update", "");
		s_pDirectoryMonitor->Update();
		s_pInputDevice->Update(fElapsedTime);
		s_pSoundSystem->Update(fElapsedTime);
		s_pPhysicsSystem->Update(fElapsedTime);
		s_pSceneManager->Update(fElapsedTime);
		s_pTerrainManager->Update(fElapsedTime);
		m_pSkyManager->Update(fElapsedTime);
		s_pActorMgr->Update(fElapsedTime);
		s_pGraphicsSystem->Update(fElapsedTime);
		s_pUIMgr->Update(fElapsedTime);
	}

	void MainSystem::Impl::Render()
	{
		PERF_TRACER_EVENT("System::Render", "");
		s_pGraphicsSystem->BeginScene(0.f, 0.f, 0.f, 1.f);
		s_pGraphicsSystem->Render();
		s_pGraphicsSystem->EndScene();
	}

	MainSystem::MainSystem()
		: m_pImpl{ std::make_unique<Impl>() }
	{	
	}

	MainSystem::~MainSystem()
	{
	}

	bool MainSystem::Initialize(const String::StringID& strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync)
	{
		return m_pImpl->Initialize(strApplicationName, nScreenWidth, nScreenHeight, isFullScreen, isVsync);
	}

	void MainSystem::Run()
	{
		m_pImpl->Run();
	}

	bool MainSystem::HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
	{
		return m_pImpl->HandleMessage(hWnd, nMsg, wParam, lParam);
	}

	float MainSystem::GetFPS() const
	{
		return m_pImpl->GetFPS();
	}

	const String::StringID& MainSystem::GetApplicationName() const
	{
		return m_pImpl->GetApplicationName();
	}

	const Math::Int2& MainSystem::GetScreenSize() const
	{
		return m_pImpl->GetScreenSize();
	}

	bool MainSystem::IsFullScreen() const
	{
		return m_pImpl->IsFullScreen();
	}
}