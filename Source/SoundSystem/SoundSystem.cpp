#include "stdafx.h"
#include "SoundSystem.h"

#include "fmod_errors.h"

#include "CommonLib/FileUtil.h"

namespace eastengine
{
	namespace Sound
	{
		struct SoundInstance
		{
			FMOD::Sound* pSound{ nullptr };
			std::list<FMOD::Channel*> listChannel;

			float m_fFlushTime{ 0.f };

			~SoundInstance();
		};

		SoundInstance::~SoundInstance()
		{
			pSound->release();
		}

		class System::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(float fElapsedTime);

			bool Play(const String::StringID& strSoundFile, float fVolume = 1.f, const math::Vector3* pf3Pos = nullptr, int mode = eLoopOff | e2D | eHardware);
			void Stop(const String::StringID& strSoundFile);

			void SetListenerPosition(const math::Vector3* pf3ListenerPos);

		private:
			FMOD::Sound* CreateSoundInst(const String::StringID& strSoundFile, int mode = eLoopOff | e2D | eHardware);

		private:
			bool m_isInitialized{ false };
			FMOD::System* m_pSystem{ nullptr };
			const math::Vector3* m_pf3ListenerPos{ nullptr };

			std::unordered_map<String::StringID, SoundInstance*> m_umapSoundInst;
			std::list<String::StringID> m_listReqStopSound;
		};

		System::Impl::Impl()
		{
			// 사운드 시스템 생성
			FMOD_RESULT fm_result = FMOD::System_Create(&m_pSystem);
			if (fm_result != FMOD_OK)
			{
				LOG_ERROR("fmod system create failed, %s", FMOD_ErrorString(fm_result));
				return;
			}

			m_pSystem->init(20, FMOD_INIT_NORMAL, nullptr);
			m_pSystem->set3DSettings(1.f, 100.f, 1.f);

			m_isInitialized = true;
		}

		System::Impl::~Impl()
		{
			m_pf3ListenerPos = nullptr;

			std::for_each(m_umapSoundInst.begin(), m_umapSoundInst.end(), DeleteSTLMapObject());
			m_umapSoundInst.clear();

			if (m_pSystem != nullptr)
			{
				m_pSystem->release();
				m_pSystem = nullptr;
			}
		}

		void System::Impl::Update(float fElapsedTime)
		{
			if (m_isInitialized == false)
				return;

			TRACER_EVENT("SoundSystem::Update");
			{
				TRACER_BEGINEVENT("SoundSystem::Update", "RequestStopSound");
				auto iter = m_listReqStopSound.begin();
				while (iter != m_listReqStopSound.end())
				{
					float fVolume = 0.f;

					auto iter_sound = m_umapSoundInst.find((*iter));
					if (iter_sound == m_umapSoundInst.end())
					{
						iter = m_listReqStopSound.erase(iter);
						continue;
					}

					SoundInstance* pSoundInst = iter_sound->second;

					if (pSoundInst->listChannel.empty())
					{
						iter = m_listReqStopSound.erase(iter);
						continue;
					}

					FMOD::Channel* pChannel = pSoundInst->listChannel.front();
					pChannel->getVolume(&fVolume);

					if (fVolume < 0.01f)
					{
						pChannel->stop();

						iter = m_listReqStopSound.erase(iter);
						continue;
					}
					else
					{
						fVolume += 0.5f;
						pChannel->setVolume(fVolume);
						++iter;
					}
				}
				TRACER_ENDEVENT();
			}

			{
				TRACER_BEGINEVENT("SoundSystem::Update", "GarbageCollect");
				auto iter = m_umapSoundInst.begin();
				while (iter != m_umapSoundInst.end())
				{
					SoundInstance* pSoundInst = iter->second;
					std::list<FMOD::Channel*>& listChannel = pSoundInst->listChannel;

					if (listChannel.empty())
					{
						pSoundInst->m_fFlushTime += fElapsedTime;
						if (pSoundInst->m_fFlushTime >= 60.f)
						{
							iter = m_umapSoundInst.erase(iter);
							continue;
						}

						++iter;
					}
					else
					{
						auto iter_channel = listChannel.begin();
						while (iter_channel != listChannel.end())
						{
							bool isPlaying = false;
							(*iter_channel)->isPlaying(&isPlaying);

							if (isPlaying == false)
							{
								iter_channel = listChannel.erase(iter_channel);
								continue;
							}

							++iter_channel;
						}
					}
				}
				TRACER_ENDEVENT();
			}

			{
				TRACER_BEGINEVENT("SoundSystem::Update", "FMOD System Update");
				m_pSystem->update();

				if (m_pf3ListenerPos != nullptr)
				{
					FMOD_VECTOR vPos = { m_pf3ListenerPos->x, m_pf3ListenerPos->y, m_pf3ListenerPos->z };
					FMOD_VECTOR vForward = { 0.f, 1.f, 0.f };
					FMOD_VECTOR vUp = { 0.f, 0.f, 1.f };
					m_pSystem->set3DListenerAttributes(0, &vPos, nullptr, &vForward, &vUp);
				}
				TRACER_ENDEVENT();
			}
		}

