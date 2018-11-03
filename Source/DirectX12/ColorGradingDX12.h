#pragma once

#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class RenderTarget;

			class ColorGrading : public IRendererDX12
			{
			public:
				ColorGrading();
				virtual ~ColorGrading();

			public:
				virtual Type GetType() const override { return IRenderer::eColorGrading; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply(Camera* pCamera, RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}