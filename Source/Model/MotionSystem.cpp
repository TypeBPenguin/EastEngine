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
			Initialize();
		}

		MotionSystem::~MotionSystem()
		{
			m_pSkeletonInstance = nullptr;
		}

		void MotionSystem::Update(float fElapsedTime)
		{
			bool isMotionUpdated = false;
			bool isEnableTransformUpdate = true;
			int nLastLayerIndex = -1;

			for (int i = 0; i < EmMotion::eLayerCount; ++i)
			{
				if (isEnableTransformUpdate == true)
				{
					nLastLayerIndex = i;
				}

				if (m_motionPlayers[i].Update(fElapsedTime, isEnableTransformUpdate) == true)
				{
					isMotionUpdated = true;

					if (Math::IsEqual(m_motionPlayers[i].GetBlendWeight(), 1.f))
					{
						isEnableTransformUpdate = false;
					}
				}
			}

			SetIdentity(isMotionUpdated);

			if (isMotionUpdated == true)
			{
				for (int i = nLastLayerIndex; i >= 0; --i)
				{
					BlendingLayers(m_motionPlayers[i]);
				}

				Binding();
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

		void MotionSystem::Initialize()
		{
			m_pSkeletonInstance->SetIdentity();

			ISkeleton* pSkeleton = m_pSkeletonInstance->GetSkeleton();
			size_t nBoneCount = pSkeleton->GetBoneCount();
			m_umapKeyframe.reserve(nBoneCount);

			for (size_t i = 0; i < nBoneCount; ++i)
			{
				ISkeleton::IBone* pBone = pSkeleton->GetBone(i);
				if (pBone != nullptr)
				{
					const Math::Matrix& matDefaultMotionData = pBone->GetDefaultMotionData();

					KeyframeTemp& keyframe = m_umapKeyframe[pBone->GetName()];

					IMotion::Keyframe& defaultKeyframe = keyframe.defaultKeyframe;
					IMotion::Keyframe& motionKeyframe = keyframe.motionKeyframe;

					matDefaultMotionData.Decompose(defaultKeyframe.f3Scale, defaultKeyframe.quatRotation, defaultKeyframe.f3Pos);
					matDefaultMotionData.Decompose(motionKeyframe.f3Scale, motionKeyframe.quatRotation, motionKeyframe.f3Pos);
				}
			}
		}

		void MotionSystem::SetIdentity(bool isMotionUpdated)
		{
			if (isMotionUpdated == true)
			{
				for (auto& iter : m_umapKeyframe)
				{
					iter.second.motionKeyframe = iter.second.defaultKeyframe;
				}
			}
			else
			{
				if (m_pSkeletonInstance->IsDirty() == true)
				{
					m_pSkeletonInstance->SetIdentity();

					for (auto& iter : m_umapKeyframe)
					{
						iter.second.motionKeyframe = iter.second.defaultKeyframe;
					}
				}
			}
		}

		void MotionSystem::BlendingLayers(const MotionPlayer& player)
		{
			if (player.IsPlaying() == false || Math::IsZero(player.GetBlendWeight()) == true)
				return;

			float fWeight = player.GetBlendWeight();
			size_t nBoneCount = m_pSkeletonInstance->GetBoneCount();
			for (size_t i = 0; i < nBoneCount; ++i)
			{
				ISkeletonInstance::IBone* pBone =  m_pSkeletonInstance->GetBone(i);
				if (pBone == nullptr)
					continue;
				
				const IMotion::Keyframe* pSrcKeyframe = player.GetKeyframe(pBone->GetName());
				if (pSrcKeyframe != nullptr)
				{
					KeyframeTemp& destKeyframe = m_umapKeyframe[pBone->GetName()];
					IMotion::Keyframe& motionKeyframe = destKeyframe.motionKeyframe;

					Math::Vector3::Lerp(motionKeyframe.f3Pos, pSrcKeyframe->f3Pos, fWeight, motionKeyframe.f3Pos);
					Math::Vector3::Lerp(motionKeyframe.f3Scale, pSrcKeyframe->f3Scale, fWeight, motionKeyframe.f3Scale);
					Math::Quaternion::Lerp(motionKeyframe.quatRotation, pSrcKeyframe->quatRotation, fWeight, motionKeyframe.quatRotation);
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

				const KeyframeTemp& keyframe = m_umapKeyframe[pBone->GetName()];
				const IMotion::Keyframe& motionKeyframe = keyframe.motionKeyframe;

				Math::Matrix matMotion;
				Math::Matrix::Compose(motionKeyframe.f3Scale, motionKeyframe.quatRotation, motionKeyframe.f3Pos, matMotion);

				pBone->SetMotionData(matMotion);
			}
		}
	}
}