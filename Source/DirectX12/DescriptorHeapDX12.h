#pragma once

#include "CommonLib/Lock.h"

#include "DefineDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			struct PersistentDescriptorAlloc
			{
				std::array<D3D12_CPU_DESCRIPTOR_HANDLE, eFrameBufferCount> cpuHandles{};
				uint32_t nIndex{ eInvalidDescriptorIndex };
			};

			struct TempDescriptorAlloc
			{
				D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle{};
				D3D12_GPU_DESCRIPTOR_HANDLE startGPUHandle{};
				uint32_t nStartIndex{ eInvalidDescriptorIndex };
			};

			class DescriptorHeap
			{
			public:
				DescriptorHeap(uint32_t nPersistentCount, uint32_t nTemporaryCount, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isShaderVisible, const wchar_t* wstrDebugName);
				~DescriptorHeap();

			public:
				PersistentDescriptorAlloc AllocatePersistent();
				void FreePersistent(uint32_t& nIndex);
				void FreePersistent(D3D12_CPU_DESCRIPTOR_HANDLE& handle);
				void FreePersistent(D3D12_GPU_DESCRIPTOR_HANDLE& handle);

				TempDescriptorAlloc AllocateTemporary(uint32_t nCount);
				void EndFrame();

			public:
				uint32_t GetPersistentCount() const { return m_nPersistentCount; }
				uint32_t GetHeapCount() const { return m_nHeapCount; }
				D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return m_heapType; }

				D3D12_CPU_DESCRIPTOR_HANDLE GetStartCPUHandle(int nFrameIndex) const;
				D3D12_GPU_DESCRIPTOR_HANDLE GetStartGPUHandle(int nFrameIndex) const;

				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleFromIndex(uint32_t nDescriptorIdx) const;
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleFromIndex(uint32_t nDescriptorIdx) const;

				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleFromIndex(uint32_t nDescriptorIdx, int nFrameIndex) const;
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleFromIndex(uint32_t nDescriptorIdx, int nFrameIndex) const;

				uint32_t IndexFromHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);
				uint32_t IndexFromHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle);

				ID3D12DescriptorHeap* GetHeap() const;
				uint32_t TotalDescriptorsCount() const;

			private:
				// 3개까지 필요해? 확인필요
				std::array<ID3D12DescriptorHeap*, eFrameBufferCount> m_pDescriptorHeaps{ nullptr };
				uint32_t m_nPersistentCount{ 0 };
				uint32_t m_nAllocatedPersistentCount{ 0 };
				std::vector<uint32_t> m_vecDeadList;

				uint32_t m_nTemporaryCount{ 0 };
				volatile int64_t m_nAllocatedTemporaryCount{ 0 };

				int m_nHeapIndex{ 0 };
				int m_nHeapCount{ 0 };
				uint32_t m_nDescriptorSize{ 0 };
				bool m_isShaderVisible{ false };

				D3D12_DESCRIPTOR_HEAP_TYPE m_heapType{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
				std::array<D3D12_CPU_DESCRIPTOR_HANDLE, eFrameBufferCount> m_startCPUHandle{};
				std::array<D3D12_GPU_DESCRIPTOR_HANDLE, eFrameBufferCount> m_startGPUHandle{};
				thread::Lock m_lock;
			};
		}
	}
}