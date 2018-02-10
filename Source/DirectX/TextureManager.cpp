#include "stdafx.h"
#include "TextureManager.h"

#include "CommonLib/Performance.h"
#include "CommonLib/FileUtil.h"
#include "CommonLib/ThreadPool.h"

#include "Texture.h"

namespace EastEngine
{
	namespace Graphics
	{
		class TextureManager::Impl
		{
		public:
			Impl();
			~Impl();

		private:
			struct RequestLoadTextureInfo
			{
				String::StringID strName;
				std::string strFilePath;
				std::shared_ptr<ITexture> pTexture_out;

				RequestLoadTextureInfo()
				{
				}

				RequestLoadTextureInfo(const String::StringID& strName, const std::string& strFilePath, const std::shared_ptr<ITexture>& pTexture_out)
					: strName(strName)
					, strFilePath(strFilePath)
					, pTexture_out(pTexture_out)
				{
				}

				RequestLoadTextureInfo(const RequestLoadTextureInfo& source)
				{
					strName = source.strName;
					strFilePath = source.strFilePath;
					pTexture_out = source.pTexture_out;
				}
			};

			struct ResultLoadTextureInfo
			{
				std::shared_ptr<ITexture> pTexture_out = nullptr;
				bool isSuccess = false;
			};

		public:
			void Flush(bool isEnableGarbageCollector);

			void AsyncLoadTexture(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const std::string& strFilePath);

			bool LoadFromFile(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const char* strFilePath);

		public:
			std::shared_ptr<ITexture> AllocateTexture(const ITexture::Key& key);
			std::shared_ptr<ITexture> GetTexture(const ITexture::Key& key);

		public:
			void ProcessRequestTexture(const RequestLoadTextureInfo& loader);

		private:
			bool m_isInitialized{ false };

			std::atomic<bool> m_isLoading{ false };

			std::mutex m_mutex;

			Concurrency::concurrent_queue<RequestLoadTextureInfo> m_conQueueRequestLoadTexture;
			Concurrency::concurrent_queue<ResultLoadTextureInfo> m_conQueueCompleteTexture;
			std::unordered_map<ITexture::Key, std::shared_ptr<ITexture>> m_umapTexture;

			boost::object_pool<Texture> m_poolTexture;
			uint32_t m_nAllocatedTextureCount{ 0 };
		};

		TextureManager::Impl::Impl()
		{
		}

		TextureManager::Impl::~Impl()
		{
			m_conQueueRequestLoadTexture.clear();
			m_conQueueCompleteTexture.clear();
			m_umapTexture.clear();
		}

		void TextureManager::Impl::Flush(bool isEnableGarbageCollector)
		{
			PERF_TRACER_EVENT("TextureManager::Flush", "");
			bool isLoading = m_isLoading.load();
			if (m_conQueueRequestLoadTexture.empty() == false && isLoading == false)
			{
				RequestLoadTextureInfo loader;
				if (m_conQueueRequestLoadTexture.try_pop(loader) == true)
				{
					if (loader.pTexture_out != nullptr)
					{
						m_isLoading.store(true);
						loader.pTexture_out->SetLoadState(EmLoadState::eLoading);

						Thread::CreateTask([this, loader]() { this->ProcessRequestTexture(loader); });
					}
				}
			}

			while (m_conQueueCompleteTexture.empty() == false)
			{
				ResultLoadTextureInfo result;
				if (m_conQueueCompleteTexture.try_pop(result) == true)
				{
					if (result.isSuccess == true)
					{
						result.pTexture_out->SetLoadState(EmLoadState::eComplete);
						result.pTexture_out->SetAlive(true);
					}
					else
					{
						result.pTexture_out->SetLoadState(EmLoadState::eInvalid);
						result.pTexture_out->SetAlive(false);
					}
				}
			}

			if (isEnableGarbageCollector == true)
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				auto iter = m_umapTexture.begin();
				while (iter != m_umapTexture.end())
				{
					std::shared_ptr<ITexture>& pTexture = iter->second;

					if (pTexture->GetLoadState() == EmLoadState::eReady ||
						pTexture->GetLoadState() == EmLoadState::eLoading)
					{
						++iter;
						continue;
					}

					if (pTexture.use_count() > 0)
					{
						pTexture->SetAlive(true);
						++iter;
						continue;
					}

					if (pTexture->IsAlive() == false)
					{
						iter = m_umapTexture.erase(iter);

						continue;
					}

					pTexture->SubtractLife();
					++iter;
				}
			}
		}

