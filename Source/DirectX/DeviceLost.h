#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		class IDeviceLost
		{
		public:
			IDeviceLost();
			virtual ~IDeviceLost() = 0;

			virtual bool OnDeviceLost() = 0;
			virtual bool OnDeviceRestored() = 0;

			static const std::list<IDeviceLost*>& GetDeviceLostHandler();
		};
	}
}