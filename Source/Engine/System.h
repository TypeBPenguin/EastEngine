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
		class Device;
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
		class TerrainManager;
		class SkyManager;
	}

	class FpsChecker;
	class SceneManager;

	class MainSystem : public Singleton<MainSystem>
	{
		friend Singleton<MainSystem>;
	private:
		MainSystem();
		virtual ~MainSystem();

	public:
		bool Init(const String::StringID& strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync);

		void Run();

		bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

	public:
		float GetFPS() const;

	public:
		const String::StringID& GetApplicationName() const;
		const Math::Int2& GetScreenSize() const;
		bool IsFullScreen() const;

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}