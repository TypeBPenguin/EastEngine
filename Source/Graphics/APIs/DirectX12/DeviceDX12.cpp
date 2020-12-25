#include "stdafx.h"
#include "DeviceDX12.h"

#include "CommonLib/Lock.h"
#include "CommonLib/Timer.h"

#include "Graphics/Interface/Window.h"
#include "Graphics/Interface/LightManager.h"

#include "UtilDX12.h"

#include "UploadDX12.h"
#include "GBufferDX12.h"
#include "RenderManagerDX12.h"
#include "DescriptorHeapDX12.h"
#include "VTFManagerDX12.h"
#include "ScreenGrab12.h"

#include "Graphics/Interface/imguiHelper.h"
#include "Graphics/Interface/imgui/imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <dxgidebug.h>

namespace sid
{
	RegisterStringID(DeviceDX12);
}

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class Device::Impl
			{
			public:
				Impl();
				~Impl();

			private:
				void Ready();
				void Update();
				void Render();
				void Present();

				void RenderImgui();

			public:
				void Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler);
				void Release();

				void Run(std::function<bool()> funcUpdate);
				void Cleanup(float elapsedTime);

			public:
				RenderTarget* GetRenderTarget(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor);
				void ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t size = 1);

				void ReleaseResource(ID3D12DeviceChild* pResource);
				void ReleaseResourceRTV(uint32_t descriptorIndex);
				void ReleaseResourceSRV(uint32_t descriptorIndex);
				void ReleaseResourceDSV(uint32_t descriptorIndex);
				void ReleaseResourceUAV(uint32_t descriptorIndex);

			public:
				void MessageHandler(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);
				void ScreenShot(ScreenShotFormat format, const std::wstring& path, std::function<void(bool, const std::wstring&)> screenShotCallback);

			public:
				const math::Viewport& GetViewport() const { return m_viewport; }
				const math::Rect& GetScissorRect() const { return m_scissorRect; }

				ID3D12Device* GetInterface() const { return m_pDevice; }
				ID3D12CommandQueue* GetCommandQueue() const { return m_pCommandQueue; }

				ID3D12Fence* GetFence() const { return m_pFence; }
				uint64_t GetFenceValue() const { return m_fenceValues[m_frameIndex]; }
				uint32_t GetFrameIndex() const { return m_frameIndex; }

				RenderTarget* GetSwapChainRenderTarget(uint32_t frameIndex) const { return m_pSwapChainRenderTargets[frameIndex].get(); }
				RenderTarget* GetBackBufferSwapChainRenderTarget(uint32_t frameIndex) const { return m_pBackBufferSwapChains[frameIndex].get(); }

				GBuffer* GetGBuffer(uint32_t frameIndex) const { return m_pGBuffers[frameIndex].get(); }
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
				size_t GetCommandListCount() const { return m_pCommandLists.size() - 1; }

				void ResetCommandList(size_t index, ID3D12PipelineState* pPipelineState);
				ID3D12GraphicsCommandList2* GetCommandList(size_t index) const;

				void GetCommandLists(std::vector<ID3D12GraphicsCommandList2*>& commandList_out) const;
				void ExecuteCommandList(ID3D12CommandList* pCommandList);
				void ExecuteCommandLists(ID3D12CommandList* const* ppCommandLists, size_t count);
				void ExecuteCommandLists(ID3D12GraphicsCommandList2* const* ppGraphicsCommandLists, size_t count);

				ID3D12GraphicsCommandList2* CreateBundle(ID3D12PipelineState* pPipelineState);

				const D3D12_DESCRIPTOR_RANGE* GetStandardDescriptorRanges() const { return m_standardDescriptorRangeDescs_SRV.data(); }

			public:
				void SetFullScreen(bool isFullScreen, std::function<void(bool)> callback);

				const std::vector<DisplayModeDesc>& GetSupportedDisplayModeDesc() const { return m_supportedDisplayModes; }
				size_t GetSelectedDisplayModeIndex() const { return m_selectedDisplayModeIndex; }
				void ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback);

			private:
				void InitializeD3D();
				void InitializeSampler();

				void WaitForGPU();
				void MoveToNextFrame();
				void EnableShaderBasedValidation();

				void OnScreenShot();
				void OnResize();

			private:
				bool m_isInitislized{ false };
				bool m_isFirstFrame_Render{ true };
				bool m_isFirstFrame_ImGui{ true };

				size_t m_videoCardMemory{ 0 };
				std::wstring m_videoCardDescription;

				size_t m_selectedDisplayModeIndex{ std::numeric_limits<size_t>::max() };
				std::vector<DisplayModeDesc> m_supportedDisplayModes;

				math::Viewport m_viewport{};
				math::Rect m_scissorRect{};

				uint32_t m_frameIndex{ 0 };
				HANDLE m_hFenceEvent{ INVALID_HANDLE_VALUE };
				std::array<uint64_t, eFrameBufferCount> m_fenceValues{ 0 };
				ID3D12Fence* m_pFence{ nullptr };

				ID3D12Debug1* m_pDebug{ nullptr };

				ID3D12Device3* m_pDevice{ nullptr };
				IDXGISwapChain3* m_pSwapChain{ nullptr };
				ID3D12CommandQueue* m_pCommandQueue{ nullptr };

				std::array<std::unique_ptr<RenderTarget>, eFrameBufferCount> m_pSwapChainRenderTargets;
				std::array<std::unique_ptr<RenderTarget>, eFrameBufferCount> m_pBackBufferSwapChains;

				std::array<std::vector<ID3D12CommandAllocator*>, eFrameBufferCount> m_pCommandAllocators;
				std::vector<ID3D12GraphicsCommandList2*> m_pCommandLists;
				ID3D12CommandAllocator* m_pBundleAllocators{ nullptr };

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
					uint32_t frameIndex{ 0 };
					float unusedTime{ 0.f };

					RenderTargetPool(std::unique_ptr<RenderTarget> pRenderTarget)
						: pRenderTarget(std::move(pRenderTarget))
					{
					}

					RenderTargetPool(RenderTargetPool&& source) noexcept
						: pRenderTarget(std::move(source.pRenderTarget))
						, isUsing(std::move(source.isUsing))
						, frameIndex(std::move(source.frameIndex))
						, unusedTime(std::move(source.unusedTime))
					{
					}
				};
				std::unordered_multimap<RenderTarget::Key, RenderTargetPool> m_renderTargetPool;

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
					uint32_t frameIndex{ 0 };

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
						frameIndex = std::move(source.frameIndex);
						return *this;
					}
				};
				std::vector<ReleaseObject> m_releaseResources;
				thread::SRWLock m_srwLock_releaseResource;

				uint32_t m_nullTextureIndex{ eInvalidDescriptorIndex };
				uint32_t m_imGuiFontSRVIndex{ eInvalidDescriptorIndex };
				std::array<uint32_t, SamplerState::TypeCount> m_samplerStates{ 0 };

				struct ScreenShotInfo
				{
					std::wstring path;
					ScreenShotFormat format{ ScreenShotFormat::eJPEG };
					std::function<void(bool, const std::wstring&)> callback;
					bool isProcessed{ false };
					bool isSuccess{ false };
				};
				ScreenShotInfo m_screeShot;

				struct ChangeDisplayModeInfo
				{
					size_t changeDisplayModeIndex{ std::numeric_limits<size_t>::max() };
					bool isFullScreen{ false };
					std::function<void(bool)> callback;
				};
				std::unique_ptr<ChangeDisplayModeInfo> m_pChangeDisplayModeInfo;

				struct ParallelRender
				{
					enum State : uint8_t
					{
						eIdle = 0,
						eProcessing,
					};

					std::mutex mutex;
					std::thread thread;
					std::condition_variable condition;
					bool isStop{ false };
					State state{ State::eIdle };
				};
				ParallelRender m_parallelRender;
			};

			Device::Impl::Impl()
			{
			}

			Device::Impl::~Impl()
			{
				Release();
			}

			void Device::Impl::Ready()
			{
				Cleanup(Timer::GetInstance()->GetElapsedTime());

				OnResize();

				if (m_screeShot.isProcessed == true)
				{
					if (m_screeShot.callback != nullptr)
					{
						m_screeShot.callback(m_screeShot.isSuccess, m_screeShot.path);
					}
					m_screeShot = {};
				}

				SwapThread();
			}

			void Device::Impl::Update()
			{
				ImGui_ImplDX12_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
			}

			void Device::Impl::Render()
			{
				TRACER_EVENT(__FUNCTIONW__);
				MoveToNextFrame();

				{
					TRACER_EVENT(L"CommandAllocatorReset");
					for (auto pCommandAllocator : m_pCommandAllocators[m_frameIndex])
					{
						HRESULT hr = pCommandAllocator->Reset();
						if (FAILED(hr))
						{
							throw_line("failed to command allocator reset");
						}
					}
				}

				// UpdateOptions
				{
					const Options& options = RenderOptions();
					const Options& prevOptions = PrevRenderOptions();
					if (prevOptions.OnMotionBlur != options.OnMotionBlur ||
						prevOptions.motionBlurConfig.IsVelocityMotionBlur() != options.motionBlurConfig.IsVelocityMotionBlur())
					{
						if (options.OnMotionBlur == true && options.motionBlurConfig.IsVelocityMotionBlur() == true)
						{
							const math::uint2& screenSize = Window::GetInstance()->GetScreenSize();
							for (int i = 0; i < eFrameBufferCount; ++i)
							{
								m_pGBuffers[i]->Resize(GBufferType::eVelocity, screenSize.x, screenSize.y);
							}
						}
						else
						{
							for (int i = 0; i < eFrameBufferCount; ++i)
							{
								m_pGBuffers[i]->Release(GBufferType::eVelocity);
							}
						}
					}
				}

				if (m_isFirstFrame_Render == true)
				{
					m_isFirstFrame_Render = false;
					return;
				}

				m_pVTFManager->Bake();

				m_pRenderManager->Render();
			}

			void Device::Impl::Present()
			{
				m_pRenderManager->Copy_RGB(m_pSwapChainRenderTargets[m_frameIndex].get(), m_pBackBufferSwapChains[m_frameIndex].get());

				{
					ID3D12GraphicsCommandList2* pCommandList = GetCommandList(0);
					ResetCommandList(0, nullptr);

					util::ChangeResourceState(pCommandList, m_pSwapChainRenderTargets[m_frameIndex].get(), D3D12_RESOURCE_STATE_PRESENT);
					util::ChangeResourceState(pCommandList, m_pBackBufferSwapChains[m_frameIndex].get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

					HRESULT hr = pCommandList->Close();
					if (FAILED(hr))
					{
						throw_line("failed to close command list");
					}

					ExecuteCommandList(pCommandList);
				}

				RenderImgui();

				m_pUploader->EndFrame(m_pCommandQueue);

				OnScreenShot();

				HRESULT hr = m_pSwapChain->Present(RenderOptions().OnVSync ? 1 : 0, 0);
				if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
				{
					LOG_ERROR(L"Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pDevice->GetDeviceRemovedReason() : hr);
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
				if (m_isFirstFrame_ImGui == false)
				{
					TRACER_EVENT(L"ImGui::Render");
					ImGui::Render();

					RenderTarget* pSwapChainRenderTarget = GetSwapChainRenderTarget(m_frameIndex);

					ID3D12GraphicsCommandList2* pCommandList = GetCommandList(0);
					ResetCommandList(0, nullptr);

					util::ChangeResourceState(pCommandList, pSwapChainRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);

					const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
					{
						pSwapChainRenderTarget->GetCPUHandle(),
					};
					pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						m_pSRVDescriptorHeap->GetHeap(),
					};
					pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					ImGui::Render();
					ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);

					util::ChangeResourceState(pCommandList, pSwapChainRenderTarget, D3D12_RESOURCE_STATE_PRESENT);

					HRESULT hr = pCommandList->Close();
					if (FAILED(hr))
					{
						throw_line("failed to close command list");
					}

					ExecuteCommandList(pCommandList);
				}
				else
				{
					m_isFirstFrame_ImGui = false;
				}
			}

			void Device::Impl::Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler)
			{
				if (m_isInitislized == true)
					return;

				auto DeviceMessageHandler = [&, messageHandler](HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam) -> HRESULT
				{
					MessageHandler(hWnd, msg, wParam, lParam);
					if (messageHandler != nullptr)
						return messageHandler(hWnd, msg, wParam, lParam);

					return 0;
				};

#if defined(DEBUG) || defined(_DEBUG)
				EnableShaderBasedValidation();
#endif

				Window* pWindow = Window::GetInstance();
				pWindow->Initialize(width, height, isFullScreen, applicationTitle, applicationName, DeviceMessageHandler);

				InitializeD3D();

				m_pVTFManager = std::make_unique<VTFManager>();
				m_pRenderManager = std::make_unique<RenderManager>();

				PersistentDescriptorAlloc srvAlloc = m_pSRVDescriptorHeap->AllocatePersistent();
				m_imGuiFontSRVIndex = srvAlloc.index;
				assert(m_imGuiFontSRVIndex != eInvalidDescriptorIndex);

				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
				cpuHandle = m_pSRVDescriptorHeap->GetCPUHandleFromIndex(m_imGuiFontSRVIndex);
				gpuHandle = m_pSRVDescriptorHeap->GetGPUHandleFromIndex(m_imGuiFontSRVIndex);

				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGui_ImplWin32_Init(pWindow->GetHwnd());
				ImGui_ImplDX12_Init(m_pDevice, eFrameBufferCount, DXGI_FORMAT_R8G8B8A8_UNORM, m_pSRVDescriptorHeap->GetHeap(), cpuHandle, gpuHandle);
				m_isInitislized = true;
			}

			void Device::Impl::Release()
			{
				if (m_isInitislized == false)
					return;

				{
					{
						std::lock_guard<std::mutex> lock(m_parallelRender.mutex);
						m_parallelRender.isStop = true;
					}
					m_parallelRender.condition.notify_all();

					if (m_parallelRender.thread.joinable() == true)
					{
						m_parallelRender.thread.join();
					}
				}

				WaitForGPU();

				CloseHandle(m_hFenceEvent);
				m_hFenceEvent = INVALID_HANDLE_VALUE;

				ImGui_ImplDX12_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();

				m_pSRVDescriptorHeap->FreePersistent(m_imGuiFontSRVIndex);

				m_pRenderManager.reset();
				m_pVTFManager.reset();

				for (uint32_t i = 0; i < SamplerState::TypeCount; ++i)
				{
					m_pSamplerDescriptorHeap->FreePersistent(m_samplerStates[i]);
				}
				m_samplerStates.fill(0);
				m_pSRVDescriptorHeap->FreePersistent(m_nullTextureIndex);

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pGBuffers[i].reset();
					m_pBackBufferSwapChains[i].reset();
					m_pSwapChainRenderTargets[i].reset();
				}

				m_renderTargetPool.clear();

				for (auto& releaseObj : m_releaseResources)
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
				m_releaseResources.clear();

				m_pUploader.reset();

				BOOL fs = FALSE;
				m_pSwapChain->GetFullscreenState(&fs, nullptr);

				if (fs == TRUE)
				{
					m_pSwapChain->SetFullscreenState(FALSE, nullptr);
				}

				for (auto pCommandList : m_pCommandLists)
				{
					SafeRelease(pCommandList);
				}
				m_pCommandLists.clear();

				SafeRelease(m_pBundleAllocators);

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pBackBufferSwapChains[i].reset();
					m_pSwapChainRenderTargets[i].reset();
					for (ID3D12CommandAllocator* pAllocator : m_pCommandAllocators[i])
					{
						SafeRelease(pAllocator);
					}
				};

				SafeRelease(m_pFence);
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

			void Device::Impl::Run(std::function<bool()> funcUpdate)
			{
				m_parallelRender.thread = std::thread([&]()
					{
						while (true)
						{
							{
								std::unique_lock<std::mutex> lock(m_parallelRender.mutex);
								m_parallelRender.condition.wait(lock, [&]()
									{
										return m_parallelRender.isStop == true || m_parallelRender.state == ParallelRender::eProcessing;
									});

								if (m_parallelRender.isStop == true)
									return;
							}

							{
								TRACER_EVENT(L"GraphicsRender");
								Render();
							}

							{
								std::lock_guard<std::mutex> lock(m_parallelRender.mutex);
								m_parallelRender.state = ParallelRender::eIdle;
							}
							m_parallelRender.condition.notify_all();
						}
					});

				auto DeviceUpdate = [&]()
				{
					Ready();

					// 업데이트 문을 메인 스레드에서 하기 위함
					{
						std::lock_guard<std::mutex> lock(m_parallelRender.mutex);
						m_parallelRender.state = ParallelRender::eProcessing;
					}
					m_parallelRender.condition.notify_all();

					//GetCursor()->Update(Timer::GetInstance()->GetElapsedTime());

					{
						TRACER_EVENT(L"GraphicsUpdate");
						Update();
					}

					bool isRunning = false;
					{
						TRACER_EVENT(L"SystemUpdate");
						isRunning = funcUpdate();
					}

					//{
					//	TRACER_EVENT(L"GraphicsRender");
					//	Render();
					//}

					{
						std::unique_lock<std::mutex> lock(m_parallelRender.mutex);
						m_parallelRender.condition.wait(lock, [&]()
							{
								return m_parallelRender.isStop == true || m_parallelRender.state == ParallelRender::eIdle;
							});
					}

					{
						TRACER_EVENT(L"GraphicsPresent");
						Present();
					}

					return isRunning;
				};
				Window::GetInstance()->Run(DeviceUpdate);
			}

			void Device::Impl::Cleanup(float elapsedTime)
			{
				TRACER_EVENT(__FUNCTIONW__);
				m_pGBuffers[m_frameIndex]->Cleanup();

				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_releaseResources.erase(std::remove_if(m_releaseResources.begin(), m_releaseResources.end(), [&](ReleaseObject& releaseObj)
					{
						if (releaseObj.frameIndex >= eFrameBufferCount)
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

						++releaseObj.frameIndex;
						return false;
					}), m_releaseResources.end());
				}

				m_pRenderManager->Cleanup();

				for (auto iter = m_renderTargetPool.begin(); iter != m_renderTargetPool.end();)
				{
					if (iter->second.isUsing == false)
					{
						if (iter->second.frameIndex >= eFrameBufferCount)
						{
							iter->second.unusedTime += elapsedTime;

							if (iter->second.unusedTime > 5.f)
							{
								iter = m_renderTargetPool.erase(iter);
								continue;
							}
						}
						else
						{
							++iter->second.frameIndex;
						}
					}

					++iter;
				}
			}

			RenderTarget* Device::Impl::GetRenderTarget(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor)
			{
				thread::SRWWriteLock writeLock(&m_renderTargetLock);

				const RenderTarget::Key key = RenderTarget::BuildKey(pDesc, clearColor);
				auto iter_range = m_renderTargetPool.equal_range(key);
				for (auto iter = iter_range.first; iter != iter_range.second; ++iter)
				{
					RenderTargetPool& pool = iter->second;
					if (pool.isUsing == false)
					{
						pool.isUsing = true;
						pool.frameIndex = 0;
						pool.unusedTime = 0.f;
						return pool.pRenderTarget.get();
					}
				}

				RenderTargetPool pool(RenderTarget::Create(pDesc, clearColor, D3D12_RESOURCE_STATE_RENDER_TARGET));
				if (pool.pRenderTarget == nullptr)
				{
					throw_line("failed to create render target");
				}

				auto iter_result = m_renderTargetPool.emplace(key, std::move(pool));
				return iter_result->second.pRenderTarget.get();
			}

			void Device::Impl::ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t size)
			{
				thread::SRWWriteLock writeLock(&m_renderTargetLock);

				for (uint32_t i = 0; i < size; ++i)
				{
					if (ppRenderTarget[i] == nullptr)
						continue;

					auto iter_range = m_renderTargetPool.equal_range(ppRenderTarget[i]->GetKey());
					for (auto iter = iter_range.first; iter != iter_range.second; ++iter)
					{
						if (iter->second.pRenderTarget.get() == ppRenderTarget[i])
						{
							iter->second.isUsing = false;
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
					m_releaseResources.emplace_back(pResource);
				}
			}

			void Device::Impl::ReleaseResourceRTV(uint32_t descriptorIndex)
			{
				if (descriptorIndex != eInvalidDescriptorIndex)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_releaseResources.emplace_back(descriptorIndex, ReleaseObject::eDescriptorHeapIndex_RTV);
				}
			}

			void Device::Impl::ReleaseResourceSRV(uint32_t descriptorIndex)
			{
				if (descriptorIndex != eInvalidDescriptorIndex)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_releaseResources.emplace_back(descriptorIndex, ReleaseObject::eDescriptorHeapIndex_SRV);
				}
			}

			void Device::Impl::ReleaseResourceDSV(uint32_t descriptorIndex)
			{
				if (descriptorIndex != eInvalidDescriptorIndex)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_releaseResources.emplace_back(descriptorIndex, ReleaseObject::eDescriptorHeapIndex_DSV);
				}
			}

			void Device::Impl::ReleaseResourceUAV(uint32_t descriptorIndex)
			{
				if (descriptorIndex != eInvalidDescriptorIndex)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_releaseResources.emplace_back(descriptorIndex, ReleaseObject::eDescriptorHeapIndex_UAV);
				}
			}

			void Device::Impl::MessageHandler(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
			{
				if (imguiHelper::MessageHandler(hWnd, nMsg, wParam, lParam) == true)
					return;

				switch (nMsg)
				{
				case WM_SIZE:
					//if (m_pDevice != nullptr && wParam != SIZE_MINIMIZED)
					//{
					//	const uint32_t width = LOWORD(lParam);
					//	const uint32_t height = HIWORD(lParam);
					//
					//	Resize(width, height);
					//}
					break;
				}
			}

			void Device::Impl::ScreenShot(ScreenShotFormat format, const std::wstring& path, std::function<void(bool, const std::wstring&)> screenShotCallback)
			{
				m_screeShot = {};
				m_screeShot.format = format;
				m_screeShot.path = path;
				m_screeShot.callback = screenShotCallback;
			}

			void Device::Impl::ResetCommandList(size_t index, ID3D12PipelineState* pPipelineState)
			{
				HRESULT hr = m_pCommandLists[index]->Reset(m_pCommandAllocators[m_frameIndex][index], pPipelineState);
				if (FAILED(hr))
				{
					throw_line("failed to reset command list");
				}
			}

			ID3D12GraphicsCommandList2* Device::Impl::GetCommandList(size_t index) const
			{
				if (index >= m_pCommandLists.size())
					return nullptr;

				return m_pCommandLists[index];
			}

			void Device::Impl::GetCommandLists(std::vector<ID3D12GraphicsCommandList2*>& commandList_out) const
			{
				commandList_out.clear();

				const size_t size = m_pCommandLists.size();
				commandList_out.resize(size);

				for (size_t i = 0; i < size; ++i)
				{
					commandList_out[i] = m_pCommandLists[i];
				}
			}

			void Device::Impl::ExecuteCommandList(ID3D12CommandList* pCommandList)
			{
				m_pCommandQueue->ExecuteCommandLists(1, &pCommandList);
			}

			void Device::Impl::ExecuteCommandLists(ID3D12CommandList* const* ppCommandLists, size_t count)
			{
				m_pCommandQueue->ExecuteCommandLists(static_cast<uint32_t>(count), ppCommandLists);
			}

			void Device::Impl::ExecuteCommandLists(ID3D12GraphicsCommandList2* const* ppGraphicsCommandLists, size_t count)
			{
				ID3D12CommandList** ppCommandLists = static_cast<ID3D12CommandList**>(_malloca(sizeof(ID3D12CommandList*) * count));
				if (ppCommandLists == nullptr)
					return;

				for (size_t i = 0; i < count; ++i)
				{
					ppCommandLists[i] = ppGraphicsCommandLists[i];
				}
				m_pCommandQueue->ExecuteCommandLists(static_cast<uint32_t>(count), ppCommandLists);
				_freea(ppCommandLists);
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

			void Device::Impl::SetFullScreen(bool isFullScreen, std::function<void(bool)> callback)
			{
				if (Window::GetInstance()->IsFullScreen() == isFullScreen)
					return;

				m_pChangeDisplayModeInfo = std::make_unique<ChangeDisplayModeInfo>();
				m_pChangeDisplayModeInfo->changeDisplayModeIndex = m_selectedDisplayModeIndex;
				m_pChangeDisplayModeInfo->isFullScreen = isFullScreen;
				m_pChangeDisplayModeInfo->callback = callback;
			}

			void Device::Impl::ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback)
			{
				if (m_selectedDisplayModeIndex == displayModeIndex)
					return;

				m_pChangeDisplayModeInfo = std::make_unique<ChangeDisplayModeInfo>();
				m_pChangeDisplayModeInfo->changeDisplayModeIndex = displayModeIndex;
				m_pChangeDisplayModeInfo->isFullScreen = Window::GetInstance()->IsFullScreen();
				m_pChangeDisplayModeInfo->callback = callback;
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
				int adapterIndex{ 0 };
				bool isAdapterFound{ false };

				while (pDxgiFactory->EnumAdapters1(adapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
				{
					DXGI_ADAPTER_DESC1 desc;
					pAdapter->GetDesc1(&desc);

					if (desc.Flags& DXGI_ADAPTER_FLAG_SOFTWARE)
					{
						++adapterIndex;
						continue;
					}

					hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
					if (SUCCEEDED(hr))
					{
						isAdapterFound = true;
						break;
					}

					SafeRelease(pAdapter);

					++adapterIndex;
				}

				if (isAdapterFound == false)
				{
					throw_line("failed to find GPUs with DirectX 12 support!");
				}

				CComPtr<IDXGIOutput> pAdapterOutput{ nullptr };
				hr = pAdapter->EnumOutputs(0, &pAdapterOutput);
				if (FAILED(hr))
				{
					throw_line("failed to find adapters");
				}

				uint32_t numDisplayMode = 0;
				hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayMode, nullptr);
				if (FAILED(hr))
				{
					throw_line("unsupported display mode");
				}

				m_selectedDisplayModeIndex = std::numeric_limits<size_t>::max();
				m_supportedDisplayModes.clear();

				std::vector<DXGI_MODE_DESC> displayModes;
				displayModes.resize(numDisplayMode);

				hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayMode, &displayModes.front());
				if (FAILED(hr))
				{
					throw_line("unsupported display mode");
				}

				DXGI_ADAPTER_DESC adapterDesc{};
				hr = pAdapter->GetDesc(&adapterDesc);
				if (FAILED(hr))
				{
					throw_line("failed to get DXGI_ADPTER_DESC");
				}

				m_videoCardMemory = static_cast<size_t>(adapterDesc.DedicatedVideoMemory / 1024Ui64 / 1024Ui64);
				m_videoCardDescription = adapterDesc.Description;

				hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
				if (FAILED(hr))
				{
					throw_line("failed to create d3d device");
				}

				SafeRelease(pAdapter);

				const math::uint2& screenSize = Window::GetInstance()->GetScreenSize();

				m_supportedDisplayModes.reserve(numDisplayMode);

				for (uint32_t i = 0; i < numDisplayMode; ++i)
				{
					if (displayModes[i].Width == 0 || displayModes[i].Height == 0)
						continue;

					auto iter = std::find_if(m_supportedDisplayModes.begin(), m_supportedDisplayModes.end(), [=](const DisplayModeDesc& displayModeDesc)
						{
							return displayModeDesc.width == displayModes[i].Width &&
								displayModeDesc.height == displayModes[i].Height;
						});

					if (iter != m_supportedDisplayModes.end())
					{
						DisplayModeDesc& displayModeDesc = *iter;
						const float refreshRate_old = static_cast<float>(displayModeDesc.refreshRate_numerator) / static_cast<float>(displayModeDesc.refreshRate_denominator);
						const float refreshRate_new = static_cast<float>(displayModes[i].RefreshRate.Numerator) / static_cast<float>(displayModes[i].RefreshRate.Denominator);
						if (refreshRate_new > refreshRate_old)
						{
							displayModeDesc.refreshRate_numerator = displayModes[i].RefreshRate.Numerator;
							displayModeDesc.refreshRate_denominator = displayModes[i].RefreshRate.Denominator;
						}
						continue;
					}

					DisplayModeDesc displayModeDesc;
					displayModeDesc.width = displayModes[i].Width;
					displayModeDesc.height = displayModes[i].Height;
					displayModeDesc.refreshRate_numerator = displayModes[i].RefreshRate.Numerator;
					displayModeDesc.refreshRate_denominator = displayModes[i].RefreshRate.Denominator;
					m_supportedDisplayModes.emplace_back(displayModeDesc);

					if (displayModes[i].Width == screenSize.x &&
						displayModes[i].Height == screenSize.y)
					{
						m_selectedDisplayModeIndex = m_supportedDisplayModes.size() - 1;
					}
				}

				if (m_supportedDisplayModes.empty() == true)
				{
					throw_line("not exists support display");
				}

				if (m_selectedDisplayModeIndex == std::numeric_limits<size_t>::max())
				{
					m_selectedDisplayModeIndex = 0;
				}

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
				m_pSamplerDescriptorHeap = std::make_unique<DescriptorHeap>(SamplerState::TypeCount, 0, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true, L"SamplerDescriptorHeap");

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
					m_pDevice->CreateShaderResourceView(nullptr, &srvDesc, srvAlloc.cpuHandle);
					m_nullTextureIndex = srvAlloc.index;
					assert(m_nullTextureIndex != eInvalidDescriptorIndex);
				}

				DXGI_MODE_DESC backBufferDesc{};
				backBufferDesc.Width = m_supportedDisplayModes[m_selectedDisplayModeIndex].width;
				backBufferDesc.Height = m_supportedDisplayModes[m_selectedDisplayModeIndex].height;
				backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				DXGI_SWAP_CHAIN_DESC swapChainDesc{};
				swapChainDesc.BufferCount = eFrameBufferCount;
				swapChainDesc.BufferDesc = backBufferDesc;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_BACK_BUFFER;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDesc.OutputWindow = Window::GetInstance()->GetHwnd();
				swapChainDesc.SampleDesc = sampleDesc;
				swapChainDesc.Windowed = Window::GetInstance()->IsFullScreen() == false;

				IDXGISwapChain* pTempSwapChain{ nullptr };
				hr = pDxgiFactory->CreateSwapChain(m_pCommandQueue, &swapChainDesc, &pTempSwapChain);
				if (FAILED(hr))
				{
					throw_line("failed to create swapchain");
				}

				m_pSwapChain = static_cast<IDXGISwapChain3*>(pTempSwapChain);
				math::Color clearColor;
				m_pSwapChain->GetBackgroundColor(reinterpret_cast<DXGI_RGBA*>(&clearColor));

				m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

				const uint32_t threadCount = std::thread::hardware_concurrency();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					ID3D12Resource* pResource = nullptr;
					hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pResource));
					if (FAILED(hr))
					{
						throw_line("failed to create render target");
					}
					m_pSwapChainRenderTargets[i] = RenderTarget::Create(pResource, clearColor);

					const D3D12_RESOURCE_DESC desc = pResource->GetDesc();
					m_pBackBufferSwapChains[i] = RenderTarget::Create(&desc, clearColor, D3D12_RESOURCE_STATE_RENDER_TARGET);

					m_pCommandAllocators[i].resize(threadCount);

					for (uint32_t j = 0; j < threadCount; ++j)
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

				m_pCommandLists.resize(threadCount);

				for (uint32_t i = 0; i < threadCount; ++i)
				{
					hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocators[0][i], nullptr, IID_PPV_ARGS(&m_pCommandLists[i]));
					if (FAILED(hr))
					{
						throw_line("failed to create command list");
					}

					m_pCommandLists[i]->Close();
				}

				hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
				if (FAILED(hr))
				{
					throw_line("failed to create fence");
				}

				m_hFenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
				if (m_hFenceEvent == nullptr || m_hFenceEvent == INVALID_HANDLE_VALUE)
				{
					throw_line("failed to create fence event");
				}

				InitializeSampler();

				m_viewport.x = 0;
				m_viewport.y = 0;
				m_viewport.width = static_cast<float>(screenSize.x);
				m_viewport.height = static_cast<float>(screenSize.y);
				m_viewport.minDepth = 0.f;
				m_viewport.maxDepth = 1.f;

				m_scissorRect.left = 0;
				m_scissorRect.top = 0;
				m_scissorRect.right = screenSize.x;
				m_scissorRect.bottom = screenSize.y;

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
					m_pGBuffers[i] = std::make_unique<GBuffer>(screenSize.x, screenSize.y);
				}
			}

			void Device::Impl::InitializeSampler()
			{
				for (uint32_t i = 0; i < SamplerState::TypeCount; ++i)
				{
					SamplerState::Type samplerState = static_cast<SamplerState::Type>(i);

					PersistentDescriptorAlloc samplerAlloc = m_pSamplerDescriptorHeap->AllocatePersistent();

					D3D12_SAMPLER_DESC samplerDesc = util::GetSamplerDesc(samplerState);
					m_pDevice->CreateSampler(&samplerDesc, samplerAlloc.cpuHandle);
					m_samplerStates[samplerState] = samplerAlloc.index;
				}
			}

			void Device::Impl::WaitForGPU()
			{
				// Schedule a Signal command in the GPU queue.
				if (SUCCEEDED(m_pCommandQueue->Signal(m_pFence, m_fenceValues[m_frameIndex])))
				{
					// Wait until the Signal has been processed.
					if (SUCCEEDED(m_pFence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_hFenceEvent)))
					{
						WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);

						// Increment the fence value for the current frame.
						++m_fenceValues[m_frameIndex];
					}
				}
			}

			void Device::Impl::MoveToNextFrame()
			{
				TRACER_EVENT(__FUNCTIONW__);

				const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
				m_pCommandQueue->Signal(m_pFence, currentFenceValue);

				m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

				util::WaitForFence(m_pFence, m_fenceValues[m_frameIndex], m_hFenceEvent);

				m_fenceValues[m_frameIndex] = currentFenceValue + 1;
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

			void Device::Impl::OnScreenShot()
			{
				TRACER_EVENT(__FUNCTIONW__);
				if (m_screeShot.path.empty() == false)
				{
					m_screeShot.isProcessed = true;

					CComPtr<ID3D12Resource> pBackBuffer = GetSwapChainRenderTarget(m_frameIndex)->GetResource();

					if (m_screeShot.format == ScreenShotFormat::eDDS)
					{
						m_screeShot.path += L".dds";

						const HRESULT hr = DirectX::SaveDDSTextureToFile(m_pCommandQueue, pBackBuffer, m_screeShot.path.c_str());
						m_screeShot.isSuccess = SUCCEEDED(hr);
					}
					else
					{
						GUID formatGuid{};

						switch (m_screeShot.format)
						{
						case ePNG:
							formatGuid = GUID_ContainerFormatPng;
							m_screeShot.path += L".png";
							break;
							//case eICO:
							//	formatGuid = GUID_ContainerFormatIco;
							//	m_screeShot.path += ".ico";
							//	break;
						case eJPEG:
							formatGuid = GUID_ContainerFormatJpeg;
							m_screeShot.path += L".jpg";
							break;
							//case eGIF:
							//	formatGuid = GUID_ContainerFormatGif;
							//	m_screeShot.path += ".gif";
							//	break;
							//case eWMP:
							//	formatGuid = GUID_ContainerFormatWmp;
							//	m_screeShot.path += ".wmp";
							//	break;
						}

						const HRESULT hr = DirectX::SaveWICTextureToFile(m_pCommandQueue, pBackBuffer, formatGuid, m_screeShot.path.c_str());
						m_screeShot.isSuccess = SUCCEEDED(hr);
					}
				}
			}

			void Device::Impl::OnResize()
			{
				if (m_pChangeDisplayModeInfo == nullptr)
					return;

				if (m_pChangeDisplayModeInfo->changeDisplayModeIndex == m_selectedDisplayModeIndex &&
					m_pChangeDisplayModeInfo->isFullScreen == Window::GetInstance()->IsFullScreen())
				{
					if (m_pChangeDisplayModeInfo->callback != nullptr)
					{
						m_pChangeDisplayModeInfo->callback(false);
					}
					m_pChangeDisplayModeInfo.reset();
					return;
				}

				if (m_pChangeDisplayModeInfo->changeDisplayModeIndex >= m_supportedDisplayModes.size())
				{
					if (m_pChangeDisplayModeInfo->callback != nullptr)
					{
						m_pChangeDisplayModeInfo->callback(false);
					}
					m_pChangeDisplayModeInfo.reset();
					return;
				}

				uint32_t width = 0;
				uint32_t height = 0;
				if (m_pChangeDisplayModeInfo->isFullScreen == true)
				{
					width = GetSystemMetrics(SM_CXSCREEN);
					height = GetSystemMetrics(SM_CYSCREEN);
				}
				else
				{
					const DisplayModeDesc& displayModeDesc = m_supportedDisplayModes[m_pChangeDisplayModeInfo->changeDisplayModeIndex];
					width = displayModeDesc.width;
					height = displayModeDesc.height;
				}

				if (Window::GetInstance()->Resize(width, height, m_pChangeDisplayModeInfo->isFullScreen) == false)
				{
					if (m_pChangeDisplayModeInfo->callback != nullptr)
					{
						m_pChangeDisplayModeInfo->callback(false);
					}
					return;
				}

				WaitForGPU();

				//WaitForPreviousFrame();
				//
				//HRESULT hr = m_pCommandQueue->Signal(m_pFences[m_frameIndex], m_fenceValues[m_frameIndex]);
				//if (FAILED(hr))
				//{
				//	throw_line("failed to command queue signal");
				//}

				ImGui_ImplDX12_InvalidateDeviceObjects();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pGBuffers[i]->Release();
					m_pBackBufferSwapChains[i].reset();
					m_pSwapChainRenderTargets[i].reset();

					//m_fenceValues[i] = m_fenceValues[m_frameIndex];
				}
				m_renderTargetPool.clear();

				{
					thread::SRWWriteLock writeLock(&m_srwLock_releaseResource);
					m_releaseResources.erase(std::remove_if(m_releaseResources.begin(), m_releaseResources.end(), [&](ReleaseObject& releaseObj)
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
						}), m_releaseResources.end());
				}

				HRESULT hr = m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
				if (FAILED(hr))
				{
					LOG_WARNING(L"Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pDevice->GetDeviceRemovedReason() : hr);
					throw_line("failed to resize device dx12");
				}

				math::Color clearColor;
				m_pSwapChain->GetBackgroundColor(reinterpret_cast<DXGI_RGBA*>(&clearColor));

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					CComPtr<ID3D12Resource> pResource = nullptr;
					hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pResource));
					if (FAILED(hr))
					{
						throw_line("failed to create render target");
					}
					m_pSwapChainRenderTargets[i] = RenderTarget::Create(pResource, clearColor);

					const D3D12_RESOURCE_DESC desc = pResource->GetDesc();
					m_pBackBufferSwapChains[i] = RenderTarget::Create(&desc, clearColor, D3D12_RESOURCE_STATE_RENDER_TARGET);
				}

				DXGI_SWAP_CHAIN_DESC1 desc{};
				if (FAILED(m_pSwapChain->GetDesc1(&desc)))
				{
					throw_line("failed to get DXGI_SWAP_CHAIN_DESC1");
				}

				m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

				m_viewport.x = 0.0f;
				m_viewport.y = 0.0f;
				m_viewport.width = static_cast<float>(desc.Width);
				m_viewport.height = static_cast<float>(desc.Height);
				m_viewport.minDepth = 0.0f;
				m_viewport.maxDepth = 1.0f;

				m_scissorRect.left = 0;
				m_scissorRect.top = 0;
				m_scissorRect.right = desc.Width;
				m_scissorRect.bottom = desc.Height;

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pGBuffers[i]->Resize(desc.Width, desc.Height);
				}

				m_selectedDisplayModeIndex = m_pChangeDisplayModeInfo->changeDisplayModeIndex;

				ImGui_ImplDX12_CreateDeviceObjects();

				if (m_pChangeDisplayModeInfo->callback != nullptr)
				{
					m_pChangeDisplayModeInfo->callback(true);
				}
				m_pChangeDisplayModeInfo.reset();

				WaitForGPU();
			}

			Device::Device()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			Device::~Device()
			{
			}

			void Device::Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler)
			{
				m_pImpl->Initialize(width, height, isFullScreen, applicationTitle, applicationName, messageHandler);
			}

			void Device::Run(std::function<bool()> funcUpdate)
			{
				m_pImpl->Run(funcUpdate);
			}

			void Device::Cleanup(float elapsedTime)
			{
				m_pImpl->Cleanup(elapsedTime);
			}

			void Device::ScreenShot(ScreenShotFormat format, const std::wstring& path, std::function<void(bool, const std::wstring&)> screenShotCallback)
			{
				m_pImpl->ScreenShot(format, path, screenShotCallback);
			}

			RenderTarget* Device::GetRenderTarget(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor)
			{
				return m_pImpl->GetRenderTarget(pDesc, clearColor);
			}

			void Device::ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t size)
			{
				m_pImpl->ReleaseRenderTargets(ppRenderTarget, size);
			}

			void Device::ReleaseResource(ID3D12DeviceChild* pResource)
			{
				m_pImpl->ReleaseResource(pResource);
			}

			void Device::ReleaseResourceRTV(uint32_t descriptorIndex)
			{
				m_pImpl->ReleaseResourceRTV(descriptorIndex);
			}

			void Device::ReleaseResourceSRV(uint32_t descriptorIndex)
			{
				m_pImpl->ReleaseResourceSRV(descriptorIndex);
			}

			void Device::ReleaseResourceDSV(uint32_t descriptorIndex)
			{
				m_pImpl->ReleaseResourceDSV(descriptorIndex);
			}

			void Device::ReleaseResourceUAV(uint32_t descriptorIndex)
			{
				m_pImpl->ReleaseResourceUAV(descriptorIndex);
			}

			HWND Device::GetHwnd() const
			{
				return Window::GetInstance()->GetHwnd();
			}

			HINSTANCE Device::GetHInstance() const
			{
				return Window::GetInstance()->GetInstanceHandle();
			}

			const math::uint2& Device::GetScreenSize() const
			{
				return Window::GetInstance()->GetScreenSize();
			}

			const math::Viewport& Device::GetViewport() const
			{
				return m_pImpl->GetViewport();
			}

			const math::Rect& Device::GetScissorRect() const
			{
				return m_pImpl->GetScissorRect();
			}

			bool Device::IsFullScreen() const
			{
				return Window::GetInstance()->IsFullScreen();
			}

			void Device::SetFullScreen(bool isFullScreen, std::function<void(bool)> callback)
			{
				m_pImpl->SetFullScreen(isFullScreen, callback);
			}

			const std::vector<DisplayModeDesc>& Device::GetSupportedDisplayModeDesc() const
			{
				return m_pImpl->GetSupportedDisplayModeDesc();
			}

			size_t Device::GetSelectedDisplayModeIndex() const
			{
				return m_pImpl->GetSelectedDisplayModeIndex();
			}

			void Device::ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback)
			{
				m_pImpl->ChangeDisplayMode(displayModeIndex, callback);
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

			RenderTarget* Device::GetSwapChainRenderTarget(uint32_t frameIndex) const
			{
				return m_pImpl->GetSwapChainRenderTarget(frameIndex);
			}

			RenderTarget* Device::GetBackBufferSwapChainRenderTarget(uint32_t frameIndex) const
			{
				return m_pImpl->GetBackBufferSwapChainRenderTarget(frameIndex);
			}

			GBuffer* Device::GetGBuffer(uint32_t frameIndex) const
			{
				return m_pImpl->GetGBuffer(frameIndex);
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

			void Device::ResetCommandList(size_t index, ID3D12PipelineState* pPipelineState)
			{
				return m_pImpl->ResetCommandList(index, pPipelineState);
			}

			ID3D12GraphicsCommandList2* Device::GetCommandList(size_t index) const
			{
				return m_pImpl->GetCommandList(index);
			}

			void Device::GetCommandLists(std::vector<ID3D12GraphicsCommandList2*>& commandList_out) const
			{
				m_pImpl->GetCommandLists(commandList_out);
			}

			void Device::ExecuteCommandList(ID3D12CommandList* pCommandList)
			{
				m_pImpl->ExecuteCommandList(pCommandList);
			}

			void Device::ExecuteCommandLists(ID3D12CommandList* const* ppCommandLists, size_t count)
			{
				m_pImpl->ExecuteCommandLists(ppCommandLists, count);
			}

			void Device::ExecuteCommandLists(ID3D12GraphicsCommandList2* const* ppGraphicsCommandLists, size_t count)
			{
				m_pImpl->ExecuteCommandLists(ppGraphicsCommandLists, count);
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