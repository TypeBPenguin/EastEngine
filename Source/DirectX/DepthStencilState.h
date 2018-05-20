#pragma once

#include "D3DInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class DepthStencilState : public IDepthStencilState
		{
		public:
			DepthStencilState();
			virtual ~DepthStencilState();

			bool Init(const DepthStencilStateDesc& depthStencilStateDesc);

		public:
			virtual ID3D11DepthStencilState* GetInterface() const override { return m_pDepthStencilState; }
			virtual const DepthStencilStateDesc& GetDesc() override { return m_depthStencilStateDesc; }
			virtual const DepthStencilStateKey& GetKey() override { return m_depthStencilStateKey; }

		protected:
			ID3D11DepthStencilState* m_pDepthStencilState;
			DepthStencilStateDesc m_depthStencilStateDesc;
			DepthStencilStateKey m_depthStencilStateKey;
		};
	}
}