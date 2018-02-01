#include "stdafx.h"
#include "MotionSystem.h"

namespace EastEngine
{
	namespace Graphics
	{
		MotionSystem::MotionSystem()
			: m_fBlemdTime(0.f)
			, m_pSkeletonInstance(nullptr)
		{
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

		void MotionSystem::Initialize(ISkeletonInstance* pSkeletonInstance)
		{
			m_pSkeletonInstance = pSkeletonInstance;
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

					Math::Transform& motionTransform = keyframe.motionTransform;
					Math::Transform& defaultTransform = keyframe.defaultTransform;

					matDefaultMotionData.Decompose(motionTransform.scale, motionTransform.rotation, motionTransform.position);
					matDefaultMotionData.Decompose(defaultTransform.scale, defaultTransform.rotation, defaultTransform.position);
				}
			}
		}

		void MotionSystem::SetIdentity(bool isMotionUpdated)
		{
			if (isMotionUpdated == true)
			{
				for (auto& iter : m_umapKeyframe)
				{
					iter.second.motionTransform = iter.second.defaultTransform;
				}
			}
			else
			{
				if (m_pSkeletonInstance->IsDirty() == true)
				{
					m_pSkeletonInstance->SetIdentity();

					for (auto& iter : m_umapKeyframe)
					{
						iter.second.motionTransform = iter.second.defaultTransform;
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
				
				const Math::Transform* pSourceTransform = player.GetTransform(pBone->GetName());
				if (pSourceTransform != nullptr)
				{
					KeyframeTemp& destKeyframe = m_umapKeyframe[pBone->GetName()];
					Math::Transform& motionTransform = destKeyframe.motionTransform;

					Math::Vector3::Lerp(motionTransform.scale, pSourceTransform->scale, fWeight, motionTransform.scale);
					Math::Quaternion::Lerp(motionTransform.rotation, pSourceTransform->rotation, fWeight, motionTransform.rotation);
					Math::Vector3::Lerp(motionTransform.position, pSourceTransform->position, fWeight, motionTransform.position);
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
				pBone->SetMotionMatrix(keyframe.motionTransform.Compose());
			}
		}
	}
}