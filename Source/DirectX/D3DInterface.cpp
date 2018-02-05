#include "stdafx.h"
#include "D3DInterface.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

#include "Device.h"

#include "TextureManager.h"
#include "Texture.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "StructuredBuffer.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "SamplerState.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "Material.h"

namespace EastEngine
{
	namespace Graphics
	{
		static boost::object_pool<Material> s_poolMaterial;
		static std::mutex s_mutexMaterial;

		IDevice* GetDevice()
		{
			return Device::GetInstance();
		}

		IDeviceContext* GetImmediateContext()
		{
			return Device::GetInstance()->GetImmediateContext();
		}

		IDeviceContext* GetDeferredContext(ThreadType emThreadType)
		{
			int nThreadID = GetThreadID(emThreadType);
			return Device::GetInstance()->GetDeferredContext(nThreadID);
		}

		IGBuffers* GetGBuffers()
		{
			return Device::GetInstance()->GetGBuffers();
		}

		IImageBasedLight* GetImageBasedLight()
		{
			return Device::GetInstance()->GetImageBasedLight();
		}

		int GetThreadID(ThreadType emThreadType)
		{
			return Device::GetInstance()->GetThreadID(emThreadType);
		}

		D3DProfiler::D3DProfiler(IDeviceContext* pDeviceContext, const wchar_t* str)
			: m_pDeviceContext(pDeviceContext)
		{
			m_pDeviceContext->GetUserDefineAnnotation()->BeginEvent(str);
		}

		D3DProfiler::~D3DProfiler()
		{
			m_pDeviceContext->GetUserDefineAnnotation()->EndEvent();
		}

		D3DProfiler D3DProfiling(IDeviceContext* pDeviceContext, const wchar_t* strBeginEvent)
		{
			return { pDeviceContext, strBeginEvent };
		}

		std::shared_ptr<ITexture> ITexture::Create(const String::StringID& strName, ID3D11Texture2D* pTexture2D, const TextureDesc2D* pCustomDesc2D)
		{
			std::shared_ptr<ITexture> pITexture = TextureManager::GetInstance()->GetTexture(strName);
			if (pITexture != nullptr)
				return pITexture;

			std::shared_ptr<Texture> pTexture = std::static_pointer_cast<Texture>(TextureManager::GetInstance()->AllocateTexture(strName));

			if (pTexture->Load(strName, pTexture2D, pCustomDesc2D) == false)
			{
				pTexture.reset();
				return nullptr;
			}

			pTexture->SetLoadState(EmLoadState::eComplete);

			return pTexture;
		}

		std::shared_ptr<ITexture> ITexture::Create(const String::StringID& strName, const TextureDesc1D& desc, D3D11_SUBRESOURCE_DATA* pData)
		{
			std::shared_ptr<ITexture> pITexture = TextureManager::GetInstance()->GetTexture(strName);
			if (pITexture != nullptr)
				return pITexture;

			std::shared_ptr<Texture> pTexture = std::static_pointer_cast<Texture>(TextureManager::GetInstance()->AllocateTexture(strName));

			if (pTexture->Load(strName, desc, pData) == false)
			{
				pTexture.reset();
				return nullptr;
			}

			pTexture->SetLoadState(EmLoadState::eComplete);

			return pTexture;
		}

		std::shared_ptr<ITexture> ITexture::Create(const String::StringID& strName, const TextureDesc2D& desc, D3D11_SUBRESOURCE_DATA* pData)
		{
			std::shared_ptr<ITexture> pITexture = TextureManager::GetInstance()->GetTexture(strName);
			if (pITexture != nullptr)
				return pITexture;

			std::shared_ptr<Texture> pTexture = std::static_pointer_cast<Texture>(TextureManager::GetInstance()->AllocateTexture(strName));

			if (pTexture->Load(strName, desc, pData) == false)
			{
				pTexture.reset();
				return nullptr;
			}

			pTexture->SetLoadState(EmLoadState::eComplete);

			return pTexture;
		}

		std::shared_ptr<ITexture> ITexture::Create(const String::StringID& strName, const TextureDesc3D& desc, D3D11_SUBRESOURCE_DATA* pData)
		{
			std::shared_ptr<ITexture> pITexture = TextureManager::GetInstance()->GetTexture(strName);
			if (pITexture != nullptr)
				return pITexture;

			std::shared_ptr<Texture> pTexture = std::static_pointer_cast<Texture>(TextureManager::GetInstance()->AllocateTexture(strName));

			if (pTexture->Load(strName, desc, pData) == false)
			{
				pTexture.reset();
				return nullptr;
			}

			pTexture->SetLoadState(EmLoadState::eComplete);

			return pTexture;
		}

