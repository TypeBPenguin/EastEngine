#include "stdafx.h"
#include "SamplerState.h"

namespace EastEngine
{
	namespace Graphics
	{
		SamplerState::SamplerState()
			: m_pSamplerState(nullptr)
			, m_samplerStateKey(String::UnregisteredKey)
		{
		}

		SamplerState::~SamplerState()
		{
			SafeRelease(m_pSamplerState);
		}

		bool SamplerState::Init(const SamplerStateDesc& samplerStateDesc)
		{
			if (FAILED(GetDevice()->CreateSamplerState(&samplerStateDesc, &m_pSamplerState)))
				return false;

			m_samplerStateDesc = samplerStateDesc;
			m_samplerStateKey = m_samplerStateDesc.GetKey();

			return true;
		}
	}
}