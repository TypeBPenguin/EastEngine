#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class SamplerState : public ISamplerState
		{
		public:
			SamplerState();
			virtual ~SamplerState();

			bool Init(const SamplerStateDesc& samplerStateDesc);

		public:
			virtual ID3D11SamplerState* GetInterface() override { return m_pSamplerState; }
			virtual const SamplerStateDesc& GetDesc() override { return m_samplerStateDesc; }
			virtual const SamplerStateKey& GetKey() override { return m_samplerStateKey; }

		private:
			ID3D11SamplerState* m_pSamplerState;
			SamplerStateDesc m_samplerStateDesc;
			SamplerStateKey m_samplerStateKey;
		};
	}
}