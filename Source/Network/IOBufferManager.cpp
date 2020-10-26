#include "stdafx.h"
#include "IOBufferManager.h"

#include "CommonLib/ObjectPool.h"

namespace est
{
	namespace network
	{
		class IOBufferManager::Impl
		{
		public:
			Impl() = default;
			~Impl() = default;

		public:
			IIOBuffer* AllocateIOBuffer(OperationType operationType)
			{
				IIOBuffer* pIOBuffer = nullptr;

				switch (operationType)
				{
				case OperationType::eSend:
					pIOBuffer = m_poolIOBufferSend.Allocate();
					break;
				case OperationType::eReceive:
					pIOBuffer = m_poolIOBufferReceive.Allocate();
					break;
				default:
					return nullptr;
				}

				pIOBuffer->IncreaseReferenceCount();
				return pIOBuffer;
			}

			void DestroyIOBuffer(IIOBuffer* pIOBuffer)
			{
				if (pIOBuffer == nullptr)
					return;

				pIOBuffer->DecreaseReferenceCount();

				if (pIOBuffer->GetReferenceCount() > 0)
					return;

				switch (pIOBuffer->GetOperationType())
				{
				case OperationType::eSend:
					m_poolIOBufferSend.Destroy(static_cast<IOBufferSend*>(pIOBuffer));
					break;
				case OperationType::eReceive:
					m_poolIOBufferReceive.Destroy(static_cast<IOBufferReceive*>(pIOBuffer));
					break;
				}
			}

			IOverlapped* AllocateOverlapped(OperationType operationType)
			{
				IOverlapped* pOverlapped = nullptr;

				switch (operationType)
				{
				case OperationType::eSend:
					pOverlapped = m_poolOverlappedSend.Allocate();
					break;
				case OperationType::eReceive:
					pOverlapped = m_poolOverlappedReceive.Allocate();
					break;
				default:
					return nullptr;
				}

				return pOverlapped;
			}

			void DestroyOverlapped(IOverlapped* pOverlapped)
			{
				if (pOverlapped == nullptr)
					return;

				switch (pOverlapped->operationType)
				{
				case OperationType::eSend:
					m_poolOverlappedSend.Destroy(static_cast<OverlappedSend*>(pOverlapped));
					break;
				case OperationType::eReceive:
					m_poolOverlappedReceive.Destroy(static_cast<OverlappedReceive*>(pOverlapped));
					break;
				}
			}

		private:
			memory::ObjectPool<IOBufferSend, eMaxUserSession / 8> m_poolIOBufferSend;
			memory::ObjectPool<IOBufferReceive, eMaxUserSession / 8> m_poolIOBufferReceive;

			memory::ObjectPool<OverlappedSend, eMaxUserSession / 8> m_poolOverlappedSend;
			memory::ObjectPool<OverlappedReceive, eMaxUserSession / 8> m_poolOverlappedReceive;
		};

		IOBufferManager::IOBufferManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		IOBufferManager::~IOBufferManager()
		{
		}

		IIOBuffer* IOBufferManager::AllocateIOBuffer(OperationType operationType)
		{
			return m_pImpl->AllocateIOBuffer(operationType);
		}

		void IOBufferManager::DestroyIOBuffer(IIOBuffer* pIOBuffer)
		{
			m_pImpl->DestroyIOBuffer(pIOBuffer);
		}

		IOverlapped* IOBufferManager::AllocateOverlapped(OperationType operationType)
		{
			return m_pImpl->AllocateOverlapped(operationType);
		}

		void IOBufferManager::DestroyOverlapped(IOverlapped* pOverlapped)
		{
			m_pImpl->DestroyOverlapped(pOverlapped);
		}
	}
}