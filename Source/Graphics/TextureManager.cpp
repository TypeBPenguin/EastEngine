#include "stdafx.h"
#include "TextureManager.h"

namespace eastengine
{
	namespace graphics
	{
		class TextureManager::Impl
		{
		public:
			Impl();
			~Impl();

		private:
			struct RequestLoadTextureInfo
			{
				std::string strFilePath;
				ITexture* pTexture_out{ nullptr };
				std::function<bool(const std::string&)> funcLoader;

				RequestLoadTextureInfo()
				{
				}

				RequestLoadTextureInfo(const std::string& strFilePath, ITexture* pTexture_out, std::function<bool(const std::string&)> funcLoader)
					: strFilePath(strFilePath)
					, pTexture_out(pTexture_out)
					, funcLoader(funcLoader)
				{
				}

				RequestLoadTextureInfo(const RequestLoadTextureInfo& source)
				{
					strFilePath = source.strFilePath;
					pTexture_out = source.pTexture_out;
					funcLoader = source.funcLoader;
				}
			};

			struct ResultLoadTextureInfo
			{
				ITexture* pTexture_out{ nullptr };
				bool isSuccess{ false };
			};

		public:
			void Cleanup(float fElapsedTime);

			void AsyncLoadTexture(ITexture* pTexture, const char* strFilePath, std::function<bool(const std::string&)> funcLoad);

		public:
			void PushTexture(ITexture* pTexture);
			ITexture* GetTexture(const ITexture::Key& key);

		private:
			float m_fTime{ 0.f };

			std::atomic<bool> m_isStop{ false };

			std::mutex m_mutex;
			std::condition_variable m_condition;

			std::thread m_thread;
			std::queue<RequestLoadTextureInfo> m_queueRequestLoadTexture;
			std::queue<ResultLoadTextureInfo> m_queueCompleteTexture;

			std::unordered_map<ITexture::Key, ITexture*> m_umapTexture;
		};

		TextureManager::Impl::Impl()
		{
			m_thread = std::thread([&]()
			{
				while (true)
				{
					RequestLoadTextureInfo requestInfo{};
					{
						std::unique_lock<std::mutex> lock(m_mutex);

						m_condition.wait(lock, [&]()
						{
							return m_isStop == true || m_queueRequestLoadTexture.empty() == false;
						});

						if (m_isStop == true && m_queueRequestLoadTexture.empty() == true)
							return false;

						requestInfo = std::move(m_queueRequestLoadTexture.front());
						m_queueRequestLoadTexture.pop();
					}

					requestInfo.pTexture_out->SetState(IResource::eLoading);

					bool isSuccess = requestInfo.funcLoader(requestInfo.strFilePath.c_str());

					std::lock_guard<std::mutex> lock(m_mutex);
					m_queueCompleteTexture.push({ requestInfo.pTexture_out, isSuccess });
				}
			});
		}

		TextureManager::Impl::~Impl()
		{
			m_isStop = true;
			m_condition.notify_all();
			m_thread.join();

			std::for_each(m_umapTexture.begin(), m_umapTexture.end(), [](std::pair<const ITexture::Key, ITexture*>& iter)
			{
				IResource* pResource = iter.second;
				if (pResource->GetReferenceCount() > 1)
				{
					string::StringID name(iter.second->GetKey());
					LOG_WARNING("failed to reference count managed : refCount[%d], name[%s]", pResource->GetReferenceCount(), name.c_str());
					Sleep(100);
				}

				SafeDelete(pResource);
			});
			m_umapTexture.clear();
		}

		void TextureManager::Impl::Cleanup(float fElapsedTime)
		{
			TRACER_EVENT("TextureManager::Flush");
			m_fTime += fElapsedTime;

			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_queueCompleteTexture.empty() == false)
			{
				while (m_queueCompleteTexture.empty())
				{
					ResultLoadTextureInfo result = m_queueCompleteTexture.front();
					m_queueCompleteTexture.pop();

					if (result.isSuccess == true)
					{
						result.pTexture_out->SetState(IResource::eComplete);
						result.pTexture_out->SetAlive(true);
					}
					else
					{
						result.pTexture_out->SetState(IResource::eInvalid);
						result.pTexture_out->SetAlive(false);
					}
				}
			}

			if (m_fTime > 10.f)
			{
				m_fTime = 0.f;

				auto iter = m_umapTexture.begin();
				while (iter != m_umapTexture.end())
				{
					ITexture* pTexture = iter->second;

					if (pTexture->GetState() == IResource::eReady ||
						pTexture->GetState() == IResource::eLoading)
					{
						++iter;
					}
					else if (pTexture->GetReferenceCount() > 0)
					{
						pTexture->SetAlive(true);
						++iter;
					}
					else
					{
						if (pTexture->IsAlive() == true)
						{
							pTexture->SubtractLife();
							++iter;
						}
						else
						{
							IResource* pResource = pTexture;
							SafeDelete(pResource);
							iter = m_umapTexture.erase(iter);
						}
					}
				}
			}
		}

		void TextureManager::Impl::AsyncLoadTexture(ITexture* pTexture, const char* strFilePath, std::function<bool(const std::string&)> funcLoad)
		{
			PushTexture(pTexture);
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_queueRequestLoadTexture.push({ strFilePath, pTexture, funcLoad });
			}
			m_condition.notify_all();
		}

		void TextureManager::Impl::PushTexture(ITexture* pTexture)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			pTexture->SetAlive(true);
			m_umapTexture.emplace(pTexture->GetKey(), pTexture);
		}

		ITexture* TextureManager::Impl::GetTexture(const ITexture::Key& key)
		{
			auto iter = m_umapTexture.find(key);
			if (iter == m_umapTexture.end())
				return nullptr;

			return iter->second;
		}

		TextureManager::TextureManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		TextureManager::~TextureManager()
		{
		}

		void TextureManager::Cleanup(float fElapsedTime)
		{
			m_pImpl->Cleanup(fElapsedTime);
		}

		void TextureManager::AsyncLoadTexture(ITexture* pTexture, const char* strFilePath, std::function<bool(const std::string&)> funcLoad)
		{
			m_pImpl->AsyncLoadTexture(pTexture, strFilePath, funcLoad);
		}

		void TextureManager::PushTexture(ITexture* pTexture)
		{
			m_pImpl->PushTexture(pTexture);
		}

		ITexture* TextureManager::GetTexture(const ITexture::Key& key)
		{
			return m_pImpl->GetTexture(key);
		}
	}
}