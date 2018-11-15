#include "stdafx.h"
#include "SoundSystem.h"

#include "CommonLib/Lock.h"
#include "CommonLib/FileUtil.h"

#include "SoundObject.h"

namespace eastengine
{
	namespace sound
	{
		class System::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(float fElapsedTime);

		public:
			void SetListenerAttributes(const ListenerAttributes& listenerAttributes) { m_listenerAttributes = listenerAttributes; }

			ChannelID Play2D(const std::string& strSoundFilePath, float fVolume = 1.f, int mode = eLoopOff | eHardware);
			ChannelID Play3D(const std::string& strSoundFilePath, const math::float3& f3Position, const math::float3& f3Velocity, float fVolume = 1.f, int mode = eLoopOff | eHardware);
			void Stop(const ChannelID& channelID, float fFadeOutTime);
			void Resume(const ChannelID& channelID);
			void Pause(const ChannelID& channelID);

		private:
			std::shared_ptr<Sound> CreateSound(const std::string& strSoundFilePath, int mode);
			Channel* CreateChannel(const std::shared_ptr<Sound>& pSound, bool isPause);

			ChannelID AllocateChannelID() { return { m_nChannelID_allocator++ }; }

		private:
			thread::SRWLock m_srwLock;

			FMOD::System* m_pFmodSystem{ nullptr };
			ListenerAttributes m_listenerAttributes;

			uint64_t m_nChannelID_allocator{ 0 };

			std::map<std::string, std::unordered_map<int, std::shared_ptr<Sound>>> m_mapModeSounds;
			std::unordered_map<ChannelID, std::unique_ptr<Channel>> m_umapChannels;

			struct StopChannel
			{
				ChannelID id{ eInvalidChannelID };
				float fFadeOutTime{ 0.3f };

				StopChannel(const ChannelID& id, float fFadeOutTime)
					: id(id)
					, fFadeOutTime(fFadeOutTime)
				{
				}
			};
			std::vector<StopChannel> m_vecReqStopChannels;
		};

		System::Impl::Impl()
		{
			// 사운드 시스템 생성
			FMOD_RESULT fm_result = FMOD::System_Create(&m_pFmodSystem);
			if (fm_result != FMOD_OK)
			{
				LOG_ERROR("fmod system create failed, %s", FMOD_ErrorString(fm_result));
				return;
			}

			m_pFmodSystem->init(eMaxChannel, FMOD_INIT_NORMAL, nullptr);
			m_pFmodSystem->set3DSettings(1.f, 100.f, 1.f);
		}

		System::Impl::~Impl()
		{
			m_umapChannels.clear();
			m_mapModeSounds.clear();

			if (m_pFmodSystem != nullptr)
			{
				m_pFmodSystem->release();
				m_pFmodSystem = nullptr;
			}
		}

