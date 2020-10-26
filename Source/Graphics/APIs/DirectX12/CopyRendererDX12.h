#pragma once

#include "RendererDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderTarget;

			class CopyRenderer : public IRendererDX12
			{
			public:
				CopyRenderer();
				virtual ~CopyRenderer();

			public:
				virtual Type GetType() const { return IRenderer::eCopy; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override {}

			public:
				void Copy_RGBA(RenderTarget* pSource, RenderTarget* pResult);
				void Copy_RGB(RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}