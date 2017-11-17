#include "stdafx.h"
#include "DeviceLost.h"

namespace EastEngine
{
	namespace Graphics
	{
		static std::list<IDeviceLost*> s_listDeviceLostHandler;

		IDeviceLost::IDeviceLost()
		{
			s_listDeviceLostHandler.emplace_back(this);
		}

		IDeviceLost::~IDeviceLost()
		{
		}

		const std::list<IDeviceLost*>& IDeviceLost::GetDeviceLostHandler() { return s_listDeviceLostHandler; }
	}
}