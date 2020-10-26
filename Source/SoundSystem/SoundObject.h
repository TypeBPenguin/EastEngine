#pragma once

#include "SoundDefine.h"

namespace est
{
	namespace sound
	{
		class Sound
		{
		public:
			Sound(const std::wstring& filePath, FMOD::Sound* pSound, int mode);
			~Sound();

		public:
			void UpdateDestroyWaitTime(float elapsedTime) { m_fDestroyWaitTime += elapsedTime; }
			float GetDestroyWaitTime() const { return m_fDestroyWaitTime; }
			void Alive() { m_fDestroyWaitTime = 0.f; }

		public:
			FMOD::Sound* GetInterface() const { return m_pInterface; }
			int GetMode() const { return m_mode; }

		private:
			std::wstring m_filePath;

			FMOD::Sound* m_pInterface{ nullptr };
			int m_mode{ 0 };

			float m_fDestroyWaitTime{ 0.f };
		};

		class Channel
		{
		public:
			Channel(const ChannelID& id, const std::shared_ptr<Sound>& pSoundObject, FMOD::Channel* pChannel);
			~Channel();

			enum State
			{
				ePlaying = 0,
				ePause,
				eEnd,
			};

		public:
			void Callback(FMOD_CHANNELCONTROL_CALLBACK_TYPE type, void* pCommanddata1, void* pCommanddata2);
			void Resume();
			void Pause();
			void Mute();

		public:
			bool IsPlaying() const;
			uint32_t GetPosition() const;
			uint32_t GetLength() const;

		public:
			const ChannelID& GetID() const { return m_id; }
			FMOD::Channel* GetInterface() const { return m_pInterface; }
			State GetState() const { return m_state; }

		private:
			ChannelID m_id{ eInvalidChannelID };
			std::shared_ptr<Sound> m_pSound;
			FMOD::Channel* m_pInterface{ nullptr };

			State m_state{ ePlaying };
		};
	}
}