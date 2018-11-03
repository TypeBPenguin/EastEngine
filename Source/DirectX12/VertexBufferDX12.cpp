#include "stdafx.h"
#include "VertexBufferDX12.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "UploadDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class VertexBuffer::Impl
			{
			public:
				Impl(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount);
				~Impl();

			public:
				uint32_t GetVertexCount() const { return m_nVertexCount; }
				uint32_t GetFormatSize() const { return m_view.StrideInBytes; }

				bool Map(void** ppData);
				void Unmap();

			public:
				ID3D12Resource* GetBuffer() const { return m_pBuffer; }
				const D3D12_VERTEX_BUFFER_VIEW* GetView() const { return &m_view; }

			private:
				ID3D12Resource* m_pBuffer{ nullptr };
				D3D12_VERTEX_BUFFER_VIEW m_view;

				uint32_t m_nVertexCount{ 0 };
			};

			VertexBuffer::Impl::Impl(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount)
				: m_nVertexCount(nVertexCount)
			{
				TRACER_EVENT("VertexBuffer_Init");
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
				CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(nBufferSize);
				HRESULT hr = pDevice->CreateCommittedResource(&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&m_pBuffer));
				if (FAILED(hr))
				{
					throw_line("failed to create vertex buffer");
				}

				m_pBuffer->SetName(L"Vertex Buffer Resource Heap");

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
					throw_line("failed to create vertex upload heap");
				}

				D3D12_SUBRESOURCE_DATA vertexData{};
				vertexData.pData = pData;
				vertexData.RowPitch = nBufferSize;
				vertexData.SlicePitch = nBufferSize;

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

				UpdateSubresources(pCommandList, m_pBuffer, pUploadHeap, 0, 0, 1, &vertexData);

				CD3DX12_RESOURCE_BARRIER trasition = CD3DX12_RESOURCE_BARRIER::Transition(m_pBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
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
				m_view.StrideInBytes = static_cast<uint32_t>(nBufferSize / nVertexCount);
				m_view.SizeInBytes = static_cast<uint32_t>(nBufferSize);
			}

			VertexBuffer::Impl::~Impl()
			{
				util::ReleaseResource(m_pBuffer);
				m_pBuffer = nullptr;
			}

			bool VertexBuffer::Impl::Map(void** ppData)
			{
				if (ppData == nullptr)
					return false;

				D3D12_RANGE readRange{ 0,0 };
				HRESULT hr = m_pBuffer->Map(0, &readRange, ppData);
				if (FAILED(hr))
				{
					(*ppData) = nullptr;
					return false;
				}

				return true;
			}

			void VertexBuffer::Impl::Unmap()
			{
				D3D12_RANGE readRange{ 0,0 };
				m_pBuffer->Unmap(0, &readRange);
			}

			VertexBuffer::VertexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount)
				: m_pImpl{ std::make_unique<Impl>(pData, nBufferSize, nVertexCount) }
			{
			}

			VertexBuffer::~VertexBuffer()
			{
			}

			bool VertexBuffer::Map(void** ppData)
			{
				return m_pImpl->Map(ppData);
			}

			void VertexBuffer::Unmap()
			{
				m_pImpl->Unmap();
			}

			uint32_t VertexBuffer::GetVertexCount() const
			{
				return m_pImpl->GetVertexCount();
			}

			uint32_t VertexBuffer::GetFormatSize() const
			{
				return m_pImpl->GetFormatSize();
			}

			ID3D12Resource* VertexBuffer::GetBuffer() const
			{
				return m_pImpl->GetBuffer();
			}

			const D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::GetView() const
			{
				return m_pImpl->GetView();
			}
		}
	}
}