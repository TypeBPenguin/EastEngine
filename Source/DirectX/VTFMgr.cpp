#include "stdafx.h"
#include "VTFMgr.h"

#include "D3DInterface.h"

namespace StrID
{
	RegisterStringID(EastEngine_VTF);
}

namespace EastEngine
{
	namespace Graphics
	{
		VTFManager::VTFManager()
			: m_bInit(false)
			, m_nAllocatedCount(0)
			, m_pVTFBuffer(nullptr)
			, m_pVTF(nullptr)
		{
		}

		VTFManager::~VTFManager()
		{
			Release();
		}

		bool VTFManager::Init()
		{
			if (m_bInit == true)
				return true;

			m_bInit = true;

			m_pVTFBuffer = new Math::Matrix[eBufferCapacity];

			TextureDesc2D desc;
			desc.MipLevels = 1;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.ArraySize = 1;
			desc.Width = eTextureWidth;
			desc.Height = eTextureWidth;
			desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			desc.SetSRVDesc();

			D3D11_SUBRESOURCE_DATA srd;
			srd.pSysMem = m_pVTFBuffer;
			srd.SysMemPitch = eTextureWidth * (sizeof(Math::Vector4));
			srd.SysMemSlicePitch = 1;

			m_pVTF = ITexture::Create(StrID::EastEngine_VTF, desc, &srd);
			if (m_pVTF == nullptr)
				return false;

			return true;
		}

		void VTFManager::Release()
		{
			if (m_bInit == false)
				return;

			SafeDeleteArray(m_pVTFBuffer);
			m_pVTF.reset();

			m_bInit = false;
		}

		bool VTFManager::Process()
		{
			if (m_nAllocatedCount == 0)
				return true;

			char* pData = nullptr;
			if (m_pVTF->Map(0, D3D11_MAP_WRITE_DISCARD, reinterpret_cast<void**>(&pData)) == false)
				return false;

			const uint32_t nDestSize = sizeof(Math::Matrix) * eBufferCapacity;

			Memory::Copy(pData, nDestSize, m_pVTFBuffer, sizeof(Math::Matrix) * m_nAllocatedCount);

			m_pVTF->Unmap(0);

			return true;
		}
	}
}