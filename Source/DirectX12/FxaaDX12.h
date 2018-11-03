#pragma once

#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderTarget;

			class Fxaa : public IRendererDX12
			{
			public:
				Fxaa();
				virtual ~Fxaa();

			public:
				virtual Type GetType() const override { return IRenderer::eFxaa; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply(RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}