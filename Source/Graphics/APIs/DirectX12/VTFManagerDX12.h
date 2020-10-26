#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class Texture;

			class VTFManager : public IVTFManager
			{
			public:
				VTFManager();
				virtual ~VTFManager();

			public:
				virtual bool Allocate(uint32_t nMatrixCount, math::Matrix** ppDest_Out, uint32_t& nVTFID_Out) override;

			public:
				bool Bake();
				void EndFrame();

			public:
				Texture* GetTexture() const;
				Texture* GetPrevTexture() const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}