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
				eStandardDescriptorRangesCount = 7,
			};

			enum : uint32_t
			{
				eInvalidDescriptorIndex = std::numeric_limits<uint32_t>::max(),
			};
		}
	}
}