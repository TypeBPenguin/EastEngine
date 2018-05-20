#include "stdafx.h"
#include "UploadDX12.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			Uploader::Uploader()
			{
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (uint64_t i = 0; i < eMaxUploadSubmissions; ++i)
				{
					UploadSubmission& submission = m_uploadSubmissions[i];

					if (FAILED(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&submission.pCommandAllocator))))
					{
						throw_line("failed to create command allocator");
					}

					if (FAILED(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, submission.pCommandAllocator, nullptr, IID_PPV_ARGS(&submission.pCommandList))))
					{
						throw_line("failed to create command list");
					}

					if (FAILED(submission.pCommandList->Close()))
					{
						throw_line("failed to command list close");
					}
				}

				D3D12_COMMAND_QUEUE_DESC queueDesc{};
				queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
				if (FAILED(pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pUploadCommandQueue))))
				{
					throw_line("failed to create command queue");
				}
				m_pUploadCommandQueue->SetName(L"Upload Copy Queue");

				if (FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pUploadFence))))
				{
					throw_line("failed to create fence");
				}

				m_hFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
				if (m_hFenceEvent == nullptr || m_hFenceEvent == INVALID_HANDLE_VALUE)
				{
					throw_line("failed to create fence event");
				}

				D3D12_RESOURCE_DESC resourceDesc{};
				resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				resourceDesc.Width = static_cast<uint32_t>(eUploadBufferSize);
				resourceDesc.Height = 1;
				resourceDesc.DepthOrArraySize = 1;
				resourceDesc.MipLevels = 1;
				resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
				resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				resourceDesc.SampleDesc.Count = 1;
				resourceDesc.SampleDesc.Quality = 0;
				resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				resourceDesc.Alignment = 0;

				const D3D12_HEAP_PROPERTIES heapProps =
				{
					D3D12_HEAP_TYPE_UPLOAD,
					D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
					D3D12_MEMORY_POOL_UNKNOWN,
					0,
					0,
				};

				if (FAILED(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pUploadBuffer))))
				{
					throw_line("failed to create resource");
				}

				D3D12_RANGE readRange{};
				if (FAILED(m_pUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pUploadBufferCPUAddr))))
				{
					throw_line("failed to map upload buffer");
				}

				// Temporary buffer memory that swaps every frame
				resourceDesc.Width = eTempBufferSize;

				for (uint64_t i = 0; i < eFrameBufferCount; ++i)
				{
					if (FAILED(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
						D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pTempFrameBuffers[i]))))
					{
						throw_line("failed to create temporary buffer");
					}

					if (FAILED(m_pTempFrameBuffers[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pTempFrameCPUMem[i]))))
					{
						throw_line("failed to map temporary buffer");
					}

					m_nTempFrameGPUMem[i] = m_pTempFrameBuffers[i]->GetGPUVirtualAddress();
				}
			}

			Uploader::~Uploader()
			{
				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					SafeRelease(m_pTempFrameBuffers[i]);
				}

				SafeRelease(m_pUploadBuffer);
				SafeRelease(m_pUploadCommandQueue);

				CloseHandle(m_hFenceEvent);
				m_hFenceEvent = INVALID_HANDLE_VALUE;
				SafeRelease(m_pUploadFence);

				for (uint64_t i = 0; i < eMaxUploadSubmissions; ++i)
				{
					SafeRelease(m_uploadSubmissions[i].pCommandAllocator);
					SafeRelease(m_uploadSubmissions[i].pCommandList);
				}
			}

			void Uploader::EndFrame(ID3D12CommandQueue* pCommandQueue)
			{
				if (TryAcquireSRWLockExclusive(&m_uploadSubmissionLock))
				{
					ClearFinishedUploads(0);

					ReleaseSRWLockExclusive(&m_uploadSubmissionLock);
				}

				{
					AcquireSRWLockExclusive(&m_uploadQueueLock);

					ClearFinishedUploads(0);
					if (FAILED(pCommandQueue->Wait(m_pUploadFence, m_nUploadFenceValue)))
					{
						throw_line("failed to command queue wait");
					}

					ReleaseSRWLockExclusive(&m_uploadQueueLock);
				}
			}

			UploadContext Uploader::BeginResourceUpload(uint64_t size)
			{
				size = util::AlignTo(size, 512);

				assert(size <= eUploadBufferSize);
				assert(size > 0);

				UploadSubmission* pSubmission = nullptr;

				{
					AcquireSRWLockExclusive(&m_uploadSubmissionLock);

					ClearFinishedUploads(0);

					pSubmission = AllocUploadSubmission(size);
					while (pSubmission == nullptr)
					{
						ClearFinishedUploads(1);
						pSubmission = AllocUploadSubmission(size);
					}

					ReleaseSRWLockExclusive(&m_uploadSubmissionLock);
				}

				if (FAILED(pSubmission->pCommandAllocator->Reset()))
				{
					throw_line("failed to commnad allocator reset");
				}

				if (FAILED(pSubmission->pCommandList->Reset(pSubmission->pCommandAllocator, nullptr)))
				{
					throw_line("failed to commnad list reset");
				}

				UploadContext context;
				context.pCommandList = pSubmission->pCommandList;
				context.pResource = m_pUploadBuffer;
				context.pCPUAddress = m_pUploadBufferCPUAddr + pSubmission->nOffset;
				context.nResourceOffset = pSubmission->nOffset;
				context.pSubmission = pSubmission;

				return context;
			}

			void Uploader::EndResourceUpload(UploadContext& context)
			{
				assert(context.pCommandList != nullptr);
				assert(context.pSubmission != nullptr);
				UploadSubmission* pSubmission = reinterpret_cast<UploadSubmission*>(context.pSubmission);

				{
					AcquireSRWLockExclusive(&m_uploadQueueLock);

					// Finish off and execute the command list
					if (FAILED(pSubmission->pCommandList->Close()))
					{
						throw_line("failed to command list close");
					}

					ID3D12CommandList* pCommandLists[] = { pSubmission->pCommandList };
					m_pUploadCommandQueue->ExecuteCommandLists(1, pCommandLists);

					++m_nUploadFenceValue;
					if (FAILED(m_pUploadCommandQueue->Signal(m_pUploadFence, m_nUploadFenceValue)))
					{
						throw_line("failed to command queue signal");
					}
					pSubmission->nFenceValue = m_nUploadFenceValue;

					ReleaseSRWLockExclusive(&m_uploadQueueLock);
				}

				context = UploadContext();
			}

			MapResult Uploader::AcquireTempBufferMem(uint64_t nSize, uint64_t nAlignment)
			{
				int nFrameIndex = Device::GetInstance()->GetFrameIndex();

				uint64_t nAllocSize = nSize + nAlignment;
				uint64_t nOffset = InterlockedAdd64(&m_nTempFrameUsed, nAllocSize) - nAllocSize;
				if (nAlignment > 0)
				{
					nOffset = util::AlignTo(nOffset, nAlignment);
				}
				assert(nOffset + nSize <= eTempBufferSize);

				MapResult result;
				result.pCPUAddress = m_pTempFrameCPUMem[nFrameIndex] + nOffset;
				result.nGPUAddress = m_nTempFrameGPUMem[nFrameIndex] + nOffset;
				result.nResourceOffset = nOffset;
				result.pResource = m_pTempFrameBuffers[nFrameIndex];

				return result;
			}

			void Uploader::ClearFinishedUploads(uint64_t nFlushCount)
			{
				const uint64_t nStart = m_nUploadSubmissionStart;
				const uint64_t nUsed = m_nUploadSubmissionUsed;
				for (uint64_t i = 0; i < nUsed; ++i)
				{
					const uint64_t nIndex = (nStart + i) % eMaxUploadSubmissions;
					UploadSubmission& subMission = m_uploadSubmissions[nIndex];
					assert(subMission.nSize > 0);
					assert(m_nUploadBufferUsed >= subMission.nSize);

					// If the submission hasn't been sent to the GPU yet we can't wait for it
					if (subMission.nFenceValue == std::numeric_limits<uint64_t>::max())
						return;

					if (i < nFlushCount)
					{
						util::WaitForFence(m_pUploadFence, subMission.nFenceValue, m_hFenceEvent);
					}

					if (m_pUploadFence->GetCompletedValue() >= subMission.nFenceValue)
					{
						m_nUploadSubmissionStart = (m_nUploadSubmissionStart + 1) % eMaxUploadSubmissions;
						m_nUploadSubmissionUsed -= 1;
						m_nUploadBufferStart = (m_nUploadBufferStart + subMission.nPadding) % eUploadBufferSize;
						assert(subMission.nOffset == m_nUploadBufferStart);
						assert(m_nUploadBufferStart + subMission.nSize <= eUploadBufferSize);
						m_nUploadBufferStart = (m_nUploadBufferStart + subMission.nSize) % eUploadBufferSize;
						m_nUploadBufferUsed -= (subMission.nSize + subMission.nPadding);
						subMission.Reset();

						if (m_nUploadBufferUsed == 0)
						{
							m_nUploadBufferStart = 0;
						}
					}
				}
			}

			Uploader::UploadSubmission* Uploader::AllocUploadSubmission(uint64_t nSize)
			{
				assert(m_nUploadSubmissionUsed <= eMaxUploadSubmissions);

				if (m_nUploadSubmissionUsed == eMaxUploadSubmissions)
					return nullptr;

				const uint64_t nSubmissionIndex = (m_nUploadSubmissionStart + m_nUploadSubmissionUsed) % eMaxUploadSubmissions;
				assert(m_uploadSubmissions[nSubmissionIndex].nSize == 0);

				assert(m_nUploadBufferUsed <= eUploadBufferSize);
				if (nSize > (eUploadBufferSize - m_nUploadBufferUsed))
					return nullptr;

				const uint64_t nStart = m_nUploadBufferStart;
				const uint64_t nEnd = m_nUploadBufferStart + m_nUploadBufferUsed;
				uint64_t nAllocOffset = std::numeric_limits<uint64_t>::max();
				uint64_t nPadding = 0;
				if (nEnd < eUploadBufferSize)
				{
					const uint64_t nEndAmt = eUploadBufferSize - nEnd;
					if (nEndAmt >= nSize)
					{
						nAllocOffset = nEnd;
					}
					else if (nStart >= nSize)
					{
						// Wrap around to the beginning
						nAllocOffset = 0;
						m_nUploadBufferUsed += nEndAmt;
						nPadding = nEndAmt;
					}
				}
				else
				{
					const uint64_t nWrappedEnd = nEnd % eUploadBufferSize;
					if ((nStart - nWrappedEnd) >= nSize)
					{
						nAllocOffset = nWrappedEnd;
					}
				}

				if (nAllocOffset == std::numeric_limits<uint64_t>::max())
					return nullptr;

				m_nUploadSubmissionUsed += 1;
				m_nUploadBufferUsed += nSize;

				UploadSubmission* submission = &m_uploadSubmissions[nSubmissionIndex];
				submission->nOffset = nAllocOffset;
				submission->nSize = nSize;
				submission->nFenceValue = std::numeric_limits<uint64_t>::max();
				submission->nPadding = nPadding;

				return submission;
			}
		}
	}
}