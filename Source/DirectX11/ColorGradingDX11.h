#pragma once

#include "GraphicsInterface/Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class RenderTarget;

			class ColorGrading : public IRenderer
			{
			public:
				ColorGrading();
				virtual ~ColorGrading();

			public:
				virtual Type GetType() const { return IRenderer::eColorGrading; }

			public:
				void Apply(Camera* pCamera, const RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}