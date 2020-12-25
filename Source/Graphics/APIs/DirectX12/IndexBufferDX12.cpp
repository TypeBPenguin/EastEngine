#include "stdafx.h"
#include "IndexBufferDX12.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "UploadDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class IndexBuffer::Impl
			{
			public:
				Impl(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic);
				~Impl();

			public:
				uint32_t GetIndexCount() const { return m_indexCount; }

				bool Map(MappedSubResourceData& mappedSubResourceData, bool isDiscard = true);
				void Unmap();

			public:
				ID3D12Resource* GetBuffer() const { return m_pBuffer; }
				const D3D12_INDEX_BUFFER_VIEW* GetView() const { return &m_view; }

			public:
				ID3D12Resource* m_pBuffer{ nullptr };
				D3D12_INDEX_BUFFER_VIEW m_view;
				uint32_t m_indexCount{ 0 };
			};

			IndexBuffer::Impl::Impl(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic)
				: m_indexCount(indexCount)
			{
				TRACER_EVENT(L"IndexBuffer_Init");
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				const size_t nSize = util::Align(indexCount * formatSize, formatSize);

				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
				CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(nSize);
				HRESULT hr = pDevice->CreateCommittedResource(&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&m_pBuffer));
				if (FAILED(hr))
				{
					throw_line("failed to create index buffer");
				}

				m_pBuffer->SetName(L"Index Buffer Resource Heap");

				ID3D12Resource* pUploadHeap{ nullptr };

				heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				hr = pDevice->CreateCommittedResource(&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&pUploadHeap));
				if (FAILED(hr))
				{
					throw_line("failed to create index upload heap");
				}

				D3D12_SUBRESOURCE_DATA indexData{};
				indexData.pData = pData;
				indexData.RowPitch = indexCount * formatSize;
				indexData.SlicePitch = indexCount * formatSize;

				ID3D12Fence* pFence{ nullptr };
				if (FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence))))
				{
					throw_line("failed to create fence");
				}

				ID3D12CommandAllocator* pCommandAllocator{ nullptr };
				if (FAILED(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator))))
				{
					throw_line("failed to create command allocator");
				}

				ID3D12GraphicsCommandList* pCommandList{ nullptr };
				if (FAILED(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator, nullptr, IID_PPV_ARGS(&pCommandList))))
				{
					throw_line("failed to create command list");
				}

				UpdateSubresources(pCommandList, m_pBuffer, pUploadHeap, 0, 0, 1, &indexData);

				CD3DX12_RESOURCE_BARRIER trasition = CD3DX12_RESOURCE_BARRIER::Transition(m_pBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
				pCommandList->ResourceBarrier(1, &trasition);

				pCommandList->Close();

				ID3D12CommandQueue* pCommandQueue = Device::GetInstance()->GetCommandQueue();

				ID3D12CommandList* ppCommandLists[] = { pCommandList };
				pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
				if (FAILED(pCommandQueue->Signal(pFence, 1)))
				{
					throw_line("failed to signal");
				}

				HANDLE hWaitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

				if (hWaitEvent == nullptr || hWaitEvent == INVALID_HANDLE_VALUE)
				{
					throw_line("failed to create wait event");
				}

				util::WaitForFence(pFence, 1, hWaitEvent);

				if (FAILED(pCommandAllocator->Reset()))
				{
					throw_line("failed to command allocator reset");
				}

				CloseHandle(hWaitEvent);

				SafeRelease(pUploadHeap);
				SafeRelease(pCommandList);
				SafeRelease(pCommandAllocator);
				SafeRelease(pFence);

				m_view.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
				switch (formatSize)
				{
				case sizeof(uint16_t) :
					m_view.Format = DXGI_FORMAT_R16_UINT;
					break;
				case sizeof(uint32_t) :
					m_view.Format = DXGI_FORMAT_R32_UINT;
					break;
				default:
					throw_line("unsupport index format");
					break;
				}
				m_view.SizeInBytes = static_cast<uint32_t>(indexCount * formatSize);
			}

			IndexBuffer::Impl::~Impl()
			{
				util::ReleaseResource(m_pBuffer);
			}

			bool IndexBuffer::Impl::Map(MappedSubResourceData& mappedSubResourceData, bool isDiscard)
			{
				D3D12_RANGE readRange{ 0,0 };
				HRESULT hr = m_pBuffer->Map(0, &readRange, &mappedSubResourceData.pData);
				if (FAILED(hr))
				{
					mappedSubResourceData = {};
					return false;
				}

				return true;
			}

			void IndexBuffer::Impl::Unmap()
			{
				D3D12_RANGE writeRange{ 0,0 };
				m_pBuffer->Unmap(0, &writeRange);
			}

			IndexBuffer::IndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic)
				: m_pImpl{ std::make_unique<Impl>(pData, indexCount, formatSize, isDynamic) }
			{
			}

			IndexBuffer::~IndexBuffer()
			{
			}

			uint32_t IndexBuffer::GetIndexCount() const
			{
				return m_pImpl->GetIndexCount();
			}

			bool IndexBuffer::Map(MappedSubResourceData& mappedSubResourceData, bool isDiscard)
			{
				return m_pImpl->Map(mappedSubResourceData, isDiscard);
			}

			void IndexBuffer::Unmap()
			{
				return m_pImpl->Unmap();
			}

			ID3D12Resource* IndexBuffer::GetBuffer() const
			{
				return m_pImpl->GetBuffer();
			}

			const D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() const
			{
				return m_pImpl->GetView();
			}
		}
	}
}