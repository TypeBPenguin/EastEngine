#pragma once

#include "GraphicsInterface/Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class IRendererDX12 : public IRenderer
			{
			public:
				IRendererDX12();
				virtual ~IRendererDX12() = 0;

			public:
				virtual void RefreshPSO(ID3D12Device* pDevice) = 0;
			};
		}
	}
}