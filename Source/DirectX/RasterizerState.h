#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class RasterizerState : public IRasterizerState
		{
		public:
			RasterizerState();
			virtual ~RasterizerState();

			bool Init(const RasterizerStateDesc& rasterizerStateDesc);

		public:
			ID3D11RasterizerState* GetInterface() const { return m_pRasterizerState; }
			const RasterizerStateDesc& GetDesc() { return m_rasterizerStateDesc; }
			const RasterizerStateKey& GetKey() { return m_rasterizerStateKey; }

		private:
			ID3D11RasterizerState* m_pRasterizerState;
			RasterizerStateDesc m_rasterizerStateDesc;
			RasterizerStateKey m_rasterizerStateKey;
		};
	}
}