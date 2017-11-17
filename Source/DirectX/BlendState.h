#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class BlendState : public IBlendState
		{
		public:
			BlendState();
			virtual ~BlendState();

			bool Init(const BlendStateDesc& blendStateDesc);

		public:
			virtual ID3D11BlendState* GetInterface() const override { return m_pBlendState; }
			virtual const BlendStateDesc& GetDesc() override { return m_blendStateDesc; }
			virtual const BlendStateKey& GetKey() override { return m_blendStateKey; }

		private:
			ID3D11BlendState* m_pBlendState;
			BlendStateDesc m_blendStateDesc;
			BlendStateKey m_blendStateKey;
		};
	}
}