		void System::Impl::Update(float fElapsedTime)
		{
			if (m_pFmodSystem == nullptr)
				return;

			TRACER_EVENT(__FUNCTION__);

			thread::SRWWriteLock writeLock(&m_srwLock);

			{
				TRACER_BEGINEVENT("RequestStopChannels");
				m_vecReqStopChannels.erase(std::remove_if(m_vecReqStopChannels.begin(), m_vecReqStopChannels.end(), [&](const StopChannel& stopChannel)
				{
					auto iter = m_umapChannels.find(stopChannel.id);
					if (iter == m_umapChannels.end())
						return true;

					Channel* pChannel = iter->second.get();
					FMOD::Channel* pFmodChannel = pChannel->GetInterface();

					float fVolume = 0.f;
					pFmodChannel->getVolume(&fVolume);

					// FadeOutTime 음향 페이드아웃 후 종료 구현하도록 하셈 흐흐
					if (fVolume < 0.01f)
					{
						pFmodChannel->stop();
						m_umapChannels.erase(iter);
						return true;
					}
					else
					{
						fVolume -= 0.5f;
						pFmodChannel->setVolume(fVolume);
						return false;
					}
				}), m_vecReqStopChannels.end());

				TRACER_ENDEVENT();
			}

			{
				TRACER_BEGINEVENT("GarbageCollect_Channel");
				for (auto iter = m_umapChannels.begin(); iter != m_umapChannels.end();)
				{
					Channel* pChannel = iter->second.get();
					if (pChannel->GetState() == Channel::eEnd)
					{
						iter = m_umapChannels.erase(iter);
					}
					else
					{
						++iter;
					}
				}
				TRACER_ENDEVENT();
			}

			{
				TRACER_BEGINEVENT("GarbageCollect_Sound");

				const float DestroyWaitTime = 60.f;

				for (auto iter_mode = m_mapModeSounds.begin(); iter_mode != m_mapModeSounds.end();)
				{
					std::unordered_map<int, std::shared_ptr<Sound>>& uampSounds = iter_mode->second;
					for (auto iter_sound = uampSounds.begin(); iter_sound != uampSounds.end();)
					{
						const std::shared_ptr<Sound>& pSound = iter_sound->second;
						if (pSound.use_count() == 1)
						{
							pSound->UpdateDestroyWaitTime(fElapsedTime);
							if (pSound->GetDestroyWaitTime() >= DestroyWaitTime)
							{
								iter_sound = uampSounds.erase(iter_sound);
							}
							else
							{
								++iter_sound;
							}
						}
						else
						{
							++iter_sound;
						}
					}

					if (uampSounds.empty() == true)
					{
						iter_mode = m_mapModeSounds.erase(iter_mode);
					}
					else
					{
						++iter_mode;
					}
				}
				TRACER_ENDEVENT();
			}

			{
				TRACER_BEGINEVENT("FMOD System Update");

				const FMOD_VECTOR* pPosition = reinterpret_cast<const FMOD_VECTOR*>(&m_listenerAttributes.f3Position);
				const FMOD_VECTOR* pVelocity = reinterpret_cast<const FMOD_VECTOR*>(&m_listenerAttributes.f3Velocity);
				const FMOD_VECTOR* pForward = reinterpret_cast<const FMOD_VECTOR*>(&m_listenerAttributes.f3Forward);
				const FMOD_VECTOR* pUp = reinterpret_cast<const FMOD_VECTOR*>(&m_listenerAttributes.f3Up);
				m_pFmodSystem->set3DListenerAttributes(0, pPosition, pVelocity, pForward, pUp);
				
				m_pFmodSystem->update();
				TRACER_ENDEVENT();
			}
		}

		ChannelID System::Impl::Play2D(const std::string& strSoundFilePath, float fVolume, int mode)
		{
			if (m_pFmodSystem == nullptr)
				return { eInvalidChannelID };

			TRACER_EVENT(__FUNCTION__);

			mode ^= e3D;
			mode |= e2D;

			thread::SRWWriteLock writeLock(&m_srwLock);

			std::shared_ptr<Sound> pSound = CreateSound(strSoundFilePath, mode);
			if (pSound == nullptr)
				return { eInvalidChannelID };

			Channel* pChannel = CreateChannel(pSound, false);
			if (pChannel == nullptr)
				return { eInvalidChannelID };

			FMOD::Channel* pFmodChannel = pChannel->GetInterface();
			pFmodChannel->setVolume(fVolume);

			return pChannel->GetID();
		}

		ChannelID System::Impl::Play3D(const std::string& strSoundFilePath, const math::float3& f3Position, const math::float3& f3Velocity, float fVolume, int mode)
		{
			if (m_pFmodSystem == nullptr)
				return { eInvalidChannelID };

			TRACER_EVENT(__FUNCTION__);

			mode ^= e2D;
			mode |= e3D;

			thread::SRWWriteLock writeLock(&m_srwLock);

			std::shared_ptr<Sound> pSound = CreateSound(strSoundFilePath, mode);
			if (pSound == nullptr)
				return { eInvalidChannelID };

			Channel* pChannel = CreateChannel(pSound, true);
			if (pChannel == nullptr)
				return { eInvalidChannelID };

			FMOD::Channel* pFmodChannel = pChannel->GetInterface();
			pFmodChannel->setVolume(fVolume);

			const FMOD_VECTOR* pPosition = reinterpret_cast<const FMOD_VECTOR*>(&f3Position);
			const FMOD_VECTOR* pVelocity = reinterpret_cast<const FMOD_VECTOR*>(&f3Velocity);
			pFmodChannel->set3DAttributes(pPosition, pVelocity);
			pFmodChannel->setPaused(false);

			return pChannel->GetID();
		}

