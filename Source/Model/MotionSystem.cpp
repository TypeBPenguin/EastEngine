#include "stdafx.h"
#include "MotionSystem.h"

namespace eastengine
{
	namespace graphics
	{
		MotionSystem::MotionSystem()
		{
		}

		MotionSystem::MotionSystem(const MotionSystem& source)
		{
			*this = source;
		}

		MotionSystem::MotionSystem(MotionSystem&& source) noexcept
		{
			*this = std::move(source);
		}

		MotionSystem::~MotionSystem()
		{
		}

		MotionSystem& MotionSystem::operator = (const MotionSystem& source)
		{
			m_pSkeletonInstance = source.m_pSkeletonInstance;
			m_motionPlayers = source.m_motionPlayers;
			m_blendMotionPlayers = source.m_blendMotionPlayers;
			m_vecMotionTransforms = source.m_vecMotionTransforms;

			return *this;
		}

		MotionSystem& MotionSystem::operator = (MotionSystem&& source) noexcept
		{
			m_pSkeletonInstance = std::move(source.m_pSkeletonInstance);
			m_motionPlayers = std::move(source.m_motionPlayers);
			m_blendMotionPlayers = std::move(source.m_blendMotionPlayers);
			m_vecMotionTransforms = std::move(source.m_vecMotionTransforms);

			return *this;
		}

		void MotionSystem::Update(float elapsedTime)
		{
			bool isMotionUpdated = false;
			bool isEnableTransformUpdate = true;
			int lastLayerIndex = -1;

			for (int i = 0; i < MotionLayers::eLayerCount; ++i)
			{
				if (isEnableTransformUpdate == true)
				{
					lastLayerIndex = i;
				}

				if (m_motionPlayers[i].Update(elapsedTime, isEnableTransformUpdate) == true)
				{
					const float blendWeight = m_motionPlayers[i].GetBlendWeight();
					if (math::IsZero(blendWeight) == false)
					{
						m_blendMotionPlayers[i].Update(elapsedTime, isEnableTransformUpdate);
					}

					isMotionUpdated = true;
				}
			}

			SetIdentity(isMotionUpdated);

			if (isMotionUpdated == true)
			{
				for (int i = lastLayerIndex; i >= 0; --i)
				{
					BlendingLayers(m_motionPlayers[i], m_blendMotionPlayers[i]);
				}

				Binding();
			}
		}

		void MotionSystem::Play(MotionLayers emLayer, IMotion* pMotion, const MotionPlaybackInfo* pPlayback)
		{
			float blendTime = 0.f;
			if (m_motionPlayers[emLayer].IsPlaying() == true)
			{
				if (pPlayback != nullptr)
				{
					IMotion* pBlendMotion = m_motionPlayers[emLayer].GetMotion();
					const float remainingTime = pBlendMotion->GetEndTime() - m_motionPlayers[emLayer].GetPlayTime();

					blendTime = std::min(pPlayback->blendTime, remainingTime);
				}

				if (math::IsZero(blendTime) == false)
				{
					m_blendMotionPlayers[emLayer] = std::move(m_motionPlayers[emLayer]);
				}
			}
			m_motionPlayers[emLayer].Play(pMotion, pPlayback, blendTime);
		}

		void MotionSystem::Stop(MotionLayers emLayer, float stopTime)
		{
			m_motionPlayers[emLayer].Stop(stopTime);
			m_blendMotionPlayers[emLayer].Stop(stopTime);
		}

		void MotionSystem::Initialize(ISkeletonInstance* pSkeletonInstance)
		{
			m_pSkeletonInstance = pSkeletonInstance;
			m_pSkeletonInstance->SetIdentity();

			ISkeleton* pSkeleton = m_pSkeletonInstance->GetSkeleton();
			uint32_t boneCount = pSkeleton->GetBoneCount();
			m_vecMotionTransforms.resize(boneCount);

			for (uint32_t i = 0; i < boneCount; ++i)
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

		void MotionSystem::BlendingLayers(const MotionPlayer& player, const MotionPlayer& blendPlayer)
		{
			if (player.IsPlaying() == false || math::IsZero(player.GetBlendWeight()) == true)
				return;

			const float weight = player.GetBlendWeight();
			const float blendWeight = 1.f - weight;

			const bool isEnableBlend = math::IsZero(blendWeight) == false && blendPlayer.IsPlaying();

			const size_t boneCount = m_pSkeletonInstance->GetBoneCount();
			for (size_t i = 0; i < boneCount; ++i)
			{
				ISkeletonInstance::IBone* pBone =  m_pSkeletonInstance->GetBone(i);
				if (pBone == nullptr)
					continue;

				const string::StringID& boneName = pBone->GetName();

				if (isEnableBlend == true)
				{
					MotionTransform& destKeyframe = m_vecMotionTransforms[i];
					math::Transform& motionTransform = destKeyframe.motionTransform;

					const math::Transform* pSourceBlendTransform = blendPlayer.GetTransform(boneName);
					if (pSourceBlendTransform != nullptr)
					{
						math::float3::Lerp(motionTransform.scale, pSourceBlendTransform->scale, blendWeight, motionTransform.scale);
						math::Quaternion::Lerp(motionTransform.rotation, pSourceBlendTransform->rotation, blendWeight, motionTransform.rotation);
						math::float3::Lerp(motionTransform.position, pSourceBlendTransform->position, blendWeight, motionTransform.position);
					}

					const math::Transform* pSourceTransform = player.GetTransform(boneName);
					if (pSourceTransform != nullptr)
					{
						math::float3::Lerp(motionTransform.scale, pSourceTransform->scale, weight, motionTransform.scale);
						math::Quaternion::Lerp(motionTransform.rotation, pSourceTransform->rotation, weight, motionTransform.rotation);
						math::float3::Lerp(motionTransform.position, pSourceTransform->position, weight, motionTransform.position);
					}
				}
				else
				{
					const math::Transform* pSourceTransform = player.GetTransform(boneName);
					if (pSourceTransform != nullptr)
					{
						MotionTransform& destKeyframe = m_vecMotionTransforms[i];
						math::Transform& motionTransform = destKeyframe.motionTransform;

						math::float3::Lerp(motionTransform.scale, pSourceTransform->scale, weight, motionTransform.scale);
						math::Quaternion::Lerp(motionTransform.rotation, pSourceTransform->rotation, weight, motionTransform.rotation);
						math::float3::Lerp(motionTransform.position, pSourceTransform->position, weight, motionTransform.position);
					}
				}
			}
		}

		void MotionSystem::Binding()
		{
			const size_t boneCount = m_pSkeletonInstance->GetBoneCount();
			if (boneCount == 0)
				return;

			m_pSkeletonInstance->SetDirty();

			for (size_t i = 0; i < boneCount; ++i)
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