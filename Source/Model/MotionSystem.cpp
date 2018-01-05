#include "stdafx.h"
#include "MotionSystem.h"

namespace EastEngine
{
	namespace Graphics
	{
		MotionSystem::MotionSystem(ISkeletonInstance* pSkeletonInstance)
			: m_fBlemdTime(0.f)
			, m_pSkeletonInstance(pSkeletonInstance)
		{
			ISkeleton* pSkeleton = m_pSkeletonInstance->GetSkeleton();
			size_t nBoneCount = pSkeleton->GetBoneCount();
			m_umapKeyframe.reserve(nBoneCount);

			for (size_t i = 0; i < nBoneCount; ++i)
			{
				ISkeleton::IBone* pBone = pSkeleton->GetBone(i);
				if (pBone != nullptr)
				{
					IMotion::Keyframe& keyframe = m_umapKeyframe[pBone->GetName()];

					const Math::Matrix& matDefaultMotionData = pBone->GetDefaultMotionData();
					matDefaultMotionData.Decompose(keyframe.f3Scale, keyframe.quatRotation, keyframe.f3Pos);
				}
			}
		}

		MotionSystem::~MotionSystem()
		{
			m_pSkeletonInstance = nullptr;
		}

		void MotionSystem::Update(float fElapsedTime)
		{
			bool isMotionUpdated = false;
			int nLastLayerIndex = -1;

			for (int i = 0; i < EmMotion::eLayerCount; ++i)
			{
				nLastLayerIndex = i;

				if (m_motionPlayers[i].Update(fElapsedTime) == true)
				{
					isMotionUpdated = true;

					if (Math::IsEqual(m_motionPlayers[i].GetWeight(), 1.f))
						break;
				}
			}

			if (isMotionUpdated == true)
			{
				for (int i = nLastLayerIndex; i >= 0; --i)
				{
					BlendingLayers(m_motionPlayers[i]);
				}

				Binding();
			}
			else
			{
				m_pSkeletonInstance->SetIdentity();
			}
		}

		void MotionSystem::Play(EmMotion::Layers emLayer, IMotion* pMotion, const MotionPlaybackInfo* pPlayback)
		{
			m_motionPlayers[emLayer].Play(pMotion, pPlayback);
		}

		void MotionSystem::Stop(EmMotion::Layers emLayer, float fStopTime)
		{
			m_motionPlayers[emLayer].Stop(fStopTime);
		}

		void MotionSystem::BlendingLayers(const MotionPlayer& player)
		{
			if (player.IsPlaying() == false || Math::IsZero(player.GetWeight()) == true)
				return;

			float fWeight = player.GetWeight();
			size_t nBoneCount = m_pSkeletonInstance->GetBoneCount();
			for (size_t i = 0; i < nBoneCount; ++i)
			{
				ISkeletonInstance::IBone* pBone =  m_pSkeletonInstance->GetBone(i);
				if (pBone == nullptr)
					continue;
				
				const IMotion::Keyframe* pSrcKeyframe = player.GetKeyframe(pBone->GetName());
				if (pSrcKeyframe != nullptr)
				{
					IMotion::Keyframe& destKeyframe = m_umapKeyframe[pBone->GetName()];
					Math::Vector3::Lerp(destKeyframe.f3Pos, pSrcKeyframe->f3Pos, fWeight, destKeyframe.f3Pos);
					Math::Vector3::Lerp(destKeyframe.f3Scale, pSrcKeyframe->f3Scale, fWeight, destKeyframe.f3Scale);
					Math::Quaternion::Lerp(destKeyframe.quatRotation, pSrcKeyframe->quatRotation, fWeight, destKeyframe.quatRotation);
				}
			}
		}

		void MotionSystem::Binding()
		{
			size_t nBoneCount = m_pSkeletonInstance->GetBoneCount();
			if (nBoneCount == 0)
				return;

			m_pSkeletonInstance->SetDirty();

			for (size_t i = 0; i < nBoneCount; ++i)
			{
				ISkeletonInstance::IBone* pBone = m_pSkeletonInstance->GetBone(i);
				if (pBone == nullptr)
					continue;

				const IMotion::Keyframe& keyframe = m_umapKeyframe[pBone->GetName()];

				Math::Matrix matMotion;
				Math::Matrix::Compose(keyframe.f3Scale, keyframe.quatRotation, keyframe.f3Pos, matMotion);

				pBone->SetMotionData(matMotion);
			}
		}
	}
}