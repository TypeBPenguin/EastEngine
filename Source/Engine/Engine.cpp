#include "stdafx.h"
#include "Engine.h"

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
	class Engine::Impl
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

	Engine::Impl::Impl()
		: m_pFpsChecker{ std::make_unique<FpsChecker>() }
	{
	}

	Engine::Impl::~Impl()
	{
		Release();
	}

	bool Engine::Impl::Initialize(const Initializer& initializer)
	{
		std::wstring dumpPath = file::GetBinPath();
		dumpPath.append(L"Dump\\");
		if (CrashHandler::Initialize(dumpPath.c_str()) == false)
			return false;

		thread::ThreadPool::GetInstance();
		jobsystem::Initialize();

		s_pTimer = Timer::GetInstance();
		s_pTimer->SetLimitElapsedTime(initializer.limitElapsedTime);

		auto SystemMesageHandler = [&](HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam) -> HRESULT
		{
			if (s_pInputDevice != nullptr)
			{
				input::Device::GetInstance()->HandleMessage(hWnd, msg, wParam, lParam);
			}
			return 0;
		};
		graphics::Initialize(initializer.emAPI, initializer.width, initializer.height, initializer.isFullScreen, initializer.isVSync, initializer.applicationTitle, initializer.applicationName, SystemMesageHandler);

		s_pInputDevice = input::Device::GetInstance();
		s_pInputDevice->Initialize(graphics::GetHInstance(), graphics::GetHwnd());

		s_pPhysicsSystem = physics::System::GetInstance();
		s_pPhysicsSystem->Initialize({});

		s_pSoundSystem = sound::System::GetInstance();

		s_pModelManager = graphics::ModelManager::GetInstance();
		s_pGameObjectManager = gameobject::GameObjectManager::GetInstance();

		s_pSceneManager = SceneManager::GetInstance();

		est::graphics::SetDefaultImageBaseLight();

		return true;
	}

	void Engine::Impl::Release()
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

	void Engine::Impl::Run(std::vector<std::unique_ptr<IScene>>&& pScenes, const string::StringID& startSceneName)
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
			Update(elapsedTime);

			return m_isRunning;
		});
	}

	void Engine::Impl::Exit()
	{
		m_isRunning = false;
	}

	void Engine::Impl::Update(float elapsedTime)
	{
		TRACER_EVENT(__FUNCTIONW__);
		performance::tracer::RefreshState();

		s_pModelManager->Cleanup(elapsedTime);

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

	Engine::Engine()
		: m_pImpl{ std::make_unique<Impl>() }
	{	
	}

	Engine::~Engine()
	{
	}

	bool Engine::Initialize(const Initializer& initializer)
	{
		return m_pImpl->Initialize(initializer);
	}

	void Engine::Run(std::vector<std::unique_ptr<IScene>>&& pScenes, const string::StringID& startSceneName)
	{
		m_pImpl->Run(std::move(pScenes), startSceneName);
	}

	void Engine::Exit()
	{
		return m_pImpl->Exit();
	}

	float Engine::GetFPS() const
	{
		return m_pImpl->GetFPS();
	}
}