		std::shared_ptr<ITexture> ITexture::Create(const std::string& strFilePath, bool isThreadLoad)
		{
			if (strFilePath.empty() == true)
				return nullptr;

			String::StringID strName = strFilePath.c_str();
			std::shared_ptr<ITexture> pITexture = TextureManager::GetInstance()->GetTexture(strName);
			if (pITexture != nullptr)
				return pITexture;

			std::shared_ptr<Texture> pTexture = std::static_pointer_cast<Texture>(TextureManager::GetInstance()->AllocateTexture(strName));

			pTexture->SetName(strName);

			if (isThreadLoad == true)
			{
				TextureManager::GetInstance()->AsyncLoadTexture(pTexture, strName, strFilePath);
			}
			else
			{
				bool isLoaded = TextureManager::GetInstance()->LoadFromFile(pTexture, strName, strFilePath.c_str());

				if (isLoaded == true)
				{
					pTexture->SetLoadState(EmLoadState::eComplete);
					pTexture->SetAlive(true);
				}
				else
				{
					pTexture->SetLoadState(EmLoadState::eInvalid);
					pTexture->SetAlive(false);
				}
			}

			return pTexture;
		}
		
		IVertexBuffer* IVertexBuffer::Create(EmVertexFormat::Type emVertexFormat, size_t nElementCount, const void* pData, D3D11_USAGE emUsage, uint32_t nOptions)
		{
			VertexBuffer* pVertexBuffer = new VertexBuffer;
			if (pVertexBuffer->Init(emVertexFormat, nElementCount, pData, emUsage, nOptions) == false)
			{
				SafeDelete(pVertexBuffer);
				return nullptr;
			}

			return pVertexBuffer;
		}

		IIndexBuffer* IIndexBuffer::Create(size_t nElementCount, const uint32_t* pData, D3D11_USAGE emUsage, uint32_t nOptions)
		{
			IndexBuffer* pIndexBuffer = new IndexBuffer;
			if (pIndexBuffer->Init(nElementCount, pData, emUsage, nOptions) == false)
			{
				SafeDelete(pIndexBuffer);
				return nullptr;
			}

			return pIndexBuffer;
		}

		IStructuredBuffer* IStructuredBuffer::Create(void* pData, size_t nNumElements, size_t nByteStride, bool isEnableCpuWrite, bool isEnableGpuWrite)
		{
			StructuredBuffer* pDataBuffer = new StructuredBuffer;
			if (pDataBuffer->Init(pData, nNumElements, nByteStride, isEnableCpuWrite, isEnableGpuWrite) == false)
			{
				SafeDelete(pDataBuffer);
				return nullptr;
			}

			return pDataBuffer;
		}

		IRenderTarget* IRenderTarget::Create(ID3D11Texture2D* pTexture2D, const RenderTargetDesc2D* pRenderTargetDesc)
		{
			RenderTarget* pRenderTarget = new RenderTarget;
			if (pRenderTarget->Init(pTexture2D, pRenderTargetDesc) == false)
			{
				SafeDelete(pRenderTarget);
				return nullptr;
			}

			return pRenderTarget;
		}

		IRenderTarget* IRenderTarget::Create(const RenderTargetDesc1D& renderTargetDesc)
		{
			RenderTarget* pRenderTarget = new RenderTarget;
			if (pRenderTarget->Init(renderTargetDesc) == false)
			{
				SafeDelete(pRenderTarget);
				return nullptr;
			}

			return pRenderTarget;
		}

		IRenderTarget* IRenderTarget::Create(const RenderTargetDesc2D& renderTargetDesc)
		{
			RenderTarget* pRenderTarget = new RenderTarget;
			if (pRenderTarget->Init(renderTargetDesc) == false)
			{
				SafeDelete(pRenderTarget);
				return nullptr;
			}

			return pRenderTarget;
		}

		IDepthStencil* IDepthStencil::Create(const DepthStencilDesc& depthStencilDesc)
		{
			DepthStencil* pDepthStencil = new DepthStencil;
			if (pDepthStencil->Init(depthStencilDesc) == false)
			{
				SafeDelete(pDepthStencil);
				return nullptr;
			}

			return pDepthStencil;
		}

