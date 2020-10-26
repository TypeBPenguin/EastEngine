#include "stdafx.h"
#include "TextureManager.h"

#include "CommonLib/Timer.h"

namespace est
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
				std::wstring filePath;
				TexturePtr pTexture_out{ nullptr };
				std::function<bool(TexturePtr pTexture, const std::wstring&)> funcLoader;

				RequestLoadTextureInfo()
				{
				}

				RequestLoadTextureInfo(const std::wstring& filePath, TexturePtr pTexture_out, std::function<bool(TexturePtr pTexture, const std::wstring&)> funcLoader)
					: filePath(filePath)
					, pTexture_out(pTexture_out)
					, funcLoader(funcLoader)
				{
				}

				RequestLoadTextureInfo(const RequestLoadTextureInfo& source)
				{
					filePath = source.filePath;
					pTexture_out = source.pTexture_out;
					funcLoader = source.funcLoader;
				}
			};

			struct ResultLoadTextureInfo
			{
				TexturePtr pTexture_out{ nullptr };
				bool isSuccess{ false };
			};

		public:
			void Cleanup(float elapsedTime);

			TexturePtr AsyncLoadTexture(const TexturePtr& pTexture, const wchar_t* filePath, std::function<bool(TexturePtr pTexture, const std::wstring&)> funcLoad);

		public:
			TexturePtr PushTexture(const TexturePtr& pTexture);
			TexturePtr GetTexture(const ITexture::Key& key);

		private:
			float m_time{ 0.f };

			std::atomic<bool> m_isStop{ false };

			std::mutex m_mutex;
			std::condition_variable m_condition;

			std::thread m_thread;
			std::queue<RequestLoadTextureInfo> m_queueRequestLoadTexture;
			std::queue<ResultLoadTextureInfo> m_queueCompleteTexture;

			struct TextureData
			{
				TexturePtr pTexture;
				double destroyWaitTime{ 0.0 };
			};
			//tsl::robin_map<ITexture::Key, TextureData> m_rmapTextures;
			std::unordered_map<ITexture::Key, TextureData> m_rmapTextures;
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

					const bool isSuccess = requestInfo.funcLoader(requestInfo.pTexture_out, requestInfo.filePath.c_str());

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

			while (m_queueCompleteTexture.empty() == false)
			{
				m_queueCompleteTexture.pop();
			}

			for (auto& iter : m_rmapTextures)
			{
				if (iter.second.pTexture.use_count() > 1)
				{
					const string::StringID name(iter.second.pTexture->GetKey());
					LOG_WARNING(L"failed to reference count managed : refCount[%d], name[%s]", iter.second.pTexture.use_count(), name.c_str());
					Sleep(100);
				}
			}
			m_rmapTextures.clear();
		}

		void TextureManager::Impl::Cleanup(float elapsedTime)
		{
			TRACER_EVENT(L"TextureManager::Flush");
			m_time += elapsedTime;

			std::lock_guard<std::mutex> lock(m_mutex);

			while (m_queueCompleteTexture.empty() == false)
			{
				ResultLoadTextureInfo result = m_queueCompleteTexture.front();
				m_queueCompleteTexture.pop();

				if (result.isSuccess == true)
				{
					result.pTexture_out->SetState(IResource::eComplete);
				}
				else
				{
					result.pTexture_out->SetState(IResource::eInvalid);
				}
			}

			if (m_time > 10.f)
			{
				m_time = 0.f;

				const double gameTime = Timer::GetInstance()->GetGameTime();

				auto iter = m_rmapTextures.begin();
				while (iter != m_rmapTextures.end())
				{
					//TextureData& data = iter.value();
					TextureData& data = iter->second;
					const TexturePtr& pTexture = data.pTexture;

					if (pTexture->GetState() == IResource::eReady ||
						pTexture->GetState() == IResource::eLoading)
					{
						++iter;
					}
					else
					{
						if (pTexture.use_count() == 1)
						{
							if (math::IsZero(data.destroyWaitTime))
							{
								data.destroyWaitTime = gameTime + 120.0;
								++iter;
							}
							else if (data.destroyWaitTime < gameTime)
							{
								iter = m_rmapTextures.erase(iter);
							}
							else
							{
								++iter;
							}
						}
						else
						{
							data.destroyWaitTime = 0.0;
							++iter;
						}
					}
				}
			}
		}

		TexturePtr TextureManager::Impl::AsyncLoadTexture(const TexturePtr& pTexture, const wchar_t* filePath, std::function<bool(TexturePtr pTexture, const std::wstring&)> funcLoad)
		{
			TexturePtr pTexture_result = PushTexture(pTexture);
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_queueRequestLoadTexture.push({ filePath, pTexture_result, funcLoad });
			}
			m_condition.notify_all();

			return pTexture_result;
		}

		TexturePtr TextureManager::Impl::PushTexture(const TexturePtr& pTexture)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			TextureData data;
			data.pTexture = pTexture;

			auto iter_result = m_rmapTextures.emplace(pTexture->GetKey(), data);
			if (iter_result.second == false)
				return nullptr;

			return iter_result.first->second.pTexture;
		}

		TexturePtr TextureManager::Impl::GetTexture(const ITexture::Key& key)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto iter = m_rmapTextures.find(key);
			if (iter != m_rmapTextures.end())
				return iter->second.pTexture;

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

		TexturePtr TextureManager::AsyncLoadTexture(const TexturePtr& pTexture, const wchar_t* filePath, std::function<bool(TexturePtr pTexture, const std::wstring&)> funcLoad)
		{
			return m_pImpl->AsyncLoadTexture(pTexture, filePath, funcLoad);
		}

		TexturePtr TextureManager::PushTexture(const TexturePtr& pTexture)
		{
			return m_pImpl->PushTexture(pTexture);
		}

		TexturePtr TextureManager::GetTexture(const ITexture::Key& key)
		{
			return m_pImpl->GetTexture(key);
		}
	}
}