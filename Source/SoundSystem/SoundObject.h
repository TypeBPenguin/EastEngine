#pragma once

#include "SoundDefine.h"

namespace eastengine
{
	namespace sound
	{
		class Sound
		{
		public:
			Sound(const std::string& strFilePath, FMOD::Sound* pSound, int mode);
			~Sound();

		public:
			void UpdateDestroyWaitTime(float elapsedTime) { m_fDestroyWaitTime += elapsedTime; }
			float GetDestroyWaitTime() const { return m_fDestroyWaitTime; }
			void Alive() { m_fDestroyWaitTime = 0.f; }

		public:
			FMOD::Sound* GetInterface() const { return m_pInterface; }
			int GetMode() const { return m_mode; }

		private:
			std::string m_strFilePath;

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
			void Callback(FMOD_CHANNEL_CALLBACKTYPE type, void* pCommanddata1, void* pCommanddata2);
			void Resume();
			void Pause();

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