		ISamplerState* ISamplerState::Create(const SamplerStateDesc& samplerStateDesc)
		{
			ISamplerState* pISamplerState = Device::GetInstance()->GetSamplerState(samplerStateDesc.GetKey());
			if (pISamplerState != nullptr)
				return pISamplerState;

			SamplerState* pSamplerState = new SamplerState;
			if (pSamplerState->Init(samplerStateDesc) == false)
			{
				SafeDelete(pSamplerState);
				return nullptr;
			}

			Device::GetInstance()->AddSamplerState(pSamplerState);

			return pSamplerState;
		}

		IBlendState* IBlendState::Create(const BlendStateDesc& blendStateDesc)
		{
			IBlendState* pIBlendState = Device::GetInstance()->GetBlendState(blendStateDesc.GetKey());
			if (pIBlendState != nullptr)
				return pIBlendState;

			BlendState* pBlendState = new BlendState;
			if (pBlendState->Init(blendStateDesc) == false)
			{
				SafeDelete(pBlendState);
				return nullptr;
			}

			Device::GetInstance()->AddBlendState(pBlendState);

			return pBlendState;
		}

		IRasterizerState* IRasterizerState::Create(const RasterizerStateDesc& rasterizerStateDesc)
		{
			IRasterizerState* pIRasterizerState = Device::GetInstance()->GetRasterizerState(rasterizerStateDesc.GetKey());
			if (pIRasterizerState != nullptr)
				return pIRasterizerState;

			RasterizerState* pRasterizerState = new RasterizerState;
			if (pRasterizerState->Init(rasterizerStateDesc) == false)
			{
				SafeDelete(pRasterizerState);
				return nullptr;
			}

			Device::GetInstance()->AddRasterizerState(pRasterizerState);

			return pRasterizerState;
		}

		IDepthStencilState* IDepthStencilState::Create(const DepthStencilStateDesc& depthStencilStateDesc)
		{
			IDepthStencilState* pIDepthStencilState = Device::GetInstance()->GetDepthStencilState(depthStencilStateDesc.GetKey());
			if (pIDepthStencilState != nullptr)
				return pIDepthStencilState;

			DepthStencilState* pDepthStencilState = new DepthStencilState;
			if (pDepthStencilState->Init(depthStencilStateDesc) == false)
			{
				SafeDelete(pDepthStencilState);
				return nullptr;
			}

			Device::GetInstance()->AddDepthStencilState(pDepthStencilState);

			return pDepthStencilState;
		}

		MaterialInfo::MaterialInfo()
			: colorAlbedo(1.f, 1.f, 1.f, 1.f)
			, colorEmissive(0.f, 0.f, 0.f, 1.f)
			, fStippleTransparencyFactor(0.f)
			, fTessellationFactor(256.f)
			, isAlbedoAlphaChannelMaskMap(false)
			, isVisible(true)
			, emSamplerState(EmSamplerState::eMinMagMipLinearWrap)
			, emBlendState(EmBlendState::eOff)
			, emRasterizerState(EmRasterizerState::eSolidCCW)
			, emDepthStencilState(EmDepthStencilState::eRead_Write_On)
		{
			Clear();
		}

		void MaterialInfo::Clear()
		{
			strName.clear();

			colorAlbedo = Math::Color(1.f, 1.f, 1.f, 1.f);
			colorEmissive = Math::Color(0.f, 0.f, 0.f, 1.f);

			f4PaddingRoughMetEmi = Math::Vector4::Zero;
			f4SurSpecTintAniso = Math::Vector4::Zero;
			f4SheenTintClearcoatGloss = Math::Vector4::Zero;

			fStippleTransparencyFactor = 0.f;
			fTessellationFactor = 256.f;
			isAlbedoAlphaChannelMaskMap = false;
			isVisible = true;
			isInsideTextureForder = true;

			for (auto& strTexName : strTextureNameArray)
			{
				strTexName.clear();
			}
			
			emSamplerState = EmSamplerState::eMinMagMipLinearWrap;
			emBlendState = EmBlendState::eOff;
			emRasterizerState = EmRasterizerState::eSolidCCW;
			emDepthStencilState = EmDepthStencilState::eRead_Write_On;
		}

		IMaterial* IMaterial::Create(const MaterialInfo* pInfo)
		{
			Material* pMaterial = nullptr;
			{
				std::lock_guard<std::mutex> lock(s_mutexMaterial);
				pMaterial = s_poolMaterial.construct();
			}

			if (pMaterial->Init(pInfo) == false)
			{
				SafeDelete(pMaterial);
				return nullptr;
			}

			return pMaterial;
		}

		IMaterial* IMaterial::Create(const String::StringID& strName)
		{
			Material* pMaterial = nullptr;
			{
				std::lock_guard<std::mutex> lock(s_mutexMaterial);
				pMaterial = s_poolMaterial.construct();
			}

			if (pMaterial->Init(strName) == false)
			{
				SafeDelete(pMaterial);
				return nullptr;
			}

			return pMaterial;
		}

