#include "stdafx.h"
#include "Memory.h"

#include "Log.h"

namespace EastEngine
{
	namespace Memory
	{
		void Move(void* pDestination, std::size_t nDestinationSize, const void* pSource, std::size_t nSourceSize)
		{
			if (pDestination == nullptr || pSource == nullptr)
				return;

			if (nDestinationSize < nSourceSize)
			{
				if (nSourceSize != _TRUNCATE)
				{
					PRINT_LOG("а╤╫иго╩О, Memory::Move(), BufferSize : %d < SourceSize : %d", nDestinationSize, nSourceSize);
				}
				nSourceSize = nDestinationSize;
			}

			memmove_s(pDestination, nDestinationSize, pSource, nSourceSize);
		}

		void Copy(void* pDestination, std::size_t nDestinationSize, const void* pSource, std::size_t nSourceSize)
		{
			if (pDestination == nullptr || pSource == nullptr)
				return;

			if (nDestinationSize < nSourceSize)
			{
				if (nSourceSize != _TRUNCATE)
				{
					PRINT_LOG("а╤╫иго╩О, Memory::Copy(), BufferSize : %d < SourceSize : %d", nDestinationSize, nSourceSize);
				}
				nSourceSize = nDestinationSize;
			}

			memcpy_s(pDestination, nDestinationSize, pSource, nSourceSize);
		}

		void Fill(void* pDestination, std::size_t nDestinationSize, int nValue)
		{
			if (pDestination == nullptr)
				return;

			FillMemory(pDestination, nDestinationSize, nValue);
		}

		void Clear(void* pDestination, std::size_t nDestinationSize)
		{
			if (pDestination == nullptr)
				return;

			ZeroMemory(pDestination, nDestinationSize);
		}
	}
}