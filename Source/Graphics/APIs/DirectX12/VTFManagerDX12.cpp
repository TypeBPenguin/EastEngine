#include "stdafx.h"
#include "VTFManagerDX12.h"

#include "CommonLib/Lock.h"
#include "Graphics/Interface/ParallelUpdateRender.h"

#include "UtilDX12.h"

#include "DeviceDX12.h"
#include "TextureDX12.h"
#include "UploadDX12.h"

namespace est
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
				bool Allocate(uint32_t matrixCount, math::Matrix** ppDest_Out, uint32_t& vtfID_Out);

			public:
				bool Bake();
				void EndFrame();

			public:
				Texture* GetTexture() const { return m_vtfInstance.pVTFs[m_frameIndex].get(); }
				Texture* GetPrevTexture() const { return m_pPrevVTF.get(); }

			private:
				thread::SRWLock m_srwLock;

				struct VTFInstance
				{
					uint32_t allocatedCount[2]{};

					std::array<std::unique_ptr<Texture>, eFrameBufferCount> pVTFs;
					std::vector<math::Matrix> buffers[2];
				};
				VTFInstance m_vtfInstance;

				std::unique_ptr<Texture> m_pPrevVTF;

				std::unique_ptr<Uploader> m_pUploader;

				uint32_t m_frameIndex{ 0 };
			};

			VTFManager::Impl::Impl()
			{
				const uint64_t bufferSize = util::Align(sizeof(math::Matrix) * static_cast<uint64_t>(eBufferCapacity), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
				m_pUploader = std::make_unique<Uploader>(static_cast<uint32_t>(bufferSize));

				m_vtfInstance.buffers[UpdateThread()].resize(eBufferCapacity);
				m_vtfInstance.buffers[RenderThread()].resize(eBufferCapacity);

				m_vtfInstance.allocatedCount[RenderThread()] = 1;
				EncodeMatrix(m_vtfInstance.buffers[RenderThread()][0]);

				const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32G32B32A32_FLOAT, eTextureWidth, eTextureWidth, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					Texture::Key key(string::Format(L"est_VTF_%d", i));
					m_vtfInstance.pVTFs[i] = std::make_unique<Texture>(key);
					m_vtfInstance.pVTFs[i]->Initialize(&desc);
				}

				Texture::Key key(L"est_VTF_Prev");

				m_pPrevVTF = std::make_unique<Texture>(key);
				m_pPrevVTF->Initialize(&desc);
			}

			VTFManager::Impl::~Impl()
			{
				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_vtfInstance.pVTFs[i].reset();
				}
				m_pPrevVTF.reset();
			}

			bool VTFManager::Impl::Allocate(uint32_t matrixCount, math::Matrix** ppDest_Out, uint32_t& vtfID_Out)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				if (m_vtfInstance.allocatedCount[UpdateThread()] + matrixCount >= eBufferCapacity)
				{
					*ppDest_Out = nullptr;
					vtfID_Out = eInvalidVTFID;
					return false;
				}

				*ppDest_Out = &m_vtfInstance.buffers[UpdateThread()][m_vtfInstance.allocatedCount[UpdateThread()]];
				vtfID_Out = m_vtfInstance.allocatedCount[UpdateThread()];

				m_vtfInstance.allocatedCount[UpdateThread()] += matrixCount;

				return true;
			}

			bool VTFManager::Impl::Bake()
			{
				TRACER_EVENT(__FUNCTIONW__);
				thread::SRWWriteLock writeLock(&m_srwLock);

				if (m_vtfInstance.allocatedCount[RenderThread()] == 0)
					return true;

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
				ID3D12CommandQueue* pCommandQueue = Device::GetInstance()->GetCommandQueue();

				D3D12_RESOURCE_DESC desc = m_vtfInstance.pVTFs[m_frameIndex]->GetResource()->GetDesc();

				D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D{};
				uint32_t numRows = 0;
				uint64_t rowSizes = 0;
				uint64_t nTextureMemSize = 0;
				pDevice->GetCopyableFootprints(&desc, 0, 1, 0, &placedTexture2D, &numRows, &rowSizes, &nTextureMemSize);

				UploadContext uploadContext = m_pUploader->BeginResourceUpload(nTextureMemSize);
				uint8_t* uploadMem = reinterpret_cast<uint8_t*>(uploadContext.pCPUAddress);

				if (m_vtfInstance.pVTFs[m_frameIndex]->GetResourceState() != D3D12_RESOURCE_STATE_COPY_DEST)
				{
					D3D12_RESOURCE_BARRIER transition{};
					m_vtfInstance.pVTFs[m_frameIndex]->Transition(D3D12_RESOURCE_STATE_COPY_DEST, &transition);

					uploadContext.pCommandList->ResourceBarrier(1, &transition);
				}

				uint8_t* dstSubResourceMem = reinterpret_cast<uint8_t*>(uploadMem) + placedTexture2D.Offset;
				memory::Copy(dstSubResourceMem, nTextureMemSize, m_vtfInstance.buffers[RenderThread()].data(), sizeof(math::Matrix) * m_vtfInstance.allocatedCount[RenderThread()]);

				D3D12_TEXTURE_COPY_LOCATION dst{};
				dst.pResource = m_vtfInstance.pVTFs[m_frameIndex]->GetResource();
				dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				dst.SubresourceIndex = 0;

				D3D12_TEXTURE_COPY_LOCATION src{};
				src.pResource = uploadContext.pResource;
				src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				src.PlacedFootprint = placedTexture2D;
				src.PlacedFootprint.Offset += uploadContext.nResourceOffset;

				uploadContext.pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

				if (m_vtfInstance.pVTFs[m_frameIndex]->GetResourceState() != D3D12_RESOURCE_STATE_COMMON)
				{
					D3D12_RESOURCE_BARRIER transition{};
					m_vtfInstance.pVTFs[m_frameIndex]->Transition(D3D12_RESOURCE_STATE_COMMON, &transition);

					uploadContext.pCommandList->ResourceBarrier(1, &transition);
				}

				m_pUploader->EndResourceUpload(uploadContext);
				m_pUploader->EndFrame(pCommandQueue);

				m_vtfInstance.allocatedCount[RenderThread()] = 1;
				EncodeMatrix(m_vtfInstance.buffers[RenderThread()][0]);

				return true;
			}

			void VTFManager::Impl::EndFrame()
			{
				std::swap(m_pPrevVTF, m_vtfInstance.pVTFs[m_frameIndex]);
				m_frameIndex = (m_frameIndex + 1) % eFrameBufferCount;
			}

			VTFManager::VTFManager()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			VTFManager::~VTFManager()
			{
				m_pImpl.reset();
			}

			bool VTFManager::Allocate(uint32_t matrixCount, math::Matrix** ppDest_Out, uint32_t& vtfID_Out)
			{
				return m_pImpl->Allocate(matrixCount, ppDest_Out, vtfID_Out);
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

			Texture* VTFManager::GetPrevTexture() const
			{
				return m_pImpl->GetPrevTexture();
			}
		}
	}
}