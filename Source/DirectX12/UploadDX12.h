#pragma once

#include "DefineDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			struct MapResult
			{
				void* pCPUAddress{ nullptr };
				uint64_t nGPUAddress{ 0 };
				uint64_t nResourceOffset{ 0 };
				ID3D12Resource* pResource{ nullptr };
			};

			struct UploadContext
			{
				ID3D12GraphicsCommandList* pCommandList{ nullptr };
				void* pCPUAddress{ nullptr };
				uint64_t nResourceOffset{ 0 };
				ID3D12Resource* pResource{ nullptr };
				void* pSubmission{ nullptr };
			};

			class Uploader
			{
			public:
				Uploader();
				~Uploader();

			public:
				void EndFrame(ID3D12CommandQueue* pCommandQueue);

			public:
				UploadContext BeginResourceUpload(uint64_t size);
				void EndResourceUpload(UploadContext& context);

				MapResult AcquireTempBufferMem(uint64_t nSize, uint64_t nAlignment);

			private:
				enum : uint64_t
				{
					eUploadBufferSize = 32 * 1024 * 1024,
					eMaxUploadSubmissions = 32,

					eTempBufferSize = 2 * 1024 * 1024,
				};

				struct UploadSubmission
				{
					ID3D12CommandAllocator* pCommandAllocator{ nullptr };
					ID3D12GraphicsCommandList1* pCommandList{ nullptr };
					uint64_t nOffset{ 0 };
					uint64_t nSize{ 0 };
					uint64_t nFenceValue{ 0 };
					uint64_t nPadding{ 0 };

					void Reset()
					{
						nOffset = 0;
						nSize = 0;
						nFenceValue = 0;
						nPadding = 0;
					}
				};

			private:
				void ClearFinishedUploads(uint64_t nFlushCount);
				UploadSubmission* AllocUploadSubmission(uint64_t nSize);

			private:
				ID3D12Resource * m_pUploadBuffer{ nullptr };
				uint8_t* m_pUploadBufferCPUAddr{ nullptr };
				SRWLOCK m_uploadSubmissionLock{ SRWLOCK_INIT };
				SRWLOCK m_uploadQueueLock{ SRWLOCK_INIT };

				ID3D12CommandQueue* m_pUploadCommandQueue{ nullptr };
				ID3D12Fence* m_pUploadFence{ nullptr };
				HANDLE m_hFenceEvent{ INVALID_HANDLE_VALUE };
				uint64_t m_nUploadFenceValue{ 0 };

				uint64_t m_nUploadBufferStart{ 0 };
				uint64_t m_nUploadBufferUsed{ 0 };
				std::array<UploadSubmission, eMaxUploadSubmissions> m_uploadSubmissions;
				uint64_t m_nUploadSubmissionStart = 0;
				uint64_t m_nUploadSubmissionUsed{ 0 };

				std::array<ID3D12Resource*, eFrameBufferCount> m_pTempFrameBuffers{ nullptr };
				std::array<uint8_t*, eFrameBufferCount> m_pTempFrameCPUMem{ nullptr };
				std::array<uint64_t, eFrameBufferCount> m_nTempFrameGPUMem{ 0 };
				volatile int64_t m_nTempFrameUsed{ 0 };
			};
		}
	}
}