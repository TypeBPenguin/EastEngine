#include "stdafx.h"
#include "VTFManagerDX12.h"

#include "CommonLib/Lock.h"

#include "UtilDX12.h"

#include "DeviceDX12.h"
#include "TextureDX12.h"

namespace StrID
{
	RegisterStringID(EastEngine_VTF);
}

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			struct VTFBuffer
			{
				std::array<math::Matrix, IVTFManager::eBufferCapacity> buffer;
			};

			class VTFManager::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				bool Allocate(uint32_t nMatrixCount, math::Matrix** ppDest_Out, uint32_t& nVTFID_Out);

			public:
				void EndFrame();

			private:
				thread::Lock m_lock;

				std::array<uint32_t, eFrameBufferCount> m_nAllocatedCount;
				ConstantBuffer<VTFBuffer> m_vtfBuffer;

				uint32_t m_nFrameIndex{ 0 };
			};

			VTFManager::Impl::Impl()
			{
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					if (util::CreateConstantBuffer(pDevice, m_vtfBuffer.AlignedSize(), &m_vtfBuffer.pUploadHeaps[i], L"VTFBuffer") == false)
					{
						throw_line("failed to create constant buffer, VTFBuffer");
					}
				}

				m_vtfBuffer.Initialize(m_vtfBuffer.AlignedSize());
			}

			VTFManager::Impl::~Impl()
			{
				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					SafeRelease(m_vtfBuffer.pUploadHeaps[i]);
				}
			}

			bool VTFManager::Impl::Allocate(uint32_t nMatrixCount, math::Matrix** ppDest_Out, uint32_t& nVTFID_Out)
			{
				thread::AutoLock autoLock(&m_lock);

				//VTFInstance& vtfInstance = m_vtfInstances[m_nFrameIndex];
				//if (vtfInstance.nAllocatedCount + nMatrixCount >= eBufferCapacity)
				if (m_nAllocatedCount[m_nFrameIndex] + nMatrixCount >= eBufferCapacity)
				{
					*ppDest_Out = nullptr;
					nVTFID_Out = eInvalidVTFID;
					return false;
				}

				VTFBuffer* pVTFBuffer = m_vtfBuffer.Cast(m_nFrameIndex);
				*ppDest_Out = &pVTFBuffer->buffer[m_nAllocatedCount[m_nFrameIndex]];
				nVTFID_Out = m_nAllocatedCount[m_nFrameIndex];

				m_nAllocatedCount[m_nFrameIndex] += nMatrixCount;

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

			void VTFManager::EndFrame()
			{
				m_pImpl->EndFrame();
			}
		}
	}
}