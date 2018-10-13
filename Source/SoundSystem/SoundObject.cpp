#include "stdafx.h"
#include "SoundObject.h"

namespace eastengine
{
	namespace sound
	{
		Sound::Sound(const std::string& strFilePath, FMOD::Sound* pSound, int mode)
			: m_strFilePath(strFilePath)
			, m_pInterface(pSound)
			, m_mode(mode)
		{
			m_pInterface->setUserData(this);
		}

		Sound::~Sound()
		{
			m_pInterface->release();
		}

		FMOD_RESULT F_CALLBACK ChannelCallback(FMOD_CHANNEL* pFmodChannel, FMOD_CHANNEL_CALLBACKTYPE type, void* pCommanddata1, void* pCommanddata2)
		{
			FMOD::Channel* pInterface = reinterpret_cast<FMOD::Channel*>(pFmodChannel);

			Channel* pChannel = nullptr;
			const FMOD_RESULT result = pInterface->getUserData(reinterpret_cast<void**>(&pChannel));
			if (result == FMOD_OK && pChannel != nullptr)
			{
				pChannel->Callback(type, pCommanddata1, pCommanddata2);
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

		void Channel::Callback(FMOD_CHANNEL_CALLBACKTYPE type, void* pCommanddata1, void* pCommanddata2)
		{
			switch (type)
			{
			case FMOD_CHANNEL_CALLBACKTYPE_END:
			{
				/* Called when a sound ends. */
				m_state = eEnd;
			}
			break;
			case FMOD_CHANNEL_CALLBACKTYPE_VIRTUALVOICE:
			{
				/* Called when a voice is swapped out or swapped in. */
			}
			break;
			case FMOD_CHANNEL_CALLBACKTYPE_SYNCPOINT:
			{
				/* Called when a syncpoint is encountered.  Can be from wav file markers. */
			}
			break;
			case FMOD_CHANNEL_CALLBACKTYPE_OCCLUSION:
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
	}
}