		void System::Impl::Stop(const ChannelID& channelID, float fFadeOutTime)
		{
			if (m_pFmodSystem == nullptr)
				return;

			TRACER_EVENT(__FUNCTION__);

			thread::SRWWriteLock writeLock(&m_srwLock);

			auto iter = std::find_if(m_vecReqStopChannels.begin(), m_vecReqStopChannels.end(), [&](const StopChannel& stopChannel)
			{
				return stopChannel.id == channelID;
			});

			if (iter != m_vecReqStopChannels.end())
				return;

			m_vecReqStopChannels.emplace_back(channelID, fFadeOutTime);
		}

		void System::Impl::Resume(const ChannelID& channelID)
		{
			if (m_pFmodSystem == nullptr)
				return;

			TRACER_EVENT(__FUNCTION__);

			auto iter = m_umapChannels.find(channelID);
			if (iter != m_umapChannels.end())
			{
				iter->second->Resume();
			}
		}

		void System::Impl::Pause(const ChannelID& channelID)
		{
			if (m_pFmodSystem == nullptr)
				return;

			TRACER_EVENT(__FUNCTION__);

			auto iter = m_umapChannels.find(channelID);
			if (iter != m_umapChannels.end())
			{
				iter->second->Pause();
			}
		}

		std::shared_ptr<Sound> System::Impl::CreateSound(const std::string& strSoundFilePath, int mode)
		{
			TRACER_EVENT(__FUNCTION__);

			std::shared_ptr<Sound> pSound;

			auto iter_mode = m_mapModeSounds.find(strSoundFilePath);
			if (iter_mode != m_mapModeSounds.end())
			{
				auto iter_sound = iter_mode->second.find(mode);
				if (iter_sound != iter_mode->second.end())
				{
					pSound = iter_sound->second;
				}
			}

			if (pSound == nullptr)
			{
				if (file::IsExists(strSoundFilePath) == false)
					return nullptr;

				FMOD::Sound* pFmodSound = nullptr;
				const FMOD_RESULT fm_result = m_pFmodSystem->createSound(strSoundFilePath.c_str(), mode, nullptr, &pFmodSound);
				if (fm_result != FMOD_OK)
					return nullptr;

				pSound = std::make_shared<Sound>(strSoundFilePath, pFmodSound, mode);

				if (iter_mode != m_mapModeSounds.end())
				{
					iter_mode->second.emplace(mode, pSound);
				}
				else
				{
					auto iter_sound = m_mapModeSounds.emplace(strSoundFilePath, std::unordered_map<int, std::shared_ptr<Sound>>());
					iter_sound.first->second.emplace(mode, pSound);
				}
			}

			pSound->Alive();

			return pSound;
		}

		Channel* System::Impl::CreateChannel(const std::shared_ptr<Sound>& pSound, bool isPause)
		{
			TRACER_EVENT(__FUNCTION__);

			FMOD::Channel* pFmodChannel = nullptr;
			const FMOD_RESULT result = m_pFmodSystem->playSound(FMOD_CHANNEL_FREE, pSound->GetInterface(), isPause, &pFmodChannel);
			if (result != FMOD_OK)
			{
				LOG_ERROR("Failed to FMOD PlaySound : ErrorCode[%d]", result);
				return nullptr;
			}

			const ChannelID channelID = AllocateChannelID();
			std::unique_ptr<Channel> pChannel = std::make_unique<Channel>(channelID, pSound, pFmodChannel);

			auto iter_result = m_umapChannels.emplace(channelID, std::move(pChannel));
			return iter_result.first->second.get();
		}

		System::System()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		System::~System()
		{
		}

		ChannelID System::Play2D(const std::string& strSoundFilePath, float fVolume, int mode)
		{
			return m_pImpl->Play2D(strSoundFilePath, fVolume, mode);
		}

		ChannelID System::Play3D(const std::string& strSoundFilePath, const math::float3& f3Position, const math::float3& f3Velocity, float fVolume, int mode)
		{
			return m_pImpl->Play3D(strSoundFilePath, f3Position, f3Velocity, fVolume, mode);
		}

		void System::Stop(const ChannelID& channelID, float fFadeOutTime)
		{
			m_pImpl->Stop(channelID, fFadeOutTime);
		}

		void System::Resume(const ChannelID& channelID)
		{
			m_pImpl->Resume(channelID);
		}

		void System::Pause(const ChannelID& channelID)
		{
			m_pImpl->Pause(channelID);
		}

		void System::SetListenerAttributes(const ListenerAttributes& listenerAttributes)
		{
			m_pImpl->SetListenerAttributes(listenerAttributes);
		}

		void System::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}
	}
}