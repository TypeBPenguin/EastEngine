#include "stdafx.h"
#include "VTFManagerDX11.h"

#include "DeviceDX11.h"
#include "TextureDX11.h"

namespace sid
{
	RegisterStringID(est_VTF);
}

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			VTFManager::VTFManager()
			{
				m_vtfInstance.buffer.resize(eBufferCapacity);

				D3D11_TEXTURE2D_DESC desc{};
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

				D3D11_SUBRESOURCE_DATA initialData;
				initialData.pSysMem = m_vtfInstance.buffer.data();
				initialData.SysMemPitch = eTextureWidth * sizeof(math::float4);
				initialData.SysMemSlicePitch = 1;

				ITexture::Key key(sid::est_VTF);
				m_vtfInstance.pVTF = std::make_unique<Texture>(key);
				m_vtfInstance.pVTF->Initialize(&desc, &initialData);

				m_pPrevVTF = std::make_unique<Texture>(key);
				m_pPrevVTF->Initialize(&desc, &initialData);
			}

			VTFManager::~VTFManager()
			{
			}

			bool VTFManager::Allocate(uint32_t nMatrixCount, math::Matrix** ppDest_Out, uint32_t& nVTFID_Out)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				if (m_vtfInstance.allocatedCount + nMatrixCount >= eBufferCapacity)
				{
					*ppDest_Out = nullptr;
					nVTFID_Out = eInvalidVTFID;
					return false;
				}

				*ppDest_Out = &m_vtfInstance.buffer[m_vtfInstance.allocatedCount];
				nVTFID_Out = m_vtfInstance.allocatedCount;

				m_vtfInstance.allocatedCount += nMatrixCount;

				return true;
			}

			bool VTFManager::Bake()
			{
				TRACER_EVENT(__FUNCTIONW__);
				thread::SRWWriteLock writeLock(&m_srwLock);

				if (m_vtfInstance.allocatedCount == 0)
					return true;

				ID3D11DeviceContext* pDeviceContext = Device::GetInstance()->GetImmediateContext();

				D3D11_MAPPED_SUBRESOURCE mapped{};
				HRESULT hr = pDeviceContext->Map(m_vtfInstance.pVTF->GetTexture2D(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped);
				if (FAILED(hr))
					return false;

				const uint32_t nDestSize = sizeof(math::Matrix) * eBufferCapacity;
				memory::Copy(mapped.pData, nDestSize, m_vtfInstance.buffer.data(), sizeof(math::Matrix) * m_vtfInstance.allocatedCount);

				pDeviceContext->Unmap(m_vtfInstance.pVTF->GetTexture2D(), 0);

				m_vtfInstance.allocatedCount = 0;

				return true;
			}

			void VTFManager::Cleanup()
			{
				std::swap(m_pPrevVTF, m_vtfInstance.pVTF);
			}
		}
	}
}