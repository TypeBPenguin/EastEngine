#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
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
			bool Init(const Math::Viewport& viewport);

		public:
			virtual IRenderTarget* GetGBuffer(EmGBuffer::Type emType) override { return m_pGBuffers[emType]; }
			virtual IRenderTarget** GetGBuffers() override { return &m_pGBuffers.front(); }

		private:
			std::array<IRenderTarget*, EmGBuffer::Count> m_pGBuffers;
		};
	}
}