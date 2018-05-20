#include "stdafx.h"
#include "BlendState.h"

namespace eastengine
{
	namespace graphics
	{
		BlendState::BlendState()
			: m_pBlendState(nullptr)
			, m_blendStateKey(String::UnregisteredKey)
		{
		}

		BlendState::~BlendState()
		{
			SafeRelease(m_pBlendState);
		}

		bool BlendState::Init(const BlendStateDesc& blendStateDesc)
		{
			if (FAILED(GetDevice()->CreateBlendState(&blendStateDesc, &m_pBlendState)))
				return false;

			m_blendStateDesc = blendStateDesc;
			m_blendStateKey = m_blendStateDesc.GetKey();

			return true;
		}
	}
}