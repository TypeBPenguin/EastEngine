#include "stdafx.h"
#include "TextureManager.h"

#include "CommonLib/Timer.h"
#include "CommonLib/FileUtil.h"
#include "CommonLib/ThreadPool.h"

#include "Texture.h"

namespace EastEngine
{
	namespace Graphics
	{
		/*int TextureManager::TextureAtlasNode::s_nFilledPixelCount = 0;
		int TextureManager::TextureAtlasNode::s_nTotalPixelCount = 0;

		TextureManager::TextureAtlasNode::TextureAtlasNode(const Math::Rect& rect)
			: m_nAtlasNodeID(0)
			, m_pLeft(nullptr)
			, m_pRight(nullptr)
			, m_rect(rect)
			, m_isFilled(false)
		{
			static int AtalasNodeID = 0;
			m_nAtlasNodeID = AtalasNodeID++;

			int nAllocatePixelCount = rect.GetWidth() * rect.GetHeight();
			if (nAllocatePixelCount > s_nTotalPixelCount)
			{
				s_nTotalPixelCount = nAllocatePixelCount;
			}
		}

		TextureManager::TextureAtlasNode::~TextureAtlasNode()
		{
			SafeDelete(m_pLeft);
			SafeDelete(m_pRight);
		}

		TextureManager::TextureAtlasNode* TextureManager::TextureAtlasNode::Insert(const Math::Rect& rect)
		{
			if (m_pLeft != nullptr && m_pRight != nullptr)
			{
				TextureAtlasNode* pNode = m_pLeft->Insert(rect);
				if (pNode != nullptr)
					return pNode;

				pNode = m_pRight->Insert(rect);
				if (pNode != nullptr)
					return pNode;

				return nullptr;
			}
			else
			{
				if (m_isFilled == true)
					return nullptr;

				if (m_rect.IsFitsIn(rect) == false)
					return nullptr;

				if (m_rect.IsSameSize(rect) == true)
				{
					s_nFilledPixelCount += rect.GetWidth() * rect.GetHeight();
					m_isFilled = true;
					return this;
				}

				int nRemainWidth = m_rect.GetWidth() - rect.GetWidth();
				int nRemainHeight = m_rect.GetHeight() - rect.GetHeight();

				if (nRemainWidth > nRemainHeight)
				{
					m_pLeft = new TextureAtlasNode(Math::Rect(m_rect.left, m_rect.top, m_rect.left + rect.GetWidth(), m_rect.bottom));
					m_pRight = new TextureAtlasNode(Math::Rect(m_rect.left + rect.GetWidth(), m_rect.top, m_rect.right, m_rect.bottom));
				}
				else
				{
					m_pLeft = new TextureAtlasNode(Math::Rect(m_rect.left, m_rect.top, m_rect.right, m_rect.top + rect.GetHeight()));
					m_pRight = new TextureAtlasNode(Math::Rect(m_rect.left, m_rect.top + rect.GetHeight(), m_rect.right, m_rect.bottom));
				}

				return m_pLeft->Insert(rect);
			}
		}

		bool TextureManager::TextureAtlasNode::Delete(int nAtalasNodeID)
		{
			if (m_nAtlasNodeID == nAtalasNodeID)
			{
				s_nFilledPixelCount -= m_rect.GetWidth() * m_rect.GetHeight();
				m_isFilled = false;
				return true;
			}

			if (m_pLeft != nullptr)
			{
				bool isDeleted = m_pLeft->Delete(nAtalasNodeID);
				if (isDeleted == true)
					return true;
			}

			if (m_pRight != nullptr)
			{
				bool isDeleted = m_pRight->Delete(nAtalasNodeID);
				if (isDeleted == true)
					return true;
			}

			return false;
		}

		TextureManager::TextureAtlasNode* TextureManager::TextureAtlasNode::GetNode(int nAtalasNodeID)
		{
			if (m_nAtlasNodeID == nAtalasNodeID)
				return this;

			if (m_pLeft != nullptr)
			{
				TextureAtlasNode* pNode = m_pLeft->GetNode(nAtalasNodeID);
				if (pNode != nullptr)
					return pNode;
			}

			if (m_pRight != nullptr)
			{
				TextureAtlasNode* pNode = m_pRight->GetNode(nAtalasNodeID);
				if (pNode != nullptr)
					return pNode;
			}

			return nullptr;
		}*/

