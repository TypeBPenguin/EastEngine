#pragma once

#include "RendererDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderTarget;

			class DownScale : public IRendererDX12
			{
			public:
				DownScale();
				virtual ~DownScale();

			public:
				virtual Type GetType() const override { return IRenderer::eDownScale; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply4SW(const RenderTarget* pSource, RenderTarget* pResult, bool isLuminance = false);
				void Apply16SW(const RenderTarget* pSource, RenderTarget* pResult);

				void ApplyHW(const RenderTarget* pSource, RenderTarget* pResult);
				void Apply16HW(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}