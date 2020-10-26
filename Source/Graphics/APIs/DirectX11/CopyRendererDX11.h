#pragma once

#include "Graphics/Interface/Renderer.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;

			class CopyRenderer : public IRenderer
			{
			public:
				CopyRenderer();
				virtual ~CopyRenderer();

			public:
				virtual Type GetType() const { return IRenderer::eCopy; }

			public:
				void Copy_RGBA(const RenderTarget* pSource, RenderTarget* pResult);
				void Copy_RGB(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}