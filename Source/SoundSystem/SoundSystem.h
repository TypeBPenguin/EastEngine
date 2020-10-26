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

namespace est
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
			void Update(float elapsedTime);

		public:
			void SetListenerAttributes(const ListenerAttributes& listenerAttributes);

			ChannelID Play2D(const std::wstring& soundFilePath, float volume = 1.f, int mode = eLoopOff);
			ChannelID Play3D(const std::wstring& soundFilePath, const math::float3& position, const math::float3& velocity, float volume = 1.f, int mode = eLoopOff);
			void Stop(const ChannelID& channelID, float fadeOutTime);
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
	struct hash<est::sound::ChannelID>
	{
		std::uint64_t operator()(const est::sound::ChannelID& key) const
		{
			return key;
		}
	};
}