#pragma once

#include "SoundDefine.h"

namespace est
{
	namespace sound
	{
		void SetListenerAttributes(const ListenerAttributes& listenerAttributes);

		ChannelID Play2D(const std::wstring& soundFilePath, float volume = 1.f, int mode = eLoopOff);
		ChannelID Play3D(const std::wstring& soundFilePath, const math::float3& f3Position, const math::float3& f3Velocity, float volume = 1.f, int mode = eLoopOff);
		void Stop(const ChannelID& channelID, float fadeOutTIme);
		void Resume(const ChannelID& channelID);
		void Pause(const ChannelID& channelID);
	}
}