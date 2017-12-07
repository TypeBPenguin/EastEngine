#pragma once

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
				Bone(const String::StringID& strBoneName, const std::vector<Keyframe>& _vecKeyframes);
				virtual ~Bone() override;

			public:
				virtual const String::StringID& GetName() const override { return m_strBoneName; }

				virtual float GetStartTime() const override
				{
					if (m_vecKeyframes.empty())
						return 0.f;

					return m_vecKeyframes.front().fTimePos;
				}

				virtual float GetEndTime() const override
				{
					if (m_vecKeyframes.empty())
						return 0.f;

					return m_vecKeyframes.back().fTimePos;
				}

				virtual void Update(IMotionPlayer* pPlayInfo) override;

			public:
				virtual uint32_t GetKeyframeCount() const override { return m_vecKeyframes.size(); }
				virtual const Keyframe* GetKeyframe(uint32_t nIndex) const override { return &m_vecKeyframes[nIndex]; }

			private:
				String::StringID m_strBoneName;
				std::vector<Keyframe> m_vecKeyframes;
			};

		public:
			Motion(const String::StringID& strName, const char* strFilePath);
			virtual ~Motion();

		public:
			virtual void Update(IMotionPlayer* pPlayInfo) override;

			virtual float GetStartTime() const override { return m_fStartTime; }
			virtual float GetEndTime() const override { return m_fEndTime; }

		public:
			virtual const String::StringID& GetName() override { return m_strName; }

			virtual uint32_t GetBoneCount() override { return m_vecBones.size(); }
			virtual const IBone* GetBone(uint32_t nIndex) const override { return m_vecBones[nIndex]; }
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
			void CalcClipTime();

		private:
			int m_nReferenceCount;

			String::StringID m_strName;
			std::string m_strFilePath;

			std::vector<Bone*> m_vecBones;
			std::unordered_map<String::StringID, Bone*> m_umapBones;

			float m_fStartTime;
			float m_fEndTime;
		};
	}
}