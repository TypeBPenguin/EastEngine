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

		public:
			virtual bool OnDeviceLost() = 0;
			virtual bool OnDeviceRestored() = 0;

		public:
			static void HandleDeviceLost();
			static void HandleDeviceRestored();
		};
	}
}