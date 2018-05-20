#include "stdafx.h"
#include "MotionSystem.h"

namespace eastengine
{
	namespace graphics
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

					if (math::IsEqual(m_motionPlayers[i].GetBlendWeight(), 1.f))
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
			uint32_t nBoneCount = pSkeleton->GetBoneCount();
			m_vecMotionTransforms.resize(nBoneCount);

			for (uint32_t i = 0; i < nBoneCount; ++i)
			{
				ISkeleton::IBone* pBone = pSkeleton->GetBone(i);
				if (pBone != nullptr)
				{
					const math::Matrix& matDefaultMotionData = pBone->GetDefaultMotionData();

					MotionTransform& keyframe = m_vecMotionTransforms[i];

					math::Transform& motionTransform = keyframe.motionTransform;
					math::Transform& defaultTransform = keyframe.defaultTransform;

					matDefaultMotionData.Decompose(motionTransform.scale, motionTransform.rotation, motionTransform.position);
					matDefaultMotionData.Decompose(defaultTransform.scale, defaultTransform.rotation, defaultTransform.position);
				}
			}
		}

		void MotionSystem::SetIdentity(bool isMotionUpdated)
		{
			if (isMotionUpdated == true)
			{
				for (MotionTransform& keyframe : m_vecMotionTransforms)
				{
					keyframe.motionTransform = keyframe.defaultTransform;
				}
			}
			else
			{
				if (m_pSkeletonInstance->IsDirty() == true)
				{
					m_pSkeletonInstance->SetIdentity();

					for (MotionTransform& keyframe : m_vecMotionTransforms)
					{
						keyframe.motionTransform = keyframe.defaultTransform;
					}
				}
			}
		}

		void MotionSystem::BlendingLayers(const MotionPlayer& player)
		{
			if (player.IsPlaying() == false || math::IsZero(player.GetBlendWeight()) == true)
				return;

			const float fWeight = player.GetBlendWeight();
			const size_t nBoneCount = m_pSkeletonInstance->GetBoneCount();
			for (size_t i = 0; i < nBoneCount; ++i)
			{
				ISkeletonInstance::IBone* pBone =  m_pSkeletonInstance->GetBone(i);
				if (pBone == nullptr)
					continue;
				
				const math::Transform* pSourceTransform = player.GetTransform(pBone->GetName());
				if (pSourceTransform != nullptr)
				{
					MotionTransform& destKeyframe = m_vecMotionTransforms[i];
					math::Transform& motionTransform = destKeyframe.motionTransform;

					math::Vector3::Lerp(motionTransform.scale, pSourceTransform->scale, fWeight, motionTransform.scale);
					math::Quaternion::Lerp(motionTransform.rotation, pSourceTransform->rotation, fWeight, motionTransform.rotation);
					math::Vector3::Lerp(motionTransform.position, pSourceTransform->position, fWeight, motionTransform.position);
				}
			}
		}

		void MotionSystem::Binding()
		{
			const size_t nBoneCount = m_pSkeletonInstance->GetBoneCount();
			if (nBoneCount == 0)
				return;

			m_pSkeletonInstance->SetDirty();

			for (size_t i = 0; i < nBoneCount; ++i)
			{
				ISkeletonInstance::IBone* pBone = m_pSkeletonInstance->GetBone(i);
				if (pBone == nullptr)
					continue;

				const MotionTransform& keyframe = m_vecMotionTransforms[i];
				pBone->SetMotionMatrix(keyframe.motionTransform.Compose());
			}
		}
	}
}