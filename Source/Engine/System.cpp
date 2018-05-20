#include "stdafx.h"
#include "System.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Timer.h"
#include "CommonLib/CrashHandler.h"

#include "Input/InputDevice.h"
#include "Model/ModelManager.h"
//#include "GameObject/ActorManager.h"

#include "FpsChecker.h"
#include "SceneManager.h"

namespace eastengine
{
	class MainSystem::Impl
	{
	public:
		Impl();
		~Impl();

	public:
		bool Initialize(graphics::APIs emAPI, uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName);
		void Release();

	public:
		void Run(IScene** ppScene, size_t nSceneCount, size_t nMainScene);

	public:
		float GetFPS() const { return m_pFpsChecker->GetFps(); }

	private:
		void Flush(float fElapsedTime);
		void Update(float fElapsedTime);

	private:
		std::unique_ptr<FpsChecker> m_pFpsChecker;
		Timer* s_pTimer{ nullptr };

		SceneManager* s_pSceneManager{ nullptr };
		input::Device* s_pInputDevice{ nullptr };
		//gameobject::ActorManager* s_pActorManager{ nullptr };
		graphics::ModelManager* s_pModelManager{ nullptr };
	};

	MainSystem::Impl::Impl()
		: m_pFpsChecker{ std::make_unique<FpsChecker>() }
	{
	}

	MainSystem::Impl::~Impl()
	{
		Release();
	}

	bool MainSystem::Impl::Initialize(graphics::APIs emAPI, uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName)
	{
		std::string strDumpPath = file::GetBinPath();
		strDumpPath.append("Dump\\");
		if (CrashHandler::Initialize(strDumpPath.c_str()) == false)
			return false;

		s_pTimer = Timer::GetInstance();

		graphics::Initialize(emAPI, nWidth, nHeight, isFullScreen, strApplicationTitle, strApplicationName);

		s_pInputDevice = input::Device::GetInstance();
		s_pInputDevice->Initialize(graphics::GetHInstance(), graphics::GetHwnd());

		graphics::AddMessageHandler([](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			input::Device::GetInstance()->HandleMessage(hWnd, nMsg, wParam, lParam);
		});

		s_pModelManager = graphics::ModelManager::GetInstance();

		s_pSceneManager = SceneManager::GetInstance();

		return true;
	}

	void MainSystem::Impl::Release()
	{
		SceneManager::DestroyInstance();
		s_pSceneManager = nullptr;

		//gameobject::ActorManager::DestroyInstance();
		//s_pActorManager = nullptr;

		graphics::ModelManager::DestroyInstance();
		s_pModelManager = nullptr;

		input::Device::DestroyInstance();
		s_pInputDevice = nullptr;

		graphics::Release();
		Timer::DestroyInstance();

		CrashHandler::Release();

		String::Release();
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

			float fElapsedTime = s_pTimer->GetElapsedTime();
			Flush(fElapsedTime);
			Update(fElapsedTime);
		});
	}

	void MainSystem::Impl::Flush(float fElapsedTime)
	{
		s_pSceneManager->Flush();
		s_pModelManager->Flush(fElapsedTime);
		graphics::Flush(fElapsedTime);
	}

	void MainSystem::Impl::Update(float fElapsedTime)
	{
		s_pInputDevice->Update(fElapsedTime);
		s_pSceneManager->Update(fElapsedTime);
		//s_pActorManager->Update(fElapsedTime);
		s_pModelManager->Update();
		graphics::Update(fElapsedTime);
	}

	MainSystem::MainSystem()
		: m_pImpl{ std::make_unique<Impl>() }
	{	
	}

	MainSystem::~MainSystem()
	{
	}

	bool MainSystem::Initialize(graphics::APIs emAPI, uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName)
	{
		return m_pImpl->Initialize(emAPI, nWidth, nHeight, isFullScreen, strApplicationTitle, strApplicationName);
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