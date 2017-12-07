#pragma once

#include "CommonLib/Singleton.h"

#include "Camera.h"

namespace EastEngine
{
	namespace Graphics
	{
		class CameraManager : public Singleton<CameraManager>
		{
			friend Singleton<CameraManager>;
		private:
			CameraManager();
			virtual ~CameraManager();

		public:
			bool Init(uint32_t nScreenWidth, uint32_t nScreenHeight, float fFov, float fNear, float fFar);
			void Release();

			void Update(float fElapsedTime);

			Camera* CreateCamera(const String::StringID& strName, const Math::Vector3& f3Pos, const Math::Vector3& f3Lookat, const Math::Vector3& vUp);
			Camera* CreateCamera(const String::StringID& strName, const Math::Vector3& f3Pos, const Math::Vector3& f3Lookat, const Math::Vector3& vUp, uint32_t nScreenWidth, uint32_t nScreenHeight, float fFov, float fNear, float fFar);
			void Remove(const String::StringID& strName);
			void RemoveAll();
			Camera* GetCamera(const String::StringID& strName);

		public:
			Camera* GetMainCamera() { return m_pMainCamera; }
			void SetMainCamera(const String::StringID& strName)
			{
				auto iter = m_umapCamera.find(strName.Key());
				if (iter != m_umapCamera.end())
				{
					m_pMainCamera = iter->second;
				}
			}
			void SetMainCamera(Camera* pCamera) { m_pMainCamera = pCamera; }

		public:
			Collision::EmContainment::Type IsFrustumContains(const Collision::AABB& box);
			Collision::EmContainment::Type IsFrustumContains(const Collision::OBB& box);
			Collision::EmContainment::Type IsFrustumContains(const Collision::Sphere& sphere);
			Collision::EmContainment::Type IsFrustumContains(const Math::Vector3& f3Pos);

		public:
			float GetFov() { return m_fFov; }
			float GetNear() { return m_fNear; }
			float GetFar() { return m_fFar; }

		private:
			uint32_t m_nScreenWidth;
			uint32_t m_nScreenHeight;
			float m_fFov;
			float m_fNear;
			float m_fFar;

			Camera* m_pMainCamera;

			std::unordered_map<String::StringID, Camera*> m_umapCamera;

			bool m_isInit;
		};
	}
}