#include "stdafx.h"
#include "GraphicsSystem.h"

#include "../DirectX/Device.h"
#include "../DirectX/ShaderMgr.h"
#include "../DirectX/TextureManager.h"
#include "../DirectX/LightMgr.h"
#include "../DirectX/CameraManager.h"
#include "../DirectX/OcclusionCulling.h"
#include "../DirectX/VTFMgr.h"

#include "../Model/ModelManager.h"
#include "../Model/MotionManager.h"

#include "../Particle/ParticleMgr.h"

#include "../Renderer/RendererManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		GraphicsSystem::GraphicsSystem()
			: s_pd3dObject(nullptr)
			, s_pShaderMgr(nullptr)
			, s_pRendererMgr(nullptr)
			, s_pTextureMgr(nullptr)
			, s_pLightMgr(nullptr)
			, s_pCameraManager(nullptr)
			, s_pModelMgr(nullptr)
			, s_pMotionMgr(nullptr)
			, s_pParticleMgr(nullptr)
			, s_pOcclusionCulling(nullptr)
			, s_pVTFMgr(nullptr)
			, m_fFlushTime(0.f)
			, m_fFlushCycleTime(60.f)
			, m_isInit(false)
		{
		}

		GraphicsSystem::~GraphicsSystem()
		{
			Release();
		}

		bool GraphicsSystem::Init(HWND hWnd, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync, float fFlushCycleTime)
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			s_pd3dObject = Device::GetInstance();
			if (s_pd3dObject->Init(hWnd, nScreenWidth, nScreenHeight, isFullScreen, isVsync) == false)
			{
				LOG_ERROR("Failed Device Initialize, s_pd3dObject");
				Release();
				return false;
			}

			s_pShaderMgr = ShaderManager::GetInstance();
			if (s_pShaderMgr->Init() == false)
			{
				LOG_ERROR("Failed ShaderManager Initialize, s_pShaderMgr");
				Release();
				return false;
			}

			s_pTextureMgr = TextureManager::GetInstance();
			if (s_pTextureMgr->Init() == false)
			{
				LOG_ERROR("Failed TextureManager Initialize, s_pTextureMgr");
				Release();
				return false;
			}

			s_pLightMgr = LightManager::GetInstance();
			if (s_pLightMgr->Init() == false)
			{
				LOG_ERROR("Failed LightManager Initialize, s_pLightMgr");
				Release();
				return false;
			}

			const Math::UInt2& n2ScreenSize = s_pd3dObject->GetScreenSize();
			s_pCameraManager = CameraManager::GetInstance();
			if (s_pCameraManager->Init(n2ScreenSize.x, n2ScreenSize.y, Math::PIDIV4, s_fScreenNear, s_fScreenDepth) == false)
			{
				LOG_ERROR("Failed CameraManager Initialize, s_pCameraManager");
				Release();
				return false;
			}

			s_pModelMgr = ModelManager::GetInstance();
			if (s_pModelMgr->Init() == false)
			{
				LOG_ERROR("Failed ModelManager Initialize, s_pModelMgr");
				Release();
				return false;
			}

			s_pMotionMgr = MotionManager::GetInstance();
			if (s_pMotionMgr->Init() == false)
			{
				LOG_ERROR("Failed MotionManager Initialize, s_pMotionMgr");
				Release();
				return false;
			}

			s_pParticleMgr = ParticleManager::GetInstance();
			if (s_pParticleMgr->Init() == false)
			{
				LOG_ERROR("Failed ParticleManager Initialize, s_pParticleMgr");
				Release();
				return false;
			}

			s_pOcclusionCulling = SOcclusionCulling::GetInstance();
			if (s_pOcclusionCulling->Init(n2ScreenSize.x, n2ScreenSize.y, s_fScreenNear) == false)
			{
				LOG_ERROR("Failed SOcclusionCulling Initialize, s_pOcclusionCulling");
				Release();
				return false;
			}

			s_pVTFMgr = VTFManager::GetInstance();
			if (s_pVTFMgr->Init() == false)
			{
				LOG_ERROR("Failed VTFManager Initialize, s_pVTFMgr");
				Release();
				return false;
			}

			s_pRendererMgr = RendererManager::GetInstance();
			if (s_pRendererMgr->Init(s_pd3dObject->GetViewport()) == false)
			{
				LOG_ERROR("Failed RendererManager Initialize, s_pRendererMgr");
				Release();
				return false;
			}

			m_fFlushTime = 0.f;

			m_fFlushCycleTime = fFlushCycleTime;

			return true;
		}

		void GraphicsSystem::Release()
		{
			if (m_isInit == false)
				return;

			SafeRelease(s_pVTFMgr);
			VTFManager::DestroyInstance();

			SafeRelease(s_pOcclusionCulling);
			SOcclusionCulling::DestroyInstance();

			SafeRelease(s_pParticleMgr);
			ParticleManager::DestroyInstance();

			SafeRelease(s_pMotionMgr);
			MotionManager::DestroyInstance();

			SafeRelease(s_pModelMgr);
			ModelManager::DestroyInstance();

			SafeRelease(s_pCameraManager);
			CameraManager::DestroyInstance();

			SafeRelease(s_pLightMgr);
			LightManager::DestroyInstance();

			SafeRelease(s_pRendererMgr);
			RendererManager::DestroyInstance();

			if (s_pd3dObject != nullptr)
			{
				s_pd3dObject->PreRelease();
			}

			SafeRelease(s_pTextureMgr);
			TextureManager::DestroyInstance();

			SafeRelease(s_pShaderMgr);
			ShaderManager::DestroyInstance();

			SafeRelease(s_pd3dObject);
			Device::DestroyInstance();

			m_isInit = false;
		}

		void GraphicsSystem::Update(float fElapsedTime)
		{
			s_pCameraManager->Update(fElapsedTime);

			s_pShaderMgr->Update();

			s_pModelMgr->Update();
			s_pTextureMgr->Update(fElapsedTime);

			s_pLightMgr->Update(fElapsedTime);

			s_pParticleMgr->Update(fElapsedTime);
		}

		void GraphicsSystem::Render()
		{
			s_pVTFMgr->Process();

			s_pRendererMgr->Render();

			for (auto& func : m_vecFuncAfterRender)
			{
				if (func != nullptr)
				{
					func();
				}
			}
		}

		void GraphicsSystem::Flush(float fElapsedTime)
		{
			s_pVTFMgr->Flush();

			s_pRendererMgr->Flush();
			s_pModelMgr->Flush();

			m_fFlushTime += fElapsedTime;
			if (m_fFlushTime < m_fFlushCycleTime)
				return;

			m_fFlushTime = 0.f;

			s_pTextureMgr->Flush();
		}

		void GraphicsSystem::BeginScene(float r, float g, float b, float a)
		{
			s_pd3dObject->BeginScene(r, g, b, a);

			s_pOcclusionCulling->ClearBuffer();
		}

		void GraphicsSystem::EndScene()
		{
			s_pd3dObject->EndScene();
		}
	}
}