#pragma once

#include "CommonLib/Singleton.h"
#include "CommonLib/CommonLib.h"

#include "Graphics/Implement/Graphics.h"

#include "EngineUtil.h"

namespace sid
{
	RegisterStringID(EastEngine);
}

namespace est
{
	class IScene;

	class MainSystem : public Singleton<MainSystem>
	{
		friend Singleton<MainSystem>;
	private:
		MainSystem();
		virtual ~MainSystem();

	public:
		struct Initializer
		{
			graphics::APIs emAPI{ graphics::APIs::eDX11 };
			uint32_t width{ 0 };
			uint32_t height{ 0 };
			bool isFullScreen{ false };
			bool isVSync{ false };
			string::StringID applicationTitle{ sid::EastEngine };
			string::StringID applicationName{ sid::EastEngine };

			double limitElapsedTime{ std::numeric_limits<double>::max() };
		};
		bool Initialize(const Initializer& initializer);

	public:
		void Run(std::vector<std::unique_ptr<IScene>>&& pScenes, const string::StringID& startSceneName);
		void Exit();

	public:
		float GetFPS() const;

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}