#pragma once

#include "DeviceContext.h"

namespace eastengine
{
	namespace graphics
	{
		class DeferredContext : public DeviceContext, public IDeferredContext
		{
		public:
			DeferredContext(ID3D11DeviceContext* pDeviceContext);
			virtual ~DeferredContext();

		public:
			virtual bool FinishCommandList() override;
			virtual void ExecuteCommandList(IDeviceContext* pImmediateContext) override;

		private:
			bool m_isClearStateUponExecuteCommandList{ false };
			ID3D11CommandList* m_pCommandList{ nullptr };
		};
	}
}