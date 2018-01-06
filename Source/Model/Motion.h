#pragma once

#include "CommonLib/plf_colony.h"

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Motion : public IMotion
		{
		public:
			class Bone : public IMotion::IBone
			{
			public:
				Bone(const String::StringID& strBoneName, const std::vector<Keyframe>& _vecKeyframes, float fFrameInterval);
				virtual ~Bone() override;

			public:
				virtual const String::StringID& GetName() const override { return m_strBoneName; }

				virtual float GetStartTime() const override
				{
					if (m_vecKeyframes.empty())
						return 0.f;

					return m_vecKeyframes.front().fTime;
				}

				virtual float GetEndTime() const override
				{
					if (m_vecKeyframes.empty())
						return 0.f;

					return m_vecKeyframes.back().fTime;
				}

				virtual void Update(IMotionRecoder* pRecoder, float fPlayTime, bool isInverse) override;

			public:
				virtual size_t GetKeyframeCount() const override { return m_vecKeyframes.size(); }
				virtual const Keyframe* GetKeyframe(size_t nIndex) const override { return &m_vecKeyframes[nIndex]; }

			private:
				String::StringID m_strBoneName;
				std::vector<Keyframe> m_vecKeyframes;
				float m_fFrameInterval;
			};

		public:
			Motion(const String::StringID& strName, const char* strFilePath);
			virtual ~Motion();

		public:
			virtual void Update(IMotionRecoder* pRecoder, float fPlayTime, bool isInverse) override;

			virtual float GetStartTime() const override { return m_fStartTime; }
			virtual float GetEndTime() const override { return m_fEndTime; }

		public:
			virtual const String::StringID& GetName() override { return m_strName; }

			virtual size_t GetBoneCount() override { return m_clnBones.size(); }
			virtual const IBone* GetBone(size_t nIndex) const override { return m_vecBonesIndexing[nIndex]; }
			virtual const IBone* GetBone(const String::StringID& strBoneName) const override;

		public:
			virtual int GetReferenceCount() const override { return m_nReferenceCount; }
			virtual int IncreaseReference() override
			{
				++m_nReferenceCount;
				SetAlive(true);

				return m_nReferenceCount;
			}
			virtual int DecreaseReference() override { return --m_nReferenceCount; }

		public:
			void AddBoneKeyframes(const String::StringID& strBoneName, const std::vector<Keyframe>& vecKeyframes);
			void SetInfo(float fStartTime, float fEndTime, float fFrameInterval);

		private:
			int m_nReferenceCount;

			String::StringID m_strName;
			std::string m_strFilePath;

			plf::colony<Bone> m_clnBones;
			std::vector<Bone*> m_vecBonesIndexing;
			std::unordered_map<String::StringID, Bone*> m_umapBones;

			float m_fStartTime;
			float m_fEndTime;
			float m_fFrameInterval;
		};
	}
}