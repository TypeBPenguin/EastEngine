#pragma once

#include "ModelInterface.h"

namespace est
{
	namespace graphics
	{
		class MotionRecorder;

		class Motion : public IMotion
		{
		public:
			class Bone : public IMotion::IBone
			{
			public:
				Bone(const string::StringID& boneName, std::vector<Keyframe>&& vecKeyframes, float frameInterval);
				Bone(const string::StringID& boneName, const Keyframe* pKeyframes, size_t keyframeCount, float frameInterval);
				virtual ~Bone() override;

			public:
				virtual const string::StringID& GetName() const override { return m_boneName; }

				virtual float GetStartTime() const override
				{
					if (m_keyframes.empty())
						return 0.f;

					return m_keyframes.front().time;
				}

				virtual float GetEndTime() const override
				{
					if (m_keyframes.empty())
						return 0.f;

					return m_keyframes.back().time;
				}

			public:
				virtual uint32_t GetKeyframeCount() const override { return static_cast<uint32_t>(m_keyframes.size()); }
				virtual const Keyframe* GetKeyframe(uint32_t index) const override { return &m_keyframes[index]; }

			public:
				void Update(float interval, MotionRecorder* pRecorder, float playTime, bool isInverse) const;

			private:
				const string::StringID m_boneName;
				const float m_frameInterval{ 0 };
				std::vector<Keyframe> m_keyframes;
			};

		public:
			Motion(Key key);
			Motion(Motion&& source) noexcept;
			virtual ~Motion();

		public:
			virtual Key GetKey() const override { return m_key; }

		public:
			virtual float GetStartTime() const override { return m_startTime; }
			virtual float GetEndTime() const override { return m_endTime; }
			virtual float GetFrameInterval() const override { return m_frameInterval; }

		public:
			virtual const string::StringID& GetName() const override { return m_name; }
			virtual const std::wstring& GetFilePath() const override { return m_filePath; }

			virtual uint32_t GetBoneCount() const override { return static_cast<uint32_t>(m_bones.size()); }
			virtual const IBone* GetBone(uint32_t index) const override { return &m_bones[index]; }
			virtual const IBone* GetBone(const string::StringID& boneName) const override;
			
		public:
			void Update(MotionRecorder* pRecorder, float playTime, bool isInverse) const;

		public:
			void SetName(const string::StringID& name) { m_name = name; }
			void SetFilePath(const std::wstring& filePath) { m_filePath = filePath; }
			void AddBoneKeyframes(const string::StringID& boneName, std::vector<Keyframe>&& vecKeyframes);
			void SetInfo(float startTime, float endTime, float frameInterval);

			void AddEvent(std::unique_ptr<const IMotionEvent> pMotionEvent) { m_events.emplace_back(std::move(pMotionEvent)); }

		public:
			bool LoadFile(const wchar_t* path);

		private:
			const Key m_key;

			string::StringID m_name;
			std::wstring m_filePath;

			float m_startTime{ 0.f };
			float m_endTime{ 0.f };
			float m_frameInterval{ 0.f };

			std::vector<Bone> m_bones;
			tsl::robin_map<string::StringID, Bone*> m_rmapBones;

			std::vector<std::unique_ptr<const IMotionEvent>> m_events;
		};
	}
}