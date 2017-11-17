#pragma once

namespace EastEngine
{
	namespace Memory
	{
		void Move(void* pDestination, std::size_t nDestinationSize, const void* pSource, std::size_t nSourceSize = _TRUNCATE);

		template <typename T, std::size_t size>
		inline void Move(T(&destination)[size], const void* pSource, std::size_t nSourceSize = _TRUNCATE)
		{
			Move(destination, sizeof(T) * size, pSource, nSourceSize);
		}

		void Copy(void* pDestination, std::size_t nDestinationSize, const void* pSource, std::size_t nSourceSize = _TRUNCATE);
		
		template <typename T, std::size_t size>
		inline void Copy(T(&destination)[size], const void* pSource, std::size_t nSourceSize = _TRUNCATE)
		{
			Copy(destination, sizeof(T) * size, pSource, nSourceSize);
		}

		void Fill(void* pDestination, std::size_t nDestinationSize, int nValue);

		template <typename T, std::size_t size>
		inline void Fill(T(&destination)[size], int nValue)
		{
			Fill(destination, sizeof(T) * size, nValue);
		}

		void Clear(void* pDestination, std::size_t nDestinationSize);

		template <typename T>
		inline void Clear(T& destination)
		{
			Clear(&destination, sizeof(T));
		}

		template <typename T, std::size_t size>
		inline void Clear(T(&destination)[size])
		{
			Clear(destination, sizeof(T) * size);
		}
	}
}