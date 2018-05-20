#include "stdafx.h"
#include "DeferredContext.h"

namespace eastengine
{
	namespace graphics
	{
		DeferredContext::DeferredContext(ID3D11DeviceContext* pDeviceContext)
			: DeviceContext(pDeviceContext)
		{
		}

		DeferredContext::~DeferredContext()
		{
			SafeRelease(m_pCommandList);
		}

		bool DeferredContext::FinishCommandList()
		{
			SafeRelease(m_pCommandList);
			return SUCCEEDED(GetInterface()->FinishCommandList(m_isClearStateUponExecuteCommandList, &m_pCommandList));
		}

		void DeferredContext::ExecuteCommandList(IDeviceContext* pImmediateContext)
		{
			if (pImmediateContext == nullptr)
				return;

			pImmediateContext->GetInterface()->ExecuteCommandList(m_pCommandList, m_isClearStateUponExecuteCommandList);
			SafeRelease(m_pCommandList);
		}
	}
}