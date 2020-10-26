#pragma once

#include "Graphics/Interface/Renderer.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;

			class BloomFilter : public IRenderer
			{
			public:
				BloomFilter();
				virtual ~BloomFilter();

			public:
				virtual Type GetType() const { return IRenderer::eBloomFilter; }

			public:
				void Apply(RenderTarget* pSourceAndResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}