		TextureManager::TextureManager()
			: m_isInit(false)
			, m_isLoading(false)
			, m_pUvCheckerTexture(nullptr)
			, m_pEmptyTextureRed(nullptr)
			//, m_pTextureAtlas(nullptr)
			//, m_pTextureAtlasNode(nullptr)
		{
		}

		TextureManager::~TextureManager()
		{
			Release();
		}

		bool TextureManager::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strUVCheckerPath = File::GetPath(File::eTexture);
			strUVCheckerPath.append("uv_checker.png");
			m_pUvCheckerTexture = ITexture::Create("uv_checker.png", strUVCheckerPath.c_str(), false);

			std::string strEmptyTexPath = File::GetPath(File::eTexture);
			strEmptyTexPath.append("EmptyTexture_Red.dds");
			m_pEmptyTextureRed = ITexture::Create("EmptyTexture_Red.dds", strEmptyTexPath.c_str(), false);

			//initAtlasPool();

			return true;
		}

		void TextureManager::Release()
		{
			if (m_isInit == false)
				return;

			m_pUvCheckerTexture.reset();
			m_pEmptyTextureRed.reset();

			//m_pTextureAtlas.reset();
			//SafeDelete(m_pTextureAtlasNode);

			for (auto& iter : m_umapTexture)
			{
				std::shared_ptr<ITexture>& pTexture = iter.second;
				pTexture.reset();
			}
			m_umapTexture.clear();

			m_isInit = false;
		}

		void TextureManager::Update(float fElapsedTime)
		{
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

			//while (m_conQueueTextureAtlas.empty() == false)
			//{
			//	std::shared_ptr<ITexture> pTexture;
			//	if (m_conQueueTextureAtlas.try_pop(pTexture) == true)
			//	{
			//		Graphics::TextureDesc2D desc1;
			//		m_pTextureAtlas->GetTexture2D()->GetDesc(&desc1);
			//
			//		Graphics::TextureDesc2D desc2;
			//		pTexture->GetTexture2D()->GetDesc(&desc2);
			//
			//		if (desc1.Format == desc2.Format)
			//		{
			//			Math::Rect rect(pTexture->GetWidth(), pTexture->GetHeight());
			//			TextureAtlasNode* pNode = m_pTextureAtlasNode->Insert(rect);
			//			if (pNode != nullptr)
			//			{
			//				D3D11_BOX box;
			//				box.front = 0;
			//				box.back = 1;
			//				box.left = 0;
			//				box.right = pTexture->GetWidth();
			//				box.top = 0;
			//				box.bottom = pTexture->GetHeight();
			//
			//				const Math::Rect& rectDest = pNode->GetRect();
			//				m_pTextureAtlas->CopySubresourceRegion(0, rectDest.left, rectDest.top, 0, pTexture, 0, &box);
			//
			//				Math::Vector4 f4Rect(rectDest.left, rectDest.top, rectDest.right, rectDest.bottom);
			//				pTexture->SetAtlasRect(f4Rect);
			//			}
			//		}
			//	}
			//}

			//while (m_conQueueTextureAtlasRemove.empty() == false)
			//{
			//	int nAtlasNodeID = -1;
			//	if (m_conQueueTextureAtlasRemove.try_pop(nAtlasNodeID) == true)
			//	{
			//		if (nAtlasNodeID != -1)
			//		{
			//			m_pTextureAtlasNode->Delete(nAtlasNodeID);
			//		}
			//	}
			//}
		}

