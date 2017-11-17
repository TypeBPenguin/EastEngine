#include "stdafx.h"
#include "SoundSystem.h"

#include "fmod_errors.h"

#include "../CommonLib/FileUtil.h"

namespace EastEngine
{
	namespace Sound
	{
		SoundInstance::~SoundInstance()
		{
			pSound->release(); listChannel.clear();
		}

		SoundSystem::SoundSystem()
			: m_isInit(false)
			, m_pSystem(nullptr)
			, m_pf3ListenerPos(nullptr)
		{
		}

		SoundSystem::~SoundSystem()
		{
			Release();
		}

		bool SoundSystem::Init()
		{
			if (m_isInit == true)
				return true;

			// 사운드 시스템 생성
			FMOD_RESULT fm_result;
			fm_result = FMOD::System_Create(&m_pSystem);
			if (fm_result != FMOD_OK)
			{
				PRINT_LOG("fmod system create failed, %s", FMOD_ErrorString(fm_result));
				return false;
			}

			m_pSystem->init(20, FMOD_INIT_NORMAL, nullptr);
			m_pSystem->set3DSettings(1.f, 100.f, 1.f);

			m_isInit = true;

			return true;
		}

		void SoundSystem::Release()
		{
			if (m_isInit == false)
				return;

			m_pf3ListenerPos = nullptr;

			std::for_each(m_umapSoundInst.begin(), m_umapSoundInst.end(), DeleteSTLMapObject());
			m_umapSoundInst.clear();


			m_isInit = false;
		}

		void SoundSystem::Play(const String::StringID& strSoundFile, float fVolume, const Math::Vector3* pf3Pos, int mode)
		{
			SoundInstance* pSoundInst = nullptr;
			auto iter = m_umapSoundInst.find(strSoundFile);
			if (iter == m_umapSoundInst.end())
			{
				FMOD::Sound* pNewSound = createSoundInst(strSoundFile, mode);
				if (pNewSound == nullptr)
					return;

				SoundInstance* pNewSoundInst = new SoundInstance;
				pNewSoundInst->pSound = pNewSound;

				m_umapSoundInst.insert(std::make_pair(strSoundFile, pNewSoundInst));

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
				return;

			pNewChannel->setVolume(fVolume);
		}

		void SoundSystem::Update(float fElapsedTime)
		{
			{
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
			}

			{
				auto iter = m_umapSoundInst.begin();
				while (iter != m_umapSoundInst.end())
				{
					SoundInstance* pSoundInst = iter->second;
					auto& listChannel = pSoundInst->listChannel;

					if (listChannel.empty())
					{
						pSoundInst->m_fFlushTime += fElapsedTime;
						if (pSoundInst->m_fFlushTime >= 300.f)
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
							bool bIsPlaying = false;
							(*iter_channel)->isPlaying(&bIsPlaying);

							if (bIsPlaying == false)
							{
								iter_channel = listChannel.erase(iter_channel);
								continue;
							}

							++iter_channel;
						}
					}
				}
			}

			m_pSystem->update();

			if (m_pf3ListenerPos == nullptr)
				return;

			FMOD_VECTOR vPos = { m_pf3ListenerPos->x, m_pf3ListenerPos->y, m_pf3ListenerPos->z };
			FMOD_VECTOR vForward = { 0.f, 1.f, 0.f };
			FMOD_VECTOR vUp = { 0.f, 0.f, 1.f };
			m_pSystem->set3DListenerAttributes(0, &vPos, nullptr, &vForward, &vUp);
		}

		FMOD::Sound* SoundSystem::createSoundInst(const String::StringID& strSoundFile, int mode)
		{
			// 사운드 로딩
			std::string strFile = File::GetPath(File::eSound);
			strFile.append(strSoundFile.c_str());

			FMOD_RESULT fm_result;
			FMOD::Sound* pNewSound = nullptr;
			fm_result = m_pSystem->createSound(strFile.c_str(), mode, 0, &pNewSound);
			if (fm_result != FMOD_OK)
				return nullptr;

			return pNewSound;
		}
	}
}