#pragma once

namespace est
{
	namespace memory
	{
		void Move(void* pDestination, size_t destinationSize, const void* pSource, size_t sourceSize = _TRUNCATE);

		template <typename T, size_t size>
		inline void Move(T(&destination)[size], const void* pSource, size_t sourceSize = _TRUNCATE)
		{
			Move(destination, sizeof(T) * size, pSource, sourceSize);
		}

		template <typename T, size_t size>
		inline void Move(std::array<T, size>& destination, const void* pSource, size_t sourceSize = _TRUNCATE)
		{
			Move(destination.data(), sizeof(T) * size, pSource, sourceSize);
		}

		void Copy(void* pDestination, size_t destinationSize, const void* pSource, size_t sourceSize = _TRUNCATE);
		
		template <typename T, size_t size>
		inline void Copy(T(&destination)[size], const void* pSource, size_t sourceSize = _TRUNCATE)
		{
			Copy(destination, sizeof(T) * size, pSource, sourceSize);
		}

		template <typename T, size_t size>
		inline void Copy(std::array<T, size>& destination, const void* pSource, size_t sourceSize = _TRUNCATE)
		{
			Copy(destination.data(), sizeof(T) * size, pSource, sourceSize);
		}

		void Fill(void* pDestination, size_t destinationSize, int nValue);

		template <typename T, size_t size>
		inline void Fill(T(&destination)[size], int nValue)
		{
			Fill(destination, sizeof(T) * size, nValue);
		}

		void Clear(void* pDestination, size_t destinationSize);

		template <typename T>
		inline void Clear(T& destination)
		{
			Clear(&destination, sizeof(T));
		}

		template <typename T, size_t size>
		inline void Clear(T(&destination)[size])
		{
			Clear(destination, sizeof(T) * size);
		}
	}
}