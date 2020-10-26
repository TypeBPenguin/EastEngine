#include "stdafx.h"
#include "Memory.h"

#include "Log.h"

namespace est
{
	namespace memory
	{
		void Move(void* pDestination, size_t destinationSize, const void* pSource, size_t sourceSize)
		{
			if (pDestination == nullptr || pSource == nullptr)
				return;

			if (destinationSize < sourceSize)
			{
				if (sourceSize != _TRUNCATE)
				{
					LOG_WARNING(L"а╤╫иго╩О, memory::Move(), BufferSize : %d < SourceSize : %d", destinationSize, sourceSize);
				}
				sourceSize = destinationSize;
			}

			memmove_s(pDestination, destinationSize, pSource, sourceSize);
		}

		void Copy(void* pDestination, size_t destinationSize, const void* pSource, size_t sourceSize)
		{
			if (pDestination == nullptr || pSource == nullptr)
				return;

			if (destinationSize < sourceSize)
			{
				if (sourceSize != _TRUNCATE)
				{
					LOG_WARNING(L"а╤╫иго╩О, memory::Copy(), BufferSize : %d < SourceSize : %d", destinationSize, sourceSize);
				}
				sourceSize = destinationSize;
			}

			memcpy_s(pDestination, destinationSize, pSource, sourceSize);
		}

		void Fill(void* pDestination, size_t destinationSize, int nValue)
		{
			if (pDestination == nullptr)
				return;

			FillMemory(pDestination, destinationSize, nValue);
		}

		void Clear(void* pDestination, size_t destinationSize)
		{
			if (pDestination == nullptr)
				return;

			ZeroMemory(pDestination, destinationSize);
		}
	}
}