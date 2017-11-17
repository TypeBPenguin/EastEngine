#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class SScreenQuad;

		class PreProcessingRenderer : public IRenderer
		{
		public:
			PreProcessingRenderer();
			virtual ~PreProcessingRenderer();

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			SScreenQuad* s_pScreenQuad;

		};
	}
}