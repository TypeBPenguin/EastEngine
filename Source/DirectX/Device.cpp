#include "stdafx.h"
#include "Device.h"

#include "DebugUtil.h"

#include "GBuffers.h"
#include "ImageBasedLight.h"

#include "DeviceLost.h"
#include "SamplerState.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"

namespace EastEngine
{
	namespace Graphics
	{
		Device::Device()
			: m_hWnd(nullptr)
			, m_isInit(false)
			, m_isVsync(false)
			, m_isFullScreen(false)
			, m_pd3dDevice(nullptr)
			, m_pd3dImmediateContext(nullptr)
			, m_pUserDefineAnnotation(nullptr)
			, m_pMainRenderTarget(nullptr)
			, m_pRenderTargetLastUse(nullptr)
			, m_pDepthStencil(nullptr)
			, m_pGBuffers(nullptr)
			, m_pImageBasedLight(nullptr)
#if defined(DEBUG) || defined(_DEBUG)
			, m_pd3dDebug(nullptr)
			, m_pd3dInfoQueue(nullptr)
#endif
		{
			m_pInputLayout.fill(nullptr);
		}

		Device::~Device()
		{
			Release();
		}

		bool Device::Init(HWND hWnd, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync)
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			m_hWnd = hWnd;

			m_isFullScreen = isFullScreen;

			HRESULT hr;

			// vsync(수직동기화 설정), 모니터 갱신 속도에 맞출것이냐, 가능한한 빨리 갱신되도록 하게 할것이냐
			m_isVsync = isVsync;

			// DirectX 그래픽 인터페이스 팩토리를 만든다.
			IDXGIFactory2* pFactory = nullptr;
			hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), (void**)&pFactory);
			if (FAILED(hr))
			{
				Release();
				return false;
			}

			// 팩토리 객체를 사용하여 첫번째 그래픽 카드 인터페이스에 대한 아답터를 만든다.
			IDXGIAdapter1* pAdapter = nullptr;
			hr = pFactory->EnumAdapters1(0, &pAdapter);
			if (FAILED(hr))
			{
				Release();
				return false;
			}

			// 출력 모니터에 대한 첫번째 아답터를 나열한다.
			IDXGIOutput* pAdapterOutput;
			hr = pAdapter->EnumOutputs(0, &pAdapterOutput);
			if (FAILED(hr))
			{
				Release();
				return false;
			}

			// DXGI_FORMAT_R8G8B8A8_UNORM 모니터 출력 디스플레이 포맷에 맞는 모드의 개수를 구한다.
			uint32_t numModes = 0;
			hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
			if (FAILED(hr))
			{
				Release();
				return false;
			}

			// 가능한 모든 모니터와 그래픽카드 조합을 저장할 리스트를 생성한다
			m_vecDisplayModes.clear();
			m_vecDisplayModes.resize(numModes);

			// 디스플레이 모드에 대한 리스트 구조를 채워 넣는다.
			hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, &m_vecDisplayModes.front());
			if (FAILED(hr))
			{
				Release();
				return false;
			}

			// 이제 모든 디스플레이 모드에 대해 화면 너비/높이에 맞는 디스플레이 모드를 찾습니다.
			// 적합한 것을 찾으면 모니터의 새로고침 비율의 분모와 분자 값을 저장합니다.
			uint32_t numerator = 0;
			uint32_t denominator = 0;
			for (uint32_t i = 0; i < numModes; ++i)
			{
				if (m_vecDisplayModes[i].Width == nScreenWidth)
				{
					if (m_vecDisplayModes[i].Height == nScreenHeight)
					{
						numerator = m_vecDisplayModes[i].RefreshRate.Numerator;
						denominator = m_vecDisplayModes[i].RefreshRate.Denominator;
					}
				}
			}

			// 아답터(그래픽카드)의 description을 가져옵니다.
			DXGI_ADAPTER_DESC adapterDesc;
			hr = pAdapter->GetDesc(&adapterDesc);
			if (FAILED(hr))
			{
				Release();
				return false;
			}

			// 현재 그래픽카드의 메모리 용량을 메가바이트 단위로 저장합니다.
			m_nVideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

			// 그래픽카드의 이름을 char형 문자열 배열로 바꾼 뒤 저장합니다.
			m_strVideoCardDescription = String::WideToMulti(adapterDesc.Description);

			D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

			uint32_t creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
			creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			ID3D11DeviceContext* pd3dImmediateContext = nullptr;

			// 스왑 체인, Direct3D 디바이스, Direct3D 디바이스 컨텍스트를 생성합니다.
			D3D_FEATURE_LEVEL emReturnedFeatureLevel = D3D_FEATURE_LEVEL_11_1;
			hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, requestedLevels, ARRAYSIZE(requestedLevels),
				D3D11_SDK_VERSION, &m_pd3dDevice, &emReturnedFeatureLevel, &pd3dImmediateContext);

			if (FAILED(hr) || emReturnedFeatureLevel < D3D_FEATURE_LEVEL_11_0)
			{
				Release();
				return false;
			}

			m_pd3dImmediateContext = new DeviceContext(pd3dImmediateContext);

			DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
			Memory::Clear(swapChainDesc);
			swapChainDesc.Width = 0;
			swapChainDesc.Height = 0;
			swapChainDesc.BufferCount = 2;
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_SHADER_INPUT;
			swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullScreenDesc;
			Memory::Clear(&swapChainFullScreenDesc, sizeof(swapChainFullScreenDesc));
			swapChainFullScreenDesc.Windowed = isFullScreen == false;
			swapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			if (m_isVsync)
			{
				swapChainFullScreenDesc.RefreshRate.Numerator = numerator;
				swapChainFullScreenDesc.RefreshRate.Denominator = denominator;
			}
			else
			{
				swapChainFullScreenDesc.RefreshRate.Numerator = 0;
				swapChainFullScreenDesc.RefreshRate.Denominator = 1;
			}

			hr = pFactory->CreateSwapChainForHwnd(m_pd3dDevice, m_hWnd, &swapChainDesc, &swapChainFullScreenDesc, nullptr, &m_pSwapChain);
			if (FAILED(hr))
			{
				Release();
				return false;
			}

			m_pSwapChain->GetDesc1(&swapChainDesc);

			// 출력 아답터를 할당 해제합니다.
			SafeRelease(pAdapterOutput);

			// 아답터를 할당 해제합니다.
			SafeRelease(pAdapter);

			// 팩토리 객체를 할당 해제합니다.
			SafeRelease(pFactory);

