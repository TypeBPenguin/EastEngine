#include "stdafx.h"
#include "DeviceDX11.h"

#include "CommonLib/Lock.h"

#include "GraphicsInterface/Window.h"

#include "UtilDX11.h"

#include "GBufferDX11.h"
#include "RenderManagerDX11.h"
#include "VTFManagerDX11.h"

#include "GraphicsInterface/imguiHelper.h"
#include "GraphicsInterface/imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace StrID
{
	RegisterStringID(DeviceDX11);
}

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			const float MipLODBias = -2.f;

			class Device::Impl : public Window
			{
			public:
				Impl();
				virtual ~Impl();

			private:
				virtual void Update() override;
				virtual void Render() override;
				virtual void Present() override;

			public:
				void Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName);
				void Release();

				void Flush(float fElapsedTime);

			public:
				RenderTarget* GetRenderTarget(const D3D11_TEXTURE2D_DESC* pDesc, bool isIncludeLastUseRenderTarget);
				void ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize, bool isSetLastRenderTarget);

			public:
				void MessageHandler(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

			public:
				HWND GetHwnd() const { return m_hWnd; }
				HINSTANCE GetHInstance() const { return m_hInstance; }
				const math::UInt2& GetScreenSize() const { return m_n2ScreenSize; }
				const D3D11_VIEWPORT* GetViewport() const { return &m_viewport; }
				const GBuffer* GetGBuffer() const { return m_pGBuffer.get(); }
				IImageBasedLight* GetImageBasedLight() const { return m_pImageBasedLight; }
				void SetImageBasedLight(IImageBasedLight* pImageBasedLight) { m_pImageBasedLight = pImageBasedLight; }
				RenderManager* GetRenderManager() const { return m_pRenderManager.get(); }
				VTFManager* GetVTFManager() const { return m_pVTFManager.get(); }

				ID3D11Device* GetInterface() const { return m_pDevice; }
				ID3D11DeviceContext* GetImmediateContext() const { return m_pImmediateContext; }

				RenderTarget* GetSwapChainRenderTarget() const { return m_pSwapChainRenderTarget.get(); }
				RenderTarget* GetLastUsedRenderTarget() const { return m_pLastUseRenderTarget; }

				ID3D11RasterizerState* GetRasterizerState(EmRasterizerState::Type emType) const { return m_pRasterizerStates[emType]; }
				ID3D11BlendState* GetBlendState(EmBlendState::Type emType) const { return m_pBlendStates[emType]; }
				ID3D11SamplerState* GetSamplerState(EmSamplerState::Type emType) const { return m_pSamplerStates[emType]; }
				ID3D11DepthStencilState* GetDepthStencilState(EmDepthStencilState::Type emType) const { return m_pDepthStencilStates[emType]; }

				ID3DUserDefinedAnnotation* GetUserDefinedAnnotation() const { return m_pUserDefinedAnnotation; }

			private:
				void InitializeD3D();
				void InitializeRasterizerState();
				void InitializeBlendState();
				void InitializeSamplerState();
				void InitializeDepthStencilState();
				void Resize(uint32_t nWidth, uint32_t nHeight);

			private:
				bool m_isInitislized{ false };

				size_t m_nVideoCardMemory{ 0 };
				std::string m_strVideoCardDescription;
				std::vector<DXGI_MODE_DESC> m_vecDisplayModes;

				thread::SRWLock m_srwLock;

				D3D11_VIEWPORT m_viewport{};

				ID3D11Device* m_pDevice{ nullptr };
				ID3D11DeviceContext* m_pImmediateContext{ nullptr };
				IDXGISwapChain1* m_pSwapChain{ nullptr };

				std::unique_ptr<RenderTarget> m_pSwapChainRenderTarget;

				struct RenderTargetPool
				{
					std::unique_ptr<RenderTarget> pRenderTarget;
					bool isUsing{ false };
					float fUnusedTime{ 0.f };

					RenderTargetPool() = default;
					RenderTargetPool(RenderTargetPool&& source) noexcept
						: pRenderTarget(std::move(source.pRenderTarget))
						, isUsing(std::move(source.isUsing))
						, fUnusedTime(std::move(source.fUnusedTime))
					{
					}
				};
				std::unordered_multimap<RenderTarget::Key, RenderTargetPool> m_ummapRenderTargetPool;
				RenderTarget* m_pLastUseRenderTarget{ nullptr };

				std::array<ID3D11RasterizerState*, EmRasterizerState::TypeCount> m_pRasterizerStates{ nullptr };
				std::array<ID3D11BlendState*, EmBlendState::TypeCount> m_pBlendStates{ nullptr };
				std::array<ID3D11SamplerState*, EmSamplerState::TypeCount> m_pSamplerStates{ nullptr };
				std::array<ID3D11DepthStencilState*, EmDepthStencilState::TypeCount> m_pDepthStencilStates{ nullptr };

				ID3DUserDefinedAnnotation* m_pUserDefinedAnnotation{ nullptr };

				std::unique_ptr<GBuffer> m_pGBuffer;
				IImageBasedLight* m_pImageBasedLight{ nullptr };

				std::unique_ptr<RenderManager> m_pRenderManager;
				std::unique_ptr<VTFManager> m_pVTFManager;
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
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
			}

			void Device::Impl::Render()
			{
				const float f4ClearColor[] = { 0.f, 0.2f, 0.4f, 1.f };
				m_pImmediateContext->ClearRenderTargetView(m_pSwapChainRenderTarget->GetRenderTargetView(), f4ClearColor);

				m_pGBuffer->Clear(m_pImmediateContext);

				m_pVTFManager->Bake();

				m_pRenderManager->Render();
			}

			void Device::Impl::Present()
			{
				ImGui::Render();

				RenderTarget* pSwapChainRenderTarget = GetSwapChainRenderTarget();
				ID3D11RenderTargetView* pRTV[] = { pSwapChainRenderTarget->GetRenderTargetView() };
				m_pImmediateContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

				DXGI_PRESENT_PARAMETERS presentParam{};
				HRESULT hr = m_pSwapChain->Present1(GetOptions().OnVSync ? 1 : 0, 0, &presentParam);
				if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
				{
					LOG_ERROR("Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pDevice->GetDeviceRemovedReason() : hr);

					//HandleDeviceLost();
				}
				else if (FAILED(hr))
				{
					throw_line("failed to swapchain present");
				}
			}

			void Device::Impl::Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName)
			{
				if (m_isInitislized == true)
					return;

				AddMessageHandler(StrID::DeviceDX11, [&](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
				{
					MessageHandler(hWnd, nMsg, wParam, lParam);
				});

				InitializeWindow(nWidth, nHeight, isFullScreen, strApplicationTitle, strApplicationName);
				InitializeD3D();
				InitializeRasterizerState();
				InitializeBlendState();
				InitializeSamplerState();
				InitializeDepthStencilState();

				m_pVTFManager = std::make_unique<VTFManager>();
				m_pRenderManager = std::make_unique<RenderManager>();

				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGui_ImplWin32_Init(m_hWnd);
				ImGui_ImplDX11_Init(m_pDevice, m_pImmediateContext);

				m_isInitislized = true;
			}

			void Device::Impl::Release()
			{
				RemoveMessageHandler(StrID::DeviceDX11);

				ImGui_ImplDX11_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();

				m_pRenderManager.reset();
				m_pVTFManager.reset();

				m_pGBuffer.reset();

				BOOL fs = FALSE;
				m_pSwapChain->GetFullscreenState(&fs, nullptr);

				if (fs == TRUE)
				{
					m_pSwapChain->SetFullscreenState(FALSE, nullptr);
				}

				std::for_each(m_pRasterizerStates.begin(), m_pRasterizerStates.end(), ReleaseSTLObject());
				m_pRasterizerStates.fill(nullptr);

				std::for_each(m_pBlendStates.begin(), m_pBlendStates.end(), ReleaseSTLObject());
				m_pBlendStates.fill(nullptr);

				std::for_each(m_pSamplerStates.begin(), m_pSamplerStates.end(), ReleaseSTLObject());
				m_pSamplerStates.fill(nullptr);

				std::for_each(m_pDepthStencilStates.begin(), m_pDepthStencilStates.end(), ReleaseSTLObject());
				m_pDepthStencilStates.fill(nullptr);

				SafeRelease(m_pUserDefinedAnnotation);

				m_ummapRenderTargetPool.clear();

				m_pSwapChainRenderTarget.reset();

				if (m_pImmediateContext != nullptr)
				{
					m_pImmediateContext->ClearState();
				}

				SafeRelease(m_pSwapChain);
				SafeRelease(m_pImmediateContext);

				util::ReportLiveObjects(m_pDevice);
				SafeRelease(m_pDevice);
			}

			void Device::Impl::Flush(float fElapsedTime)
			{
				m_pRenderManager->Flush();

				for (auto iter = m_ummapRenderTargetPool.begin(); iter != m_ummapRenderTargetPool.end();)
				{
					if (iter->second.isUsing == false)
					{
						iter->second.fUnusedTime += fElapsedTime;

						if (iter->second.fUnusedTime > 120.f)
						{
							iter = m_ummapRenderTargetPool.erase(iter);
							continue;
						}
					}

					++iter;
				}
			}

			RenderTarget* Device::Impl::GetRenderTarget(const D3D11_TEXTURE2D_DESC* pDesc, bool isIncludeLastUseRenderTarget)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				RenderTarget::Key key = RenderTarget::BuildKey(pDesc);
				auto iter_range = m_ummapRenderTargetPool.equal_range(key);
				for (auto iter = iter_range.first; iter != iter_range.second; ++iter)
				{
					RenderTargetPool& pool = iter->second;
					if (pool.isUsing == false)
					{
						if (isIncludeLastUseRenderTarget == false &&
							pool.pRenderTarget.get() == m_pLastUseRenderTarget)
							continue;

						pool.fUnusedTime = 0.f;
						pool.isUsing = true;
						return pool.pRenderTarget.get();
					}
				}

				RenderTargetPool pool;
				pool.pRenderTarget = RenderTarget::Create(pDesc);
				if (pool.pRenderTarget == nullptr)
				{
					throw_line("failed to create render target");
				}

				auto iter_result = m_ummapRenderTargetPool.emplace(key, std::move(pool));
				return iter_result->second.pRenderTarget.get();
			}

			void Device::Impl::ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize, bool isSetLastRenderTarget)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

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

			void Device::Impl::MessageHandler(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
			{
				if (imguiHelper::MessageHandler(hWnd, nMsg, wParam, lParam) == true)
					return;

				switch (nMsg)
				{
				case WM_SIZE:
					if (m_pDevice != nullptr && wParam != SIZE_MINIMIZED)
					{
						uint32_t nWidth = LOWORD(lParam);
						uint32_t nHeight = HIWORD(lParam);

						Resize(nWidth, nHeight);
					}
					break;
				}
			}

			void Device::Impl::InitializeD3D()
			{
				IDXGIFactory4* pDxgiFactory{ nullptr };
				HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&pDxgiFactory));
				if (FAILED(hr))
				{
					throw_line("failed to create dxgi factory");
				}

				IDXGIAdapter1* pAdapter{ nullptr };
				hr = pDxgiFactory->EnumAdapters1(0, &pAdapter);
				if (FAILED(hr))
				{
					throw_line("failed to find adapters");
				}

				IDXGIOutput* pAdapterOutput{ nullptr };
				hr = pAdapter->EnumOutputs(0, &pAdapterOutput);
				if (FAILED(hr))
				{
					throw_line("failed to find adapters");
				}

				uint32_t numModes = 0;
				hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
				if (FAILED(hr))
				{
					throw_line("unsupported display mode");
				}

				m_vecDisplayModes.clear();
				m_vecDisplayModes.resize(numModes);

				hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, &m_vecDisplayModes.front());
				if (FAILED(hr))
				{
					throw_line("unsupported display mode");
				}

				uint32_t numerator = 0;
				uint32_t denominator = 0;
				for (uint32_t i = 0; i < numModes; ++i)
				{
					if (m_vecDisplayModes[i].Width == m_n2ScreenSize.x)
					{
						if (m_vecDisplayModes[i].Height == m_n2ScreenSize.y)
						{
							numerator = m_vecDisplayModes[i].RefreshRate.Numerator;
							denominator = m_vecDisplayModes[i].RefreshRate.Denominator;
						}
					}
				}

				DXGI_ADAPTER_DESC adapterDesc{};
				hr = pAdapter->GetDesc(&adapterDesc);
				if (FAILED(hr))
				{
					throw_line("failed to get DXGI_ADPTER_DESC");
				}

				m_nVideoCardMemory = static_cast<size_t>(adapterDesc.DedicatedVideoMemory / 1024Ui64 / 1024Ui64);

				m_strVideoCardDescription = String::WideToMulti(adapterDesc.Description);

				D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

				uint32_t creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

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

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
				swapChainDesc.Width = 0;
				swapChainDesc.Height = 0;
				swapChainDesc.BufferCount = 2;
				swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swapChainDesc.Stereo = false;
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
				swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
				swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullScreenDesc{};
				swapChainFullScreenDesc.Windowed = m_isFullScreen == false;
				swapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

				if (GetOptions().OnVSync == true)
				{
					swapChainFullScreenDesc.RefreshRate.Numerator = numerator;
					swapChainFullScreenDesc.RefreshRate.Denominator = denominator;
				}
				else
				{
					swapChainFullScreenDesc.RefreshRate.Numerator = 0;
					swapChainFullScreenDesc.RefreshRate.Denominator = 1;
				}

				hr = pDxgiFactory->CreateSwapChainForHwnd(m_pDevice, m_hWnd, &swapChainDesc, &swapChainFullScreenDesc, nullptr, &m_pSwapChain);
				if (FAILED(hr))
				{
					throw_line("failed to create swapchain");
				}

				if (FAILED(m_pSwapChain->GetDesc1(&swapChainDesc)))
				{
					throw_line("failed to get DXGI_SWAP_CHAIN_DESC1");
				}

				ID3D11Texture2D* pBackBuffer = nullptr;
				if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
				{
					throw_line("failed to get back buffer");
				}

				m_pSwapChainRenderTarget = RenderTarget::Create(pBackBuffer);

				SafeRelease(pBackBuffer);

				SafeRelease(pAdapterOutput);
				SafeRelease(pAdapter);
				SafeRelease(pDxgiFactory);

				hr = m_pImmediateContext->QueryInterface(IID_PPV_ARGS(&m_pUserDefinedAnnotation));
				if (FAILED(hr))
				{
					assert(false);
				}

				// Setup the viewport for rendering.
				m_viewport.TopLeftX = 0.0f;
				m_viewport.TopLeftY = 0.0f;
				m_viewport.Width = static_cast<float>(swapChainDesc.Width);
				m_viewport.Height = static_cast<float>(swapChainDesc.Height);
				m_viewport.MinDepth = 0.0f;
				m_viewport.MaxDepth = 1.0f;

				m_n2ScreenSize.x = swapChainDesc.Width;
				m_n2ScreenSize.y = swapChainDesc.Height;

				m_pGBuffer = std::make_unique<GBuffer>(m_n2ScreenSize.x, m_n2ScreenSize.y);
			}

			void Device::Impl::InitializeRasterizerState()
			{
				auto CreateRasterizerState = [&](EmRasterizerState::Type emRasterizerState)
				{
					CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);
					switch (emRasterizerState)
					{
					case EmRasterizerState::eSolidCCW:
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
					case EmRasterizerState::eSolidCW:
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
					case EmRasterizerState::eSolidCullNone:
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
					case EmRasterizerState::eWireframeCCW:
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
					case EmRasterizerState::eWireframeCW:
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
					case EmRasterizerState::eWireframeCullNone:
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

					HRESULT hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerStates[emRasterizerState]);
					if (FAILED(hr))
					{
						std::string str = String::Format("failed to create rasterizer state : %d", emRasterizerState);
						throw_line(str.c_str());
					}
				};

				for (int i = 0; i < EmRasterizerState::TypeCount; ++i)
				{
					EmRasterizerState::Type emType = static_cast<EmRasterizerState::Type>(i);
					CreateRasterizerState(emType);
				}
			}

			void Device::Impl::InitializeBlendState()
			{
				auto CreateBlendState = [&](EmBlendState::Type emBlendState)
				{
					CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
					desc.AlphaToCoverageEnable = false;
					desc.IndependentBlendEnable = false;

					switch (emBlendState)
					{
					case EmBlendState::eOff:
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
					case EmBlendState::eLinear:
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
					case EmBlendState::eAdditive:
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
					case EmBlendState::eSubTractive:
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
					case EmBlendState::eMultiplicative:
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
					case EmBlendState::eSquared:
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
					case EmBlendState::eNegative:
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
					case EmBlendState::eOpacity:
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
					case EmBlendState::eAlphaBlend:
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

					HRESULT hr = m_pDevice->CreateBlendState(&desc, &m_pBlendStates[emBlendState]);
					if (FAILED(hr))
					{
						std::string str = String::Format("failed to create blend state : %d", emBlendState);
						throw_line(str.c_str());
					}
				};

				for (int i = 0; i < EmBlendState::TypeCount; ++i)
				{
					EmBlendState::Type emType = static_cast<EmBlendState::Type>(i);
					CreateBlendState(emType);
				}
			}

			void Device::Impl::InitializeSamplerState()
			{
				auto CreateSamplerState = [&](EmSamplerState::Type emSamplerState)
				{
					CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
					switch (emSamplerState)
					{
					case EmSamplerState::eMinMagMipLinearWrap:
					case EmSamplerState::eMinMagMipLinearClamp:
					case EmSamplerState::eMinMagMipLinearBorder:
					case EmSamplerState::eMinMagMipLinearMirror:
					case EmSamplerState::eMinMagMipLinearMirrorOnce:
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
						switch (emSamplerState)
						{
						case EmSamplerState::eMinMagMipLinearWrap:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
							break;
						case EmSamplerState::eMinMagMipLinearClamp:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
							break;
						case EmSamplerState::eMinMagMipLinearBorder:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
							break;
						case EmSamplerState::eMinMagMipLinearMirror:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
							break;
						case EmSamplerState::eMinMagMipLinearMirrorOnce:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							break;
						}
						break;
					case EmSamplerState::eMinMagLinearMipPointWrap:
					case EmSamplerState::eMinMagLinearMipPointClamp:
					case EmSamplerState::eMinMagLinearMipPointBorder:
					case EmSamplerState::eMinMagLinearMipPointMirror:
					case EmSamplerState::eMinMagLinearMipPointMirrorOnce:
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

						switch (emSamplerState)
						{
						case EmSamplerState::eMinMagLinearMipPointWrap:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
							break;
						case EmSamplerState::eMinMagLinearMipPointClamp:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
							break;
						case EmSamplerState::eMinMagLinearMipPointBorder:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
							break;
						case EmSamplerState::eMinMagLinearMipPointMirror:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
							break;
						case EmSamplerState::eMinMagLinearMipPointMirrorOnce:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							break;
						}
						break;
					case EmSamplerState::eAnisotropicWrap:
					case EmSamplerState::eAnisotropicClamp:
					case EmSamplerState::eAnisotropicBorder:
					case EmSamplerState::eAnisotropicMirror:
					case EmSamplerState::eAnisotropicMirrorOnce:
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
						switch (emSamplerState)
						{
						case EmSamplerState::eAnisotropicWrap:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
							break;
						case EmSamplerState::eAnisotropicClamp:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
							break;
						case EmSamplerState::eAnisotropicBorder:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
							break;
						case EmSamplerState::eAnisotropicMirror:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
							break;
						case EmSamplerState::eAnisotropicMirrorOnce:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							break;
						}
						break;
					case EmSamplerState::eMinMagMipPointWrap:
					case EmSamplerState::eMinMagMipPointClamp:
					case EmSamplerState::eMinMagMipPointBorder:
					case EmSamplerState::eMinMagMipPointMirror:
					case EmSamplerState::eMinMagMipPointMirrorOnce:
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
						switch (emSamplerState)
						{
						case EmSamplerState::eMinMagMipPointWrap:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
							break;
						case EmSamplerState::eMinMagMipPointClamp:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
							break;
						case EmSamplerState::eMinMagMipPointBorder:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
							break;
						case EmSamplerState::eMinMagMipPointMirror:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
							break;
						case EmSamplerState::eMinMagMipPointMirrorOnce:
							desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
							break;
						}
						break;
					default:
						assert(false);
					}

					HRESULT hr = m_pDevice->CreateSamplerState(&desc, &m_pSamplerStates[emSamplerState]);
					if (FAILED(hr))
					{
						std::string str = String::Format("failed to create sampler state : %d", emSamplerState);
						throw_line(str.c_str());
					}
				};

				for (int i = 0; i < EmSamplerState::TypeCount; ++i)
				{
					EmSamplerState::Type emType = static_cast<EmSamplerState::Type>(i);
					CreateSamplerState(emType);
				}
			}

			void Device::Impl::InitializeDepthStencilState()
			{
				auto CreateDepthStencilState = [&](EmDepthStencilState::Type emDepthStencilState)
				{
					CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
					switch (emDepthStencilState)
					{
					case EmDepthStencilState::eRead_Write_On:
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
					case EmDepthStencilState::eRead_Write_Off:
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
					case EmDepthStencilState::eRead_On_Write_Off:
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
					case EmDepthStencilState::eRead_Off_Write_On:
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

					HRESULT hr = m_pDevice->CreateDepthStencilState(&desc, &m_pDepthStencilStates[emDepthStencilState]);
					if (FAILED(hr))
					{
						std::string str = String::Format("failed to create depth stencil state : %d", emDepthStencilState);
						throw_line(str.c_str());
					}
				};

				for (int i = 0; i < EmDepthStencilState::TypeCount; ++i)
				{
					EmDepthStencilState::Type emType = static_cast<EmDepthStencilState::Type>(i);
					CreateDepthStencilState(emType);
				}
			}

			void Device::Impl::Resize(uint32_t nWidth, uint32_t nHeight)
			{
				if (nWidth == m_n2ScreenSize.x && nHeight == m_n2ScreenSize.y)
					return;

				ImGui_ImplDX11_InvalidateDeviceObjects();

				for (auto iter = m_ummapRenderTargetPool.begin(); iter != m_ummapRenderTargetPool.end();)
				{
					if (iter->second.isUsing == true)
					{
						++iter;
					}
					else
					{
						iter = m_ummapRenderTargetPool.erase(iter);
					}
				}
				m_pLastUseRenderTarget = nullptr;
				m_pSwapChainRenderTarget.reset();

				HRESULT hr = m_pSwapChain->ResizeBuffers(0, nWidth, nHeight, DXGI_FORMAT_UNKNOWN, 0);
				if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
				{
					LOG_WARNING("Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pDevice->GetDeviceRemovedReason() : hr);

					//HandleDeviceLost();

					return;
				}

				DXGI_SWAP_CHAIN_DESC1 desc{};
				if (FAILED(m_pSwapChain->GetDesc1(&desc)))
				{
					throw_line("failed to get DXGI_SWAP_CHAIN_DESC1");
				}

				ID3D11Texture2D* pBackBuffer = nullptr;
				if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
				{
					throw_line("failed to get back buffer");
				}

				m_pSwapChainRenderTarget = RenderTarget::Create(pBackBuffer);

				m_viewport.TopLeftX = 0.0f;
				m_viewport.TopLeftY = 0.0f;
				m_viewport.Width = static_cast<float>(desc.Width);
				m_viewport.Height = static_cast<float>(desc.Height);
				m_viewport.MinDepth = 0.0f;
				m_viewport.MaxDepth = 1.0f;

				m_n2ScreenSize.x = desc.Width;
				m_n2ScreenSize.y = desc.Height;

				m_pGBuffer->Resize(m_n2ScreenSize.x, m_n2ScreenSize.y);

				ImGui_ImplDX11_CreateDeviceObjects();
			}

			Device::Device()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			Device::~Device()
			{
			}

			void Device::Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName)
			{
				m_pImpl->Initialize(nWidth, nHeight, isFullScreen, strApplicationTitle, strApplicationName);
			}

			void Device::Run(std::function<void()> funcUpdate)
			{
				m_pImpl->Run(funcUpdate);
			}

			void Device::Flush(float fElapsedTime)
			{
				m_pImpl->Flush(fElapsedTime);
			}

			RenderTarget* Device::GetRenderTarget(const D3D11_TEXTURE2D_DESC* pDesc, bool isIncludeLastUseRenderTarget)
			{
				return m_pImpl->GetRenderTarget(pDesc, isIncludeLastUseRenderTarget);
			}

			void Device::ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize, bool isSetLastRenderTarget)
			{
				m_pImpl->ReleaseRenderTargets(ppRenderTarget, nSize, isSetLastRenderTarget);
			}

			HWND Device::GetHwnd() const
			{
				return m_pImpl->GetHwnd();
			}

			HINSTANCE Device::GetHInstance() const
			{
				return m_pImpl->GetHInstance();
			}

			void Device::AddMessageHandler(const String::StringID& strName, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler)
			{
				m_pImpl->AddMessageHandler(strName, funcHandler);
			}

			void Device::RemoveMessageHandler(const String::StringID& strName)
			{
				m_pImpl->RemoveMessageHandler(strName);
			}

			const math::UInt2& Device::GetScreenSize() const
			{
				return m_pImpl->GetScreenSize();
			}

			const D3D11_VIEWPORT* Device::GetViewport() const
			{
				return m_pImpl->GetViewport();
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

			ID3D11Device* Device::GetInterface() const
			{
				return m_pImpl->GetInterface();
			}

			ID3D11DeviceContext* Device::GetImmediateContext() const
			{
				return m_pImpl->GetImmediateContext();
			}

			RenderTarget* Device::GetSwapChainRenderTarget() const
			{
				return m_pImpl->GetSwapChainRenderTarget();
			}

			RenderTarget* Device::GetLastUsedRenderTarget() const
			{
				return m_pImpl->GetLastUsedRenderTarget();
			}

			ID3D11RasterizerState* Device::GetRasterizerState(EmRasterizerState::Type emType) const
			{
				return m_pImpl->GetRasterizerState(emType);
			}

			ID3D11BlendState* Device::GetBlendState(EmBlendState::Type emType) const
			{
				return m_pImpl->GetBlendState(emType);
			}

			ID3D11SamplerState* Device::GetSamplerState(EmSamplerState::Type emType) const
			{
				return m_pImpl->GetSamplerState(emType);
			}

			ID3D11DepthStencilState* Device::GetDepthStencilState(EmDepthStencilState::Type emType) const
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