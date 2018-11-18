#include "stdafx.h"
#include "DeviceDX12.h"

#include "CommonLib/Lock.h"

#include "GraphicsInterface/Window.h"

#include "UtilDX12.h"

#include "UploadDX12.h"
#include "GBufferDX12.h"
#include "RenderManagerDX12.h"
#include "DescriptorHeapDX12.h"
#include "VTFManagerDX12.h"

#include "GraphicsInterface/imguiHelper.h"
#include "GraphicsInterface/imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <dxgidebug.h>

namespace StrID
{
	RegisterStringID(DeviceDX12);
}

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class Device::Impl : public Window
			{
			public:
				Impl();
				virtual ~Impl();

			private:
				virtual void Update() override;
				virtual void Render() override;
				virtual void Present() override;

				void RenderImgui();

			public:
				void Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName);
				void Release();

				void Cleanup(float fElapsedTime);

			public:
				RenderTarget* GetRenderTarget(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor, bool isIncludeLastUseRenderTarget = true);
				void ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize = 1, bool isSetLastRenderTarget = true);

				void ReleaseResource(ID3D12DeviceChild* pResource);
				void ReleaseResourceRTV(uint32_t nDescriptorIndex);
				void ReleaseResourceSRV(uint32_t nDescriptorIndex);
				void ReleaseResourceDSV(uint32_t nDescriptorIndex);
				void ReleaseResourceUAV(uint32_t nDescriptorIndex);

			public:
				void MessageHandler(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);
				void EnableShaderBasedValidation();

			public:
				HWND GetHwnd() const { return m_hWnd; }
				HINSTANCE GetHInstance() const { return m_hInstance; }
				const math::uint2& GetScreenSize() const { return m_n2ScreenSize; }
				const D3D12_VIEWPORT* GetViewport() const { return &m_viewport; }
				const math::Rect* GetScissorRect() const { return &m_scissorRect; }

				ID3D12Device* GetInterface() const { return m_pDevice; }
				ID3D12CommandQueue* GetCommandQueue() const { return m_pCommandQueue; }

				ID3D12Fence* GetFence() const { return m_pFences[m_nFrameIndex]; }
				uint64_t GetFenceValue() const { return m_nFenceValues[m_nFrameIndex]; }
				uint32_t GetFrameIndex() const { return m_nFrameIndex; }

				RenderTarget* GetSwapChainRenderTarget(uint32_t nFrameIndex) const { return m_pSwapChainRenderTargets[nFrameIndex].get(); }
				RenderTarget* GetLastUsedRenderTarget() const { return m_pLastUseRenderTarget; }

				GBuffer* GetGBuffer(uint32_t nFrameIndex) const { return m_pGBuffers[nFrameIndex].get(); }
				IImageBasedLight* GetImageBasedLight() const { return m_pImageBasedLight; }
				void SetImageBasedLight(IImageBasedLight* pImageBasedLight) { m_pImageBasedLight = pImageBasedLight; }
				RenderManager* GetRenderManager() const { return m_pRenderManager.get(); }
				VTFManager* GetVTFManager() const { return m_pVTFManager.get(); }

				Uploader* GetUploader() const { return m_pUploader.get(); }

				DescriptorHeap* GetRTVDescriptorHeap() const { return m_pRTVDescriptorHeap.get(); }
				DescriptorHeap* GetSRVDescriptorHeap() const { return m_pSRVDescriptorHeap.get(); }
				DescriptorHeap* GetDSVDescriptorHeap() const { return m_pDSVDescriptorHeap.get(); }
				DescriptorHeap* GetUAVDescriptorHeap() const { return m_pUAVDescriptorHeap.get(); }
				DescriptorHeap* GetSamplerDescriptorHeap() const { return m_pSamplerDescriptorHeap.get(); }

			public:
				size_t GetCommandListCount() const { return m_vecCommandLists.size(); }

				void ResetCommandList(size_t nIndex, ID3D12PipelineState* pPipelineState);
				ID3D12GraphicsCommandList2* GetCommandList(size_t nIndex) const;
				void GetCommandLists(ID3D12GraphicsCommandList2** ppGraphicsCommandLists_out) const;
				void ExecuteCommandList(ID3D12CommandList* pCommandList);
				void ExecuteCommandLists(ID3D12CommandList* const* ppCommandLists, size_t nCount);
				void ExecuteCommandLists(ID3D12GraphicsCommandList2* const* ppGraphicsCommandLists, size_t nCount);

				ID3D12GraphicsCommandList2* CreateBundle(ID3D12PipelineState* pPipelineState);

				const D3D12_DESCRIPTOR_RANGE* GetStandardDescriptorRanges() const { return m_standardDescriptorRangeDescs_SRV.data(); }

			private:
				void InitializeD3D();
				void InitializeSampler();

				void Resize(uint32_t nWidth, uint32_t nHeight);

				void WaitForPreviousFrame();

			private:
				bool m_isInitislized{ false };

				D3D12_VIEWPORT m_viewport{};
				math::Rect m_scissorRect{};

				uint32_t m_nFrameIndex{ 0 };
				HANDLE m_hFenceEvent{ INVALID_HANDLE_VALUE };
				std::array<uint64_t, eFrameBufferCount> m_nFenceValues{ 0 };
				std::array<ID3D12Fence*, eFrameBufferCount> m_pFences{ nullptr };

				ID3D12Debug1* m_pDebug{ nullptr };

				ID3D12Device3* m_pDevice{ nullptr };
				IDXGISwapChain3* m_pSwapChain{ nullptr };
				ID3D12CommandQueue* m_pCommandQueue{ nullptr };

				std::array<std::unique_ptr<RenderTarget>, eFrameBufferCount> m_pSwapChainRenderTargets;

				std::array<std::vector<ID3D12CommandAllocator*>, eFrameBufferCount> m_pCommandAllocators;
				ID3D12CommandAllocator* m_pBundleAllocators{ nullptr };
				std::vector<ID3D12GraphicsCommandList2*> m_vecCommandLists;

				std::array<D3D12_DESCRIPTOR_RANGE, eStandardDescriptorRangesCount_SRV> m_standardDescriptorRangeDescs_SRV{};

				std::array<std::unique_ptr<GBuffer>, eFrameBufferCount> m_pGBuffers;
				IImageBasedLight* m_pImageBasedLight{ nullptr };

				std::unique_ptr<RenderManager> m_pRenderManager;
				std::unique_ptr<VTFManager> m_pVTFManager;

				std::unique_ptr<Uploader> m_pUploader;

				std::unique_ptr<DescriptorHeap> m_pRTVDescriptorHeap;
				std::unique_ptr<DescriptorHeap> m_pSRVDescriptorHeap;
				std::unique_ptr<DescriptorHeap> m_pDSVDescriptorHeap;
				std::unique_ptr<DescriptorHeap> m_pUAVDescriptorHeap;
				std::unique_ptr<DescriptorHeap> m_pSamplerDescriptorHeap;

				thread::SRWLock m_renderTargetLock;

				struct RenderTargetPool
				{
					std::unique_ptr<RenderTarget> pRenderTarget;
					bool isUsing{ false };
					uint32_t nFrameIndex{ 0 };
					float fUnusedTime{ 0.f };

					RenderTargetPool() = default;
					RenderTargetPool(RenderTargetPool&& source) noexcept
						: pRenderTarget(std::move(source.pRenderTarget))
						, isUsing(std::move(source.isUsing))
						, nFrameIndex(std::move(source.nFrameIndex))
						, fUnusedTime(std::move(source.fUnusedTime))
					{
					}
				};
				std::unordered_multimap<RenderTarget::Key, RenderTargetPool> m_ummapRenderTargetPool;
				RenderTarget* m_pLastUseRenderTarget{ nullptr };

				struct ReleaseObject
				{
					enum Type
					{
						eDeviceChild = 0,
						eDescriptorHeapIndex_RTV,
						eDescriptorHeapIndex_SRV,
						eDescriptorHeapIndex_DSV,
						eDescriptorHeapIndex_UAV,
					};

					union
					{
						ID3D12DeviceChild* pDeviceChild;
						uint32_t nDescriptorHeapIndex;
					};
					Type emType{ eDeviceChild };
					uint32_t nFrameIndex{ 0 };

					ReleaseObject(ID3D12DeviceChild* pDeviceChild)
						: pDeviceChild(pDeviceChild)
						, emType(eDeviceChild)
					{
					}

					ReleaseObject(uint32_t nDescriptorHeapIndex, Type emType)
						: nDescriptorHeapIndex(nDescriptorHeapIndex)
						, emType(emType)
					{
					}

					ReleaseObject(ReleaseObject&& source) noexcept
					{
						*this = std::move(source);
					}

					ReleaseObject& operator = (ReleaseObject&& source) noexcept
					{
						pDeviceChild = std::move(source.pDeviceChild);
						emType = std::move(source.emType);
						nFrameIndex = std::move(source.nFrameIndex);
						return *this;
					}
				};
				std::vector<ReleaseObject> m_vecReleaseResources;
				thread::SRWLock m_srwLock_releaseResource;

				uint32_t m_nNullTextureIndex{ eInvalidDescriptorIndex };
				uint32_t m_nImGuiFontSRVIndex{ eInvalidDescriptorIndex };
				std::array<uint32_t, EmSamplerState::TypeCount> m_nSamplerStates{ 0 };
			};

			Device::Impl::Impl()
			{
			}

			Device::Impl::~Impl()
			{
				Release();
			}

			void Device::Impl::Update()
			{
				ImGui_ImplDX12_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
			}

			void Device::Impl::Render()
			{
				TRACER_EVENT(__FUNCTION__);
				WaitForPreviousFrame();

				{
					TRACER_EVENT("CommandAllocatorReset");
					for (auto pCommandAllocator : m_pCommandAllocators[m_nFrameIndex])
					{
						HRESULT hr = pCommandAllocator->Reset();
						if (FAILED(hr))
						{
							throw_line("failed to command allocator reset");
						}
					}
				}

				m_pVTFManager->Bake();

				m_pRenderManager->Render();
			}

			void Device::Impl::Present()
			{
				RenderImgui();

				m_pUploader->EndFrame(m_pCommandQueue);

				HRESULT hr = m_pCommandQueue->Signal(m_pFences[m_nFrameIndex], m_nFenceValues[m_nFrameIndex]);
				if (FAILED(hr))
				{
					throw_line("failed to command queue signal");
				}

				hr = m_pSwapChain->Present(GetOptions().OnVSync ? 1 : 0, 0);
				if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
				{
					LOG_ERROR("Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pDevice->GetDeviceRemovedReason() : hr);

					//HandleDeviceLost();
				}
				else if (FAILED(hr))
				{
					throw_line("failed to swapchain present");
				}

				m_pRTVDescriptorHeap->EndFrame();
				m_pSRVDescriptorHeap->EndFrame();
				m_pDSVDescriptorHeap->EndFrame();
				m_pUAVDescriptorHeap->EndFrame();
				m_pSamplerDescriptorHeap->EndFrame();
				m_pVTFManager->EndFrame();
			}

			void Device::Impl::RenderImgui()
			{
				RenderTarget* pSwapChainRenderTarget = GetSwapChainRenderTarget(m_nFrameIndex);

				ID3D12GraphicsCommandList2* pCommandList = GetCommandList(0);
				ResetCommandList(0, nullptr);

				if (pSwapChainRenderTarget->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
				{
					const D3D12_RESOURCE_BARRIER transition[] =
					{
						pSwapChainRenderTarget->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET),
					};
					pCommandList->ResourceBarrier(_countof(transition), transition);
				}

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
				{
					pSwapChainRenderTarget->GetCPUHandle(),
				};
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					m_pSRVDescriptorHeap->GetHeap(0),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				ImGui::Render();
				ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);

				{
					const D3D12_RESOURCE_BARRIER transition[] =
					{
						pSwapChainRenderTarget->Transition(D3D12_RESOURCE_STATE_PRESENT),
					};
					pCommandList->ResourceBarrier(_countof(transition), transition);
				}

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				ExecuteCommandList(pCommandList);
			}

			void Device::Impl::Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName)
			{
				if (m_isInitislized == true)
					return;

				AddMessageHandler(StrID::DeviceDX12, [&](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
				{
					MessageHandler(hWnd, nMsg, wParam, lParam);
				});

#if defined(DEBUG) || defined(_DEBUG)
				//EnableShaderBasedValidation();
#endif

				InitializeWindow(nWidth, nHeight, isFullScreen, strApplicationTitle, strApplicationName);
				InitializeD3D();

				m_pVTFManager = std::make_unique<VTFManager>();
				m_pRenderManager = std::make_unique<RenderManager>();

				PersistentDescriptorAlloc srvAlloc = m_pSRVDescriptorHeap->AllocatePersistent();
				m_nImGuiFontSRVIndex = srvAlloc.nIndex;
				assert(m_nImGuiFontSRVIndex != eInvalidDescriptorIndex);

				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGui_ImplWin32_Init(m_hWnd);
				ImGui_ImplDX12_Init(m_pDevice, eFrameBufferCount, DXGI_FORMAT_R8G8B8A8_UNORM,
					m_pSRVDescriptorHeap->GetCPUHandleFromIndex(m_nImGuiFontSRVIndex, 0),
					m_pSRVDescriptorHeap->GetGPUHandleFromIndex(m_nImGuiFontSRVIndex, 0));

				m_isInitislized = true;
			}

			void Device::Impl::Release()
			{
				if (m_isInitislized == false)
					return;

				WaitForPreviousFrame();

				HRESULT hr = m_pCommandQueue->Signal(m_pFences[m_nFrameIndex], m_nFenceValues[m_nFrameIndex]);
				if (FAILED(hr))
				{
					throw_line("failed to command queue signal");
				}

				CloseHandle(m_hFenceEvent);
				m_hFenceEvent = INVALID_HANDLE_VALUE;

				ImGui_ImplDX12_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();

				RemoveMessageHandler(StrID::DeviceDX12);

				m_pSRVDescriptorHeap->FreePersistent(m_nImGuiFontSRVIndex);

				m_pRenderManager.reset();
				m_pVTFManager.reset();

				for (uint32_t i = 0; i < EmSamplerState::TypeCount; ++i)
				{
					m_pSamplerDescriptorHeap->FreePersistent(m_nSamplerStates[i]);
				}
				m_nSamplerStates.fill(0);
				m_pSRVDescriptorHeap->FreePersistent(m_nNullTextureIndex);

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pGBuffers[i].reset();
					m_pSwapChainRenderTargets[i].reset();
				}

				m_ummapRenderTargetPool.clear();

				for (auto& releaseObj : m_vecReleaseResources)
				{
					switch (releaseObj.emType)
					{
					case ReleaseObject::eDeviceChild:
						SafeRelease(releaseObj.pDeviceChild);
						break;
					case ReleaseObject::eDescriptorHeapIndex_RTV:
						m_pRTVDescriptorHeap->FreePersistent(releaseObj.nDescriptorHeapIndex);
						break;
					case ReleaseObject::eDescriptorHeapIndex_SRV:
						m_pSRVDescriptorHeap->FreePersistent(releaseObj.nDescriptorHeapIndex);
						break;
					case ReleaseObject::eDescriptorHeapIndex_DSV:
						m_pDSVDescriptorHeap->FreePersistent(releaseObj.nDescriptorHeapIndex);
						break;
					case ReleaseObject::eDescriptorHeapIndex_UAV:
						m_pUAVDescriptorHeap->FreePersistent(releaseObj.nDescriptorHeapIndex);
						break;
					}
				}
				m_vecReleaseResources.clear();

				m_pUploader.reset();

				BOOL fs = FALSE;
				m_pSwapChain->GetFullscreenState(&fs, nullptr);

				if (fs == TRUE)
				{
					m_pSwapChain->SetFullscreenState(FALSE, nullptr);
				}

				for (ID3D12GraphicsCommandList2* pCommandList : m_vecCommandLists)
				{
					SafeRelease(pCommandList);
				}
				m_vecCommandLists.clear();

				SafeRelease(m_pBundleAllocators);

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pSwapChainRenderTargets[i].reset();
					for (ID3D12CommandAllocator* pAllocator : m_pCommandAllocators[i])
					{
						SafeRelease(pAllocator);
					}

					SafeRelease(m_pFences[i]);
				};

				SafeRelease(m_pSwapChain);

				m_pRTVDescriptorHeap.reset();
				m_pSRVDescriptorHeap.reset();
				m_pDSVDescriptorHeap.reset();
				m_pUAVDescriptorHeap.reset();
				m_pSamplerDescriptorHeap.reset();

				SafeRelease(m_pCommandQueue);
				SafeRelease(m_pDebug);

				SafeRelease(m_pDevice);

				util::ReportLiveObjects();

				m_isInitislized = false;
			}

			void Device::Impl::Cleanup(float fElapsedTime)
			{
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_vecReleaseResources.erase(std::remove_if(m_vecReleaseResources.begin(), m_vecReleaseResources.end(), [&](ReleaseObject& releaseObj)
					{
						if (releaseObj.nFrameIndex >= eFrameBufferCount)
						{
							switch (releaseObj.emType)
							{
							case ReleaseObject::eDeviceChild:
								SafeRelease(releaseObj.pDeviceChild);
								break;
							case ReleaseObject::eDescriptorHeapIndex_RTV:
								m_pRTVDescriptorHeap->FreePersistent(releaseObj.nDescriptorHeapIndex);
								break;
							case ReleaseObject::eDescriptorHeapIndex_SRV:
								m_pSRVDescriptorHeap->FreePersistent(releaseObj.nDescriptorHeapIndex);
								break;
							case ReleaseObject::eDescriptorHeapIndex_DSV:
								m_pDSVDescriptorHeap->FreePersistent(releaseObj.nDescriptorHeapIndex);
								break;
							case ReleaseObject::eDescriptorHeapIndex_UAV:
								m_pUAVDescriptorHeap->FreePersistent(releaseObj.nDescriptorHeapIndex);
								break;
							}
							return true;
						}

						++releaseObj.nFrameIndex;
						return false;
					}), m_vecReleaseResources.end());
				}

				m_pRenderManager->Cleanup();

				for (auto iter = m_ummapRenderTargetPool.begin(); iter != m_ummapRenderTargetPool.end();)
				{
					if (iter->second.isUsing == false)
					{
						if (iter->second.nFrameIndex >= eFrameBufferCount)
						{
							iter->second.fUnusedTime += fElapsedTime;

							if (iter->second.fUnusedTime > 30.f)
							{
								iter = m_ummapRenderTargetPool.erase(iter);
								continue;
							}
						}
						else
						{
							++iter->second.nFrameIndex;
						}
					}

					++iter;
				}
			}

			RenderTarget* Device::Impl::GetRenderTarget(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor, bool isIncludeLastUseRenderTarget)
			{
				thread::SRWWriteLock writeLock(&m_renderTargetLock);

				RenderTarget::Key key = RenderTarget::BuildKey(pDesc, clearColor);
				auto iter_range = m_ummapRenderTargetPool.equal_range(key);
				for (auto iter = iter_range.first; iter != iter_range.second; ++iter)
				{
					RenderTargetPool& pool = iter->second;
					if (pool.isUsing == false)
					{
						if (isIncludeLastUseRenderTarget == false &&
							pool.pRenderTarget.get() == m_pLastUseRenderTarget)
							continue;

						pool.isUsing = true;
						pool.nFrameIndex = 0;
						pool.fUnusedTime = 0.f;
						return pool.pRenderTarget.get();
					}
				}

				RenderTargetPool pool;
				pool.pRenderTarget = RenderTarget::Create(pDesc, clearColor);
				if (pool.pRenderTarget == nullptr)
				{
					throw_line("failed to create render target");
				}

				auto iter_result = m_ummapRenderTargetPool.emplace(key, std::move(pool));
				return iter_result->second.pRenderTarget.get();
			}

			void Device::Impl::ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize, bool isSetLastRenderTarget)
			{
				thread::SRWWriteLock writeLock(&m_renderTargetLock);

				for (uint32_t i = 0; i < nSize; ++i)
				{
					if (ppRenderTarget[i] == nullptr)
						continue;

					auto iter_range = m_ummapRenderTargetPool.equal_range(ppRenderTarget[i]->GetKey());
					for (auto iter = iter_range.first; iter != iter_range.second; ++iter)
					{
						if (iter->second.pRenderTarget.get() == ppRenderTarget[i])
						{
							iter->second.isUsing = false;

							if (isSetLastRenderTarget == true)
							{
								m_pLastUseRenderTarget = iter->second.pRenderTarget.get();
							}
							ppRenderTarget[i] = nullptr;
							break;
						}
					}
				}
			}

			void Device::Impl::ReleaseResource(ID3D12DeviceChild* pResource)
			{
				if (pResource != nullptr)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_vecReleaseResources.emplace_back(pResource);
				}
			}

			void Device::Impl::ReleaseResourceRTV(uint32_t nDescriptorIndex)
			{
				if (nDescriptorIndex != eInvalidDescriptorIndex)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_vecReleaseResources.emplace_back(nDescriptorIndex, ReleaseObject::eDescriptorHeapIndex_RTV);
				}
			}

			void Device::Impl::ReleaseResourceSRV(uint32_t nDescriptorIndex)
			{
				if (nDescriptorIndex != eInvalidDescriptorIndex)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_vecReleaseResources.emplace_back(nDescriptorIndex, ReleaseObject::eDescriptorHeapIndex_SRV);
				}
			}

			void Device::Impl::ReleaseResourceDSV(uint32_t nDescriptorIndex)
			{
				if (nDescriptorIndex != eInvalidDescriptorIndex)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_vecReleaseResources.emplace_back(nDescriptorIndex, ReleaseObject::eDescriptorHeapIndex_DSV);
				}
			}

			void Device::Impl::ReleaseResourceUAV(uint32_t nDescriptorIndex)
			{
				if (nDescriptorIndex != eInvalidDescriptorIndex)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_vecReleaseResources.emplace_back(nDescriptorIndex, ReleaseObject::eDescriptorHeapIndex_UAV);
				}
			}

			void Device::Impl::MessageHandler(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
			{
				if (imguiHelper::MessageHandler(hWnd, nMsg, wParam, lParam) == true)
					return;

				switch (nMsg)
				{
				case WM_SIZE:
					if (m_pDevice != nullptr && wParam != SIZE_MINIMIZED)
					{
						const uint32_t nWidth = LOWORD(lParam);
						const uint32_t nHeight = HIWORD(lParam);

						Resize(nWidth, nHeight);
					}
					break;
				}
			}

			void Device::Impl::EnableShaderBasedValidation()
			{
				HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebug));
				if (FAILED(hr))
				{
					throw_line("faile to enable shader based validation");
				}

				m_pDebug->EnableDebugLayer();
				m_pDebug->SetEnableGPUBasedValidation(true);
			}

			void Device::Impl::ResetCommandList(size_t nIndex, ID3D12PipelineState* pPipelineState)
			{
				HRESULT hr = m_vecCommandLists[nIndex]->Reset(m_pCommandAllocators[m_nFrameIndex][nIndex], pPipelineState);
				if (FAILED(hr))
				{
					throw_line("failed to reset command list");
				}
			}

			ID3D12GraphicsCommandList2* Device::Impl::GetCommandList(size_t nIndex) const
			{
				if (nIndex >= m_vecCommandLists.size())
					return nullptr;

				return m_vecCommandLists[nIndex];
			}

			void Device::Impl::GetCommandLists(ID3D12GraphicsCommandList2** ppGraphicsCommandLists_out) const
			{
				const size_t nSize = m_vecCommandLists.size();
				for (size_t i = 0; i < nSize; ++i)
				{
					ppGraphicsCommandLists_out[i] = m_vecCommandLists[i];
				}
			}

			void Device::Impl::ExecuteCommandList(ID3D12CommandList* pCommandList)
			{
				m_pCommandQueue->ExecuteCommandLists(1, &pCommandList);
			}

			void Device::Impl::ExecuteCommandLists(ID3D12CommandList* const* ppCommandLists, size_t nCount)
			{
				m_pCommandQueue->ExecuteCommandLists(static_cast<uint32_t>(nCount), ppCommandLists);
			}

			void Device::Impl::ExecuteCommandLists(ID3D12GraphicsCommandList2* const* ppGraphicsCommandLists, size_t nCount)
			{
				ID3D12CommandList** ppCommandLists = static_cast<ID3D12CommandList**>(_alloca(sizeof(ID3D12CommandList*) * nCount));
				for (size_t i = 0; i < nCount; ++i)
				{
					ppCommandLists[i] = ppGraphicsCommandLists[i];
				}
				m_pCommandQueue->ExecuteCommandLists(static_cast<uint32_t>(nCount), ppCommandLists);
			}

			ID3D12GraphicsCommandList2* Device::Impl::CreateBundle(ID3D12PipelineState* pPipelineState)
			{
				ID3D12GraphicsCommandList2* pBundle = nullptr;
				HRESULT hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_pBundleAllocators, pPipelineState, IID_PPV_ARGS(&pBundle));
				if (FAILED(hr))
				{
					throw_line("failed to create bundle");
				}

				return pBundle;
			}

			void Device::Impl::InitializeD3D()
			{
				uint32_t nDxgiFactoryFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
				IDXGIInfoQueue* pDxgiInfoQueue = nullptr;
				if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDxgiInfoQueue))))
				{
					nDxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

					pDxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
					pDxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
					SafeRelease(pDxgiInfoQueue);
				}