		IMaterial* IMaterial::Clone(const IMaterial* pSource)
		{
			Material* pMaterial = nullptr;
			{
				std::lock_guard<std::mutex> lock(s_mutexMaterial);
				pMaterial = s_poolMaterial.construct();
			}

			if (pMaterial->Init(pSource) == false)
			{
				SafeDelete(pMaterial);
				return nullptr;
			}

			return pMaterial;
		}

		IMaterial* IMaterial::Create(const char* strFileName, const char* strFilePath)
		{
			std::string strFullPath(strFilePath);
			strFullPath.append(strFileName);

			File::FileStream file;
			if (file.Open(strFullPath.c_str(), File::EmState::eRead | File::EmState::eBinary) == false)
			{
				LOG_WARNING("아 실패함");
				return nullptr;
			}

			MaterialInfo materialInfo;

			std::string strBuf;
			file >> strBuf;

			materialInfo.strName = strBuf.c_str();
			materialInfo.strPath = strFilePath;

			file.Read(&materialInfo.colorAlbedo.r, 4);
			file.Read(&materialInfo.colorEmissive.r, 4);

			file.Read(&materialInfo.f4PaddingRoughMetEmi.x, 4);
			file.Read(&materialInfo.f4SurSpecTintAniso.x, 4);
			file.Read(&materialInfo.f4SheenTintClearcoatGloss.x, 4);

			file >> materialInfo.isVisible;
			file >> materialInfo.fStippleTransparencyFactor;
			file >> materialInfo.fTessellationFactor;
			file >> materialInfo.isAlbedoAlphaChannelMaskMap;

			for (int i = 0; i < EmMaterial::TypeCount; ++i)
			{
				file >> strBuf;

				if (strBuf != "None")
				{
					materialInfo.strTextureNameArray[i] = strBuf.c_str();
				}
			}

			file >> *reinterpret_cast<int*>(&materialInfo.emSamplerState);
			file >> *reinterpret_cast<int*>(&materialInfo.emBlendState);
			file >> *reinterpret_cast<int*>(&materialInfo.emRasterizerState);
			file >> *reinterpret_cast<int*>(&materialInfo.emDepthStencilState);

			file.Close();

			return IMaterial::Create(&materialInfo);
		}

		void IMaterial::Destroy(IMaterial** ppMaterial)
		{
			if (ppMaterial == nullptr || *ppMaterial == nullptr)
				return;

			std::lock_guard<std::mutex> lock(s_mutexMaterial);

			int nRefCount = (*ppMaterial)->DecreaseReference();
			if (nRefCount <= 0)
			{
				Material* pMaterial = static_cast<Material*>(*ppMaterial);
				s_poolMaterial.destroy(pMaterial);
			}

			*ppMaterial = nullptr;
		}

		bool IMaterial::SaveToFile(IMaterial* pMaterial, const char* strFilePath)
		{
			std::string strFullPath(strFilePath);
			strFullPath.append(pMaterial->GetName().c_str());
			strFullPath.append(".emtl");

			File::FileStream file;
			if (file.Open(strFullPath.c_str(), File::EmState::eWrite | File::EmState::eBinary) == false)
			{
				LOG_WARNING("아 실패함");
				return false;
			}

			file << pMaterial->GetName().c_str();

			file.Write(&pMaterial->GetAlbedoColor().r, 4);
			file.Write(&pMaterial->GetEmissiveColor().r, 4);

			file.Write(&pMaterial->GetPaddingRoughMetEmi().x, 4);
			file.Write(&pMaterial->GetSurSpecTintAniso().x, 4);
			file.Write(&pMaterial->GetSheenTintClearcoatGloss().x, 4);

			file << pMaterial->IsVisible();
			file << pMaterial->GetStippleTransparencyFactor();
			file << pMaterial->GetTessellationFactor();
			file << pMaterial->IsAlbedoAlphaChannelMaskMap();

			for (int i = 0; i < EmMaterial::TypeCount; ++i)
			{
				const String::StringID& strName = pMaterial->GetTextureName(static_cast<EmMaterial::Type>(i));
				if (strName.empty() == true)
				{
					file << "None";
				}
				else
				{
					file << strName.c_str();
				}
			}

			file << pMaterial->GetSamplerState();
			file << pMaterial->GetBlendState();
			file << pMaterial->GetRasterizerState();
			file << pMaterial->GetDepthStencilState();

			file.Close();

			return true;
		}
	}
}