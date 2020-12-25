#pragma once

#include "CommonLib/Singleton.h"
#include "Graphics/Interface/GraphicsInterface.h"

struct D3D11_TEXTURE2D_DESC;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;
struct ID3DUserDefinedAnnotation;

namespace est
{
	namespace graphics
	{
		class IImageBasedLight;

		namespace dx11
		{
			class RenderManager;
			class RenderTarget;
			class DepthStencil;
			class GBuffer;
			class VTFManager;
			class LightResourceManager;

			class Device : public Singleton<Device>
			{
				friend Singleton<Device>;
			private:
				Device();
				virtual ~Device();

			public:
				void Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler);
				void Run(std::function<bool()> funcUpdate);
				void Cleanup(float elapsedTime);

			public:
				void ScreenShot(ScreenShotFormat format, const std::wstring& path, std::function<void(bool, const std::wstring&)> screenShotCallback);

			public:
				RenderTarget* GetRenderTarget(const D3D11_TEXTURE2D_DESC* pDesc);
				void ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize = 1);

				DepthStencil* GetDepthStencil(const D3D11_TEXTURE2D_DESC* pDesc);
				void ReleaseDepthStencil(DepthStencil** ppDepthStencil);

			public:
				HWND GetHwnd() const;
				HINSTANCE GetHInstance() const;

				const math::uint2& GetScreenSize() const;
				const math::Viewport& GetViewport() const;

				bool IsFullScreen() const;
				void SetFullScreen(bool isFullScreen, std::function<void(bool)> callback);

				const std::vector<DisplayModeDesc>& GetSupportedDisplayModeDesc() const;
				size_t GetSelectedDisplayModeIndex() const;
				void ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback);

				const GBuffer* GetGBuffer() const;
				IImageBasedLight* GetImageBasedLight() const;
				void SetImageBasedLight(IImageBasedLight* pImageBasedLight);
				RenderManager* GetRenderManager() const;
				VTFManager* GetVTFManager() const;
				LightResourceManager* GetLightResourceManager() const;
				
				ID3D11Device* GetInterface() const;
				ID3D11DeviceContext* GetImmediateContext() const;

				ID3D11DeviceContext* GetRenderContext() const;

				RenderTarget* GetSwapChainRenderTarget() const;

				ID3D11RasterizerState* GetRasterizerState(RasterizerState::Type emType) const;
				ID3D11BlendState* GetBlendState(BlendState::Type emType) const;
				ID3D11SamplerState* GetSamplerState(SamplerState::Type emType) const;
				ID3D11DepthStencilState* GetDepthStencilState(DepthStencilState::Type emType) const;

				ID3DUserDefinedAnnotation* GetUserDefinedAnnotation() const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}