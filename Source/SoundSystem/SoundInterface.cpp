#include "stdafx.h"
#include "SoundInterface.h"

#include "SoundSystem.h"

namespace eastengine
{
	namespace sound
	{
		void SetListenerAttributes(const ListenerAttributes& listenerAttributes)
		{
			System::GetInstance()->SetListenerAttributes(listenerAttributes);
		}

		ChannelID Play2D(const std::string& strSoundFilePath, float fVolume, int mode)
		{
			return System::GetInstance()->Play2D(strSoundFilePath, fVolume, mode);
		}

		ChannelID Play3D(const std::string& strSoundFilePath, const math::float3& f3Position, const math::float3& f3Velocity, float fVolume, int mode)
		{
			return System::GetInstance()->Play3D(strSoundFilePath, f3Position, f3Velocity, fVolume, mode);
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