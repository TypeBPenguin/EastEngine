#pragma once

#include "GraphicsInterface/RenderJob.h"
#include "GraphicsInterface/Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class EnvironmentRenderer : public IRenderer
			{
			public:
				EnvironmentRenderer();
				virtual ~EnvironmentRenderer();

			public:
				virtual Type GetType() const { return IRenderer::eEnvironment; }

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