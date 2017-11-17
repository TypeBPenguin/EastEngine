#pragma once

#include "../CommonLib/Singleton.h"

#include "../DirectX/D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Device;
		class ShaderManager;
		class RendererManager;
		class TextureManager;
		class LightManager;
		class CameraManager;
		class ModelManager;
		class MotionManager;
		class ParticleManager;
		class SOcclusionCulling;
		class VTFManager;

		typedef std::function<void()> FuncAfterRender;

		static const float s_fScreenDepth = 10000.f;
		static const float s_fScreenNear = 0.1f;

		class GraphicsSystem : public Singleton<GraphicsSystem>
		{
			friend Singleton<GraphicsSystem>;
		private:
			GraphicsSystem();
			virtual ~GraphicsSystem();

		public:
			bool Init(HWND hWnd, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync, float fFlushCycleTime);
			void Release();

			void Update(float fElapsedTime);
			void Render();

			void Flush(float fElapsedTime);

		public:
			void BeginScene(float r, float g, float b, float a);
			void EndScene();

			void AddFuncAfterRender(FuncAfterRender func) { if (func != nullptr) { m_vecFuncAfterRender.emplace_back(func); } }

		public:
			Device* GetD3D() { return s_pd3dObject; }
			ShaderManager* GetShaderMgr() { return s_pShaderMgr; }
			RendererManager* GetRendererMgr() { return s_pRendererMgr; }
			TextureManager* GetTextureMgr() { return s_pTextureMgr; }
			LightManager* GetLightMgr() { return s_pLightMgr; }
			CameraManager* GetCameraManager() { return s_pCameraManager; }
			ModelManager* GetModelMgr() { return s_pModelMgr; }
			MotionManager* GetMotionMgr() { return s_pMotionMgr; }
			ParticleManager* GetEffectMgr() { return s_pParticleMgr; }
			SOcclusionCulling* GetOcclusionCulling() { return s_pOcclusionCulling; }
			VTFManager* GetVTFMgr() { return s_pVTFMgr; }

		private:
			Device* s_pd3dObject;
			ShaderManager* s_pShaderMgr;
			RendererManager* s_pRendererMgr;
			TextureManager* s_pTextureMgr;
			LightManager* s_pLightMgr;
			CameraManager* s_pCameraManager;
			ModelManager* s_pModelMgr;
			MotionManager* s_pMotionMgr;
			ParticleManager* s_pParticleMgr;
			SOcclusionCulling* s_pOcclusionCulling;
			VTFManager* s_pVTFMgr;

			float m_fFlushTime;
			float m_fFlushCycleTime;

			bool m_isInit;

			std::vector<FuncAfterRender> m_vecFuncAfterRender;
		};
	}
}