		void TextureManager::Flush(bool isForceFlush)
		{
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

				if (pTexture->IsAlive() == false || isForceFlush == true)
				{
					pTexture.reset();

					std::lock_guard<std::mutex> lock(m_mutex);
					iter = m_umapTexture.erase(iter);

					continue;
				}

				pTexture->SubtractLife();
				++iter;
			}
		}

		void TextureManager::ProcessRequestTexture(const RequestLoadTextureInfo& loader)
		{
			ResultLoadTextureInfo result;
			result.pTexture_out = loader.pTexture_out;
			result.isSuccess = LoadFromFile(loader.pTexture_out, loader.strName, loader.strFilePath.c_str());

			m_conQueueCompleteTexture.push(result);

			m_isLoading.store(false);
		}
		
		void TextureManager::LoadTextureSync(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const std::string& strFilePath)
		{
			pTexture->SetAlive(true);
			
			m_conQueueRequestLoadTexture.push(RequestLoadTextureInfo(strName, strFilePath, pTexture));
		}

		bool TextureManager::AddTexture(const String::StringID& strFileName, const std::shared_ptr<ITexture>& pTexture)
		{
			if (GetTexture(strFileName) != nullptr)
				return false;

			std::lock_guard<std::mutex> lock(m_mutex);

			auto iter_result = m_umapTexture.emplace(strFileName, pTexture);
			if (iter_result.second == true)
				return true;

			return false;
		}

		const std::shared_ptr<ITexture>& TextureManager::GetTexture(const String::StringID& strFileName)
		{
			auto iter = m_umapTexture.find(strFileName);
			if (iter == m_umapTexture.end())
			{
				static std::shared_ptr<ITexture> pNullptr;
				return pNullptr;
			}

			return iter->second;
		}
		
		bool TextureManager::LoadFromFile(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const char* strFilePath)
		{
			HRESULT hr;
			DirectX::ScratchImage image;

			double dStartTime = Timer::GetInstance()->GetGameTime();

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
				PRINT_LOG("Can't found Texture File : %s", strFilePath);
				return false;
			}

			ID3D11Texture2D* pTexture2D = nullptr;

			if (image.GetMetadata().mipLevels == 1)
			{
				DirectX::ScratchImage mipChain;
				if (FAILED(DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain)))
				{
					PRINT_LOG("Can't GenerateMipMaps : %s", strFilePath);
					return false;
				}

				if (FAILED(DirectX::CreateTexture(GetDevice()->GetInterface(), mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), reinterpret_cast<ID3D11Resource**>(&pTexture2D))))
				{
					PRINT_LOG("Can't CreateTexture : %s", strFilePath);
					return false;
				}

				std::swap(image, mipChain);
			}
			else
			{
				if (FAILED(DirectX::CreateTexture(GetDevice()->GetInterface(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), reinterpret_cast<ID3D11Resource**>(&pTexture2D))))
				{
					PRINT_LOG("Can't CreateTexture : %s", strFilePath);
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
				PRINT_LOG("Can't CreateShaderResourceView : %s", strFilePath);
				return false;
			}

			//if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
			//{
			//	WriteTextureAtlas(pTexture);	
			//}

			double dEndTime = Timer::GetInstance()->GetGameTime();

			PRINT_LOG("Loading Time[%s] : %lf", strFilePath, dEndTime - dStartTime);

			Texture* pRealTexture = static_cast<Texture*>(pTexture.get());
			return pRealTexture->Load(pTexture2D, pShaderResourceView);
		}

		//bool TextureManager::initAtlasPool()
		//{
		//	Graphics::TextureDesc2D desc(DXGI_FORMAT_R8G8B8A8_UNORM, eTextureAtlasWidth, eTextureAtlasHeight);
		//	desc.Build();
		//
		//	m_pTextureAtlas = Graphics::Texture::Create("EastEngine_TextureAtlas", desc, nullptr);
		//
		//	if (m_pTextureAtlas == nullptr)
		//		return false;
		//
		//	m_pTextureAtlasNode = new TextureAtlasNode(Math::Rect(0, 0, eTextureAtlasWidth, eTextureAtlasHeight));
		//
		//	return true;
		//}
	}
}