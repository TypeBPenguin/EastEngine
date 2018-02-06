#include "stdafx.h"
#include "GraphicsSystem.h"

#include "CommonLib/Performance.h"

#include "DirectX/Device.h"
#include "DirectX/ShaderMgr.h"
#include "DirectX/TextureManager.h"
#include "DirectX/LightMgr.h"
#include "DirectX/Camera.h"
#include "DirectX/OcclusionCulling.h"
#include "DirectX/VTFManager.h"

#include "Model/ModelManager.h"
#include "Model/MotionManager.h"

#include "Particle/ParticleMgr.h"

#include "Renderer/RendererManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		class GraphicsSystem::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			bool Initialize(HWND hWnd, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync, float fFlushCycleTime);

		public:
			bool HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			void Update(float fElapsedTime);
			void Render();

			void Synchronize();
			void Flush(float fElapsedTime);

		public:
			void BeginScene(float r, float g, float b, float a);
			void EndScene();

			void AddFuncAfterRender(FuncAfterRender func);

		private:
			bool m_isInitialized{ false };

			Device* s_pDevice{ nullptr };
			ShaderManager* s_pShaderMgr{ nullptr };
			RendererManager* s_pRendererMgr{ nullptr };
			TextureManager* s_pTextureMgr{ nullptr };
			LightManager* s_pLightMgr{ nullptr };
			Camera* s_pCamera{ nullptr };
			ModelManager* s_pModelMgr{ nullptr };
			MotionManager* s_pMotionMgr{ nullptr };
			ParticleManager* s_pParticleMgr{ nullptr };
			OcclusionCulling* s_pOcclusionCulling{ nullptr };
			VTFManager* s_pVTFManager{ nullptr };

			float m_fFlushTime{ 0.f };
			float m_fFlushCycleTime{ 30.f };

			std::vector<FuncAfterRender> m_vecFuncAfterRender;
		};

		GraphicsSystem::Impl::Impl()
		{
		}

		GraphicsSystem::Impl::~Impl()
		{
			SafeRelease(s_pVTFManager);
			VTFManager::DestroyInstance();

			OcclusionCulling::DestroyInstance();
			s_pOcclusionCulling = nullptr;

			SafeRelease(s_pParticleMgr);
			ParticleManager::DestroyInstance();

			ModelManager::DestroyInstance();
			s_pModelMgr = nullptr;

			MotionManager::DestroyInstance();
			s_pMotionMgr = nullptr;

			Camera::DestroyInstance();
			s_pCamera = nullptr;

			SafeRelease(s_pLightMgr);
			LightManager::DestroyInstance();

			SafeRelease(s_pRendererMgr);
			RendererManager::DestroyInstance();

			if (s_pDevice != nullptr)
			{
				s_pDevice->PreRelease();
			}

			TextureManager::DestroyInstance();
			s_pTextureMgr = nullptr;

			SafeRelease(s_pShaderMgr);
			ShaderManager::DestroyInstance();

			SafeRelease(s_pDevice);
			Device::DestroyInstance();
		}

		bool GraphicsSystem::Impl::Initialize(HWND hWnd, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync, float fFlushCycleTime)
		{
			if (m_isInitialized == true)
				return true;

			s_pDevice = Device::GetInstance();
			if (s_pDevice->Initialize(hWnd, nScreenWidth, nScreenHeight, isFullScreen, isVsync) == false)
			{
				LOG_ERROR("Failed Device Initialize, s_pDevice");
				return false;
			}

			s_pShaderMgr = ShaderManager::GetInstance();
			if (s_pShaderMgr->Init() == false)
			{
				LOG_ERROR("Failed ShaderManager Initialize, s_pShaderMgr");
				return false;
			}

			s_pTextureMgr = TextureManager::GetInstance();

			s_pLightMgr = LightManager::GetInstance();
			if (s_pLightMgr->Init() == false)
			{
				LOG_ERROR("Failed LightManager Initialize, s_pLightMgr");
				return false;
			}

			const Math::UInt2& n2ScreenSize = s_pDevice->GetScreenSize();
			s_pCamera = Camera::GetInstance();
			s_pCamera->SetProjection(n2ScreenSize.x, n2ScreenSize.y, Math::PIDIV4, s_fScreenNear, s_fScreenDepth);

			s_pModelMgr = ModelManager::GetInstance();
			s_pMotionMgr = MotionManager::GetInstance();

			s_pParticleMgr = ParticleManager::GetInstance();
			if (s_pParticleMgr->Init() == false)
			{
				LOG_ERROR("Failed ParticleManager Initialize, s_pParticleMgr");
				return false;
			}

			//s_pOcclusionCulling = OcclusionCulling::GetInstance();
			//s_pOcclusionCulling->Initialize(n2ScreenSize.x, n2ScreenSize.y, s_fScreenNear);

			s_pVTFManager = VTFManager::GetInstance();
			if (s_pVTFManager->Initialize() == false)
			{
				LOG_ERROR("Failed VTFManager Initialize, s_pVTFManager");
				return false;
			}

			s_pRendererMgr = RendererManager::GetInstance();
			if (s_pRendererMgr->Init(s_pDevice->GetViewport()) == false)
			{
				LOG_ERROR("Failed RendererManager Initialize, s_pRendererMgr");
				return false;
			}

			m_fFlushTime = 0.f;

			m_fFlushCycleTime = fFlushCycleTime;

			m_isInitialized = true;

			return true;
		}

		bool GraphicsSystem::Impl::HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			return s_pDevice->HandleMessage(hWnd, nMsg, wParam, lParam);
		}

		void GraphicsSystem::Impl::Update(float fElapsedTime)
		{
			s_pCamera->Update(fElapsedTime);

			s_pModelMgr->Update();

			s_pLightMgr->Update(fElapsedTime);

			s_pParticleMgr->Update(fElapsedTime);
		}

		void GraphicsSystem::Impl::Render()
		{
			s_pRendererMgr->Render();

			for (auto& func : m_vecFuncAfterRender)
			{
				if (func != nullptr)
				{
					func();
				}
			}
		}

		void GraphicsSystem::Impl::Synchronize()
		{
			s_pLightMgr->Synchronize();
			s_pVTFManager->Synchronize();
		}

		void GraphicsSystem::Impl::Flush(float fElapsedTime)
		{
			bool isEnableGarbageCollector = false;
			m_fFlushTime += fElapsedTime;
			if (m_fFlushTime >= m_fFlushCycleTime)
			{
				isEnableGarbageCollector = true;
				m_fFlushTime -= m_fFlushCycleTime;
			}

			s_pTextureMgr->Flush(isEnableGarbageCollector);
			s_pModelMgr->Flush(isEnableGarbageCollector);
			s_pTextureMgr->Flush(isEnableGarbageCollector);

			s_pShaderMgr->Flush();

			s_pRendererMgr->Flush();

			s_pDevice->Flush();
		}

		void GraphicsSystem::Impl::BeginScene(float r, float g, float b, float a)
		{
			s_pDevice->BeginScene(r, g, b, a);

			//s_pOcclusionCulling->ClearBuffer();
		}

		void GraphicsSystem::Impl::EndScene()
		{
			s_pDevice->EndScene();
		}

		void GraphicsSystem::Impl::AddFuncAfterRender(FuncAfterRender func)
		{
			if (func == nullptr)
				return;

			m_vecFuncAfterRender.emplace_back(func);
		}

		GraphicsSystem::GraphicsSystem()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		GraphicsSystem::~GraphicsSystem()
		{
		}

		bool GraphicsSystem::Initialize(HWND hWnd, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync, float fFlushCycleTime)
		{
			return m_pImpl->Initialize(hWnd, nScreenWidth, nScreenHeight, isFullScreen, isVsync, fFlushCycleTime);
		}

		bool GraphicsSystem::HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			return m_pImpl->HandleMessage(hWnd, nMsg, wParam, lParam);
		}

		void GraphicsSystem::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}

		void GraphicsSystem::Render()
		{
			m_pImpl->Render();
		}

		void GraphicsSystem::Synchronize()
		{
			m_pImpl->Synchronize();
		}

		void GraphicsSystem::Flush(float fElapsedTime)
		{
			m_pImpl->Flush(fElapsedTime);
		}

		void GraphicsSystem::BeginScene(float r, float g, float b, float a)
		{
			m_pImpl->BeginScene(r, g, b, a);
		}

		void GraphicsSystem::EndScene()
		{
			m_pImpl->EndScene();
		}

		void GraphicsSystem::AddFuncAfterRender(FuncAfterRender func)
		{
			m_pImpl->AddFuncAfterRender(func);
		}
	}
}