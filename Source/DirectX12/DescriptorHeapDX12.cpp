#include "stdafx.h"
#include "DescriptorHeapDX12.h"

#include "DeviceDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			DescriptorHeap::DescriptorHeap(uint32_t nPersistentCount, uint32_t nTemporaryCount, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isShaderVisible, const wchar_t* wstrDebugName)
			{
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				const uint32_t nTotalDescriptorCount = nPersistentCount + nTemporaryCount;
				assert(nTotalDescriptorCount > 0);

				m_nPersistentCount = nPersistentCount;
				m_nTemporaryCount = nTemporaryCount;
				m_heapType = heapType;
				m_isShaderVisible = isShaderVisible;
				if (heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
				{
					m_isShaderVisible = false;
				}

				m_nHeapCount = m_isShaderVisible ? eFrameBufferCount : 1;

				m_vecDeadList.resize(nPersistentCount);
				for (uint32_t i = 0; i < nPersistentCount; ++i)
				{
					m_vecDeadList[i] = i;
				}

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
				heapDesc.NumDescriptors = nTotalDescriptorCount;
				heapDesc.Type = heapType;
				if (m_isShaderVisible == true)
				{
					heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				}
				else
				{
					heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				}

				for (uint32_t i = 0; i < m_nHeapCount; ++i)
				{
					if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pDescriptorHeaps[i]))))
					{
						throw_line("failed to create descriptor heap");
					}
					m_pDescriptorHeaps[i]->SetName(wstrDebugName);

					m_startCPUHandle[i] = m_pDescriptorHeaps[i]->GetCPUDescriptorHandleForHeapStart();

					if (m_isShaderVisible == true)
					{
						m_startGPUHandle[i] = m_pDescriptorHeaps[i]->GetGPUDescriptorHandleForHeapStart();
					}
				}

				m_nDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(heapType);
			}

			DescriptorHeap::~DescriptorHeap()
			{
				assert(m_nAllocatedPersistentCount == 0);
				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					SafeRelease(m_pDescriptorHeaps[i]);
				}
			}

			PersistentDescriptorAlloc DescriptorHeap::AllocatePersistent()
			{
				assert(m_pDescriptorHeaps[0] != nullptr);

				uint32_t nIndex = eInvalidDescriptorIndex;

				{
					thread::SRWWriteLock writeLock(&m_srwLock);

					assert(m_nAllocatedPersistentCount < m_nPersistentCount);
					nIndex = m_vecDeadList[m_nAllocatedPersistentCount];
					++m_nAllocatedPersistentCount;
				}

				PersistentDescriptorAlloc alloc;
				alloc.nIndex = nIndex;
				for (uint32_t i = 0; i < m_nHeapCount; ++i)
				{
					alloc.cpuHandles[i] = m_startCPUHandle[i];
					alloc.cpuHandles[i].ptr += nIndex * m_nDescriptorSize;
				}

				return alloc;
			}

			void DescriptorHeap::FreePersistent(uint32_t& nIndex)
			{
				if (nIndex == eInvalidDescriptorIndex)
					return;

				assert(nIndex < m_nPersistentCount);
				assert(m_pDescriptorHeaps[0] != nullptr);

				{
					thread::SRWWriteLock writeLock(&m_srwLock);

					assert(m_nAllocatedPersistentCount > 0);
					m_vecDeadList[m_nAllocatedPersistentCount - 1] = nIndex;
					--m_nAllocatedPersistentCount;
				}

				nIndex = eInvalidDescriptorIndex;
			}

			void DescriptorHeap::FreePersistent(D3D12_CPU_DESCRIPTOR_HANDLE& handle)
			{
				assert(m_nHeapCount == 1);
				if (handle.ptr != 0)
				{
					uint32_t nIndex = IndexFromHandle(handle);
					FreePersistent(nIndex);
					handle = {};
				}
			}

			void DescriptorHeap::FreePersistent(D3D12_GPU_DESCRIPTOR_HANDLE& handle)
			{
				assert(m_nHeapCount == 1);
				if (handle.ptr != 0)
				{
					uint32_t nIndex = IndexFromHandle(handle);
					FreePersistent(nIndex);
					handle = {};
				}
			}
			
			TempDescriptorAlloc DescriptorHeap::AllocateTemporary(uint32_t nCount)
			{
				assert(m_pDescriptorHeaps[0] != nullptr);
				assert(nCount > 0);

				uint32_t nTempIndex = uint32_t(InterlockedAdd64(&m_nAllocatedTemporaryCount, nCount)) - nCount;
				assert(nTempIndex < m_nTemporaryCount);

				uint32_t nFinalIndex = nTempIndex + m_nPersistentCount;

				TempDescriptorAlloc alloc;
				alloc.startCPUHandle = m_startCPUHandle[m_nHeapIndex];
				alloc.startCPUHandle.ptr += nFinalIndex * m_nDescriptorSize;
				alloc.startGPUHandle = m_startGPUHandle[m_nHeapIndex];
				alloc.startGPUHandle.ptr += nFinalIndex * m_nDescriptorSize;
				alloc.nStartIndex = nFinalIndex;

				return alloc;
			}

			void DescriptorHeap::EndFrame()
			{
				assert(m_pDescriptorHeaps[0] != nullptr);
				m_nAllocatedTemporaryCount = 0;
				m_nHeapIndex = (m_nHeapIndex + 1) % m_nHeapCount;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetStartCPUHandle(uint32_t nFrameIndex) const
			{
				return m_startCPUHandle[nFrameIndex];
			}

			D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetStartGPUHandle(uint32_t nFrameIndex) const
			{
				return m_startGPUHandle[nFrameIndex];
			}

			D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandleFromIndex(uint32_t nDescriptorIndex) const
			{
				return GetCPUHandleFromIndex(nDescriptorIndex, m_nHeapIndex);
			}

			D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandleFromIndex(uint32_t nDescriptorIndex) const
			{
				return GetGPUHandleFromIndex(nDescriptorIndex, m_nHeapIndex);
			}

			D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandleFromIndex(uint32_t nDescriptorIndex, uint32_t nFrameIndex) const
			{
				assert(m_pDescriptorHeaps[0] != nullptr);
				assert(nFrameIndex < m_nHeapCount);
				assert(nDescriptorIndex != eInvalidDescriptorIndex);
				assert(nDescriptorIndex < TotalDescriptorsCount());
				D3D12_CPU_DESCRIPTOR_HANDLE handle = m_startCPUHandle[nFrameIndex];
				handle.ptr += nDescriptorIndex * m_nDescriptorSize;
				return handle;
			}

			D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandleFromIndex(uint32_t nDescriptorIndex, uint32_t nFrameIndex) const
			{
				assert(m_pDescriptorHeaps[0] != nullptr);
				assert(nFrameIndex < m_nHeapCount);
				assert(nDescriptorIndex != eInvalidDescriptorIndex);
				assert(nDescriptorIndex < TotalDescriptorsCount());
				assert(m_isShaderVisible);
				D3D12_GPU_DESCRIPTOR_HANDLE handle = m_startGPUHandle[nFrameIndex];
				handle.ptr += nDescriptorIndex * m_nDescriptorSize;
				return handle;
			}

			uint32_t DescriptorHeap::IndexFromHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle)
			{
				assert(m_pDescriptorHeaps[0] != nullptr);
				assert(handle.ptr >= m_startCPUHandle[m_nHeapIndex].ptr);
				assert(handle.ptr < m_startCPUHandle[m_nHeapIndex].ptr + m_nDescriptorSize * TotalDescriptorsCount());
				assert((handle.ptr - m_startCPUHandle[m_nHeapIndex].ptr) % m_nDescriptorSize == 0);
				return uint32_t(handle.ptr - m_startCPUHandle[m_nHeapIndex].ptr) / m_nDescriptorSize;
			}

			uint32_t DescriptorHeap::IndexFromHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle)
			{
				assert(m_pDescriptorHeaps[0] != nullptr);
				assert(handle.ptr >= m_startGPUHandle[m_nHeapIndex].ptr);
				assert(handle.ptr < m_startGPUHandle[m_nHeapIndex].ptr + m_nDescriptorSize * TotalDescriptorsCount());
				assert((handle.ptr - m_startGPUHandle[m_nHeapIndex].ptr) % m_nDescriptorSize == 0);
				return uint32_t(handle.ptr - m_startGPUHandle[m_nHeapIndex].ptr) / m_nDescriptorSize;
			}

			ID3D12DescriptorHeap* DescriptorHeap::GetHeap() const
			{
				assert(m_pDescriptorHeaps[m_nHeapIndex] != nullptr);
				return m_pDescriptorHeaps[m_nHeapIndex];
			}

			ID3D12DescriptorHeap* DescriptorHeap::GetHeap(uint32_t nFrameIndex) const
			{
				assert(m_pDescriptorHeaps[nFrameIndex] != nullptr);
				return m_pDescriptorHeaps[nFrameIndex];
			}

			uint32_t DescriptorHeap::TotalDescriptorsCount() const
			{
				return m_nPersistentCount + m_nTemporaryCount;
			}
		}
	}
}