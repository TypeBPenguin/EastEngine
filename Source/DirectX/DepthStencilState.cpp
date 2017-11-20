#include "stdafx.h"
#include "DepthStencilState.h"

namespace EastEngine
{
	namespace Graphics
	{
		DepthStencilState::DepthStencilState()
			: m_pDepthStencilState(nullptr)
			, m_depthStencilStateKey(String::UnregisteredKey)
		{
		}

		DepthStencilState::~DepthStencilState()
		{
			SafeRelease(m_pDepthStencilState);
		}

		bool DepthStencilState::Init(const DepthStencilStateDesc& depthStencilStateDesc)
		{
			if (FAILED(GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &m_pDepthStencilState)))
				return false;

			m_depthStencilStateDesc = depthStencilStateDesc;
			m_depthStencilStateKey = m_depthStencilStateDesc.GetKey();

			return true;
		}
	}
}