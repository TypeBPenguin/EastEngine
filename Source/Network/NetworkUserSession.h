#pragma once

#include "NetworkInterface.h"
#include "IOBuffer.h"

namespace est
{
	namespace network
	{
		class UserSession : public IUserSession
		{
		public:
			UserSession();
			virtual ~UserSession();

		public:
			virtual void Disconnect() override;
			virtual bool IsConnected() const override { return m_socket != INVALID_SOCKET; }

		public:
			virtual void Send(const packet::Header* pPacket) override;
			virtual void Send(IIOBuffer* pIOBuffer) override;

		public:
			virtual size_t GetID() const override { return m_id; }
			virtual const char* GetIPAddress() const override { return m_ipAddress.data(); }

			virtual void* GetUserData() override { return m_pUserData; }
			virtual void SetUserData(void* pUserData) override { m_pUserData = pUserData; }

		public:
			void Connect(SOCKET socket, size_t id, const char* ipAddress);

			void Receive(DWORD numTransferredBytes);
			const packet::Header* GetReceivedPacket();
			void PopReceivedPacket();

			void SetReceiveState();

		private:
			SOCKET m_socket{ INVALID_SOCKET };
			size_t m_id{ eInvaliUserSessionID };

			std::array<char, 32> m_ipAddress{};

			OverlappedReceive* m_pRecvOverlapped{ nullptr };

			uint32_t m_receivedPacketSize{ 0 };
			IOBufferReceive* m_pReceiveIOBuffer{ nullptr };

			void* m_pUserData{ nullptr };
		};
	}
}