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
				Bone(const string::StringID& strBoneName, std::vector<Keyframe>&& vecKeyframes, float fFrameInterval);
				Bone(const string::StringID& strBoneName, const Keyframe* pKeyframes, size_t keyframeCount, float fFrameInterval);
				virtual ~Bone() override;

			public:
				virtual const string::StringID& GetName() const override { return m_boneName; }

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
				const string::StringID m_boneName;
				const float m_frameInterval{ 0 };
				std::vector<Keyframe> m_vecKeyframes;
			};

		public:
			Motion(Key key);
			Motion(Motion&& source) noexcept;
			virtual ~Motion();

		public:
			virtual Key GetKey() const override { return m_key; }

		public:
			virtual void Update(IMotionRecorder* pRecorder, float fPlayTime, bool isInverse, bool isEnableTransformUpdate) const override;

			virtual float GetStartTime() const override { return m_startTime; }
			virtual float GetEndTime() const override { return m_endTime; }
			virtual float GetFrameInterval() const override { return m_frameInterval; }

		public:
			virtual const string::StringID& GetName() const override { return m_strName; }
			virtual const std::string& GetFilePath() const override { return m_strFilePath; }

			virtual uint32_t GetBoneCount() const override { return static_cast<uint32_t>(m_vecBones.size()); }
			virtual const IBone* GetBone(uint32_t nIndex) const override { return &m_vecBones[nIndex]; }
			virtual const IBone* GetBone(const string::StringID& strBoneName) const override;
			
		public:
			void SetName(const string::StringID& strName) { m_strName = strName; }
			void SetFilePath(const std::string& strFilePath) { m_strFilePath = strFilePath; }
			void AddBoneKeyframes(const string::StringID& strBoneName, std::vector<Keyframe>&& vecKeyframes);
			void SetInfo(float fStartTime, float fEndTime, float fFrameInterval);

			void AddEvent(const IMotionEvent* pMotionEvent) { m_vecEvents.emplace_back(pMotionEvent); }

		public:
			bool LoadFile(const char* path);

		private:
			const Key m_key;

			string::StringID m_strName;
			std::string m_strFilePath;

			float m_startTime{ 0.f };
			float m_endTime{ 0.f };
			float m_frameInterval{ 0.f };

			std::vector<Bone> m_vecBones;
			tsl::robin_map<string::StringID, Bone*> m_rmapBones;

			std::vector<const IMotionEvent*> m_vecEvents;
		};
	}
}