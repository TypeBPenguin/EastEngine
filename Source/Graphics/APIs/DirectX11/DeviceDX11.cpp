#include "stdafx.h"
#include "DeviceDX11.h"

#include "CommonLib/Lock.h"
#include "CommonLib/Timer.h"

#include "Graphics/Interface/Window.h"
#include "Graphics/Interface/LightManager.h"

#include "UtilDX11.h"

#include "GBufferDX11.h"
#include "RenderManagerDX11.h"
#include "VTFManagerDX11.h"
#include "LightResourceManagerDX11.h"
#include "ScreenGrab11.h"

#include "Graphics/Interface/imguiHelper.h"
#include "Graphics/Interface/imgui/imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			const float MipLODBias = 0.f;

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

			public:
				void Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler);
				void Release();

				void Run(std::function<bool()> funcUpdate);
				void Cleanup(float elapsedTime);

			public:
				RenderTarget* GetRenderTarget(const D3D11_TEXTURE2D_DESC* pDesc);
				void ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize);

				DepthStencil* GetDepthStencil(const D3D11_TEXTURE2D_DESC* pDesc);
				void ReleaseDepthStencil(DepthStencil** ppDepthStencil);

			public:
				void MessageHandler(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);
				void ScreenShot(ScreenShotFormat format, const std::wstring& path, std::function<void(bool, const std::wstring&)> screenShotCallback);

			public:
				const math::Viewport& GetViewport() const { return m_viewport; }
				const GBuffer* GetGBuffer() const { return m_pGBuffer.get(); }
				IImageBasedLight* GetImageBasedLight() const { return m_pImageBasedLight; }
				void SetImageBasedLight(IImageBasedLight* pImageBasedLight) { m_pImageBasedLight = pImageBasedLight; }
				RenderManager* GetRenderManager() const { return m_pRenderManager.get(); }
				VTFManager* GetVTFManager() const { return m_pVTFManager.get(); }
				LightResourceManager* GetLightResourceManager() const { return m_pLightResourceManager.get(); }

				ID3D11Device* GetInterface() const { return m_pDevice; }
				ID3D11DeviceContext* GetImmediateContext() const { return m_pImmediateContext; }

				ID3D11DeviceContext* GetRenderContext() const { return m_pRenderContext; }

				RenderTarget* GetSwapChainRenderTarget() const { return m_pSwapChainRenderTarget.get(); }

				ID3D11RasterizerState* GetRasterizerState(RasterizerState::Type emType) const { return m_pRasterizerStates[emType]; }
				ID3D11BlendState* GetBlendState(BlendState::Type emType) const { return m_pBlendStates[emType]; }
				ID3D11SamplerState* GetSamplerState(SamplerState::Type emType) const { return m_pSamplerStates[emType]; }
				ID3D11DepthStencilState* GetDepthStencilState(DepthStencilState::Type emType) const { return m_pDepthStencilStates[emType]; }

				ID3DUserDefinedAnnotation* GetUserDefinedAnnotation() const { return m_pUserDefinedAnnotation; }

			public:
				void SetFullScreen(bool isFullScreen, std::function<void(bool)> callback);

				const std::vector<DisplayModeDesc>& GetSupportedDisplayModeDesc() const { return m_supportedDisplayModes; }
				size_t GetSelectedDisplayModeIndex() const { return m_selectedDisplayModeIndex; }
				void ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback);

			private:
				void InitializeD3D();
				void InitializeRasterizerState();
				void InitializeBlendState();
				void InitializeSamplerState();
				void InitializeDepthStencilState();

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

				CComPtr<ID3D11Device> m_pDevice{ nullptr };
				CComPtr<ID3D11DeviceContext> m_pImmediateContext{ nullptr };
				CComPtr<ID3D11DeviceContext> m_pRenderContext{ nullptr };
				CComPtr<IDXGISwapChain1> m_pSwapChain{ nullptr };

				std::unique_ptr<RenderTarget> m_pSwapChainRenderTarget;

				template <typename T>
				struct ResourcePool
				{
					std::unique_ptr<T> pResource;
					bool isUsing{ false };
					float unusedTime{ 0.f };

					ResourcePool(std::unique_ptr<T> pResource)
						: pResource(std::move(pResource))
					{
					}

					ResourcePool(ResourcePool&& source) noexcept
						: pResource(std::move(source.pResource))
						, isUsing(std::move(source.isUsing))
						, unusedTime(std::move(source.unusedTime))
					{
					}
				};

				thread::SRWLock m_srwLock;

				std::unordered_multimap<RenderTarget::Key, ResourcePool<RenderTarget>> m_renderTargetPool;
				std::unordered_multimap<DepthStencil::Key, ResourcePool<DepthStencil>> m_depthStencilPool;

				std::array<CComPtr<ID3D11RasterizerState>, RasterizerState::TypeCount> m_pRasterizerStates{ nullptr };
				std::array<CComPtr<ID3D11BlendState>, BlendState::TypeCount> m_pBlendStates{ nullptr };
				std::array<CComPtr<ID3D11SamplerState>, SamplerState::TypeCount> m_pSamplerStates{ nullptr };
				std::array<CComPtr<ID3D11DepthStencilState>, DepthStencilState::TypeCount> m_pDepthStencilStates{ nullptr };

				CComPtr<ID3DUserDefinedAnnotation> m_pUserDefinedAnnotation{ nullptr };

				std::unique_ptr<GBuffer> m_pGBuffer;
				IImageBasedLight* m_pImageBasedLight{ nullptr };

				std::unique_ptr<RenderManager> m_pRenderManager;
				std::unique_ptr<VTFManager> m_pVTFManager;
				std::unique_ptr<LightResourceManager> m_pLightResourceManager;

				struct ScreenShotInfo
				{
					std::wstring path;
					ScreenShotFormat format{ ScreenShotFormat::eJPEG };
					std::function<void(bool, const std::wstring&)> callback;
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
				OnScreenShot();

				SwapThread();
			}

			void Device::Impl::Update()
			{
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
			}

			void Device::Impl::Render()
			{
				ID3D11DeviceContext* pRenderContext = GetRenderContext();

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
							m_pGBuffer->Resize(GBufferType::eVelocity, screenSize.x, screenSize.y);
						}
						else
						{
							m_pGBuffer->Release(GBufferType::eVelocity);
						}
					}
				}

				if (m_isFirstFrame_Render == true)
				{
					m_isFirstFrame_Render = false;
					return;
				}

				const float clearColor[] = { 0.f, 0.2f, 0.4f, 1.f };
				pRenderContext->ClearRenderTargetView(GetSwapChainRenderTarget()->GetRenderTargetView(), clearColor);

				m_pGBuffer->Clear(pRenderContext);

				m_pVTFManager->Bake();

				m_pRenderManager->Render();

				{
					TRACER_EVENT(L"RenderCommandList");
					CComPtr<ID3D11CommandList> pCommandList = nullptr;
					pRenderContext->FinishCommandList(FALSE, &pCommandList);
					m_pImmediateContext->ExecuteCommandList(pCommandList, FALSE);
				}
			}

			void Device::Impl::Present()
			{
				if (m_isFirstFrame_ImGui == false)
				{
					TRACER_EVENT(L"ImGui::Render");
					ImGui::Render();

					ID3D11RenderTargetView* pRTV[] = { GetSwapChainRenderTarget()->GetRenderTargetView() };
					m_pImmediateContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

					ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
				}
				else
				{
					m_isFirstFrame_ImGui = false;
				}

				DXGI_PRESENT_PARAMETERS presentParam{};
				HRESULT hr = m_pSwapChain->Present1(RenderOptions().OnVSync ? 1 : 0, 0, &presentParam);
				if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
				{
					LOG_ERROR(L"Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pDevice->GetDeviceRemovedReason() : hr);
				}
				else if (FAILED(hr))
				{
					throw_line("failed to swapchain present");
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

				Window* pWindow = Window::GetInstance();
				pWindow->Initialize(width, height, isFullScreen, applicationTitle, applicationName, DeviceMessageHandler);

				InitializeD3D();
				InitializeRasterizerState();
				InitializeBlendState();
				InitializeSamplerState();
				InitializeDepthStencilState();

				m_pRenderManager = std::make_unique<RenderManager>();
				m_pVTFManager = std::make_unique<VTFManager>();
				m_pLightResourceManager = std::make_unique<LightResourceManager>(LightManager::GetInstance());

				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGui_ImplWin32_Init(pWindow->GetHwnd());
				ImGui_ImplDX11_Init(m_pDevice, m_pImmediateContext);
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

				ImGui_ImplDX11_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();

				m_pRenderManager.reset();
				m_pVTFManager.reset();
				m_pLightResourceManager.reset();

				m_pGBuffer.reset();

				BOOL fs = FALSE;
				m_pSwapChain->GetFullscreenState(&fs, nullptr);

				if (fs == TRUE)
				{
					m_pSwapChain->SetFullscreenState(FALSE, nullptr);
				}

				m_pRasterizerStates.fill(nullptr);
				m_pBlendStates.fill(nullptr);
				m_pSamplerStates.fill(nullptr);
				m_pDepthStencilStates.fill(nullptr);

				m_pUserDefinedAnnotation.Release();

				m_renderTargetPool.clear();
				m_depthStencilPool.clear();

				m_pSwapChainRenderTarget.reset();

				if (m_pImmediateContext != nullptr)
				{
					m_pImmediateContext->ClearState();
				}

				m_pSwapChain.Release();
				m_pImmediateContext.Release();
				m_pRenderContext.Release();

				util::ReportLiveObjects(m_pDevice);
				m_pDevice.Release();

				Window::DestroyInstance();
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
				m_pGBuffer->Cleanup();
				m_pVTFManager->Cleanup();
				m_pRenderManager->Cleanup();
				m_pLightResourceManager->Cleanup();

				for (auto iter = m_renderTargetPool.begin(); iter != m_renderTargetPool.end();)
				{
					if (iter->second.isUsing == false)
					{
						iter->second.unusedTime += elapsedTime;

						if (iter->second.unusedTime > 5.f)
						{
							iter = m_renderTargetPool.erase(iter);
							continue;
						}
					}

					++iter;
				}

				for (auto iter = m_depthStencilPool.begin(); iter != m_depthStencilPool.end();)
				{
					if (iter->second.isUsing == false)
					{
						iter->second.unusedTime += elapsedTime;

						if (iter->second.unusedTime > 5.f)
						{
							iter = m_depthStencilPool.erase(iter);
							continue;
						}
					}

					++iter;
				}
			}

			RenderTarget* Device::Impl::GetRenderTarget(const D3D11_TEXTURE2D_DESC* pDesc)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				const RenderTarget::Key key = RenderTarget::BuildKey(pDesc);
				auto iter_range = m_renderTargetPool.equal_range(key);
				for (auto iter = iter_range.first; iter != iter_range.second; ++iter)
				{
					ResourcePool<RenderTarget>& pool = iter->second;
					if (pool.isUsing == false)
					{
						pool.unusedTime = 0.f;
						pool.isUsing = true;
						return pool.pResource.get();
					}
				}

				ResourcePool<RenderTarget> pool(RenderTarget::Create(pDesc));
				if (pool.pResource == nullptr)
				{
					throw_line("failed to create render target");
				}

				auto iter_result = m_renderTargetPool.emplace(key, std::move(pool));
				return iter_result->second.pResource.get();
			}

			void Device::Impl::ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				for (uint32_t i = 0; i < nSize; ++i)
				{
					if (ppRenderTarget[i] == nullptr)
						continue;

					auto iter_range = m_renderTargetPool.equal_range(ppRenderTarget[i]->GetKey());
					for (auto iter = iter_range.first; iter != iter_range.second; ++iter)
					{
						if (iter->second.pResource.get() == ppRenderTarget[i])
						{
							iter->second.isUsing = false;
							ppRenderTarget[i] = nullptr;
							break;
						}
					}
				}
			}

			DepthStencil* Device::Impl::GetDepthStencil(const D3D11_TEXTURE2D_DESC* pDesc)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				const DepthStencil::Key key = DepthStencil::BuildKey(pDesc);
				auto iter_range = m_depthStencilPool.equal_range(key);
				for (auto iter = iter_range.first; iter != iter_range.second; ++iter)
				{
					ResourcePool<DepthStencil>& pool = iter->second;
					if (pool.isUsing == false)
					{
						pool.unusedTime = 0.f;
						pool.isUsing = true;
						return pool.pResource.get();
					}
				}

				ResourcePool<DepthStencil> pool(DepthStencil::Create(pDesc));
				if (pool.pResource == nullptr)
				{
					throw_line("failed to create render target");
				}

				auto iter_result = m_depthStencilPool.emplace(key, std::move(pool));
				return iter_result->second.pResource.get();
			}

			void Device::Impl::ReleaseDepthStencil(DepthStencil** ppDepthStencil)
			{
				if (ppDepthStencil == nullptr || *ppDepthStencil == nullptr)
					return;

				thread::SRWWriteLock writeLock(&m_srwLock);

				auto iter_range = m_depthStencilPool.equal_range((*ppDepthStencil)->GetKey());
				for (auto iter = iter_range.first; iter != iter_range.second; ++iter)
				{
					if (iter->second.pResource.get() == (*ppDepthStencil))
					{
						iter->second.isUsing = false;
						(*ppDepthStencil) = nullptr;
						break;
					}
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
					//	uint32_t width = LOWORD(lParam);
					//	uint32_t height = HIWORD(lParam);
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
				CComPtr<IDXGIFactory4> pDxgiFactory{ nullptr };
				HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&pDxgiFactory));
				if (FAILED(hr))
				{
					throw_line("failed to create dxgi factory");
				}

				CComPtr<IDXGIAdapter1> pAdapter{ nullptr };
				hr = pDxgiFactory->EnumAdapters1(0, &pAdapter);
				if (FAILED(hr))
				{
					throw_line("failed to find adapters");
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

				D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

				uint32_t creationFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
				creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

				D3D_FEATURE_LEVEL emReturnedFeatureLevel = D3D_FEATURE_LEVEL_11_1;
				hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, requestedLevels, ARRAYSIZE(requestedLevels),
					D3D11_SDK_VERSION, &m_pDevice, &emReturnedFeatureLevel, &m_pImmediateContext);

				if (FAILED(hr) || emReturnedFeatureLevel < D3D_FEATURE_LEVEL_11_0)
				{
					throw_line("failed to create device");
				}

				{
					CComPtr<ID3D10Multithread> pMultithread = nullptr;
					hr = m_pImmediateContext->QueryInterface<ID3D10Multithread>(&pMultithread);
					if (FAILED(hr))
					{
						throw_line("failed to get multithread object");
					}
					pMultithread->SetMultithreadProtected(TRUE);
				}

				m_pDevice->CreateDeferredContext(0, &m_pRenderContext);

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

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
				swapChainDesc.Width = 0;
				swapChainDesc.Height = 0;
				swapChainDesc.BufferCount = 2;
				swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swapChainDesc.Stereo = false;
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_BACK_BUFFER;
				swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
				swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullScreenDesc{};
				swapChainFullScreenDesc.Windowed = Window::GetInstance()->IsFullScreen() == false;
				swapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				swapChainFullScreenDesc.RefreshRate.Numerator = m_supportedDisplayModes[m_selectedDisplayModeIndex].refreshRate_numerator;
				swapChainFullScreenDesc.RefreshRate.Denominator = m_supportedDisplayModes[m_selectedDisplayModeIndex].refreshRate_denominator;

				hr = pDxgiFactory->CreateSwapChainForHwnd(m_pDevice, Window::GetInstance()->GetHwnd(), &swapChainDesc, &swapChainFullScreenDesc, nullptr, &m_pSwapChain);
				if (FAILED(hr))
				{
					throw_line("failed to create swapchain");
				}

				// Disagle Alt + Enter
				{
					CComPtr<IDXGIDevice> pDXGIDevice;
					hr = m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
					if (FAILED(hr))
					{
						throw_line("failed to get IDXGIDevice");
					}

					CComPtr<IDXGIAdapter> pDXGIAdapter;
					hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);
					if (FAILED(hr))
					{
						throw_line("failed to get IDXGIAdapter");
					}

					CComPtr<IDXGIFactory> pIDXGIFactory;
					hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);
					if (FAILED(hr))
					{
						throw_line("failed to get IDXGIFactory");
					}

					hr = pIDXGIFactory->MakeWindowAssociation(Window::GetInstance()->GetHwnd(), DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
					if (FAILED(hr))
					{
						throw_line("failed to MakeWindowAssociation");
					}
				}

				if (FAILED(m_pSwapChain->GetDesc1(&swapChainDesc)))
				{
					throw_line("failed to get DXGI_SWAP_CHAIN_DESC1");
				}

				CComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
				if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
				{
					throw_line("failed to get back buffer");
				}

				m_pSwapChainRenderTarget = RenderTarget::Create(pBackBuffer);

				D3D11_TEXTURE2D_DESC desc{};
				pBackBuffer->GetDesc(&desc);
				
				hr = m_pImmediateContext->QueryInterface(IID_PPV_ARGS(&m_pUserDefinedAnnotation));
				if (FAILED(hr))
				{
					assert(false);
				}

				// Setup the viewport for rendering.
				m_viewport.x = 0.0f;
				m_viewport.y = 0.0f;
				m_viewport.width = static_cast<float>(swapChainDesc.Width);
				m_viewport.height = static_cast<float>(swapChainDesc.Height);
				m_viewport.minDepth = 0.0f;
				m_viewport.maxDepth = 1.0f;

				m_pGBuffer = std::make_unique<GBuffer>(screenSize.x, screenSize.y);

				CComPtr<ID3D11Debug> pDebug = nullptr;
				if (SUCCEEDED(m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pDebug))))
				{
					CComPtr<ID3D11InfoQueue> pQueue = nullptr;
					if (SUCCEEDED(pDebug->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&pQueue))))
					{
						D3D11_MESSAGE_ID hide[] =
						{
							D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
						};

						D3D11_INFO_QUEUE_FILTER filter{};
						filter.DenyList.NumIDs = _countof(hide);
						filter.DenyList.pIDList = hide;
						pQueue->AddStorageFilterEntries(&filter);
					}
				}
			}

			void Device::Impl::InitializeRasterizerState()
			{
				auto CreateRasterizerState = [&](RasterizerState::Type rasterizerState)
				{
					CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);
					switch (rasterizerState)
					{
					case RasterizerState::eSolidCCW:
						desc.AntialiasedLineEnable = false;
						desc.FillMode = D3D11_FILL_SOLID;
						desc.CullMode = D3D11_CULL_BACK;
						desc.DepthBias = 0;
						desc.DepthBiasClamp = 0.f;
						desc.DepthClipEnable = true;
						desc.FrontCounterClockwise = false;
						desc.MultisampleEnable = false;
						desc.ScissorEnable = false;
						desc.SlopeScaledDepthBias = 0.f;
						break;
					case RasterizerState::eSolidCW:
						desc.AntialiasedLineEnable = false;
						desc.FillMode = D3D11_FILL_SOLID;
						desc.CullMode = D3D11_CULL_FRONT;
						desc.DepthBias = 0;
						desc.DepthBiasClamp = 0.f;
						desc.DepthClipEnable = true;
						desc.FrontCounterClockwise = false;
						desc.MultisampleEnable = false;
						desc.ScissorEnable = false;
						desc.SlopeScaledDepthBias = 0.f;
						break;
					case RasterizerState::eSolidCullNone:
						desc.AntialiasedLineEnable = false;
						desc.FillMode = D3D11_FILL_SOLID;
						desc.CullMode = D3D11_CULL_NONE;
						desc.DepthBias = 0;
						desc.DepthBiasClamp = 0.f;
						desc.DepthClipEnable = true;
						desc.FrontCounterClockwise = false;
						desc.MultisampleEnable = false;
						desc.ScissorEnable = false;
						desc.SlopeScaledDepthBias = 0.f;
						break;
					case RasterizerState::eWireframeCCW:
						desc.AntialiasedLineEnable = false;
						desc.FillMode = D3D11_FILL_WIREFRAME;
						desc.CullMode = D3D11_CULL_BACK;
						desc.DepthBias = 0;
						desc.DepthBiasClamp = 0.f;
						desc.DepthClipEnable = true;
						desc.FrontCounterClockwise = false;
						desc.MultisampleEnable = false;
						desc.ScissorEnable = false;
						desc.SlopeScaledDepthBias = 0.f;
						break;
					case RasterizerState::eWireframeCW:
						desc.AntialiasedLineEnable = false;
						desc.FillMode = D3D11_FILL_WIREFRAME;
						desc.CullMode = D3D11_CULL_FRONT;
						desc.DepthBias = 0;
						desc.DepthBiasClamp = 0.f;
						desc.DepthClipEnable = true;
						desc.FrontCounterClockwise = false;
						desc.MultisampleEnable = false;
						desc.ScissorEnable = false;
						desc.SlopeScaledDepthBias = 0.f;
						break;
					case RasterizerState::eWireframeCullNone:
						desc.AntialiasedLineEnable = false;
						desc.FillMode = D3D11_FILL_WIREFRAME;
						desc.CullMode = D3D11_CULL_NONE;
						desc.DepthBias = 0;
						desc.DepthBiasClamp = 0.f;
						desc.DepthClipEnable = true;
						desc.FrontCounterClockwise = false;
						desc.MultisampleEnable = false;
						desc.ScissorEnable = false;
						desc.SlopeScaledDepthBias = 0.f;
						break;
					default:
						assert(false);
					}

					HRESULT hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerStates[rasterizerState]);
					if (FAILED(hr))
					{
						std::string str = string::Format("failed to create rasterizer state : %d", rasterizerState);
						throw_line(str.c_str());
					}
				};

				for (int i = 0; i < RasterizerState::TypeCount; ++i)
				{
					RasterizerState::Type emType = static_cast<RasterizerState::Type>(i);
					CreateRasterizerState(emType);
				}
			}

			void Device::Impl::InitializeBlendState()
			{
				auto CreateBlendState = [&](BlendState::Type blendState)
				{
					CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
					desc.AlphaToCoverageEnable = false;
					desc.IndependentBlendEnable = false;

					switch (blendState)
					{
					case BlendState::eOff:
						// 알파 OFF
						for (int i = 0; i < 8; ++i)
						{
							desc.RenderTarget[i].BlendEnable = false;
							desc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
							desc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
							desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
							desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
						}
						break;
					case BlendState::eLinear:
						// 선형합성
						for (int i = 0; i < 8; ++i)
						{
							desc.RenderTarget[i].BlendEnable = true;
							desc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
							desc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
							desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
							desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
						}
						break;
					case BlendState::eAdditive:
						// 가산합성
						for (int i = 0; i < 8; ++i)
						{
							desc.RenderTarget[i].BlendEnable = true;
							desc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
							desc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
							desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
						}
						break;
					case BlendState::eSubTractive:
						// 감산합성
						for (int i = 0; i < 8; ++i)
						{
							desc.RenderTarget[i].BlendEnable = true;
							desc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
							desc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
							desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_SUBTRACT;
							desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
						}
						break;
					case BlendState::eMultiplicative:
						// 곱셈합성
						for (int i = 0; i < 8; ++i)
						{
							desc.RenderTarget[i].BlendEnable = true;
							desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ZERO;
							desc.RenderTarget[i].DestBlend = D3D11_BLEND_SRC_COLOR;
							desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
						}
						break;
					case BlendState::eSquared:
						// 제곱합성
						for (int i = 0; i < 8; ++i)
						{
							desc.RenderTarget[i].BlendEnable = true;
							desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ZERO;
							desc.RenderTarget[i].DestBlend = D3D11_BLEND_DEST_COLOR;
							desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
						}
						break;
					case BlendState::eNegative:
						// 반전합성
						for (int i = 0; i < 8; ++i)
						{
							desc.RenderTarget[i].BlendEnable = true;
							desc.RenderTarget[i].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
							desc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
							desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
						}
						break;
					case BlendState::eOpacity:
						// 불투명합성
						for (int i = 0; i < 8; ++i)
						{
							desc.RenderTarget[i].BlendEnable = true;
							desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
							desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
							desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
						}
						break;
					case BlendState::eAlphaBlend:
						for (int i = 0; i < 8; ++i)
						{
							desc.RenderTarget[i].BlendEnable = true;
							desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
							desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
							desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
							desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
							desc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
						}
						break;
					default:
						assert(false);
					}

					HRESULT hr = m_pDevice->CreateBlendState(&desc, &m_pBlendStates[blendState]);
					if (FAILED(hr))
					{
						std::string str = string::Format("failed to create blend state : %d", blendState);
						throw_line(str.c_str());
					}
				};

				for (int i = 0; i < BlendState::TypeCount; ++i)
				{
					BlendState::Type emType = static_cast<BlendState::Type>(i);
					CreateBlendState(emType);
				}
			}

			void Device::Impl::InitializeSamplerState()
			{
				auto CreateSamplerState = [&](SamplerState::Type samplerState)
				{
					CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
					switch (samplerState)
					{
					case SamplerState::eMinMagMipLinearWrap:
					case SamplerState::eMinMagMipLinearClamp:
					case SamplerState::eMinMagMipLinearBorder:
					case SamplerState::eMinMagMipLinearMirror:
					case SamplerState::eMinMagMipLinearMirrorOnce:
						desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
						desc.MipLODBias = MipLODBias;
						desc.MaxAnisotropy = 1;
						desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
						desc.BorderColor[0] = 0;
						desc.BorderColor[1] = 0;
						desc.BorderColor[2] = 0;
						desc.BorderColor[3] = 0;
						desc.MinLOD = 0;
						desc.MaxLOD = D3D11_FLOAT32_MAX;
						switch (samplerState)
						{
						case SamplerState::eMinMagMipLinearWrap:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
							break;
						case SamplerState::eMinMagMipLinearClamp:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
							break;
						case SamplerState::eMinMagMipLinearBorder:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
							break;
						case SamplerState::eMinMagMipLinearMirror:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
							break;
						case SamplerState::eMinMagMipLinearMirrorOnce:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							break;
						}
						break;
					case SamplerState::eMinMagLinearMipPointWrap:
					case SamplerState::eMinMagLinearMipPointClamp:
					case SamplerState::eMinMagLinearMipPointBorder:
					case SamplerState::eMinMagLinearMipPointMirror:
					case SamplerState::eMinMagLinearMipPointMirrorOnce:
						desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
						desc.MipLODBias = MipLODBias;
						desc.MaxAnisotropy = 1;
						desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
						desc.BorderColor[0] = 0.f;
						desc.BorderColor[1] = 0.f;
						desc.BorderColor[2] = 0.f;
						desc.BorderColor[3] = 0.f;
						desc.MinLOD = 0.f;
						desc.MaxLOD = D3D11_FLOAT32_MAX;

						switch (samplerState)
						{
						case SamplerState::eMinMagLinearMipPointWrap:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
							break;
						case SamplerState::eMinMagLinearMipPointClamp:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
							break;
						case SamplerState::eMinMagLinearMipPointBorder:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
							break;
						case SamplerState::eMinMagLinearMipPointMirror:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
							break;
						case SamplerState::eMinMagLinearMipPointMirrorOnce:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							break;
						}
						break;
					case SamplerState::eAnisotropicWrap:
					case SamplerState::eAnisotropicClamp:
					case SamplerState::eAnisotropicBorder:
					case SamplerState::eAnisotropicMirror:
					case SamplerState::eAnisotropicMirrorOnce:
						desc.Filter = D3D11_FILTER_ANISOTROPIC;
						desc.MipLODBias = MipLODBias;
						desc.MaxAnisotropy = 16;
						desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
						desc.BorderColor[0] = 0.f;
						desc.BorderColor[1] = 0.f;
						desc.BorderColor[2] = 0.f;
						desc.BorderColor[3] = 0.f;
						desc.MinLOD = 0.f;
						desc.MaxLOD = D3D11_FLOAT32_MAX;
						switch (samplerState)
						{
						case SamplerState::eAnisotropicWrap:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
							break;
						case SamplerState::eAnisotropicClamp:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
							break;
						case SamplerState::eAnisotropicBorder:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
							break;
						case SamplerState::eAnisotropicMirror:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
							break;
						case SamplerState::eAnisotropicMirrorOnce:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							break;
						}
						break;
					case SamplerState::eMinMagMipPointWrap:
					case SamplerState::eMinMagMipPointClamp:
					case SamplerState::eMinMagMipPointBorder:
					case SamplerState::eMinMagMipPointMirror:
					case SamplerState::eMinMagMipPointMirrorOnce:
						desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
						desc.MipLODBias = MipLODBias;
						desc.MaxAnisotropy = 1;
						desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
						desc.BorderColor[0] = 0;
						desc.BorderColor[1] = 0;
						desc.BorderColor[2] = 0;
						desc.BorderColor[3] = 0;
						desc.MinLOD = 0;
						desc.MaxLOD = D3D11_FLOAT32_MAX;
						switch (samplerState)
						{
						case SamplerState::eMinMagMipPointWrap:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
							break;
						case SamplerState::eMinMagMipPointClamp:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
							break;
						case SamplerState::eMinMagMipPointBorder:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
							break;
						case SamplerState::eMinMagMipPointMirror:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
							break;
						case SamplerState::eMinMagMipPointMirrorOnce:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							break;
						}
						break;
					default:
						assert(false);
					}

					HRESULT hr = m_pDevice->CreateSamplerState(&desc, &m_pSamplerStates[samplerState]);
					if (FAILED(hr))
					{
						std::string str = string::Format("failed to create sampler state : %d", samplerState);
						throw_line(str.c_str());
					}
				};

				for (int i = 0; i < SamplerState::TypeCount; ++i)
				{
					SamplerState::Type emType = static_cast<SamplerState::Type>(i);
					CreateSamplerState(emType);
				}
			}

			void Device::Impl::InitializeDepthStencilState()
			{
				auto CreateDepthStencilState = [&](DepthStencilState::Type depthStencilState)
				{
					CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
					switch (depthStencilState)
					{
					case DepthStencilState::eRead_Write_On:
					{
						// 스텐실 상태의 description을 작성합니다.
						desc.DepthEnable = true;
						desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
						desc.DepthFunc = D3D11_COMPARISON_LESS;
						desc.StencilEnable = true;
						desc.StencilReadMask = 0xFF;
						desc.StencilWriteMask = 0xFF;

						// Stencil operations if pixel is front-facing.
						desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
						desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

						// Stencil operations if pixel is back-facing.
						desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
						desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
					}
					break;
					case DepthStencilState::eRead_Write_Off:
					{
						desc.DepthEnable = false;
						desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
						desc.DepthFunc = D3D11_COMPARISON_NEVER;
						desc.StencilEnable = false;
						desc.StencilReadMask = 0xFF;
						desc.StencilWriteMask = 0xFF;

						// Stencil operations if pixel is front-facing.
						desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

						// Stencil operations if pixel is back-facing.
						desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
					}
					break;
					case DepthStencilState::eRead_On_Write_Off:
					{
						desc.DepthEnable = true;
						desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
						desc.DepthFunc = D3D11_COMPARISON_LESS;
						desc.StencilEnable = true;
						desc.StencilReadMask = 0xFF;
						desc.StencilWriteMask = 0xFF;

						// Stencil operations if pixel is front-facing.
						desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
						desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

						// Stencil operations if pixel is back-facing.
						desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
						desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
					}
					break;
					case DepthStencilState::eRead_Off_Write_On:
					{
						// 스텐실 상태의 description을 작성합니다.
						desc.DepthEnable = true;
						desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
						desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
						desc.StencilEnable = true;
						desc.StencilReadMask = 0xFF;
						desc.StencilWriteMask = 0xFF;

						// Stencil operations if pixel is front-facing.
						desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

						// Stencil operations if pixel is back-facing.
						desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
						desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
					}
					break;
					default:
						assert(false);
					}

					HRESULT hr = m_pDevice->CreateDepthStencilState(&desc, &m_pDepthStencilStates[depthStencilState]);
					if (FAILED(hr))
					{
						std::string str = string::Format("failed to create depth stencil state : %d", depthStencilState);
						throw_line(str.c_str());
					}
				};

				for (int i = 0; i < DepthStencilState::TypeCount; ++i)
				{
					DepthStencilState::Type emType = static_cast<DepthStencilState::Type>(i);
					CreateDepthStencilState(emType);
				}
			}

			void Device::Impl::OnScreenShot()
			{
				TRACER_EVENT(__FUNCTIONW__);
				if (m_screeShot.path.empty() == false)
				{
					bool isSuccess = false;

					CComPtr<ID3D11Texture2D> pBackBufferTexture;
					m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBufferTexture));

					if (m_screeShot.format == ScreenShotFormat::eDDS)
					{
						m_screeShot.path += L".dds";

						const HRESULT hr = DirectX::SaveDDSTextureToFile(m_pImmediateContext, pBackBufferTexture, m_screeShot.path.c_str());
						isSuccess = SUCCEEDED(hr);
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

						const HRESULT hr = DirectX::SaveWICTextureToFile(m_pImmediateContext, pBackBufferTexture, formatGuid, m_screeShot.path.c_str());
						isSuccess = SUCCEEDED(hr);
					}

					if (m_screeShot.callback != nullptr)
					{
						m_screeShot.callback(isSuccess, m_screeShot.path);
					}
					m_screeShot = {};
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

				ImGui_ImplDX11_InvalidateDeviceObjects();

				m_pGBuffer->Release();

				m_renderTargetPool.clear();
				m_depthStencilPool.clear();

				m_pSwapChainRenderTarget.reset();

				m_pImmediateContext->ClearState();

				HRESULT hr = m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
				if (FAILED(hr))
				{
					LOG_WARNING(L"Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pDevice->GetDeviceRemovedReason() : hr);
					throw_line("failed to resize device dx11");
				}

				CComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
				if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
				{
					throw_line("failed to get back buffer");
				}

				m_pSwapChainRenderTarget = RenderTarget::Create(pBackBuffer);

				D3D11_TEXTURE2D_DESC desc{};
				pBackBuffer->GetDesc(&desc);

				m_viewport.x = 0.0f;
				m_viewport.y = 0.0f;
				m_viewport.width = static_cast<float>(desc.Width);
				m_viewport.height = static_cast<float>(desc.Height);
				m_viewport.minDepth = 0.0f;
				m_viewport.maxDepth = 1.0f;

				m_pGBuffer->Resize(desc.Width, desc.Height);

				m_selectedDisplayModeIndex = m_pChangeDisplayModeInfo->changeDisplayModeIndex;

				ImGui_ImplDX11_CreateDeviceObjects();

				if (m_pChangeDisplayModeInfo->callback != nullptr)
				{
					m_pChangeDisplayModeInfo->callback(true);
				}
				m_pChangeDisplayModeInfo.reset();

				m_isFirstFrame_Render = true;
				m_isFirstFrame_ImGui = true;
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

			RenderTarget* Device::GetRenderTarget(const D3D11_TEXTURE2D_DESC* pDesc)
			{
				return m_pImpl->GetRenderTarget(pDesc);
			}

			void Device::ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize)
			{
				m_pImpl->ReleaseRenderTargets(ppRenderTarget, nSize);
			}

			DepthStencil* Device::GetDepthStencil(const D3D11_TEXTURE2D_DESC* pDesc)
			{
				return m_pImpl->GetDepthStencil(pDesc);
			}

			void Device::ReleaseDepthStencil(DepthStencil** ppDepthStencil)
			{
				m_pImpl->ReleaseDepthStencil(ppDepthStencil);
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

			const GBuffer* Device::GetGBuffer() const
			{
				return m_pImpl->GetGBuffer();
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

			LightResourceManager* Device::GetLightResourceManager() const
			{
				return m_pImpl->GetLightResourceManager();
			}

			ID3D11Device* Device::GetInterface() const
			{
				return m_pImpl->GetInterface();
			}

			ID3D11DeviceContext* Device::GetImmediateContext() const
			{
				return m_pImpl->GetImmediateContext();
			}

			ID3D11DeviceContext* Device::GetRenderContext() const
			{
				return m_pImpl->GetRenderContext();
			}

			RenderTarget* Device::GetSwapChainRenderTarget() const
			{
				return m_pImpl->GetSwapChainRenderTarget();
			}

			ID3D11RasterizerState* Device::GetRasterizerState(RasterizerState::Type emType) const
			{
				return m_pImpl->GetRasterizerState(emType);
			}

			ID3D11BlendState* Device::GetBlendState(BlendState::Type emType) const
			{
				return m_pImpl->GetBlendState(emType);
			}

			ID3D11SamplerState* Device::GetSamplerState(SamplerState::Type emType) const
			{
				return m_pImpl->GetSamplerState(emType);
			}

			ID3D11DepthStencilState* Device::GetDepthStencilState(DepthStencilState::Type emType) const
			{
				return m_pImpl->GetDepthStencilState(emType);
			}

			ID3DUserDefinedAnnotation* Device::GetUserDefinedAnnotation() const
			{
				return m_pImpl->GetUserDefinedAnnotation();
			}
		}
	}
}