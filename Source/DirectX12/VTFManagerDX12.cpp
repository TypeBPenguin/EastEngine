#include "stdafx.h"
#include "VTFManagerDX12.h"

#include "CommonLib/Lock.h"

#include "UtilDX12.h"

#include "DeviceDX12.h"
#include "TextureDX12.h"
#include "UploadDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class VTFManager::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				bool Allocate(uint32_t nMatrixCount, math::Matrix** ppDest_Out, uint32_t& nVTFID_Out);

			public:
				bool Bake();
				void EndFrame();

			public:
				Texture* GetTexture() const { return m_vtfInstance.pVTFs[m_nFrameIndex].get(); }

			private:
				thread::SRWLock m_srwLock;

				struct VTFInstance
				{
					uint32_t nAllocatedCount{ 0 };

					std::array<std::unique_ptr<Texture>, eFrameBufferCount> pVTFs;
					std::vector<math::Matrix> buffer;
				};
				VTFInstance m_vtfInstance;

				std::unique_ptr<Uploader> m_pUploader;

				uint32_t m_nFrameIndex{ 0 };
			};

			VTFManager::Impl::Impl()
			{
				const uint64_t nBufferSize = util::Align(sizeof(math::Matrix) * static_cast<uint64_t>(eBufferCapacity), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
				m_pUploader = std::make_unique<Uploader>(static_cast<uint32_t>(nBufferSize));

				m_vtfInstance.buffer.resize(eBufferCapacity);

				const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32G32B32A32_FLOAT, eTextureWidth, eTextureWidth, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
				
				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					string::StringID strName;
					strName.Format("EastEngine_VTF_%d", i);
					Texture::Key key(strName);

					m_vtfInstance.pVTFs[i] = std::make_unique<Texture>(key);
					m_vtfInstance.pVTFs[i]->Initialize(&desc);
				}
			}

			VTFManager::Impl::~Impl()
			{
				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_vtfInstance.pVTFs[i].reset();
				}
			}

			bool VTFManager::Impl::Allocate(uint32_t nMatrixCount, math::Matrix** ppDest_Out, uint32_t& nVTFID_Out)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				if (m_vtfInstance.nAllocatedCount + nMatrixCount >= eBufferCapacity)
				{
					*ppDest_Out = nullptr;
					nVTFID_Out = eInvalidVTFID;
					return false;
				}

				*ppDest_Out = &m_vtfInstance.buffer[m_vtfInstance.nAllocatedCount];
				nVTFID_Out = m_vtfInstance.nAllocatedCount;

				m_vtfInstance.nAllocatedCount += nMatrixCount;

				return true;
			}

			bool VTFManager::Impl::Bake()
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				if (m_vtfInstance.nAllocatedCount == 0)
					return true;

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
				ID3D12CommandQueue* pCommandQueue = Device::GetInstance()->GetCommandQueue();

				D3D12_RESOURCE_DESC desc = m_vtfInstance.pVTFs[m_nFrameIndex]->GetResource()->GetDesc();

				D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D{};
				uint32_t numRows = 0;
				uint64_t rowSizes = 0;
				uint64_t nTextureMemSize = 0;
				pDevice->GetCopyableFootprints(&desc, 0, 1, 0, &placedTexture2D, &numRows, &rowSizes, &nTextureMemSize);

				UploadContext uploadContext = m_pUploader->BeginResourceUpload(nTextureMemSize);
				uint8_t* uploadMem = reinterpret_cast<uint8_t*>(uploadContext.pCPUAddress);

				if (m_vtfInstance.pVTFs[m_nFrameIndex]->GetResourceState() != D3D12_RESOURCE_STATE_COPY_DEST)
				{
					D3D12_RESOURCE_BARRIER transition{};
					m_vtfInstance.pVTFs[m_nFrameIndex]->Transition(D3D12_RESOURCE_STATE_COPY_DEST, &transition);

					uploadContext.pCommandList->ResourceBarrier(1, &transition);
				}

				uint8_t* dstSubResourceMem = reinterpret_cast<uint8_t*>(uploadMem) + placedTexture2D.Offset;
				Memory::Copy(dstSubResourceMem, nTextureMemSize, m_vtfInstance.buffer.data(), sizeof(math::Matrix) * m_vtfInstance.nAllocatedCount);

				D3D12_TEXTURE_COPY_LOCATION dst{};
				dst.pResource = m_vtfInstance.pVTFs[m_nFrameIndex]->GetResource();
				dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				dst.SubresourceIndex = 0;

				D3D12_TEXTURE_COPY_LOCATION src{};
				src.pResource = uploadContext.pResource;
				src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				src.PlacedFootprint = placedTexture2D;
				src.PlacedFootprint.Offset += uploadContext.nResourceOffset;

				uploadContext.pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

				if (m_vtfInstance.pVTFs[m_nFrameIndex]->GetResourceState() != D3D12_RESOURCE_STATE_COMMON)
				{
					D3D12_RESOURCE_BARRIER transition{};
					m_vtfInstance.pVTFs[m_nFrameIndex]->Transition(D3D12_RESOURCE_STATE_COMMON, &transition);

					uploadContext.pCommandList->ResourceBarrier(1, &transition);
				}

				m_pUploader->EndResourceUpload(uploadContext);
				m_pUploader->EndFrame(pCommandQueue);

				m_vtfInstance.nAllocatedCount = 0;

				return true;
			}

			void VTFManager::Impl::EndFrame()
			{
				m_nFrameIndex = (m_nFrameIndex + 1) % eFrameBufferCount;
			}

			VTFManager::VTFManager()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			VTFManager::~VTFManager()
			{
				m_pImpl.reset();
			}

			bool VTFManager::Allocate(uint32_t nMatrixCount, math::Matrix** ppDest_Out, uint32_t& nVTFID_Out)
			{
				return m_pImpl->Allocate(nMatrixCount, ppDest_Out, nVTFID_Out);
			}

			bool VTFManager::Bake()
			{
				return m_pImpl->Bake();
			}

			void VTFManager::EndFrame()
			{
				m_pImpl->EndFrame();
			}

			Texture* VTFManager::GetTexture() const
			{
				return m_pImpl->GetTexture();
			}
		}
	}
}