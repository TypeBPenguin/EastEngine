#include "stdafx.h"
#include "DeviceLost.h"

namespace eastengine
{
	namespace graphics
	{
		static std::vector<IDeviceLost*> s_vecDeviceLostHandlers;

		IDeviceLost::IDeviceLost()
		{
			s_vecDeviceLostHandlers.emplace_back(this);
		}

		IDeviceLost::~IDeviceLost()
		{
			auto iter = std::find(s_vecDeviceLostHandlers.begin(), s_vecDeviceLostHandlers.end(), this);
			if (iter != s_vecDeviceLostHandlers.end())
			{
				s_vecDeviceLostHandlers.erase(iter);
			}
		}

		void IDeviceLost::HandleDeviceLost()
		{
			std::for_each(s_vecDeviceLostHandlers.begin(), s_vecDeviceLostHandlers.end(), [&](IDeviceLost* pDeviceLost)
			{
				if (pDeviceLost->OnDeviceLost() == false)
				{
					LOG_ERROR("Failed Handle Device Lost");
					assert(false);
				}
			});
		}

		void IDeviceLost::HandleDeviceRestored()
		{
			std::for_each(s_vecDeviceLostHandlers.begin(), s_vecDeviceLostHandlers.end(), [&](IDeviceLost* pDeviceLost)
			{
				if (pDeviceLost->OnDeviceLost() == false)
				{
					LOG_ERROR("Failed Handle Device Restored");
					assert(false);
				}
			});
		}
	}
}