#endif

				IDXGIFactory4* pDxgiFactory{ nullptr };
				HRESULT hr = CreateDXGIFactory2(nDxgiFactoryFlags, IID_PPV_ARGS(&pDxgiFactory));
				if (FAILED(hr))
				{
					throw_line("failed to create dxgi factory");
				}

				IDXGIAdapter1* pAdapter{ nullptr };
				int nAdapterIndex{ 0 };
				bool isAdapterFound{ false };

				while (pDxgiFactory->EnumAdapters1(nAdapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
				{
					DXGI_ADAPTER_DESC1 desc;
					pAdapter->GetDesc1(&desc);

					if (desc.Flags& DXGI_ADAPTER_FLAG_SOFTWARE)
					{
						++nAdapterIndex;
						continue;
					}

					hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
					if (SUCCEEDED(hr))
					{
						isAdapterFound = true;
						break;
					}

					SafeRelease(pAdapter);

					++nAdapterIndex;
				}

				if (isAdapterFound == false)
				{
					throw_line("failed to find GPUs with DirectX 12 support!");
				}

				hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
				if (FAILED(hr))
				{
					throw_line("failed to create d3d device");
				}

				SafeRelease(pAdapter);

				D3D12_COMMAND_QUEUE_DESC cqDesc{};
				cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
				hr = m_pDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&m_pCommandQueue));
				if (FAILED(hr))
				{
					throw_line("failed to create command queue");
				}
				m_pCommandQueue->SetName(L"DX12_CommandQueue");

				m_pRTVDescriptorHeap = std::make_unique<DescriptorHeap>(eDescriptorHeap_Capacity_RTV, 0, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false, L"RTVDescriptorHeap");
				m_pSRVDescriptorHeap = std::make_unique<DescriptorHeap>(eDescriptorHeap_Capacity_SRV, eDescriptorHeap_Capacity_SRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true, L"SRVDescriptorHeap");
				m_pDSVDescriptorHeap = std::make_unique<DescriptorHeap>(eDescriptorHeap_Capacity_DSV, 0, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false, L"DSVDescriptorHeap");
				m_pUAVDescriptorHeap = std::make_unique<DescriptorHeap>(eDescriptorHeap_Capacity_UAV, 0, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false, L"UAVDescriptorHeap");
				m_pSamplerDescriptorHeap = std::make_unique<DescriptorHeap>(EmSamplerState::TypeCount, 0, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true, L"SamplerDescriptorHeap");

				{
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
					srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					srvDesc.Texture2D.MipLevels = 1;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.PlaneSlice = 0;
					srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

					PersistentDescriptorAlloc srvAlloc = m_pSRVDescriptorHeap->AllocatePersistent();
					for (uint32_t i = 0; i < eFrameBufferCount; ++i)
					{
						m_pDevice->CreateShaderResourceView(nullptr, &srvDesc, srvAlloc.cpuHandles[i]);
					}
					m_nNullTextureIndex = srvAlloc.nIndex;
					assert(m_nNullTextureIndex != eInvalidDescriptorIndex);
				}

				DXGI_MODE_DESC backBufferDesc{};
				backBufferDesc.Width = m_n2ScreenSize.x;
				backBufferDesc.Height = m_n2ScreenSize.y;
				backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				DXGI_SWAP_CHAIN_DESC swapChainDesc{};
				swapChainDesc.BufferCount = eFrameBufferCount;
				swapChainDesc.BufferDesc = backBufferDesc;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDesc.OutputWindow = m_hWnd;
				swapChainDesc.SampleDesc = sampleDesc;
				swapChainDesc.Windowed = m_isFullScreen == false;

				IDXGISwapChain* pTempSwapChain{ nullptr };
				hr = pDxgiFactory->CreateSwapChain(m_pCommandQueue, &swapChainDesc, &pTempSwapChain);
				if (FAILED(hr))
				{
					throw_line("failed to create swapchain");
				}

				m_pSwapChain = static_cast<IDXGISwapChain3*>(pTempSwapChain);
				math::Color clearColor;
				m_pSwapChain->GetBackgroundColor(reinterpret_cast<DXGI_RGBA*>(&clearColor));

				m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

				const uint32_t nThreadCount = std::thread::hardware_concurrency();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					ID3D12Resource* pResource = nullptr;
					hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pResource));
					if (FAILED(hr))
					{
						throw_line("failed to create render target");
					}
					m_pSwapChainRenderTargets[i] = RenderTarget::Create(pResource, clearColor);

					m_pCommandAllocators[i].resize(nThreadCount);

					for (uint32_t j = 0; j < nThreadCount; ++j)
					{
						hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocators[i][j]));
						if (FAILED(hr))
						{
							throw_line("failed to create command allocator");
						}
					}
				}

				hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_pBundleAllocators));
				if (FAILED(hr))
				{
					throw_line("failed to create command allocator");
				}

				m_vecCommandLists.resize(nThreadCount);

				for (uint32_t i = 0; i < nThreadCount; ++i)
				{
					hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocators[0][0], nullptr, IID_PPV_ARGS(&m_vecCommandLists[i]));
					if (FAILED(hr))
					{
						throw_line("failed to create command list");
					}

					m_vecCommandLists[i]->Close();
				}

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFences[i]));
					if (FAILED(hr))
					{
						throw_line("failed to create fence");
					}
				}

				m_hFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
				if (m_hFenceEvent == nullptr || m_hFenceEvent == INVALID_HANDLE_VALUE)
				{
					throw_line("failed to create fence event");
				}

				InitializeSampler();

				m_viewport.TopLeftX = 0;
				m_viewport.TopLeftY = 0;
				m_viewport.Width = static_cast<float>(m_n2ScreenSize.x);
				m_viewport.Height = static_cast<float>(m_n2ScreenSize.y);
				m_viewport.MinDepth = 0.f;
				m_viewport.MaxDepth = 1.f;

				m_scissorRect.left = 0;
				m_scissorRect.top = 0;
				m_scissorRect.right = m_n2ScreenSize.x;
				m_scissorRect.bottom = m_n2ScreenSize.y;

				for (uint32_t i = 0; i < eStandardDescriptorRangesCount_SRV; ++i)
				{
					m_standardDescriptorRangeDescs_SRV[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					m_standardDescriptorRangeDescs_SRV[i].NumDescriptors = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					m_standardDescriptorRangeDescs_SRV[i].BaseShaderRegister = 0;
					m_standardDescriptorRangeDescs_SRV[i].RegisterSpace = i;
					m_standardDescriptorRangeDescs_SRV[i].OffsetInDescriptorsFromTableStart = 0;
				}

				m_pUploader = std::make_unique<Uploader>();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pGBuffers[i] = std::make_unique<GBuffer>(m_n2ScreenSize.x, m_n2ScreenSize.y);
				}
			}

			void Device::Impl::InitializeSampler()
			{
				for (uint32_t i = 0; i < EmSamplerState::TypeCount; ++i)
				{
					EmSamplerState::Type emSamplerState = static_cast<EmSamplerState::Type>(i);

					PersistentDescriptorAlloc samplerAlloc = m_pSamplerDescriptorHeap->AllocatePersistent();

					D3D12_SAMPLER_DESC samplerDesc = util::GetSamplerDesc(emSamplerState);
					for (int j = 0; j < eFrameBufferCount; ++j)
					{
						m_pDevice->CreateSampler(&samplerDesc, samplerAlloc.cpuHandles[j]);
					}
					m_nSamplerStates[emSamplerState] = samplerAlloc.nIndex;
				}
			}

			void Device::Impl::Resize(uint32_t nWidth, uint32_t nHeight)
			{
				if (nWidth == m_n2ScreenSize.x && nHeight == m_n2ScreenSize.y)
					return;

				WaitForPreviousFrame();

				ImGui_ImplDX12_InvalidateDeviceObjects();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pSwapChainRenderTargets[i].reset();
				}
				m_ummapRenderTargetPool.clear();
				m_pLastUseRenderTarget = nullptr;

				HRESULT hr = m_pSwapChain->ResizeBuffers(0, nWidth, nHeight, DXGI_FORMAT_UNKNOWN, 0);
				if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
				{
					LOG_WARNING("Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pDevice->GetDeviceRemovedReason() : hr);

					//HandleDeviceLost();

					return;
				}

				math::Color clearColor;
				m_pSwapChain->GetBackgroundColor(reinterpret_cast<DXGI_RGBA*>(&clearColor));

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					ID3D12Resource* pResource = nullptr;
					hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pResource));
					if (FAILED(hr))
					{
						throw_line("failed to create render target");
					}
					m_pSwapChainRenderTargets[i] = RenderTarget::Create(pResource, clearColor);
				}

				DXGI_SWAP_CHAIN_DESC1 desc{};
				if (FAILED(m_pSwapChain->GetDesc1(&desc)))
				{
					throw_line("failed to get DXGI_SWAP_CHAIN_DESC1");
				}

				m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

				m_n2ScreenSize.x = desc.Width;
				m_n2ScreenSize.y = desc.Height;

				m_viewport.TopLeftX = 0.0f;
				m_viewport.TopLeftY = 0.0f;
				m_viewport.Width = static_cast<float>(desc.Width);
				m_viewport.Height = static_cast<float>(desc.Height);
				m_viewport.MinDepth = 0.0f;
				m_viewport.MaxDepth = 1.0f;

				m_scissorRect.left = 0;
				m_scissorRect.top = 0;
				m_scissorRect.right = m_n2ScreenSize.x;
				m_scissorRect.bottom = m_n2ScreenSize.y;

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pGBuffers[i]->Resize(m_n2ScreenSize.x, m_n2ScreenSize.y);
				}

				ImGui_ImplDX12_CreateDeviceObjects();
			}

			void Device::Impl::WaitForPreviousFrame()
			{
				TRACER_EVENT(__FUNCTION__);
				m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

				util::WaitForFence(m_pFences[m_nFrameIndex], m_nFenceValues[m_nFrameIndex], m_hFenceEvent);

				++m_nFenceValues[m_nFrameIndex];
			}

			Device::Device()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			Device::~Device()
			{
			}

			void Device::Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName)
			{
				m_pImpl->Initialize(nWidth, nHeight, isFullScreen, strApplicationTitle, strApplicationName);
			}

			void Device::Run(std::function<void()> funcUpdate)
			{
				m_pImpl->Run(funcUpdate);
			}

			void Device::Cleanup(float fElapsedTime)
			{
				m_pImpl->Cleanup(fElapsedTime);
			}

			RenderTarget* Device::GetRenderTarget(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor, bool isIncludeLastUseRenderTarget)
			{
				return m_pImpl->GetRenderTarget(pDesc, clearColor, isIncludeLastUseRenderTarget);
			}

			void Device::ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize, bool isSetLastRenderTarget)
			{
				m_pImpl->ReleaseRenderTargets(ppRenderTarget, nSize, isSetLastRenderTarget);
			}

			void Device::ReleaseResource(ID3D12DeviceChild* pResource)
			{
				m_pImpl->ReleaseResource(pResource);
			}

			void Device::ReleaseResourceRTV(uint32_t nDescriptorIndex)
			{
				m_pImpl->ReleaseResourceRTV(nDescriptorIndex);
			}

			void Device::ReleaseResourceSRV(uint32_t nDescriptorIndex)
			{
				m_pImpl->ReleaseResourceSRV(nDescriptorIndex);
			}

			void Device::ReleaseResourceDSV(uint32_t nDescriptorIndex)
			{
				m_pImpl->ReleaseResourceDSV(nDescriptorIndex);
			}

			void Device::ReleaseResourceUAV(uint32_t nDescriptorIndex)
			{
				m_pImpl->ReleaseResourceUAV(nDescriptorIndex);
			}

			HWND Device::GetHwnd() const
			{
				return m_pImpl->GetHwnd();
			}

			HINSTANCE Device::GetHInstance() const
			{
				return m_pImpl->GetHInstance();
			}

			void Device::AddMessageHandler(const string::StringID& strName, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler)
			{
				m_pImpl->AddMessageHandler(strName, funcHandler);
			}

			void Device::RemoveMessageHandler(const string::StringID& strName)
			{
				m_pImpl->RemoveMessageHandler(strName);
			}

			const math::uint2& Device::GetScreenSize() const
			{
				return m_pImpl->GetScreenSize();
			}

			const D3D12_VIEWPORT* Device::GetViewport() const
			{
				return m_pImpl->GetViewport();
			}

			const math::Rect* Device::GetScissorRect() const
			{
				return m_pImpl->GetScissorRect();
			}

			ID3D12Device* Device::GetInterface() const
			{
				return m_pImpl->GetInterface();
			}

			ID3D12CommandQueue* Device::GetCommandQueue() const
			{
				return m_pImpl->GetCommandQueue();
			}

			ID3D12Fence* Device::GetFence() const
			{
				return m_pImpl->GetFence();
			}

			uint64_t Device::GetFenceValue() const
			{
				return m_pImpl->GetFenceValue();
			}

			uint32_t Device::GetFrameIndex() const
			{
				return m_pImpl->GetFrameIndex();
			}

			RenderTarget* Device::GetSwapChainRenderTarget(uint32_t nFrameIndex) const
			{
				return m_pImpl->GetSwapChainRenderTarget(nFrameIndex);
			}

			RenderTarget* Device::GetLastUsedRenderTarget() const
			{
				return m_pImpl->GetLastUsedRenderTarget();
			}

			GBuffer* Device::GetGBuffer(uint32_t nFrameIndex) const
			{
				return m_pImpl->GetGBuffer(nFrameIndex);
			}

			IImageBasedLight* Device::GetImageBasedLight() const
			{
				return m_pImpl->GetImageBasedLight();
			}

			void Device::SetImageBasedLight(IImageBasedLight* pImageBasedLight)
			{
				m_pImpl->SetImageBasedLight(pImageBasedLight);
			}

			RenderManager* Device::GetRenderManager() const
			{
				return m_pImpl->GetRenderManager();
			}

			VTFManager* Device::GetVTFManager() const
			{
				return m_pImpl->GetVTFManager();
			}

			Uploader* Device::GetUploader() const
			{
				return m_pImpl->GetUploader();
			}

			DescriptorHeap* Device::GetRTVDescriptorHeap() const
			{
				return m_pImpl->GetRTVDescriptorHeap();
			}

			DescriptorHeap* Device::GetSRVDescriptorHeap() const
			{
				return m_pImpl->GetSRVDescriptorHeap();
			}

			DescriptorHeap* Device::GetDSVDescriptorHeap() const
			{
				return m_pImpl->GetDSVDescriptorHeap();
			}

			DescriptorHeap* Device::GetUAVDescriptorHeap() const
			{
				return m_pImpl->GetUAVDescriptorHeap();
			}

			DescriptorHeap* Device::GetSamplerDescriptorHeap() const
			{
				return m_pImpl->GetSamplerDescriptorHeap();
			}

			size_t Device::GetCommandListCount() const
			{
				return m_pImpl->GetCommandListCount();
			}

			void Device::ResetCommandList(size_t nIndex, ID3D12PipelineState* pPipelineState)
			{
				return m_pImpl->ResetCommandList(nIndex, pPipelineState);
			}

			ID3D12GraphicsCommandList2* Device::GetCommandList(size_t nIndex) const
			{
				return m_pImpl->GetCommandList(nIndex);
			}

			void Device::GetCommandLists(ID3D12GraphicsCommandList2** ppGraphicsCommandLists_out) const
			{
				m_pImpl->GetCommandLists(ppGraphicsCommandLists_out);
			}

			void Device::ExecuteCommandList(ID3D12CommandList* pCommandList)
			{
				m_pImpl->ExecuteCommandList(pCommandList);
			}

			void Device::ExecuteCommandLists(ID3D12CommandList* const* ppCommandLists, size_t nCount)
			{
				m_pImpl->ExecuteCommandLists(ppCommandLists, nCount);
			}

			void Device::ExecuteCommandLists(ID3D12GraphicsCommandList2* const* ppGraphicsCommandLists, size_t nCount)
			{
				m_pImpl->ExecuteCommandLists(ppGraphicsCommandLists, nCount);
			}

			ID3D12GraphicsCommandList2* Device::CreateBundle(ID3D12PipelineState* pPipelineState)
			{
				return m_pImpl->CreateBundle(pPipelineState);
			}

			const D3D12_DESCRIPTOR_RANGE* Device::GetStandardDescriptorRanges() const
			{
				return m_pImpl->GetStandardDescriptorRanges();
			}
		}
	}
}