#if defined(DEBUG) || defined(_DEBUG)
			if (SUCCEEDED(m_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_pd3dDebug)))
			{
				if (SUCCEEDED(m_pd3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&m_pd3dInfoQueue)))
				{
					m_pd3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
					m_pd3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

					D3D11_MESSAGE_ID hide[] =
					{
						D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
						// Add more message IDs here as needed
					};

					D3D11_INFO_QUEUE_FILTER filter;
					Memory::Clear(&filter, sizeof(filter));
					filter.DenyList.NumIDs = _countof(hide);
					filter.DenyList.pIDList = hide;
					m_pd3dInfoQueue->AddStorageFilterEntries(&filter);
				}
			}
#endif

			// Setup the viewport for rendering.
			m_viewport.x = 0.0f;
			m_viewport.y = 0.0f;
			m_viewport.width = static_cast<float>(swapChainDesc.Width);
			m_viewport.height = static_cast<float>(swapChainDesc.Height);
			m_viewport.minDepth = 0.0f;
			m_viewport.maxDepth = 1.0f;

			m_n2ScreenSize.x = swapChainDesc.Width;
			m_n2ScreenSize.y = swapChainDesc.Height;

			// Create the viewport.
			m_pd3dImmediateContext->SetViewport(m_viewport);

			if (FAILED(createDepthStencil()))
			{
				Release();
				return false;
			}
			m_pd3dImmediateContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_On, 1);

			if (FAILED(createRasterizeState()))
			{
				Release();
				return false;
			}
			m_pd3dImmediateContext->SetRasterizerState(EmRasterizerState::eSolidCCW);

			if (FAILED(createBlendState()))
			{
				Release();
				return false;
			}
			m_pd3dImmediateContext->SetBlendState(EmBlendState::eOff);

			if (FAILED(createSamplerState()))
			{
				Release();
				return false;
			}

			GBuffers* pGBuffers = new GBuffers;
			m_pGBuffers = pGBuffers;
			if (pGBuffers->Init(m_viewport) == false)
			{
				Release();
				return false;
			}

			m_pImageBasedLight = new ImageBasedLight;

			hr = m_pd3dImmediateContext->GetInterface()->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&m_pUserDefineAnnotation));
			if (FAILED(hr))
			{
				Release();
				return false;
			}

			for (int i = 0; i < ThreadCount; ++i)
			{
				ID3D11DeviceContext* pd3dDeferredContext = nullptr;
				GetInterface()->CreateDeferredContext(0, &pd3dDeferredContext);
				m_pd3dDeferredContext[i] = new DeferredContext(pd3dDeferredContext);
			}

			Debug::Init();

			return true;
		}

		void Device::Release()
		{
			if (m_isInit == false)
				return;

			PreRelease();

			Debug::Release();

			// 종료하기 전에 이렇게 윈도우 모드로 바꾸지 않으면 스왑체인을 할당 해제할 때 예외가 발생한다.
			if (m_pSwapChain != nullptr)
			{
				m_pSwapChain->SetFullscreenState(false, nullptr);
			}

			SafeRelease(m_pUserDefineAnnotation);

			for (auto pInputLayout : m_pInputLayout)
			{
				SafeRelease(pInputLayout);
			}

			std::for_each(m_umapSamplerState.begin(), m_umapSamplerState.end(), [](std::pair<const SamplerStateKey, ISamplerState*>& iter)
			{
				SamplerState* pSamplerState = static_cast<SamplerState*>(iter.second);
				SafeDelete(pSamplerState);
			});
			m_umapSamplerState.clear();
			m_umapSamplerStateKey.clear();

			std::for_each(m_umapBlendState.begin(), m_umapBlendState.end(), [](std::pair<const BlendStateKey, IBlendState*>& iter)
			{
				BlendState* pBlendState = static_cast<BlendState*>(iter.second);
				SafeDelete(pBlendState);
			});
			m_umapBlendState.clear();
			m_umapBlendStateKey.clear();

			std::for_each(m_umapDepthStencilState.begin(), m_umapDepthStencilState.end(), [](std::pair<const DepthStencilStateKey, IDepthStencilState*>& iter)
			{
				DepthStencilState* pDepthStencilState = static_cast<DepthStencilState*>(iter.second);
				SafeDelete(pDepthStencilState);
			});
			m_umapDepthStencilState.clear();
			m_umapDepthStencilStateKey.clear();

			std::for_each(m_umapRasterizerState.begin(), m_umapRasterizerState.end(), [](std::pair<const RasterizerStateKey, IRasterizerState*>& iter)
			{
				RasterizerState* pRasterizerState = static_cast<RasterizerState*>(iter.second);
				SafeDelete(pRasterizerState);
			});
			m_umapRasterizerState.clear();
			m_umapRasterizerStateKey.clear();

			SafeRelease(m_pSwapChain);

			std::for_each(m_pd3dDeferredContext.begin(), m_pd3dDeferredContext.end(), [](DeferredContext* pDeferredContext)
			{
				SafeDelete(pDeferredContext);
			});
			m_pd3dDeferredContext.fill(nullptr);

			SafeDelete(m_pd3dImmediateContext);

#if defined(DEBUG) || defined(_DEBUG)
			m_pd3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
			SafeRelease(m_pd3dInfoQueue);
			SafeRelease(m_pd3dDebug);
#endif

			SafeRelease(m_pd3dDevice);

			m_isInit = false;
		}

		void Device::PreRelease()
		{
			GBuffers* pGBuffers = static_cast<GBuffers*>(m_pGBuffers);
			SafeDelete(pGBuffers);
			m_pGBuffers = nullptr;

			ImageBasedLight* pIBL = static_cast<ImageBasedLight*>(m_pImageBasedLight);
			SafeDelete(pIBL);
			m_pImageBasedLight = nullptr;

			for (auto& iter : m_ummapRenderTarget)
			{
				SafeDelete(iter.second.first);
			}
			m_ummapRenderTarget.clear();

			SafeDelete(m_pDepthStencil);
			SafeDelete(m_pMainRenderTarget);
		}

		bool Device::HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			if (IsInit() == false)
				return false;

			switch (nMsg)
			{
			case WM_SIZE:
				if (m_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
				{
					SafeDelete(m_pMainRenderTarget);

					uint32_t nWidth = LOWORD(lParam);
					uint32_t nHeight = HIWORD(lParam);
					HRESULT hr = m_pSwapChain->ResizeBuffers(0, nWidth, nHeight, DXGI_FORMAT_UNKNOWN, 0);
					if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
					{
						LOG_WARNING("Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pd3dDevice->GetDeviceRemovedReason() : hr);

						HandleDeviceLost();

						return false;
					}

					DXGI_SWAP_CHAIN_DESC1 desc;
					m_pSwapChain->GetDesc1(&desc);

					ID3D11Texture2D* pBackBuffer = nullptr;
					m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));

					m_pMainRenderTarget = IRenderTarget::Create(pBackBuffer);
					if (m_pMainRenderTarget == nullptr)
					{
						LOG_ERROR("Failed resize Render Target");
						Release();
						return false;
					}

					m_viewport.x = 0.0f;
					m_viewport.y = 0.0f;
					m_viewport.width = static_cast<float>(desc.Width);
					m_viewport.height = static_cast<float>(desc.Height);
					m_viewport.minDepth = 0.0f;
					m_viewport.maxDepth = 1.0f;

					SafeRelease(pBackBuffer);
				}
				break;
			case WM_KEYDOWN:
				break;
			case WM_KEYUP:
				break;
			}

			return false;
		}

		void Device::HandleDeviceLost()
		{
			const std::list<IDeviceLost*>& listDeviceLostHandler = IDeviceLost::GetDeviceLostHandler();
			std::for_each(listDeviceLostHandler.begin(), listDeviceLostHandler.end(), [](IDeviceLost* pHandler)
			{
				if (pHandler->OnDeviceLost() == false)
				{
					LOG_ERROR("Failed Handle Device Lost");
					assert(false);
				}
			});

			Release();
			Init(m_hWnd, m_n2ScreenSize.x, m_n2ScreenSize.y, m_isFullScreen, m_isVsync);

			std::for_each(listDeviceLostHandler.begin(), listDeviceLostHandler.end(), [](IDeviceLost* pHandler)
			{
				if (pHandler->OnDeviceRestored() == false)
				{
					LOG_ERROR("Failed Handle Device Restored");
					assert(false);
				}
			});
		}

		void Device::BeginScene(float r, float g, float b, float a)
		{
			if (IsInit() == false)
				return;

			int nThreadID = GetThreadID(ThreadType::eRender);

			// 백버퍼의 내용을 지웁니다.
			m_pd3dDeferredContext[nThreadID]->ClearRenderTargetView(m_pMainRenderTarget, Math::Color(r, g, b, a));

			// 깊이 버퍼를 지웁니다.
			m_pd3dDeferredContext[nThreadID]->ClearDepthStencilView(m_pDepthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);

			for (uint32_t i = 0; i < EmGBuffer::Count; ++i)
			{
				EmGBuffer::Type emType = static_cast<EmGBuffer::Type>(i);
				m_pd3dDeferredContext[nThreadID]->ClearRenderTargetView(m_pGBuffers->GetGBuffer(emType), Math::Color::Transparent);
			}
		}

		void Device::EndScene()
		{
			if (IsInit() == false)
				return;

			int nThreadID = GetThreadID(ThreadType::eRender);
			if (m_pd3dDeferredContext[nThreadID]->FinishCommandList() == false)
			{
				assert(false);
			}
			
			m_pd3dDeferredContext[nThreadID]->ExecuteCommandList(m_pd3dImmediateContext);

			DXGI_PRESENT_PARAMETERS  presentParam;
			Memory::Clear(&presentParam, sizeof(DXGI_PRESENT_PARAMETERS));
			HRESULT hr = m_pSwapChain->Present1(m_isVsync ? 1 : 0, 0, &presentParam);
			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
				LOG_WARNING("Device Lost : Reason code 0x%08X", (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_pd3dDevice->GetDeviceRemovedReason() : hr);

				HandleDeviceLost();
			}
		}

		void Device::Flush()
		{
			std::swap(m_nThreadID[ThreadType::eUpdate], m_nThreadID[ThreadType::eRender]);
		}

		HRESULT Device::CreateInputLayout(EmVertexFormat::Type emInputLayout, const uint8_t* pIAInputSignature, std::size_t IAInputSignatureSize, ID3D11InputLayout** ppInputLayout)
		{
			if (IsInit() == false)
				return E_FAIL;

			if (m_pInputLayout[emInputLayout] != nullptr)
			{
				if (ppInputLayout != nullptr && *ppInputLayout != nullptr)
				{
					*ppInputLayout = m_pInputLayout[emInputLayout];
				}
				return S_OK;
			}

			std::vector<D3D11_INPUT_ELEMENT_DESC> vecInputElementDesc;

			switch (emInputLayout)
			{
			case EmVertexFormat::ePos:
				vecInputElementDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case EmVertexFormat::ePos4:
				vecInputElementDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case EmVertexFormat::ePosCol:
				vecInputElementDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case EmVertexFormat::ePosTex:
				vecInputElementDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case EmVertexFormat::ePosTexCol:
				vecInputElementDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case EmVertexFormat::ePosTexNor:
				vecInputElementDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case EmVertexFormat::ePosTexNorCol:
				vecInputElementDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case EmVertexFormat::ePosTexNorTanBin:
				vecInputElementDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case EmVertexFormat::ePosTexNorWeiIdx:
				vecInputElementDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				vecInputElementDesc.push_back({ "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 });
				break;
			case EmVertexFormat::eUI:
				break;
			case EmVertexFormat::eUnknown:
				return S_OK;
			}

			ID3D11InputLayout* pNewLayout = nullptr;

			HRESULT hr = m_pd3dDevice->CreateInputLayout(&vecInputElementDesc.front(), vecInputElementDesc.size(), pIAInputSignature, IAInputSignatureSize, &pNewLayout);

			if (pNewLayout != nullptr)
			{
				if (ppInputLayout != nullptr && *ppInputLayout != nullptr)
				{
					*ppInputLayout = pNewLayout;
				}

				m_pInputLayout[emInputLayout] = pNewLayout;
			}

			return hr;
		}

		HRESULT Device::CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateBuffer(pDesc, pInitialData, ppBuffer);
		}

		HRESULT Device::CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateUnorderedAccessView(pResource, pDesc, ppUAView);
		}

		HRESULT Device::CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateTexture1D(pDesc, pInitialData, ppTexture1D);
		}

		HRESULT Device::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateTexture2D(pDesc, pInitialData, ppTexture2D);
		}

		HRESULT Device::CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateTexture3D(pDesc, pInitialData, ppTexture3D);
		}

		HRESULT Device::CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateRenderTargetView(pResource, pDesc, ppRTView);
		}

		HRESULT Device::CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
		}

		HRESULT Device::CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppShaderResourceView)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateShaderResourceView(pResource, pDesc, ppShaderResourceView);
		}

		HRESULT Device::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateSamplerState(pSamplerDesc, ppSamplerState);
		}

		HRESULT Device::CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState **ppBlendState)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateBlendState(pBlendStateDesc, ppBlendState);
		}

		HRESULT Device::CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateRasterizerState(pRasterizerDesc, ppRasterizerState);
		}

		HRESULT Device::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState)
		{
			if (IsInit() == false)
				return E_FAIL;

			return m_pd3dDevice->CreateDepthStencilState(pDepthStencilDesc, ppDepthStencilState);
		}

		const SamplerStateDesc& Device::GetSamplerStateDesc(EmSamplerState::Type emSamplerState)
		{
			if (IsInit() == false)
				return SamplerStateDesc::DefaultDesc();

			ISamplerState* pSamplerState = GetSamplerState(emSamplerState);
			if (pSamplerState != nullptr)
				return pSamplerState->GetDesc();

			return SamplerStateDesc::DefaultDesc();
		}

		ISamplerState* Device::GetSamplerState(const SamplerStateKey& key)
		{
			if (IsInit() == false)
				return nullptr;

			auto iter = m_umapSamplerState.find(key);
			if (iter != m_umapSamplerState.end())
				return iter->second;

			return nullptr;
		}

		ISamplerState* Device::GetSamplerState(const SamplerStateDesc& samplerStateDesc)
		{
			if (IsInit() == false)
				return nullptr;

			ISamplerState* pSamplerState = GetSamplerState(samplerStateDesc.GetKey());
			if (pSamplerState != nullptr)
				return pSamplerState;

			pSamplerState = ISamplerState::Create(samplerStateDesc);
			if (pSamplerState == nullptr)
				return nullptr;

			return pSamplerState;
		}

		ISamplerState* Device::GetSamplerState(EmSamplerState::Type emSamplerState)
		{
			if (IsInit() == false)
				return nullptr;

			auto iter = m_umapSamplerStateKey.find(emSamplerState);
			if (iter != m_umapSamplerStateKey.end())
				return GetSamplerState(iter->second);

			SamplerStateDesc samplerDesc;

			switch (emSamplerState)
			{
			case EmSamplerState::eMinMagMipLinearWrap:
			case EmSamplerState::eMinMagMipLinearClamp:
			case EmSamplerState::eMinMagMipLinearBorder:
			case EmSamplerState::eMinMagMipLinearMirror:
			case EmSamplerState::eMinMagMipLinearMirrorOnce:
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				samplerDesc.MipLODBias = -3.f;
				samplerDesc.MaxAnisotropy = 1;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
				samplerDesc.BorderColor[0] = 0;
				samplerDesc.BorderColor[1] = 0;
				samplerDesc.BorderColor[2] = 0;
				samplerDesc.BorderColor[3] = 0;
				samplerDesc.MinLOD = 0;
				samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
				switch (emSamplerState)
				{
				case EmSamplerState::eMinMagMipLinearWrap:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
					break;
				case EmSamplerState::eMinMagMipLinearClamp:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					break;
				case EmSamplerState::eMinMagMipLinearBorder:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
					break;
				case EmSamplerState::eMinMagMipLinearMirror:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
					break;
				case EmSamplerState::eMinMagMipLinearMirrorOnce:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					break;
				}
				break;
			case EmSamplerState::eMinMagLinearMipPointWrap:
			case EmSamplerState::eMinMagLinearMipPointClamp:
			case EmSamplerState::eMinMagLinearMipPointBorder:
			case EmSamplerState::eMinMagLinearMipPointMirror:
			case EmSamplerState::eMinMagLinearMipPointMirrorOnce:
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				samplerDesc.MipLODBias = -3.f;
				samplerDesc.MaxAnisotropy = 1;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
				samplerDesc.BorderColor[0] = 0.f;
				samplerDesc.BorderColor[1] = 0.f;
				samplerDesc.BorderColor[2] = 0.f;
				samplerDesc.BorderColor[3] = 0.f;
				samplerDesc.MinLOD = 0.f;
				samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

				switch (emSamplerState)
				{
				case EmSamplerState::eMinMagLinearMipPointWrap:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
					break;
				case EmSamplerState::eMinMagLinearMipPointClamp:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					break;
				case EmSamplerState::eMinMagLinearMipPointBorder:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
					break;
				case EmSamplerState::eMinMagLinearMipPointMirror:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
					break;
				case EmSamplerState::eMinMagLinearMipPointMirrorOnce:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					break;
				}
				break;
			case EmSamplerState::eAnisotropicWrap:
			case EmSamplerState::eAnisotropicClamp:
			case EmSamplerState::eAnisotropicBorder:
			case EmSamplerState::eAnisotropicMirror:
			case EmSamplerState::eAnisotropicMirrorOnce:
				samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
				samplerDesc.MaxAnisotropy = 16;
				samplerDesc.MipLODBias = -3.f;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
				samplerDesc.BorderColor[0] = 0.f;
				samplerDesc.BorderColor[1] = 0.f;
				samplerDesc.BorderColor[2] = 0.f;
				samplerDesc.BorderColor[3] = 0.f;
				samplerDesc.MinLOD = 0.f;
				samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
				switch (emSamplerState)
				{
				case EmSamplerState::eAnisotropicWrap:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
					break;
				case EmSamplerState::eAnisotropicClamp:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					break;
				case EmSamplerState::eAnisotropicBorder:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
					break;
				case EmSamplerState::eAnisotropicMirror:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
					break;
				case EmSamplerState::eAnisotropicMirrorOnce:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					break;
				}
				break;
			case EmSamplerState::eMinMagMipPointWrap:
			case EmSamplerState::eMinMagMipPointClamp:
			case EmSamplerState::eMinMagMipPointBorder:
			case EmSamplerState::eMinMagMipPointMirror:
			case EmSamplerState::eMinMagMipPointMirrorOnce:
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				samplerDesc.MipLODBias = -3.0f;
				samplerDesc.MaxAnisotropy = 1;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
				samplerDesc.BorderColor[0] = 0;
				samplerDesc.BorderColor[1] = 0;
				samplerDesc.BorderColor[2] = 0;
				samplerDesc.BorderColor[3] = 0;
				samplerDesc.MinLOD = 0;
				samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
				switch (emSamplerState)
				{
				case EmSamplerState::eMinMagMipPointWrap:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
					break;
				case EmSamplerState::eMinMagMipPointClamp:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					break;
				case EmSamplerState::eMinMagMipPointBorder:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
					break;
				case EmSamplerState::eMinMagMipPointMirror:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
					break;
				case EmSamplerState::eMinMagMipPointMirrorOnce:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
					break;
				}
				break;
			default:
				return nullptr;
			}

			ISamplerState* pSamplerState = GetSamplerState(samplerDesc);
			if (pSamplerState == nullptr)
				return nullptr;

			m_umapSamplerStateKey.emplace(emSamplerState, samplerDesc.GetKey());

			return pSamplerState;
		}

		void Device::AddSamplerState(ISamplerState* pSamplerState)
		{
			if (pSamplerState == nullptr)
				return;

			const SamplerStateKey& key = pSamplerState->GetKey();
			if (GetSamplerState(key) != nullptr)
				return;

			m_umapSamplerState.emplace(key, pSamplerState);
		}

		const BlendStateDesc& Device::GetBlendStateDesc(EmBlendState::Type emBlendState)
		{
			if (IsInit() == false)
				return BlendStateDesc::DefaultDesc();

			IBlendState* pBlendState = GetBlendState(emBlendState);
			if (pBlendState != nullptr)
				return pBlendState->GetDesc();

			return BlendStateDesc::DefaultDesc();
		}

		IBlendState* Device::GetBlendState(const BlendStateKey& key)
		{
			if (IsInit() == false)
				return nullptr;

			auto iter = m_umapBlendState.find(key);
			if (iter != m_umapBlendState.end())
				return iter->second;

			return nullptr;
		}

		IBlendState* Device::GetBlendState(const BlendStateDesc& blendStateDesc)
		{
			if (IsInit() == false)
				return nullptr;

			IBlendState* pBlendState = GetBlendState(blendStateDesc.GetKey());
			if (pBlendState != nullptr)
				return pBlendState;

			pBlendState = IBlendState::Create(blendStateDesc);
			if (pBlendState == nullptr)
				return nullptr;

			return pBlendState;
		}

		IBlendState* Device::GetBlendState(EmBlendState::Type emBlendState)
		{
			if (IsInit() == false)
				return nullptr;

			auto iter = m_umapBlendStateKey.find(emBlendState);
			if (iter != m_umapBlendStateKey.end())
				return GetBlendState(iter->second);

			BlendStateDesc blendStateDesc;

			switch (emBlendState)
			{
			case EmBlendState::eOff:
				// 알파 OFF
				for (int i = 0; i < 8; ++i)
				{
					blendStateDesc.RenderTarget[i].BlendEnable = false;
					blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
					blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
					blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
				}
				break;
			case EmBlendState::eLinear:
				// 선형합성
				for (int i = 0; i < 8; ++i)
				{
					blendStateDesc.RenderTarget[i].BlendEnable = true;
					blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
					blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
					blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
				}
				break;
			case EmBlendState::eAdditive:
				// 가산합성
				for (int i = 0; i < 8; ++i)
				{
					blendStateDesc.RenderTarget[i].BlendEnable = true;
					blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
					blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
				}
				break;
			case EmBlendState::eSubTractive:
				// 감산합성
				for (int i = 0; i < 8; ++i)
				{
					blendStateDesc.RenderTarget[i].BlendEnable = true;
					blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
					blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_SUBTRACT;
					blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
				}
				break;
			case EmBlendState::eMultiplicative:
				// 곱셈합성
				for (int i = 0; i < 8; ++i)
				{
					blendStateDesc.RenderTarget[i].BlendEnable = true;
					blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ZERO;
					blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_SRC_COLOR;
					blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
				}
				break;
			case EmBlendState::eSquared:
				// 제곱합성
				for (int i = 0; i < 8; ++i)
				{
					blendStateDesc.RenderTarget[i].BlendEnable = true;
					blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ZERO;
					blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_DEST_COLOR;
					blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
				}
				break;
			case EmBlendState::eNegative:
				// 반전합성
				for (int i = 0; i < 8; ++i)
				{
					blendStateDesc.RenderTarget[i].BlendEnable = true;
					blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
					blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
					blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
				}
				break;
			case EmBlendState::eOpacity:
				// 불투명합성
				for (int i = 0; i < 8; ++i)
				{
					blendStateDesc.RenderTarget[i].BlendEnable = true;
					blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
					blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
					blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
				}
				break;
			case EmBlendState::eAlphaBlend:
				for (int i = 0; i < 8; ++i)
				{
					blendStateDesc.RenderTarget[i].BlendEnable = true;
					blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
					blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
				}
				break;
			default:
				return nullptr;
			}

			IBlendState* pBlendState = GetBlendState(blendStateDesc);
			if (pBlendState == nullptr)
				return nullptr;

			m_umapBlendStateKey.emplace(emBlendState, blendStateDesc.GetKey());

			return pBlendState;
		}

		void Device::AddBlendState(IBlendState* pBlendState)
		{
			if (pBlendState == nullptr)
				return;

			const BlendStateKey& key = pBlendState->GetKey();
			if (GetBlendState(key) != nullptr)
				return;

			m_umapBlendState.emplace(key, pBlendState);
		}

		const RasterizerStateDesc& Device::GetRasterizerStateDesc(EmRasterizerState::Type emRasterizerState)
		{
			if (IsInit() == false)
				return RasterizerStateDesc::DefaultDesc();

			IRasterizerState* pRasterizerState = GetRasterizerState(emRasterizerState);
			if (pRasterizerState != nullptr)
				return pRasterizerState->GetDesc();

			return RasterizerStateDesc::DefaultDesc();
		}

		IRasterizerState* Device::GetRasterizerState(const RasterizerStateKey& key)
		{
			if (IsInit() == false)
				return nullptr;

			auto iter = m_umapRasterizerState.find(key);
			if (iter != m_umapRasterizerState.end())
				return iter->second;

			return nullptr;
		}

		IRasterizerState* Device::GetRasterizerState(const RasterizerStateDesc& rasterizerStateDesc)
		{
			if (IsInit() == false)
				return nullptr;

			IRasterizerState* pRasterizerState = GetRasterizerState(rasterizerStateDesc.GetKey());
			if (pRasterizerState != nullptr)
				return pRasterizerState;

			pRasterizerState = IRasterizerState::Create(rasterizerStateDesc);
			if (pRasterizerState == nullptr)
				return nullptr;

			return pRasterizerState;
		}

		IRasterizerState* Device::GetRasterizerState(EmRasterizerState::Type emRasterizerState)
		{
			if (IsInit() == false)
				return nullptr;

			auto iter = m_umapRasterizerStateKey.find(emRasterizerState);
			if (iter != m_umapRasterizerStateKey.end())
				return GetRasterizerState(iter->second);

			RasterizerStateDesc rasterizerStateDesc;

			switch (emRasterizerState)
			{
			case EmRasterizerState::eSolidCCW:
				rasterizerStateDesc.AntialiasedLineEnable = false;
				rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
				rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
				rasterizerStateDesc.DepthBias = 0;
				rasterizerStateDesc.DepthBiasClamp = 0.f;
				rasterizerStateDesc.DepthClipEnable = true;
				rasterizerStateDesc.FrontCounterClockwise = false;
				rasterizerStateDesc.MultisampleEnable = false;
				rasterizerStateDesc.ScissorEnable = false;
				rasterizerStateDesc.SlopeScaledDepthBias = 0.f;
				break;
			case EmRasterizerState::eSolidCW:
				rasterizerStateDesc.AntialiasedLineEnable = false;
				rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
				rasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
				rasterizerStateDesc.DepthBias = 0;
				rasterizerStateDesc.DepthBiasClamp = 0.f;
				rasterizerStateDesc.DepthClipEnable = true;
				rasterizerStateDesc.FrontCounterClockwise = false;
				rasterizerStateDesc.MultisampleEnable = false;
				rasterizerStateDesc.ScissorEnable = false;
				rasterizerStateDesc.SlopeScaledDepthBias = 0.f;
				break;
			case EmRasterizerState::eSolidCullNone:
				rasterizerStateDesc.AntialiasedLineEnable = false;
				rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
				rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
				rasterizerStateDesc.DepthBias = 0;
				rasterizerStateDesc.DepthBiasClamp = 0.f;
				rasterizerStateDesc.DepthClipEnable = true;
				rasterizerStateDesc.FrontCounterClockwise = false;
				rasterizerStateDesc.MultisampleEnable = false;
				rasterizerStateDesc.ScissorEnable = false;
				rasterizerStateDesc.SlopeScaledDepthBias = 0.f;
				break;
			case EmRasterizerState::eWireframeCCW:
				rasterizerStateDesc.AntialiasedLineEnable = false;
				rasterizerStateDesc.FillMode = D3D11_FILL_WIREFRAME;
				rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
				rasterizerStateDesc.DepthBias = 0;
				rasterizerStateDesc.DepthBiasClamp = 0.f;
				rasterizerStateDesc.DepthClipEnable = true;
				rasterizerStateDesc.FrontCounterClockwise = false;
				rasterizerStateDesc.MultisampleEnable = false;
				rasterizerStateDesc.ScissorEnable = false;
				rasterizerStateDesc.SlopeScaledDepthBias = 0.f;
				break;
			case EmRasterizerState::eWireframeCW:
				rasterizerStateDesc.AntialiasedLineEnable = false;
				rasterizerStateDesc.FillMode = D3D11_FILL_WIREFRAME;
				rasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
				rasterizerStateDesc.DepthBias = 0;
				rasterizerStateDesc.DepthBiasClamp = 0.f;
				rasterizerStateDesc.DepthClipEnable = true;
				rasterizerStateDesc.FrontCounterClockwise = false;
				rasterizerStateDesc.MultisampleEnable = false;
				rasterizerStateDesc.ScissorEnable = false;
				rasterizerStateDesc.SlopeScaledDepthBias = 0.f;
				break;
			case EmRasterizerState::eWireframeCullNone:
				rasterizerStateDesc.AntialiasedLineEnable = false;
				rasterizerStateDesc.FillMode = D3D11_FILL_WIREFRAME;
				rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
				rasterizerStateDesc.DepthBias = 0;
				rasterizerStateDesc.DepthBiasClamp = 0.f;
				rasterizerStateDesc.DepthClipEnable = true;
				rasterizerStateDesc.FrontCounterClockwise = false;
				rasterizerStateDesc.MultisampleEnable = false;
				rasterizerStateDesc.ScissorEnable = false;
				rasterizerStateDesc.SlopeScaledDepthBias = 0.f;
				break;
			default:
				return nullptr;
			}

			IRasterizerState* pRasterizerState = GetRasterizerState(rasterizerStateDesc);
			if (pRasterizerState == nullptr)
				return nullptr;

			m_umapRasterizerStateKey.emplace(emRasterizerState, rasterizerStateDesc.GetKey());

			return pRasterizerState;
		}

		void Device::AddRasterizerState(IRasterizerState* pRasterizerState)
		{
			if (pRasterizerState == nullptr)
				return;

			const RasterizerStateKey& key = pRasterizerState->GetKey();
			if (GetRasterizerState(key) != nullptr)
				return;

			m_umapRasterizerState.emplace(key, pRasterizerState);
		}

		const DepthStencilStateDesc& Device::GetDepthStencilStateDesc(EmDepthStencilState::Type emDepthStencilState)
		{
			if (IsInit() == false)
				return DepthStencilStateDesc::DefaultDesc();

			IDepthStencilState* pDepthStencilState = GetDepthStencilState(emDepthStencilState);
			if (pDepthStencilState != nullptr)
				return pDepthStencilState->GetDesc();

			return DepthStencilStateDesc::DefaultDesc();
		}

		IDepthStencilState* Device::GetDepthStencilState(const DepthStencilStateKey& key)
		{
			if (IsInit() == false)
				return nullptr;

			auto iter = m_umapDepthStencilState.find(key);
			if (iter != m_umapDepthStencilState.end())
				return iter->second;

			return nullptr;
		}

		IDepthStencilState* Device::GetDepthStencilState(const DepthStencilStateDesc& depthStencilStateDesc)
		{
			if (IsInit() == false)
				return nullptr;

			IDepthStencilState* pDepthStencilState = GetDepthStencilState(depthStencilStateDesc.GetKey());
			if (pDepthStencilState != nullptr)
				return pDepthStencilState;

			pDepthStencilState = IDepthStencilState::Create(depthStencilStateDesc);
			if (pDepthStencilState == nullptr)
				return nullptr;

			return pDepthStencilState;
		}

		IDepthStencilState* Device::GetDepthStencilState(EmDepthStencilState::Type emDepthStencilState)
		{
			if (IsInit() == false)
				return nullptr;

			auto iter = m_umapDepthStencilStateKey.find(emDepthStencilState);
			if (iter != m_umapDepthStencilStateKey.end())
				return GetDepthStencilState(iter->second);

			DepthStencilStateDesc depthStencilDesc;

			switch (emDepthStencilState)
			{
			case EmDepthStencilState::eRead_Write_On:
			{
				// 스텐실 상태의 description을 작성합니다.
				depthStencilDesc.DepthEnable = true;
				depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
				depthStencilDesc.StencilEnable = true;
				depthStencilDesc.StencilReadMask = 0xFF;
				depthStencilDesc.StencilWriteMask = 0xFF;

				// Stencil operations if pixel is front-facing.
				depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
				depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

				// Stencil operations if pixel is back-facing.
				depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
				depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			}
			break;
			case EmDepthStencilState::eRead_Write_Off:
			{
				depthStencilDesc.DepthEnable = false;
				depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				depthStencilDesc.DepthFunc = D3D11_COMPARISON_NEVER;
				depthStencilDesc.StencilEnable = false;
				depthStencilDesc.StencilReadMask = 0xFF;
				depthStencilDesc.StencilWriteMask = 0xFF;

				// Stencil operations if pixel is front-facing.
				depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

				// Stencil operations if pixel is back-facing.
				depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			}
			break;
			case EmDepthStencilState::eRead_On_Write_Off:
			{
				depthStencilDesc.DepthEnable = true;
				depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
				depthStencilDesc.StencilEnable = true;
				depthStencilDesc.StencilReadMask = 0xFF;
				depthStencilDesc.StencilWriteMask = 0xFF;

				// Stencil operations if pixel is front-facing.
				depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
				depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

				// Stencil operations if pixel is back-facing.
				depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
				depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			}
			break;
			case EmDepthStencilState::eRead_Off_Write_On:
			{
				// 스텐실 상태의 description을 작성합니다.
				depthStencilDesc.DepthEnable = true;
				depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
				depthStencilDesc.StencilEnable = true;
				depthStencilDesc.StencilReadMask = 0xFF;
				depthStencilDesc.StencilWriteMask = 0xFF;

				// Stencil operations if pixel is front-facing.
				depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

				// Stencil operations if pixel is back-facing.
				depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			}
			break;
			default:
				return nullptr;
			}

			IDepthStencilState* pDepthStencilState = GetDepthStencilState(depthStencilDesc);
			if (pDepthStencilState == nullptr)
				return nullptr;

			m_umapDepthStencilStateKey.emplace(emDepthStencilState, depthStencilDesc.GetKey());

			return pDepthStencilState;
		}

		void Device::AddDepthStencilState(IDepthStencilState* pDepthStencilState)
		{
			if (pDepthStencilState == nullptr)
				return;

			const DepthStencilStateKey& key = pDepthStencilState->GetKey();
			if (GetDepthStencilState(key) != nullptr)
				return;

			m_umapDepthStencilState.emplace(key, pDepthStencilState);
		}

		IRenderTarget* Device::GetRenderTarget(const RenderTargetDesc2D& renderTargetInfo, bool isIncludeLastUseRenderTarget)
		{
			if (IsInit() == false)
				return nullptr;

			RenderTargetKey keyRenderTarget = renderTargetInfo.GetKey();
			IRenderTarget* pRenderTarget = GetRenderTarget(keyRenderTarget, isIncludeLastUseRenderTarget);
			if (pRenderTarget != nullptr)
				return pRenderTarget;

			pRenderTarget = createRenderTarget(renderTargetInfo);
			m_ummapRenderTarget.emplace(keyRenderTarget, std::make_pair(pRenderTarget, true));

			return pRenderTarget;
		}

		IRenderTarget* Device::GetRenderTarget(const RenderTargetKey& renderTargetKey, bool isIncludeLastUseRenderTarget)
		{
			if (IsInit() == false)
				return nullptr;

			auto iter_pair = m_ummapRenderTarget.equal_range(renderTargetKey);
			for (auto iter = iter_pair.first; iter != iter_pair.second; ++iter)
			{
				if (iter->second.second == false)
				{
					if (isIncludeLastUseRenderTarget == false &&
						iter->second.first == m_pRenderTargetLastUse)
						continue;

					iter->second.second = true;
					return iter->second.first;
				}
			}

			return nullptr;
		}

		void Device::ReleaseRenderTargets(IRenderTarget** ppRenderTarget, uint32_t nSize, bool isSetLastRenderTarget)
		{
			if (IsInit() == false)
				return;

			for (uint32_t i = 0; i < nSize; ++i)
			{
				if (ppRenderTarget[i] == nullptr)
					continue;

				auto iter_pair = m_ummapRenderTarget.equal_range(ppRenderTarget[i]->GetKey());
				if (iter_pair.first == iter_pair.second)
					continue;

				for (auto iter = iter_pair.first; iter != iter_pair.second; ++iter)
				{
					if (iter->second.first == ppRenderTarget[i])
					{
						iter->second.second = false;

						if (isSetLastRenderTarget == true)
						{
							m_pRenderTargetLastUse = iter->second.first;
						}
						ppRenderTarget[i] = nullptr;
						break;
					}
				}
			}
		}

		ID3D11InputLayout* Device::GetInputLayout(EmVertexFormat::Type emVertexFormat)
		{
			if (IsInit() == false)
				return nullptr;

			return m_pInputLayout[emVertexFormat];
		}

		bool Device::createRasterizeState()
		{
			if (IsInit() == false)
				return false;

			for (uint32_t i = 0; i < EmRasterizerState::TypeCount; ++i)
			{
				if (GetRasterizerState(static_cast<EmRasterizerState::Type>(i)) == nullptr)
					return false;
			}

			return true;
		}

		IRenderTarget* Device::createRenderTarget(const RenderTargetDesc2D& renderTargetInfo)
		{
			if (IsInit() == false)
				return nullptr;

			return IRenderTarget::Create(renderTargetInfo);
		}

		bool Device::createDepthStencil()
		{
			if (IsInit() == false)
				return false;

			DepthStencilDesc depthStencilDesc(
				DXGI_FORMAT_R32_TYPELESS,
				m_n2ScreenSize.x,
				m_n2ScreenSize.y);
			depthStencilDesc.Build();

			m_pDepthStencil = IDepthStencil::Create(depthStencilDesc);
			if (m_pDepthStencil == nullptr)
				return false;

			for (uint32_t i = 0; i < EmDepthStencilState::TypeCount; ++i)
			{
				if (GetDepthStencilState(static_cast<EmDepthStencilState::Type>(i)) == nullptr)
					return false;
			}

			return true;
		}

		bool Device::createBlendState()
		{
			if (IsInit() == false)
				return false;

			for (uint32_t i = 0; i < EmBlendState::TypeCount; ++i)
			{
				if (GetBlendState(static_cast<EmBlendState::Type>(i)) == nullptr)
					return false;
			}

			return true;
		}

		bool Device::createSamplerState()
		{
			if (IsInit() == false)
				return false;

			for (uint32_t i = 0; i < EmSamplerState::TypeCount; ++i)
			{
				if (GetSamplerState(static_cast<EmSamplerState::Type>(i)) == nullptr)
					return false;
			}

			return true;
		}
	}
}