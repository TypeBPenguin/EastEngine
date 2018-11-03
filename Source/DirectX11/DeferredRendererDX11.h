#pragma once

#include "GraphicsInterface/Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class DeferredRenderer : public IRenderer
			{
			public:
				DeferredRenderer();
				virtual ~DeferredRenderer();

			public:
				virtual Type GetType() const { return IRenderer::eDeferred; }

			public:
				void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera);
				void Cleanup();

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}