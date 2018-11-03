#pragma once

#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class DeferredRenderer : public IRendererDX12
			{
			public:
				DeferredRenderer();
				virtual ~DeferredRenderer();

			public:
				virtual Type GetType() const override { return IRenderer::eDeferred; }
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