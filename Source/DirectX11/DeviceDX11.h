#pragma once

#include "CommonLib/Singleton.h"
#include "GraphicsInterface/GraphicsInterface.h"

struct D3D11_VIEWPORT;
struct D3D11_TEXTURE2D_DESC;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;

namespace eastengine
{
	namespace graphics
	{
		class ImageBasedLight;

		namespace dx11
		{
			class RenderManager;
			class RenderTarget;
			class GBuffer;
			class VTFManager;

			class Device : public Singleton<Device>
			{
				friend Singleton<Device>;
			private:
				Device();
				virtual ~Device();

			public:
				void Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName);

				void Run(std::function<void()> funcUpdate);

				void Flush(float fElapsedTime);

			public:
				RenderTarget* GetRenderTarget(const D3D11_TEXTURE2D_DESC* pDesc, bool isIncludeLastUseRenderTarget = true);
				void ReleaseRenderTarget(RenderTarget** ppRenderTarget, uint32_t nSize = 1, bool isSetLastRenderTarget = true);

			public:
				HWND GetHwnd() const;
				HINSTANCE GetHInstance() const;
				void AddMessageHandler(std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler);
				const math::UInt2& GetScreenSize() const;
				const D3D11_VIEWPORT* GetViewport() const;
				const GBuffer* GetGBuffer() const;
				ImageBasedLight* GetImageBasedLight() const;
				RenderManager* GetRenderManager() const;
				VTFManager* GetVTFManager() const;

				ID3D11Device* GetInterface() const;
				ID3D11DeviceContext* GetImmediateContext() const;

				RenderTarget* GetSwapChainRenderTarget() const;
				RenderTarget* GetLastUseRenderTarget() const;

				ID3D11RasterizerState* GetRasterizerState(EmRasterizerState::Type emType) const;
				ID3D11BlendState* GetBlendState(EmBlendState::Type emType) const;
				ID3D11SamplerState* GetSamplerState(EmSamplerState::Type emType) const;
				ID3D11DepthStencilState* GetDepthStencilState(EmDepthStencilState::Type emType) const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}