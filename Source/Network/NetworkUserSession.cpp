#include "stdafx.h"
#include "NetworkUserSession.h"

#include "IOBufferManager.h"

namespace est
{
	namespace network
	{
		UserSession::UserSession()
		{
		}

		UserSession::~UserSession()
		{
			IOBufferManager::GetInstance()->DestroyIOBuffer(m_pReceiveIOBuffer);
			m_pReceiveIOBuffer = nullptr;
		}

		void UserSession::Disconnect()
		{
			if (IsConnected() == false)
				return;

			closesocket(m_socket);
			m_socket = INVALID_SOCKET;

			m_id = eInvaliUserSessionID;
			m_ipAddress.fill(0);

			m_receivedPacketSize = 0;

			IOBufferManager::GetInstance()->DestroyOverlapped(m_pRecvOverlapped);
			m_pRecvOverlapped = nullptr;

			IOBufferManager::GetInstance()->DestroyIOBuffer(m_pReceiveIOBuffer);
			m_pReceiveIOBuffer = nullptr;
		}

		void UserSession::Send(const packet::Header* pPacket)
		{
			if (pPacket == nullptr || pPacket->length == 0)
				return;

			if (pPacket->length > eMaxPacketSize)
			{
				//LOG_FATAL("failed to broadcast packet, packet size over : %u > %u", pPacket->length, eMaxPacketSize);
				return;
			}

			IIOBuffer* pIOBuffer = CreateIOBuffer(pPacket);

			Send(pIOBuffer);

			IOBufferManager::GetInstance()->DestroyIOBuffer(pIOBuffer);
			pIOBuffer = nullptr;
		}

		void UserSession::Send(IIOBuffer* pIOBuffer)
		{
			if (pIOBuffer == nullptr)
				return;

			pIOBuffer->IncreaseReferenceCount();

			OverlappedSend* pOverlapped = static_cast<OverlappedSend*>(IOBufferManager::GetInstance()->AllocateOverlapped(OperationType::eSend));
			pOverlapped->pIOBuffer = static_cast<IOBufferSend*>(pIOBuffer);
			pOverlapped->wsaBuf.len = pIOBuffer->Length();
			pOverlapped->wsaBuf.buf = pIOBuffer->GetBuffer();

			DWORD flags = 0;
			const int result = WSASend(m_socket, &pOverlapped->wsaBuf, 1, nullptr, flags, pOverlapped, nullptr);
			if (result == SOCKET_ERROR)
			{
				const int error = WSAGetLastError();
				if (error != ERROR_IO_PENDING)
				{
					LogError(L"WSASend", error);
				}
			}
		}

		void UserSession::Connect(SOCKET socket, size_t id, const char* ipAddress)
		{
			m_socket = socket;
			m_id = id;
			strcpy_s(m_ipAddress.data(), m_ipAddress.size(), ipAddress);

			m_pRecvOverlapped = static_cast<OverlappedReceive*>(IOBufferManager::GetInstance()->AllocateOverlapped(OperationType::eReceive));
			m_pRecvOverlapped->wsaBuf.len = static_cast<uint32_t>(m_pRecvOverlapped->buffer.size());
			m_pRecvOverlapped->wsaBuf.buf = m_pRecvOverlapped->buffer.data();

			m_receivedPacketSize = 0;
		}

		void UserSession::Receive(DWORD numTransferredBytes)
		{
			if (m_pReceiveIOBuffer == nullptr)
			{
				m_pReceiveIOBuffer = static_cast<IOBufferReceive*>(IOBufferManager::GetInstance()->AllocateIOBuffer(OperationType::eReceive));
			}

			m_pReceiveIOBuffer->Receive(m_receivedPacketSize, m_pRecvOverlapped->buffer.data(), numTransferredBytes);
			m_receivedPacketSize += numTransferredBytes;
		}

		const packet::Header* UserSession::GetReceivedPacket()
		{
			if (IsConnected() == false || m_pReceiveIOBuffer == nullptr)
				return nullptr;

			if (m_receivedPacketSize < sizeof(packet::Header))
				return nullptr;

			const packet::Header* pHeader = m_pReceiveIOBuffer->GetPacket();
			if (m_receivedPacketSize < pHeader->length)
				return nullptr;

			return pHeader;
		}

		void UserSession::PopReceivedPacket()
		{
			if (IsConnected() == false || m_pReceiveIOBuffer == nullptr)
				return;

			if (m_receivedPacketSize < sizeof(packet::Header))
				return;

			const packet::Header* pHeader = m_pReceiveIOBuffer->GetPacket();
			if (m_receivedPacketSize < pHeader->length)
				return;

			const uint32_t length = pHeader->length;
			m_receivedPacketSize -= length;

			if (m_receivedPacketSize > 0)
			{
				m_pReceiveIOBuffer->PopReceivedPacket(m_receivedPacketSize);
			}
			else
			{
				IOBufferManager::GetInstance()->DestroyIOBuffer(m_pReceiveIOBuffer);
				m_pReceiveIOBuffer = nullptr;
			}
		}

		void UserSession::SetReceiveState()
		{
			if (IsConnected() == false)
				return;

			DWORD flags = 0;

			const int result = WSARecv(m_socket, &m_pRecvOverlapped->wsaBuf, 1, nullptr, &flags, m_pRecvOverlapped, nullptr);
			if (result == SOCKET_ERROR)
			{
				const int error = WSAGetLastError();
				if (error != ERROR_IO_PENDING)
				{
					LogError(L"WSARecv", error);
				}
			}
		}
	}
}