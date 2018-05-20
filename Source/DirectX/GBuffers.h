#pragma once

#include "D3DInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class IRenderTarget;
		class ITexture;

		class ImageBasedLight;

		class GBuffers : public IGBuffers
		{
		public:
			GBuffers();
			virtual ~GBuffers();

		public:
			bool Init(const math::Viewport& viewport);

		public:
			virtual IRenderTarget* GetGBuffer(EmGBuffer::Type emType) override { return m_pGBuffers[emType]; }
			virtual IRenderTarget** GetGBuffers() override { return &m_pGBuffers.front(); }

		private:
			std::array<IRenderTarget*, EmGBuffer::Count> m_pGBuffers;
		};
	}
}