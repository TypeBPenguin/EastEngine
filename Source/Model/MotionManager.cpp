#include "stdafx.h"
#include "MotionManager.h"

#include "ModelInterface.h"
#include "CommonLib/FileStream.h"

namespace EastEngine
{
	namespace Graphics
	{
		MotionManager::MotionManager()
			: m_isInit(false)
		{
		}

		MotionManager::~MotionManager()
		{
			Release();
		}

		bool MotionManager::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			return true;
		}

		void MotionManager::Release()
		{
			if (m_isInit == false)
				return;

			for (auto& iter : m_umapMotion)
			{
				IMotion* pMotion = iter.second;
				IMotion::Destroy(&pMotion);
			}
			m_umapMotion.clear();

			m_isInit = false;
		}

		void MotionManager::Update()
		{
		}

		void MotionManager::Flush()
		{
			auto iter = m_umapMotion.begin();
			while (iter != m_umapMotion.end())
			{
				IMotion* pMotion = iter->second;

				if (pMotion->GetLoadState() == EmLoadState::eReady ||
					pMotion->GetLoadState() == EmLoadState::eLoading)
				{
					++iter;
					continue;
				}

				if (pMotion->GetReferenceCount() > 0)
				{
					pMotion->SetAlive(true);
					++iter;
					continue;
				}

				if (pMotion->IsAlive() == false)
				{
					IMotion::Destroy(&pMotion);
					iter = m_umapMotion.erase(iter);
					continue;
				}

				pMotion->SubtractLife();
				++iter;
			}
		}

		void MotionManager::ProcessRequestMotionLoader()
		{
		}

		bool MotionManager::AddMotion(const String::StringID& strMotionName, IMotion* pModel)
		{
			if (pModel == nullptr)
				return false;

			auto iter = m_umapMotion.find(strMotionName);
			if (iter != m_umapMotion.end())
				return false;

			m_umapMotion.emplace(strMotionName, pModel);

			return true;
		}

		IMotion* MotionManager::GetMotion(const String::StringID& strName)
		{
			auto iter = m_umapMotion.find(strName);
			if (iter != m_umapMotion.end())
				return iter->second;

			return nullptr;
		}
	}
}