#include "stdafx.h"
#include "SoundInterface.h"

#include "SoundSystem.h"

namespace est
{
	namespace sound
	{
		void SetListenerAttributes(const ListenerAttributes& listenerAttributes)
		{
			System::GetInstance()->SetListenerAttributes(listenerAttributes);
		}

		ChannelID Play2D(const std::wstring& soundFilePath, float volume, int mode)
		{
			return System::GetInstance()->Play2D(soundFilePath, volume, mode);
		}

		ChannelID Play3D(const std::wstring& soundFilePath, const math::float3& position, const math::float3& f3Velocity, float volume, int mode)
		{
			return System::GetInstance()->Play3D(soundFilePath, position, f3Velocity, volume, mode);
		}

		void Stop(const ChannelID& channelID, float fFadeOutTime)
		{
			System::GetInstance()->Stop(channelID, fFadeOutTime);
		}

		void Resume(const ChannelID& channelID)
		{
			System::GetInstance()->Resume(channelID);
		}

		void Pause(const ChannelID& channelID)
		{
			System::GetInstance()->Pause(channelID);
		}
	}
}