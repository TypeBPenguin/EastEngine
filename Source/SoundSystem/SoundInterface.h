#pragma once

#include "SoundDefine.h"

namespace eastengine
{
	namespace sound
	{
		void SetListenerAttributes(const ListenerAttributes& listenerAttributes);

		ChannelID Play2D(const std::string& strSoundFilePath, float fVolume = 1.f, int mode = eLoopOff | eHardware);
		ChannelID Play3D(const std::string& strSoundFilePath, const math::Vector3& f3Position, const math::Vector3& f3Velocity, float fVolume = 1.f, int mode = eLoopOff | eHardware);
		void Stop(const ChannelID& channelID, float fFadeOutTime);
		void Resume(const ChannelID& channelID);
		void Pause(const ChannelID& channelID);
	}
}