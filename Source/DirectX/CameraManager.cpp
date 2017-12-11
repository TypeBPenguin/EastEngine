#include "stdafx.h"
#include "Camera.h"
#include "CameraManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		CameraManager::CameraManager()
			: m_nScreenWidth(0)
			, m_nScreenHeight(0)
			, m_fFov(0.f)
			, m_fNear(0.f)
			, m_fFar(0.f)
			, m_pMainCamera(nullptr)
			, m_isInit(false)
		{
		}

		CameraManager::~CameraManager()
		{
			Release();
		}

		bool CameraManager::Init(uint32_t nScreenWidth, uint32_t nScreenHeight, float fFov, float fNear, float fFar)
		{
			if (m_isInit == true)
				return true;

			m_nScreenWidth = nScreenWidth;
			m_nScreenHeight = nScreenHeight;

			m_fFov = fFov;
			m_fNear = fNear;
			m_fFar = fFar;

			m_isInit = true;

			return true;
		}

		void CameraManager::Release()
		{
			if (m_isInit == false)
				return;

			m_pMainCamera = nullptr;

			std::for_each(m_umapCamera.begin(), m_umapCamera.end(), DeleteSTLMapObject());
			m_umapCamera.clear();

			m_isInit = false;
		}

		void CameraManager::Update(float fElapsedTime)
		{
			for (auto iter = m_umapCamera.begin(); iter != m_umapCamera.end(); ++iter)
			{
				iter->second->Update(fElapsedTime);
			}
		}

		Camera* CameraManager::CreateCamera(const String::StringID& strName, const Math::Vector3& f3Pos, const Math::Vector3& f3Lookat, const Math::Vector3& vUp)
		{
			return CreateCamera(strName, f3Pos, f3Lookat, vUp, m_nScreenWidth, m_nScreenHeight, m_fFov, m_fNear, m_fFar);
		}

		Camera* CameraManager::CreateCamera(const String::StringID& strName, const Math::Vector3& f3Pos, const Math::Vector3& f3Lookat, const Math::Vector3& vUp, uint32_t nScreenWidth, uint32_t nScreenHeight, float fFov, float fNear, float fFar)
		{
			auto iter = m_umapCamera.find(strName);
			if (iter != m_umapCamera.end())
				return iter->second;

			Camera* pCamera = new Camera;
			pCamera->InitView(f3Pos, f3Lookat, vUp);
			pCamera->InitProjection(nScreenWidth, nScreenHeight, fFov, fNear, fFar);
			pCamera->UpdateOrtho();
			pCamera->UpdateFrustum();

			m_umapCamera.emplace(strName, pCamera);

			if (m_umapCamera.size() == 1)
			{
				m_pMainCamera = pCamera;
			}

			return pCamera;
		}

		void CameraManager::Remove(const String::StringID& strName)
		{
			auto iter = m_umapCamera.find(strName);
			if (iter == m_umapCamera.end())
				return;

			if (iter->second == m_pMainCamera)
			{
				SafeDelete(iter->second);
				m_umapCamera.erase(iter);
				if (m_umapCamera.empty() == false)
				{
					m_pMainCamera = m_umapCamera.begin()->second;
				}
				else
				{
					m_pMainCamera = nullptr;
				}
			}
			else
			{
				SafeDelete(iter->second);
				m_umapCamera.erase(iter);
			}
		}

		void CameraManager::RemoveAll()
		{
			m_pMainCamera = nullptr;

			for (auto& iter : m_umapCamera)
			{
				SafeDelete(iter.second);
			}
			m_umapCamera.clear();
		}

		Camera* CameraManager::GetCamera(const String::StringID& strName)
		{
			auto iter = m_umapCamera.find(strName);
			if (iter != m_umapCamera.end())
				return iter->second;

			return nullptr;
		}

		Collision::EmContainment::Type CameraManager::IsFrustumContains(const Collision::AABB& box)
		{
			if (m_pMainCamera == nullptr)
				return Collision::EmContainment::eDisjoint;

			return m_pMainCamera->IsFrustumContains(box);
		}

		Collision::EmContainment::Type CameraManager::IsFrustumContains(const Collision::OBB& box)
		{
			if (m_pMainCamera == nullptr)
				return Collision::EmContainment::eDisjoint;

			return m_pMainCamera->IsFrustumContains(box);
		}

		Collision::EmContainment::Type CameraManager::IsFrustumContains(const Collision::Sphere& sphere)
		{
			if (m_pMainCamera == nullptr)
				return Collision::EmContainment::eDisjoint;

			return m_pMainCamera->IsFrustumContains(sphere);
		}

		Collision::EmContainment::Type CameraManager::IsFrustumContains(const Math::Vector3& f3Pos)
		{
			if (m_pMainCamera == nullptr)
				return Collision::EmContainment::eDisjoint;

			return m_pMainCamera->IsFrustumContains(f3Pos);
		}
	}
}