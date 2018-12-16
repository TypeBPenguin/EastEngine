#include "stdafx.h"
#include "System.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Timer.h"
#include "CommonLib/ThreadPool.h"
#include "CommonLib/CrashHandler.h"

#include "Input/InputDevice.h"
#include "Model/ModelManager.h"
#include "GameObject/GameObjectManager.h"
#include "Physics/PhysicsSystem.h"
#include "SoundSystem/SoundSystem.h"

#include "FpsChecker.h"
#include "SceneManager.h"

namespace StrID
{
	RegisterStringID(Input);
}

namespace eastengine
{
	class MainSystem::Impl
	{
	public:
		Impl();
		~Impl();

	public:
		bool Initialize(graphics::APIs emAPI, uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName);
		void Release();

	public:
		void Run(IScene** ppScene, size_t nSceneCount, size_t nMainScene);

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
	};

	MainSystem::Impl::Impl()
		: m_pFpsChecker{ std::make_unique<FpsChecker>() }
	{
	}

	MainSystem::Impl::~Impl()
	{
		Release();
	}

	bool MainSystem::Impl::Initialize(graphics::APIs emAPI, uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName)
	{
		std::string strDumpPath = file::GetBinPath();
		strDumpPath.append("Dump\\");
		if (CrashHandler::Initialize(strDumpPath.c_str()) == false)
			return false;

		thread::ThreadPool::GetInstance();

		s_pTimer = Timer::GetInstance();

		graphics::Initialize(emAPI, nWidth, nHeight, isFullScreen, strApplicationTitle, strApplicationName);

		s_pInputDevice = input::Device::GetInstance();
		s_pInputDevice->Initialize(graphics::GetHInstance(), graphics::GetHwnd());

		graphics::AddMessageHandler(StrID::Input, [](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			input::Device::GetInstance()->HandleMessage(hWnd, nMsg, wParam, lParam);
		});

		s_pPhysicsSystem = physics::System::GetInstance();
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

	void MainSystem::Impl::Run(IScene** ppScene, size_t nSceneCount, size_t nMainScene)
	{
		s_pTimer->Start();

		for (size_t i = 0; i < nSceneCount; ++i)
		{
			s_pSceneManager->AddScene(ppScene[i]);
		}

		s_pSceneManager->ChangeScene(ppScene[nMainScene]->GetName());

		graphics::Run([&]()
		{
			s_pTimer->Tick();

			const float elapsedTime = s_pTimer->GetElapsedTime();
			Cleanup(elapsedTime);
			Update(elapsedTime);
		});
	}

	void MainSystem::Impl::Cleanup(float elapsedTime)
	{
		s_pModelManager->Cleanup(elapsedTime);
		graphics::Cleanup(elapsedTime);
	}

	void MainSystem::Impl::Update(float elapsedTime)
	{
		performance::tracer::RefreshState();

		m_pFpsChecker->Update(elapsedTime);
		s_pInputDevice->Update(elapsedTime);

		graphics::Update(elapsedTime);

		s_pSceneManager->Update(elapsedTime);
		s_pPhysicsSystem->Update(elapsedTime);
		s_pGameObjectManager->Update(elapsedTime);
		s_pModelManager->Update();
		s_pSoundSystem->Update(elapsedTime);

		graphics::PostUpdate();
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
		return m_pImpl->Initialize(initializer.emAPI, initializer.width, initializer.height, initializer.isFullScreen, initializer.applicationTitle, initializer.applicationName);
	}

	void MainSystem::Run(IScene** ppScene, size_t nSceneCount, size_t nMainScene)
	{
		m_pImpl->Run(ppScene, nSceneCount, nMainScene);
	}

	float MainSystem::GetFPS() const
	{
		return m_pImpl->GetFPS();
	}
}