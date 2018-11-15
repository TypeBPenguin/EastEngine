#pragma once

#include "CommonLib/Singleton.h"

#include "SoundDefine.h"

namespace FMOD
{
	class System;
	class Sound;
	class Channel;
	class ChannelGroup;
	class SoundGroup;
	class Reverb;
	class DSP;
	class DSPConnection;
	class Geometry;
}

namespace eastengine
{
	namespace sound
	{
		class System : public Singleton<System>
		{
			friend Singleton<System>;
		private:
			System();
			virtual ~System();

		public:
			void Update(float fElapsedTime);

		public:
			void SetListenerAttributes(const ListenerAttributes& listenerAttributes);

			ChannelID Play2D(const std::string& strSoundFilePath, float fVolume = 1.f, int mode = eLoopOff | eHardware);
			ChannelID Play3D(const std::string& strSoundFilePath, const math::float3& f3Position, const math::float3& f3Velocity, float fVolume = 1.f, int mode = eLoopOff | eHardware);
			void Stop(const ChannelID& channelID, float fFadeOutTime);
			void Resume(const ChannelID& channelID);
			void Pause(const ChannelID& channelID);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}

namespace std
{
	template <>
	struct hash<eastengine::sound::ChannelID>
	{
		std::uint64_t operator()(const eastengine::sound::ChannelID& key) const
		{
			return key;
		}
	};
}