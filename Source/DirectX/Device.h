#pragma once

#include "CommonLib/Singleton.h"

#include "D3DInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class DeviceContext;
		class DeferredContext;

		class Device : public IDevice, public Singleton<Device>
		{
			friend Singleton<Device>;
		private:
			Device();
			virtual ~Device();

		public:
			virtual ID3D11Device* GetInterface() override { return m_pd3dDevice; }

		public:
			bool Initialize(HWND hWnd, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync);
			void Release();
			void PreRelease();

			bool HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);
			void HandleDeviceLost();

			void BeginScene(float r, float g, float b, float a);
			void EndScene();

			void Flush();

		public:
			virtual HRESULT CreateInputLayout(EmVertexFormat::Type emInputLayout, const uint8_t* pIAInputSignature, std::size_t IAInputSignatureSize, ID3D11InputLayout** ppInputLayout = nullptr) override;
			virtual HRESULT CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) override;
			virtual HRESULT CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView) override;
			virtual HRESULT CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D) override;
			virtual HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) override;
			virtual HRESULT CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D) override;
			virtual HRESULT CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView) override;
			virtual HRESULT CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView) override;
			virtual HRESULT CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppShaderResourceView) override;
			virtual HRESULT CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState) override;
			virtual HRESULT CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState **ppBlendState) override;
			virtual HRESULT CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState) override;
			virtual HRESULT CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) override;

		public:
			virtual IDeviceContext* GetImmediateContext() override;

			virtual int GetThreadID(ThreadType emThreadType) const override;
			virtual IDeviceContext* GetDeferredContext(int nThreadID) override;

			virtual IGBuffers* GetGBuffers() override { return m_pGBuffers; }
			virtual IImageBasedLight* GetImageBasedLight() override { return m_pImageBasedLight; }
			virtual IRenderTarget* GetMainRenderTarget() override { return m_pMainRenderTarget; }
			virtual IDepthStencil* GetMainDepthStencil() override { return m_pDepthStencil; }
			virtual IRenderTarget* GetLastUseRenderTarget() override { return m_pRenderTargetLastUse; }

			// required call function ReleaseRenderTargets
			virtual IRenderTarget* GetRenderTarget(const RenderTargetDesc2D& renderTargetDesc, bool isIncludeLastUseRenderTarget = true) override;
			virtual IRenderTarget* GetRenderTarget(const RenderTargetKey& renderTargetKey, bool isIncludeLastUseRenderTarget = true) override;
			virtual void ReleaseRenderTargets(IRenderTarget** ppRenderTarget, uint32_t nSize = 1, bool isSetLastRenderTarget = true) override;

			virtual ID3D11InputLayout* GetInputLayout(EmVertexFormat::Type emVertexFormat) override;

			virtual const SamplerStateDesc& GetSamplerStateDesc(EmSamplerState::Type emSamplerState) override;
			virtual ISamplerState* GetSamplerState(const SamplerStateKey& key) override;
			virtual ISamplerState* GetSamplerState(const SamplerStateDesc& samplerStateDesc) override;
			virtual ISamplerState* GetSamplerState(EmSamplerState::Type emSamplerState) override;

			virtual const BlendStateDesc& GetBlendStateDesc(EmBlendState::Type emBlendState) override;
			virtual IBlendState* GetBlendState(const BlendStateKey& key) override;
			virtual IBlendState* GetBlendState(const BlendStateDesc& blendStateDesc) override;
			virtual IBlendState* GetBlendState(EmBlendState::Type emBlendState) override;

			virtual const RasterizerStateDesc& GetRasterizerStateDesc(EmRasterizerState::Type emRasterizerState) override;
			virtual IRasterizerState* GetRasterizerState(const RasterizerStateKey& key) override;
			virtual IRasterizerState* GetRasterizerState(const RasterizerStateDesc& rasterizerStateDesc) override;
			virtual IRasterizerState* GetRasterizerState(EmRasterizerState::Type emRasterizerState) override;

			virtual const DepthStencilStateDesc& GetDepthStencilStateDesc(EmDepthStencilState::Type emDepthStencilState) override;
			virtual IDepthStencilState* GetDepthStencilState(const DepthStencilStateKey& key) override;
			virtual IDepthStencilState* GetDepthStencilState(const DepthStencilStateDesc& depthStencilStateDesc) override;
			virtual IDepthStencilState* GetDepthStencilState(EmDepthStencilState::Type emDepthStencilState) override;

			virtual const math::Viewport& GetViewport() override { return m_viewport;}

		public:
			virtual void SetDebugName(ID3D11DeviceChild* pResource, const std::string& strName) override
			{
#ifdef _DEBUG
				pResource->SetPrivateData(WKPDID_D3DDebugObjectName, strName.size(), strName.c_str());
#endif // _DEBUG
			}

		public:
			virtual HWND GetHWND() override { return m_hWnd; }
			virtual const math::uint2& GetScreenSize() const override { return m_n2ScreenSize; }
			virtual bool IsFullScreen() const override { return m_isFullScreen; }
			virtual bool IsVSync() const override { return m_isVsync; }
			virtual void SetVSync(bool isVSync) override { m_isVsync = isVSync; }

			virtual void GetVideoCardInfo(std::string& strCardName, int& nMemory) const override { strCardName = m_strVideoCardDescription; nMemory = m_nVideoCardMemory; }

		public:
			bool IsInit() { return m_isInit; }
			void AddSamplerState(ISamplerState* pSamplerState);
			void AddBlendState(IBlendState* pBlendState);
			void AddRasterizerState(IRasterizerState* pRasterizerState);
			void AddDepthStencilState(IDepthStencilState* pDepthStencilState);

		private:
			IRenderTarget* createRenderTarget(const RenderTargetDesc2D& renderTargetInfo);
			
			bool createDepthStencil();
			bool createRasterizeState();
			bool createBlendState();
			bool createSamplerState();
			
		private:
			HWND m_hWnd;

			math::uint2 m_n2ScreenSize;

			bool m_isInit;
			bool m_isVsync;
			bool m_isFullScreen;
			int m_nVideoCardMemory;
			std::string m_strVideoCardDescription;

			std::vector<DXGI_MODE_DESC> m_vecDisplayModes;

			math::Viewport m_viewport;

			IDXGISwapChain1* m_pSwapChain;
			ID3D11Device* m_pd3dDevice;
			DeviceContext* m_pd3dImmediateContext{ nullptr };

			std::array<int, ThreadCount> m_nThreadID{ 0, 1 };
			std::array<DeferredContext*, ThreadCount> m_pd3dDeferredContext{ nullptr };

			IRenderTarget* m_pMainRenderTarget;
			IRenderTarget* m_pRenderTargetLastUse;
			IDepthStencil* m_pDepthStencil;

			std::array<ID3D11InputLayout*, EmVertexFormat::eCount> m_pInputLayout;

			std::unordered_multimap<RenderTargetKey, std::pair<IRenderTarget*, bool>> m_ummapRenderTarget;

			std::unordered_map<SamplerStateKey, ISamplerState*> m_umapSamplerState;
			std::unordered_map<EmSamplerState::Type, SamplerStateKey> m_umapSamplerStateKey;

			std::unordered_map<BlendStateKey, IBlendState*> m_umapBlendState;
			std::unordered_map<EmBlendState::Type, BlendStateKey> m_umapBlendStateKey;

			std::unordered_map<RasterizerStateKey, IRasterizerState*> m_umapRasterizerState;
			std::unordered_map<EmRasterizerState::Type, RasterizerStateKey> m_umapRasterizerStateKey;

			std::unordered_map<DepthStencilStateKey, IDepthStencilState*> m_umapDepthStencilState;
			std::unordered_map<EmDepthStencilState::Type, DepthStencilStateKey> m_umapDepthStencilStateKey;

			IGBuffers* m_pGBuffers;
			IImageBasedLight* m_pImageBasedLight;

#if defined(DEBUG) || defined(_DEBUG)
			ID3D11Debug* m_pd3dDebug;
			ID3D11InfoQueue* m_pd3dInfoQueue;
#endif
		};
	}
}