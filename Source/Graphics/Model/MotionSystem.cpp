#include "stdafx.h"
#include "MotionSystem.h"

namespace est
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
			m_motionTransforms = source.m_motionTransforms;
			m_isDirtyMotionTransform = source.m_isDirtyMotionTransform;

			return *this;
		}

		MotionSystem& MotionSystem::operator = (MotionSystem&& source) noexcept
		{
			m_pSkeletonInstance = std::move(source.m_pSkeletonInstance);
			m_motionPlayers = std::move(source.m_motionPlayers);
			m_blendMotionPlayers = std::move(source.m_blendMotionPlayers);
			m_motionTransforms = std::move(source.m_motionTransforms);
			m_isDirtyMotionTransform = std::move(source.m_isDirtyMotionTransform);

			return *this;
		}

		void MotionSystem::Update(float elapsedTime)
		{
			bool isMotionUpdated = false;

			for (int i = 0; i < MotionLayers::eLayerCount; ++i)
			{
				if (m_motionPlayers[i].Update(elapsedTime) == true)
				{
					const float blendWeight = m_motionPlayers[i].GetBlendWeight();
					if (math::IsZero(blendWeight) == false)
					{
						m_blendMotionPlayers[i].Update(elapsedTime);
					}
					else
					{
						m_blendMotionPlayers[i].Stop(0.f);
					}

					isMotionUpdated = true;
				}
			}

			SetIdentity(isMotionUpdated);

			if (isMotionUpdated == true)
			{
				for (int i = 0; i < MotionLayers::eLayerCount; ++i)
				{
					BlendingLayers(m_motionPlayers[i], m_blendMotionPlayers[i]);
				}

				Binding();
			}
		}

		void MotionSystem::Play(MotionLayers emLayer, const MotionPtr& pMotion, const MotionPlaybackInfo* pPlayback)
		{
			float blendTime = 0.f;
			if (m_motionPlayers[emLayer].IsPlaying() == true)
			{
				if (pPlayback != nullptr)
				{
					const MotionPtr pBlendMotion = m_motionPlayers[emLayer].GetMotion();
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
			const uint32_t boneCount = pSkeleton->GetBoneCount();
			m_motionTransforms.resize(boneCount);
			m_isDirtyMotionTransform = true;

			for (uint32_t i = 0; i < boneCount; ++i)
			{
				ISkeleton::IBone* pBone = pSkeleton->GetBone(i);
				if (pBone != nullptr)
				{
					const math::Matrix& matDefaultMotionData = pBone->GetDefaultMotionData();

					MotionTransform& keyframe = m_motionTransforms[i];

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
				if (m_isDirtyMotionTransform == true)
				{
					for (MotionTransform& keyframe : m_motionTransforms)
					{
						keyframe.motionTransform = keyframe.defaultTransform;
					}
					m_isDirtyMotionTransform = false;
				}
			}
			else
			{
				if (m_pSkeletonInstance->IsDirty() == true)
				{
					m_pSkeletonInstance->SetIdentity();

					if (m_isDirtyMotionTransform == true)
					{
						for (MotionTransform& keyframe : m_motionTransforms)
						{
							keyframe.motionTransform = keyframe.defaultTransform;
						}
						m_isDirtyMotionTransform = false;
					}
				}
			}
		}

		void MotionSystem::BlendingLayers(const MotionPlayer& player, const MotionPlayer& blendPlayer)
		{
			if (player.IsPlaying() == false || math::IsZero(player.GetBlendWeight()) == true)
				return;

			const size_t boneCount = m_pSkeletonInstance->GetBoneCount();
			if (boneCount == 0)
				return;

			m_isDirtyMotionTransform = true;

			const float weight = player.GetBlendWeight();
			const float blendWeight = 1.f - weight;

			const bool isEnableBlend = math::IsZero(blendWeight) == false && blendPlayer.IsPlaying();

			for (size_t i = 0; i < boneCount; ++i)
			{
				ISkeletonInstance::IBone* pBone =  m_pSkeletonInstance->GetBone(i);
				if (pBone == nullptr)
					continue;

				const string::StringID& boneName = pBone->GetName();

				if (isEnableBlend == true)
				{
					MotionTransform& destKeyframe = m_motionTransforms[i];
					math::Transform& motionTransform = destKeyframe.motionTransform;

					const math::Transform* pSourceBlendTransform = blendPlayer.GetTransform(boneName);
					if (pSourceBlendTransform != nullptr)
					{
						math::Transform::Lerp(motionTransform, *pSourceBlendTransform, weight, motionTransform);
					}

					const math::Transform* pSourceTransform = player.GetTransform(boneName);
					if (pSourceTransform != nullptr)
					{
						math::Transform::Lerp(motionTransform, *pSourceTransform, weight, motionTransform);
					}
				}
				else
				{
					const math::Transform* pSourceTransform = player.GetTransform(boneName);
					if (pSourceTransform != nullptr)
					{
						MotionTransform& destKeyframe = m_motionTransforms[i];
						math::Transform& motionTransform = destKeyframe.motionTransform;

						math::Transform::Lerp(motionTransform, *pSourceTransform, weight, motionTransform);
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

				const MotionTransform& keyframe = m_motionTransforms[i];
				pBone->SetMotionMatrix(keyframe.motionTransform.Compose());
			}
		}
	}
}