		void TextureManager::Impl::AsyncLoadTexture(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const std::string& strFilePath)
		{
			pTexture->SetAlive(true);

			m_conQueueRequestLoadTexture.push(RequestLoadTextureInfo(strName, strFilePath, pTexture));
		}

		std::shared_ptr<ITexture> TextureManager::Impl::AllocateTexture(const ITexture::Key& key)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			++m_nAllocatedTextureCount;

			std::shared_ptr<Texture> pTexture = std::shared_ptr<Texture>(m_poolTexture.construct(key), [&](Texture* pTexture)
			{
				--m_nAllocatedTextureCount;
				m_poolTexture.destroy(pTexture);
			});

			m_umapTexture.emplace(key, pTexture);

			return pTexture;
		}

		std::shared_ptr<ITexture> TextureManager::Impl::GetTexture(const ITexture::Key& key)
		{
			auto iter = m_umapTexture.find(key);
			if (iter == m_umapTexture.end())
				return nullptr;

			return iter->second;
		}

		bool TextureManager::Impl::LoadFromFile(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const char* strFilePath)
		{
			HRESULT hr;
			DirectX::ScratchImage image;

			Performance::Counter counter;
			counter.Start();

			std::string strFileExtension(File::GetFileExtension(strFilePath));

			if (String::IsEqualsNoCase(strFileExtension.c_str(), "dds"))
			{
				hr = DirectX::LoadFromDDSFile(String::MultiToWide(strFilePath).c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
			}
			else if (String::IsEqualsNoCase(strFileExtension.c_str(), "tga"))
			{
				hr = DirectX::LoadFromTGAFile(String::MultiToWide(strFilePath).c_str(), nullptr, image);
			}
			else
			{
				hr = DirectX::LoadFromWICFile(String::MultiToWide(strFilePath).c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
			}

			if (FAILED(hr))
			{
				LOG_WARNING("Can't found Texture File : %s", strFilePath);
				return false;
			}

			ID3D11Texture2D* pTexture2D = nullptr;

			if (image.GetMetadata().mipLevels == 1)
			{
				DirectX::ScratchImage mipChain;
				if (FAILED(DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain)))
				{
					LOG_ERROR("Can't GenerateMipMaps : %s", strFilePath);
					return false;
				}

				if (FAILED(DirectX::CreateTexture(GetDevice()->GetInterface(), mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), reinterpret_cast<ID3D11Resource**>(&pTexture2D))))
				{
					LOG_ERROR("Can't CreateTexture : %s", strFilePath);
					return false;
				}

				std::swap(image, mipChain);
			}
			else
			{
				if (FAILED(DirectX::CreateTexture(GetDevice()->GetInterface(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), reinterpret_cast<ID3D11Resource**>(&pTexture2D))))
				{
					LOG_ERROR("Can't CreateTexture : %s", strFilePath);
					return false;
				}
			}

			GetDevice()->SetDebugName(pTexture2D, strName.c_str());

			const DirectX::TexMetadata& metadata = image.GetMetadata();

			CD3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
			Memory::Clear(&SRVDesc, sizeof(SRVDesc));
			SRVDesc.Format = metadata.format;

			Graphics::TextureDesc2D desc;
			pTexture2D->GetDesc(&desc);

			switch (metadata.dimension)
			{
			case DirectX::TEX_DIMENSION_TEXTURE1D:
				if (metadata.arraySize > 1)
				{
					SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1DARRAY;
					SRVDesc.Texture1DArray.MipLevels = static_cast<UINT>(metadata.mipLevels);
					SRVDesc.Texture1DArray.ArraySize = static_cast<UINT>(metadata.arraySize);
				}
				else
				{
					SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1D;
					SRVDesc.Texture1D.MipLevels = static_cast<UINT>(metadata.mipLevels);
				}
				break;
			case DirectX::TEX_DIMENSION_TEXTURE2D:
				if (metadata.IsCubemap())
				{
					if (metadata.arraySize > 6)
					{
						assert((metadata.arraySize % 6) == 0);
						SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBEARRAY;
						SRVDesc.TextureCubeArray.MipLevels = static_cast<UINT>(metadata.mipLevels);
						SRVDesc.TextureCubeArray.NumCubes = static_cast<UINT>(metadata.arraySize / 6);
					}
					else
					{
						SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
						SRVDesc.TextureCube.MipLevels = static_cast<UINT>(metadata.mipLevels);
					}
				}
				else if (metadata.arraySize > 1)
				{
					SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
					SRVDesc.Texture2DArray.MipLevels = static_cast<UINT>(metadata.mipLevels);
					SRVDesc.Texture2DArray.ArraySize = static_cast<UINT>(metadata.arraySize);
				}
				else
				{
					SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
					SRVDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
				}
				break;
			case DirectX::TEX_DIMENSION_TEXTURE3D:
				assert(metadata.arraySize == 1);
				SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE3D;
				SRVDesc.Texture3D.MipLevels = static_cast<UINT>(metadata.mipLevels);
				break;
			default:
				return false;
			}

			ID3D11ShaderResourceView* pShaderResourceView = nullptr;
			if (FAILED(GetDevice()->CreateShaderResourceView(pTexture2D, &SRVDesc, &pShaderResourceView)))
			{
				LOG_ERROR("Can't CreateShaderResourceView : %s", strFilePath);
				return false;
			}

			//if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
			//{
			//	WriteTextureAtlas(pTexture);	
			//}

			counter.End();

			LOG_MESSAGE("Loading Time[%s] : %lf", strFilePath, counter.Count());

			Texture* pRealTexture = static_cast<Texture*>(pTexture.get());
			return pRealTexture->Load(pTexture2D, pShaderResourceView);
		}

		void TextureManager::Impl::ProcessRequestTexture(const RequestLoadTextureInfo& loader)
		{
			ResultLoadTextureInfo result;
			result.pTexture_out = loader.pTexture_out;
			result.isSuccess = LoadFromFile(loader.pTexture_out, loader.strName, loader.strFilePath.c_str());

			m_conQueueCompleteTexture.push(result);

			m_isLoading.store(false);
		}

		TextureManager::TextureManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		TextureManager::~TextureManager()
		{
		}

		void TextureManager::Flush(bool isEnableGarbageCollector)
		{
			m_pImpl->Flush(isEnableGarbageCollector);
		}

		void TextureManager::AsyncLoadTexture(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const std::string& strFilePath)
		{
			m_pImpl->AsyncLoadTexture(pTexture, strName, strFilePath);
		}

		bool TextureManager::LoadFromFile(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const char* strFilePath)
		{
			return m_pImpl->LoadFromFile(pTexture, strName, strFilePath);
		}

		std::shared_ptr<ITexture> TextureManager::AllocateTexture(const String::StringID& strFileName)
		{
			ITexture::Key key(strFileName.Key());
			return m_pImpl->AllocateTexture(key);
		}

		std::shared_ptr<ITexture> TextureManager::GetTexture(const String::StringID& strFileName)
		{
			ITexture::Key key(strFileName.Key());
			return m_pImpl->GetTexture(key);
		}
	}
}