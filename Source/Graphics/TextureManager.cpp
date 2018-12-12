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
				std::function<bool(ITexture* pTexture, const std::string&)> funcLoader;

				RequestLoadTextureInfo()
				{
				}

				RequestLoadTextureInfo(const std::string& strFilePath, ITexture* pTexture_out, std::function<bool(ITexture* pTexture, const std::string&)> funcLoader)
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
			void Cleanup(float elapsedTime);

			ITexture* AsyncLoadTexture(std::unique_ptr<IResource> pResource, const char* strFilePath, std::function<bool(ITexture* pTexture, const std::string&)> funcLoad);

		public:
			ITexture* PushTexture(std::unique_ptr<IResource> pResource);
			ITexture* GetTexture(const ITexture::Key& key);

		private:
			float m_fTime{ 0.f };

			std::atomic<bool> m_isStop{ false };

			std::mutex m_mutex;
			std::condition_variable m_condition;

			std::thread m_thread;
			std::queue<RequestLoadTextureInfo> m_queueRequestLoadTexture;
			std::queue<ResultLoadTextureInfo> m_queueCompleteTexture;

			tsl::robin_map<ITexture::Key, std::unique_ptr<IResource>> m_rmapTextures;
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

					const bool isSuccess = requestInfo.funcLoader(requestInfo.pTexture_out, requestInfo.strFilePath.c_str());

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

			for (auto& iter : m_rmapTextures)
			{
				ITexture* pTexture = static_cast<ITexture*>(iter.second.get());
				if (pTexture->GetReferenceCount() > 1)
				{
					const string::StringID name(pTexture->GetKey());
					LOG_WARNING("failed to reference count managed : refCount[%d], name[%s]", pTexture->GetReferenceCount(), name.c_str());
					Sleep(100);
				}
			}
			m_rmapTextures.clear();
		}

		void TextureManager::Impl::Cleanup(float elapsedTime)
		{
			TRACER_EVENT("TextureManager::Flush");
			m_fTime += elapsedTime;

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

				auto iter = m_rmapTextures.begin();
				while (iter != m_rmapTextures.end())
				{
					ITexture* pTexture = static_cast<ITexture*>(iter.value().get());

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
							iter = m_rmapTextures.erase(iter);
						}
					}
				}
			}
		}

		ITexture* TextureManager::Impl::AsyncLoadTexture(std::unique_ptr<IResource> pResource, const char* strFilePath, std::function<bool(ITexture* pTexture, const std::string&)> funcLoad)
		{
			ITexture* pTexture = PushTexture(std::move(pResource));
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_queueRequestLoadTexture.push({ strFilePath, pTexture, funcLoad });
			}
			m_condition.notify_all();

			return pTexture;
		}

		ITexture* TextureManager::Impl::PushTexture(std::unique_ptr<IResource> pResource)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			ITexture* pTexture = static_cast<ITexture*>(pResource.get());
			pTexture->SetAlive(true);
			auto iter_result = m_rmapTextures.emplace(pTexture->GetKey(), std::move(pResource));
			if (iter_result.second == false)
				return nullptr;

			return static_cast<ITexture*>(iter_result.first.value().get());
		}

		ITexture* TextureManager::Impl::GetTexture(const ITexture::Key& key)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto iter = m_rmapTextures.find(key);
			if (iter != m_rmapTextures.end())
				return static_cast<ITexture*>(iter.value().get());

			return nullptr;
		}

		TextureManager::TextureManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		TextureManager::~TextureManager()
		{
		}

		void TextureManager::Cleanup(float elapsedTime)
		{
			m_pImpl->Cleanup(elapsedTime);
		}

		ITexture* TextureManager::AsyncLoadTexture(std::unique_ptr<IResource> pResource, const char* strFilePath, std::function<bool(ITexture* pTexture, const std::string&)> funcLoad)
		{
			return m_pImpl->AsyncLoadTexture(std::move(pResource), strFilePath, funcLoad);
		}

		ITexture* TextureManager::PushTexture(std::unique_ptr<IResource> pResource)
		{
			return m_pImpl->PushTexture(std::move(pResource));
		}

		ITexture* TextureManager::GetTexture(const ITexture::Key& key)
		{
			return m_pImpl->GetTexture(key);
		}
	}
}