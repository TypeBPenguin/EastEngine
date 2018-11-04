#pragma once

#include "CommonLib/PhantomType.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			enum
			{
				eFrameBufferCount = 3,

				eDescriptorHeap_Capacity_RTV = 256,
				eDescriptorHeap_Capacity_SRV = 1024,
				eDescriptorHeap_Capacity_DSV = 32,
				eDescriptorHeap_Capacity_UAV = 256,

				eStandardDescriptorRangesCount_SRV = 9,
			};

			enum : uint32_t
			{
				eInvalidDescriptorIndex = std::numeric_limits<uint32_t>::max(),
			};
		}
	}
}