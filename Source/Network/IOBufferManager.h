#pragma once

#include "IOBuffer.h"

namespace est
{
	namespace network
	{
		class IOBufferManager : public Singleton<IOBufferManager>
		{
			friend Singleton<IOBufferManager>;
		private:
			IOBufferManager();
			virtual ~IOBufferManager();

		public:
			IIOBuffer* AllocateIOBuffer(OperationType operationType);
			void DestroyIOBuffer(IIOBuffer* pIOBuffer);

			IOverlapped* AllocateOverlapped(OperationType operationType);
			void DestroyOverlapped(IOverlapped* pOverlapped);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}