#include "stdafx.h"
#include "System.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Timer.h"
#include "CommonLib/ThreadPool.h"
#include "CommonLib/CrashHandler.h"

#include "Input/InputDevice.h"
#include "Graphics/Model/ModelManager.h"
#include "GameObject/GameObjectManager.h"
#include "Physics/PhysicsSystem.h"
#include "SoundSystem/SoundSystem.h"

#include "FpsChecker.h"
#include "SceneManager.h"

namespace sid
{
	RegisterStringID(Input);
}

namespace est
{
	class MainSystem::Impl
	{
	public:
		Impl();
		~Impl();

	public:
		bool Initialize(const Initializer& initializer);
		void Release();

	public:
		void Run(std::vector<std::unique_ptr<IScene>>&& pScenes, const string::StringID& startSceneName);
		void Exit();

	public:
		float GetFPS() const { return m_pFpsChecker->GetFps(); }

	private:
		void Cleanup(float elapsedTime);
		void Update(float elapsedTime);

	private:
		std::unique_ptr<FpsChecker> m_pFpsChecker;
		Timer* s_pTimer{ nullptr };

		SceneManager* s_pSceneManager{ nullptr };
		input::Device* s_pInputDevice{ nullptr };
		graphics::ModelManager* s_pModelManager{ nullptr };
		gameobject::GameObjectManager* s_pGameObjectManager{ nullptr };
		physics::System* s_pPhysicsSystem{ nullptr };
		sound::System* s_pSoundSystem{ nullptr };

		bool m_isRunning{ true };
	};

	MainSystem::Impl::Impl()
		: m_pFpsChecker{ std::make_unique<FpsChecker>() }
	{
	}

	MainSystem::Impl::~Impl()
	{
		Release();
	}

	bool MainSystem::Impl::Initialize(const Initializer& initializer)
	{
		std::wstring dumpPath = file::GetBinPath();
		dumpPath.append(L"Dump\\");
		if (CrashHandler::Initialize(dumpPath.c_str()) == false)
			return false;

		thread::ThreadPool::GetInstance();
		jobsystem::Initialize();

		s_pTimer = Timer::GetInstance();
		s_pTimer->SetLimitElapsedTime(initializer.limitElapsedTime);

		graphics::Initialize(initializer.emAPI, initializer.width, initializer.height, initializer.isFullScreen, initializer.isVSync, initializer.applicationTitle, initializer.applicationName);

		s_pInputDevice = input::Device::GetInstance();
		s_pInputDevice->Initialize(graphics::GetHInstance(), graphics::GetHwnd());

		graphics::AddMessageHandler(sid::Input, [](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			input::Device::GetInstance()->HandleMessage(hWnd, nMsg, wParam, lParam);
		});

		s_pPhysicsSystem = physics::System::GetInstance();
		s_pPhysicsSystem->Initialize({});

		s_pSoundSystem = sound::System::GetInstance();

		s_pModelManager = graphics::ModelManager::GetInstance();
		s_pGameObjectManager = gameobject::GameObjectManager::GetInstance();

		s_pSceneManager = SceneManager::GetInstance();

		return true;
	}

	void MainSystem::Impl::Release()
	{
		thread::ThreadPool::DestroyInstance();

		SceneManager::DestroyInstance();
		s_pSceneManager = nullptr;

		s_pGameObjectManager->Release();
		gameobject::GameObjectManager::DestroyInstance();
		s_pGameObjectManager = nullptr;

		graphics::ModelManager::DestroyInstance();
		s_pModelManager = nullptr;

		sound::System::DestroyInstance();
		s_pSoundSystem = nullptr;

		physics::System::DestroyInstance();
		s_pPhysicsSystem = nullptr;

		input::Device::DestroyInstance();
		s_pInputDevice = nullptr;

		graphics::Release();
		Timer::DestroyInstance();

		CrashHandler::Release();

		string::Release();
	}

	void MainSystem::Impl::Run(std::vector<std::unique_ptr<IScene>>&& pScenes, const string::StringID& startSceneName)
	{
		for (auto& pScene : pScenes)
		{
			s_pSceneManager->AddScene(std::move(pScene));
		}
		pScenes.clear();

		s_pSceneManager->ChangeScene(startSceneName);

		s_pTimer->Reset();

		graphics::Run([&]()
		{
			s_pTimer->Tick();

			const float elapsedTime = s_pTimer->GetElapsedTime();
			Cleanup(elapsedTime);
			Update(elapsedTime);

			return m_isRunning;
		});
	}

	void MainSystem::Impl::Exit()
	{
		m_isRunning = false;
	}

	void MainSystem::Impl::Cleanup(float elapsedTime)
	{
		TRACER_EVENT(__FUNCTIONW__);
		s_pModelManager->Cleanup(elapsedTime);
		graphics::Cleanup(elapsedTime);
	}

	void MainSystem::Impl::Update(float elapsedTime)
	{
		TRACER_EVENT(__FUNCTIONW__);
		performance::tracer::RefreshState();

		m_pFpsChecker->Update(elapsedTime);
		s_pInputDevice->Update(elapsedTime);

		graphics::Update(elapsedTime);

		s_pSceneManager->Update(elapsedTime);
		s_pPhysicsSystem->Update(elapsedTime);
		s_pGameObjectManager->Update(elapsedTime);
		s_pModelManager->Update();
		s_pSoundSystem->Update(elapsedTime);

		graphics::PostUpdate(elapsedTime);
	}

	MainSystem::MainSystem()
		: m_pImpl{ std::make_unique<Impl>() }
	{	
	}

	MainSystem::~MainSystem()
	{
	}

	bool MainSystem::Initialize(const Initializer& initializer)
	{
		return m_pImpl->Initialize(initializer);
	}

	void MainSystem::Run(std::vector<std::unique_ptr<IScene>>&& pScenes, const string::StringID& startSceneName)
	{
		m_pImpl->Run(std::move(pScenes), startSceneName);
	}

	void MainSystem::Exit()
	{
		return m_pImpl->Exit();
	}

	float MainSystem::GetFPS() const
	{
		return m_pImpl->GetFPS();
	}
}