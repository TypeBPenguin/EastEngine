#pragma once

#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class EnvironmentRenderer : public IRendererDX12
			{
			public:
				EnvironmentRenderer();
				virtual ~EnvironmentRenderer();

			public:
				virtual Type GetType() const override { return IRenderer::eEnvironment; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Render(Camera* pCamera);
				void Cleanup();

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}