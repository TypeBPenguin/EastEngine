#include "stdafx.h"
#include "SoundObject.h"

namespace est
{
	namespace sound
	{
		Sound::Sound(const std::wstring& filePath, FMOD::Sound* pSound, int mode)
			: m_filePath(filePath)
			, m_pInterface(pSound)
			, m_mode(mode)
		{
			m_pInterface->setUserData(this);
		}

		Sound::~Sound()
		{
			m_pInterface->release();
		}

		FMOD_RESULT F_CALLBACK ChannelCallback(FMOD_CHANNELCONTROL* pChannelcontrol, FMOD_CHANNELCONTROL_TYPE controltype, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbacktype, void* pCommanddata1, void* pCommanddata2)
		{
			FMOD::ChannelControl* pInterface = reinterpret_cast<FMOD::ChannelControl*>(pChannelcontrol);

			Channel* pChannel = nullptr;
			const FMOD_RESULT result = pInterface->getUserData(reinterpret_cast<void**>(&pChannel));
			if (result == FMOD_OK && pChannel != nullptr)
			{
				pChannel->Callback(callbacktype, pCommanddata1, pCommanddata2);
			}

			return FMOD_OK;
		}

		Channel::Channel(const ChannelID& id, const std::shared_ptr<Sound>& pSound, FMOD::Channel* pChannel)
			: m_id(id)
			, m_pSound(pSound)
			, m_pInterface(pChannel)
		{
			m_pInterface->setUserData(this);
			m_pInterface->setCallback(ChannelCallback);
		}

		Channel::~Channel()
		{
			m_pInterface->stop();
			m_pInterface = nullptr;
		}

		void Channel::Callback(FMOD_CHANNELCONTROL_CALLBACK_TYPE type, void* pCommanddata1, void* pCommanddata2)
		{
			switch (type)
			{
			case FMOD_CHANNELCONTROL_CALLBACK_END:
			{
				/* Called when a sound ends. */
				m_state = eEnd;
			}
			break;
			case FMOD_CHANNELCONTROL_CALLBACK_VIRTUALVOICE:
			{
				/* Called when a voice is swapped out or swapped in. */
			}
			break;
			case FMOD_CHANNELCONTROL_CALLBACK_SYNCPOINT:
			{
				/* Called when a syncpoint is encountered.  Can be from wav file markers. */
			}
			break;
			case FMOD_CHANNELCONTROL_CALLBACK_OCCLUSION:
			{
				/* Called when the channel has its geometry occlusion value calculated.  Can be used to clamp or change the value. */
			}
			break;
			}
		}

		void Channel::Resume()
		{
			m_pInterface->setPaused(false);
		}

		void Channel::Pause()
		{
			m_pInterface->setPaused(true);
		}

		void Channel::Mute()
		{
			m_pInterface->setMute(true);
		}

		bool Channel::IsPlaying() const
		{
			bool isPlaying{ false };
			m_pInterface->isPlaying(&isPlaying);
			return isPlaying;
		}

		uint32_t Channel::GetPosition() const
		{
			uint32_t position{ 0 };
			m_pInterface->getPosition(&position, FMOD_TIMEUNIT_MS);
			return position;
		}

		uint32_t Channel::GetLength() const
		{
			uint32_t length{ 0 };
			m_pSound->GetInterface()->getLength(&length, FMOD_TIMEUNIT_MS);
			return length;
		}
	}
}