		bool System::Impl::Play(const String::StringID& strSoundFile, float fVolume, const math::Vector3* pf3Pos, int mode)
		{
			if (m_isInitialized == false)
				return false;

			SoundInstance* pSoundInst = nullptr;
			auto iter = m_umapSoundInst.find(strSoundFile);
			if (iter == m_umapSoundInst.end())
			{
				FMOD::Sound* pNewSound = CreateSoundInst(strSoundFile, mode);
				if (pNewSound == nullptr)
					return false;

				SoundInstance* pNewSoundInst = new SoundInstance;
				pNewSoundInst->pSound = pNewSound;

				m_umapSoundInst.emplace(strSoundFile, pNewSoundInst);

				pSoundInst = pNewSoundInst;
			}
			else
			{
				pSoundInst = iter->second;
			}

			pSoundInst->m_fFlushTime = 0.f;

			pSoundInst->pSound->getMode(reinterpret_cast<FMOD_MODE*>(&mode));

			FMOD::Channel* pNewChannel = nullptr;

			if (mode & e3D)
			{
				FMOD_VECTOR v = { 0.f, 0.f, 0.f };
				if (pf3Pos)
				{
					v.x = pf3Pos->x;
					v.y = pf3Pos->y;
					v.z = pf3Pos->z;
				}

				m_pSystem->playSound(FMOD_CHANNEL_FREE, pSoundInst->pSound, true, &pNewChannel);
				pNewChannel->set3DAttributes(&v, nullptr);
				pNewChannel->setPaused(false);

				pSoundInst->listChannel.push_back(pNewChannel);
			}
			else
			{
				m_pSystem->playSound(FMOD_CHANNEL_FREE, pSoundInst->pSound, false, &pNewChannel);
			}

			if (pNewChannel == nullptr)
				return false;

			pNewChannel->setVolume(fVolume);

			return true;
		}

		void System::Impl::Stop(const String::StringID& strSoundFile)
		{
			if (m_isInitialized == false)
				return;

			m_listReqStopSound.emplace_back(strSoundFile);
		}

		void System::Impl::SetListenerPosition(const math::Vector3* pf3ListenerPos)
		{
			if (m_isInitialized == false)
				return;

			m_pf3ListenerPos = pf3ListenerPos;
		}

		FMOD::Sound* System::Impl::CreateSoundInst(const String::StringID& strSoundFile, int mode)
		{
			if (m_isInitialized == false)
				return nullptr;

			// 사운드 로딩
			std::string strFile = file::GetPath(file::eSound);
			strFile.append(strSoundFile.c_str());

			FMOD_RESULT fm_result;
			FMOD::Sound* pNewSound = nullptr;
			fm_result = m_pSystem->createSound(strFile.c_str(), mode, 0, &pNewSound);
			if (fm_result != FMOD_OK)
				return nullptr;

			return pNewSound;
		}

		System::System()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		System::~System()
		{
		}

		bool System::Play(const String::StringID& strSoundFile, float fVolume, const math::Vector3* pf3Pos, int mode)
		{
			return m_pImpl->Play(strSoundFile, fVolume, pf3Pos, mode);
		}

		void System::Stop(const String::StringID& strSoundFile)
		{
			m_pImpl->Stop(strSoundFile);
		}

		void System::SetListenerPosition(const math::Vector3* pf3ListenerPos)
		{
			m_pImpl->SetListenerPosition(pf3ListenerPos);
		}

		void System::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}
	}
}