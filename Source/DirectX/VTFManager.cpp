#include "stdafx.h"
#include "VTFManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		VTFManager::VTFManager()
			: m_isInit(false)
		{
		}

		VTFManager::~VTFManager()
		{
			Release();
		}

		bool VTFManager::Initialize()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			for (size_t i = 0; i < m_vtfInstances.size(); ++i)
			{
				m_vtfInstances[i].buffer.resize(eBufferCapacity);

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
				srd.pSysMem = m_vtfInstances[i].buffer.data();
				srd.SysMemPitch = eTextureWidth * (sizeof(Math::Vector4));
				srd.SysMemSlicePitch = 1;

				String::StringID strName;
				strName.Format("EastEngine_VTF%d", i);
				m_vtfInstances[i].pVTF = ITexture::Create(strName, desc, &srd);
				if (m_vtfInstances[i].pVTF == nullptr)
					return false;
			}

			return true;
		}

		void VTFManager::Release()
		{
			if (m_isInit == false)
				return;

			for (size_t i = 0; i < m_vtfInstances.size(); ++i)
			{
				m_vtfInstances[i].buffer.clear();
				m_vtfInstances[i].buffer.shrink_to_fit();
				m_vtfInstances[i].pVTF.reset();
			}

			m_isInit = false;
		}

		bool VTFManager::Allocate(size_t nMatrixCount, Math::Matrix** ppDest_Out, size_t& nVTFID_Out)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			int nThreadID = GetThreadID(ThreadType::eUpdate);

			if (m_vtfInstances[nThreadID].nAllocatedCount + nMatrixCount >= eBufferCapacity)
			{
				*ppDest_Out = nullptr;
				nVTFID_Out = eInvalidVTFID;
				return false;
			}

			*ppDest_Out = &m_vtfInstances[nThreadID].buffer[m_vtfInstances[nThreadID].nAllocatedCount];
			nVTFID_Out = m_vtfInstances[nThreadID].nAllocatedCount;

			m_vtfInstances[nThreadID].nAllocatedCount += nMatrixCount;

			return true;
		}

		void VTFManager::Synchronize()
		{
			int nThreadID = GetThreadID(ThreadType::eRender);
			if (m_vtfInstances[nThreadID].nAllocatedCount > 0)
			{
				char* pData = nullptr;
				bool isSucceeded = m_vtfInstances[nThreadID].pVTF->Map(ThreadType::eImmediate, 0, D3D11_MAP_WRITE_DISCARD, reinterpret_cast<void**>(&pData));
				if (isSucceeded == true)
				{
					const uint32_t nDestSize = sizeof(Math::Matrix) * eBufferCapacity;

					Memory::Copy(pData, nDestSize, m_vtfInstances[nThreadID].buffer.data(), sizeof(Math::Matrix) * m_vtfInstances[nThreadID].nAllocatedCount);

					m_vtfInstances[nThreadID].pVTF->Unmap(ThreadType::eImmediate, 0);
				}

				m_vtfInstances[nThreadID].nAllocatedCount = 0;
			}
		}

		const std::shared_ptr<ITexture>& VTFManager::GetTexture()
		{
			int nThreadID = GetThreadID(ThreadType::eRender);
			return m_vtfInstances[nThreadID].pVTF;
		}
	}
}