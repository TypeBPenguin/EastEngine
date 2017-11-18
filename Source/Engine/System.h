#pragma once

#include "CommonLib/Singleton.h"

#include "CommonLib/CommonLib.h"

#include "Windows/Windows.h"
#include "Graphics/GraphicsSystem.h"

namespace EastEngine
{
	class Timer;

	namespace Config
	{
		class SCommandLine;
	}

	namespace File
	{
		class DirectoryMonitor;
	}

	namespace Lua
	{
		class LuaSystem;
	}

	namespace Windows
	{
		class WindowsManager;
	}

	namespace Input
	{
		class InputDevice;
	}

	namespace Physics
	{
		class PhysicsSystem;
	}

	namespace Sound
	{
		class SoundSystem;
	}

	namespace UI
	{
		class UIManager;
	}

	namespace GameObject
	{
		class ActorManager;
	}

	class FpsChecker;
	class SSceneMgr;

	class MainSystem : public Singleton<MainSystem>
	{
		friend Singleton<MainSystem>;
	private:
		MainSystem();
		virtual ~MainSystem();

	public:
		bool Init(const String::StringID& strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync);
		void Release();

		void Run();

		bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

	public:
		float GetFPS();

	public:
		const String::StringID& GetApplicationName() { return m_strApplicationName; }
		const Math::Int2& GetScreenSize() { return m_n2ScreenSize; }
		bool IsFullScreen() { return m_isFullScreen; }

	private:
		void flush(float fElapsedTime);
		void update(float fElapsedTime);
		void render();

	private:
		void processPipeMessage();

	private:
		FpsChecker* m_pFpsChecker;
		SSceneMgr* s_pSceneMgr;

		//SPipeStream* s_pPipeStream;

		Config::SCommandLine* s_pCommandLine;
		File::DirectoryMonitor* s_pDirectoryMonitor;
		Timer* s_pTimer;
		Lua::LuaSystem* s_pLuaSystem;
		Windows::WindowsManager* s_pWindows;
		Graphics::GraphicsSystem* s_pGraphicsSystem;
		Input::InputDevice* s_pInputDevice;
		Physics::PhysicsSystem* s_pPhysicsSystem;
		Sound::SoundSystem* s_pSoundSystem;
		UI::UIManager* s_pUIMgr;
		GameObject::ActorManager* s_pActorMgr;

		String::StringID m_strApplicationName;
		Math::Int2 m_n2ScreenSize;
		bool m_isFullScreen;
		bool m_isVsync;
		bool m_isInit;
	};
}