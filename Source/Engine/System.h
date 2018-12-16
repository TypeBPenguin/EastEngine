#pragma once

#include "CommonLib/Singleton.h"

#include "CommonLib/CommonLib.h"

#include "Graphics/Graphics.h"

namespace eastengine
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
			uint32_t width{ 1600 };
			uint32_t height{ 900 };
			bool isFullScreen{ false };
			string::StringID applicationTitle;
			string::StringID applicationName;
		};

	public:
		bool Initialize(const Initializer& initializer);

	public:
		void Run(IScene** ppScene, size_t nSceneCount, size_t nMainScene);

	public:
		float GetFPS() const;

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}