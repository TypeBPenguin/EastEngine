#include "stdafx.h"
#include "DescriptorHeapDX12.h"

#include "DeviceDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			DescriptorHeap::DescriptorHeap(uint32_t persistentCount, uint32_t temporaryCount, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isShaderVisible, const wchar_t* wstrDebugName)
			{
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				const uint32_t totalDescriptorCount = persistentCount + temporaryCount;
				assert(totalDescriptorCount > 0);

				m_persistentCount = persistentCount;
				m_temporaryCount = temporaryCount;
				m_heapType = heapType;
				m_isShaderVisible = isShaderVisible;
				if (heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
				{
					m_isShaderVisible = false;
				}

				m_deadList.resize(persistentCount);
				for (uint32_t i = 0; i < persistentCount; ++i)
				{
					m_deadList[i] = i;
				}

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
				heapDesc.NumDescriptors = totalDescriptorCount;
				heapDesc.Type = heapType;
				if (m_isShaderVisible == true)
				{
					heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				}
				else
				{
					heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				}

				if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pDescriptorHeap))))
				{
					throw_line("failed to create descriptor heap");
				}
				m_pDescriptorHeap->SetName(wstrDebugName);

				m_startCPUHandle = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

				if (m_isShaderVisible == true)
				{
					m_startGPUHandle = m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
				}

				m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(heapType);
			}

			DescriptorHeap::~DescriptorHeap()
			{
				assert(m_allocatedPersistentCount == 0);
				SafeRelease(m_pDescriptorHeap);
			}

			PersistentDescriptorAlloc DescriptorHeap::AllocatePersistent()
			{
				assert(m_pDescriptorHeap != nullptr);

				uint32_t index = eInvalidDescriptorIndex;
				{
					thread::SRWWriteLock writeLock(&m_srwLock);

					assert(m_allocatedPersistentCount < m_persistentCount);
					index = m_deadList[m_allocatedPersistentCount];
					++m_allocatedPersistentCount;
				}

				PersistentDescriptorAlloc alloc;
				alloc.index = index;
				alloc.cpuHandle = m_startCPUHandle;
				alloc.cpuHandle.ptr += index * m_descriptorSize;

				return alloc;
			}

			void DescriptorHeap::FreePersistent(uint32_t& index)
			{
				if (index == eInvalidDescriptorIndex)
					return;

				assert(index < m_persistentCount);
				assert(m_pDescriptorHeap != nullptr);

				{
					thread::SRWWriteLock writeLock(&m_srwLock);

					assert(m_allocatedPersistentCount > 0);
					m_deadList[m_allocatedPersistentCount - 1] = index;
					--m_allocatedPersistentCount;
				}

				index = eInvalidDescriptorIndex;
			}

			void DescriptorHeap::FreePersistent(D3D12_CPU_DESCRIPTOR_HANDLE& handle)
			{
				if (handle.ptr != 0)
				{
					uint32_t index = IndexFromHandle(handle);
					FreePersistent(index);
					handle = {};
				}
			}

			void DescriptorHeap::FreePersistent(D3D12_GPU_DESCRIPTOR_HANDLE& handle)
			{
				if (handle.ptr != 0)
				{
					uint32_t index = IndexFromHandle(handle);
					FreePersistent(index);
					handle = {};
				}
			}
			
			TempDescriptorAlloc DescriptorHeap::AllocateTemporary(uint32_t nCount)
			{
				assert(m_pDescriptorHeap != nullptr);
				assert(nCount > 0);

				uint32_t nTempIndex = uint32_t(InterlockedAdd64(&m_allocatedTemporaryCount, nCount)) - nCount;
				assert(nTempIndex < m_temporaryCount);

				uint32_t nFinalIndex = nTempIndex + m_persistentCount;

				TempDescriptorAlloc alloc;
				alloc.startCPUHandle = m_startCPUHandle;
				alloc.startCPUHandle.ptr += nFinalIndex * m_descriptorSize;
				alloc.startGPUHandle = m_startGPUHandle;
				alloc.startGPUHandle.ptr += nFinalIndex * m_descriptorSize;
				alloc.startIndex = nFinalIndex;

				return alloc;
			}

			void DescriptorHeap::EndFrame()
			{
				assert(m_pDescriptorHeap != nullptr);
				m_allocatedTemporaryCount = 0;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandleFromIndex(uint32_t descriptorIndex) const
			{
				assert(m_pDescriptorHeap != nullptr);
				assert(descriptorIndex != eInvalidDescriptorIndex);
				assert(descriptorIndex < TotalDescriptorsCount());
				D3D12_CPU_DESCRIPTOR_HANDLE handle = m_startCPUHandle;
				handle.ptr += descriptorIndex * m_descriptorSize;
				return handle;
			}

			D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandleFromIndex(uint32_t descriptorIndex) const
			{
				assert(m_pDescriptorHeap != nullptr);
				assert(descriptorIndex != eInvalidDescriptorIndex);
				assert(descriptorIndex < TotalDescriptorsCount());
				assert(m_isShaderVisible);
				D3D12_GPU_DESCRIPTOR_HANDLE handle = m_startGPUHandle;
				handle.ptr += descriptorIndex * m_descriptorSize;
				return handle;
			}

			uint32_t DescriptorHeap::IndexFromHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle)
			{
				assert(m_pDescriptorHeap != nullptr);
				assert(handle.ptr >= m_startCPUHandle.ptr);
				assert(handle.ptr < m_startCPUHandle.ptr + m_descriptorSize * TotalDescriptorsCount());
				assert((handle.ptr - m_startCPUHandle.ptr) % m_descriptorSize == 0);
				return uint32_t(handle.ptr - m_startCPUHandle.ptr) / m_descriptorSize;
			}

			uint32_t DescriptorHeap::IndexFromHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle)
			{
				assert(m_pDescriptorHeap != nullptr);
				assert(handle.ptr >= m_startGPUHandle.ptr);
				assert(handle.ptr < m_startGPUHandle.ptr + m_descriptorSize * TotalDescriptorsCount());
				assert((handle.ptr - m_startGPUHandle.ptr) % m_descriptorSize == 0);
				return uint32_t(handle.ptr - m_startGPUHandle.ptr) / m_descriptorSize;
			}

			ID3D12DescriptorHeap* DescriptorHeap::GetHeap() const
			{
				assert(m_pDescriptorHeap != nullptr);
				return m_pDescriptorHeap;
			}
		}
	}
}