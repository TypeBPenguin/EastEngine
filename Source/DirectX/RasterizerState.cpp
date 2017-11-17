#include "stdafx.h"
#include "RasterizerState.h"

namespace EastEngine
{
	namespace Graphics
	{
		RasterizerState::RasterizerState()
			: m_pRasterizerState(nullptr)
		{
		}

		RasterizerState::~RasterizerState()
		{
			SafeRelease(m_pRasterizerState);
		}

		bool RasterizerState::Init(const RasterizerStateDesc& rasterizerStateDesc)
		{
			if (FAILED(GetDevice()->CreateRasterizerState(&rasterizerStateDesc, &m_pRasterizerState)))
				return false;

			m_rasterizerStateDesc = rasterizerStateDesc;
			m_rasterizerStateKey = m_rasterizerStateDesc.GetKey();

			return true;
		}
	}
}