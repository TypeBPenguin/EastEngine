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
		bool Initialize(graphics::APIs emAPI, uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName);

	public:
		void Run(IScene** ppScene, size_t nSceneCount, size_t nMainScene);

	public:
		float GetFPS() const;

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}