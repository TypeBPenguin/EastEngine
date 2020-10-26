#pragma once

#include "CommonLib/Lock.h"

#include "DefineDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			struct PersistentDescriptorAlloc
			{
				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
				uint32_t index{ eInvalidDescriptorIndex };
			};

			struct TempDescriptorAlloc
			{
				D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle{};
				D3D12_GPU_DESCRIPTOR_HANDLE startGPUHandle{};
				uint32_t startIndex{ eInvalidDescriptorIndex };
			};

			class DescriptorHeap
			{
			public:
				DescriptorHeap(uint32_t persistentCount, uint32_t temporaryCount, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isShaderVisible, const wchar_t* wstrDebugName);
				~DescriptorHeap();

			public:
				PersistentDescriptorAlloc AllocatePersistent();
				void FreePersistent(uint32_t& index);
				void FreePersistent(D3D12_CPU_DESCRIPTOR_HANDLE& handle);
				void FreePersistent(D3D12_GPU_DESCRIPTOR_HANDLE& handle);

				TempDescriptorAlloc AllocateTemporary(uint32_t nCount);
				void EndFrame();

			public:
				uint32_t GetPersistentCount() const { return m_persistentCount; }
				D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return m_heapType; }

				D3D12_CPU_DESCRIPTOR_HANDLE GetStartCPUHandle() const { return m_startCPUHandle; }
				D3D12_GPU_DESCRIPTOR_HANDLE GetStartGPUHandle() const { return m_startGPUHandle; }

				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleFromIndex(uint32_t descriptorIndex) const;
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleFromIndex(uint32_t descriptorIndex) const;

				uint32_t IndexFromHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);
				uint32_t IndexFromHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle);

				ID3D12DescriptorHeap* GetHeap() const;
				uint32_t TotalDescriptorsCount() const { return m_persistentCount + m_temporaryCount;; }

			private:
				ID3D12DescriptorHeap* m_pDescriptorHeap{ nullptr };
				uint32_t m_persistentCount{ 0 };
				uint32_t m_allocatedPersistentCount{ 0 };
				std::vector<uint32_t> m_deadList;

				uint32_t m_temporaryCount{ 0 };
				volatile int64_t m_allocatedTemporaryCount{ 0 };

				uint32_t m_descriptorSize{ 0 };
				bool m_isShaderVisible{ false };

				D3D12_DESCRIPTOR_HEAP_TYPE m_heapType{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
				D3D12_CPU_DESCRIPTOR_HANDLE m_startCPUHandle{};
				D3D12_GPU_DESCRIPTOR_HANDLE m_startGPUHandle{};

				thread::SRWLock m_srwLock;
			};
		}
	}
}