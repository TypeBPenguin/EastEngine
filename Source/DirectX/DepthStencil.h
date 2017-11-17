#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class DepthStencil : public IDepthStencil
		{
		public:
			DepthStencil();
			virtual ~DepthStencil();

			bool Init(const DepthStencilDesc& depthStencilDesc);

		public:
			virtual const std::shared_ptr<ITexture>& GetTexture() const override { return m_pTexture; }
			virtual ID3D11DepthStencilView* GetDepthStencilView() const override { return m_pDepthStencilView; }

		protected:
			std::shared_ptr<ITexture> m_pTexture;
			ID3D11DepthStencilView* m_pDepthStencilView;
		};
	}
}