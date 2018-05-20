#pragma once

#include "ModelInterface.h"

namespace eastengine
{
	namespace graphics
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

				virtual void Update(float fInterval, IMotionRecorder* pRecorder, float fPlayTime, bool isInverse) const override;

			public:
				virtual uint32_t GetKeyframeCount() const override { return static_cast<uint32_t>(m_vecKeyframes.size()); }
				virtual const Keyframe* GetKeyframe(uint32_t nIndex) const override { return &m_vecKeyframes[nIndex]; }

			private:
				String::StringID m_strBoneName;
				std::vector<Keyframe> m_vecKeyframes;
				float m_fFrameInterval;
			};

		public:
			Motion(Key key);
			Motion(const Motion& source);
			Motion(Motion&& source) noexcept;
			virtual ~Motion() = default;

		public:
			virtual Key GetKey() const override { return m_key; }

		public:
			virtual void Update(IMotionRecorder* pRecorder, float fPlayTime, bool isInverse) const override;

			virtual float GetStartTime() const override { return m_fStartTime; }
			virtual float GetEndTime() const override { return m_fEndTime; }

		public:
			virtual const String::StringID& GetName() const override { return m_strName; }
			virtual const std::string& GetFilePath() const override { return m_strFilePath; }

			virtual uint32_t GetBoneCount() const override { return static_cast<uint32_t>(m_vecBones.size()); }
			virtual const IBone* GetBone(uint32_t nIndex) const override { return &m_vecBones[nIndex]; }
			virtual const IBone* GetBone(const String::StringID& strBoneName) const override;

		public:
			void SetName(const String::StringID& strName) { m_strName; }
			void SetFilePath(const std::string& strFilePath) { m_strFilePath = strFilePath; }
			void AddBoneKeyframes(const String::StringID& strBoneName, const std::vector<Keyframe>& vecKeyframes);
			void SetInfo(float fStartTime, float fEndTime, float fFrameInterval);

		private:
			const Key m_key;

			String::StringID m_strName;
			std::string m_strFilePath;

			std::vector<Bone> m_vecBones;
			std::unordered_map<String::StringID, Bone*> m_umapBones;

			float m_fStartTime{ 0.f };
			float m_fEndTime{ 0.f };
			float m_fFrameInterval{ 0.f };
		};
	}
}