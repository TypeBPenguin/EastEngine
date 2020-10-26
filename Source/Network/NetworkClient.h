#pragma once

#include "NetworkInterface.h"
#include "PacketBuffer.h"

namespace est
{
	namespace network
	{
		class NetClient : public INetClient
		{
		public:
			NetClient();
			virtual ~NetClient();

		public:
			virtual bool Connect(const char* ipAddress, uint16_t port, std::function<void(int)> errorCallback) override;
			virtual void Disconnect() override;

			virtual bool IsConnected() const override { return m_socket != INVALID_SOCKET; }

		public:
			virtual void Send(const packet::Header* pPacket) override;
			virtual bool IsEmptySendBuffer() override;

		public:
			virtual void ProcessReceivedPacket(std::function<void(const packet::Header*)> func) override;

		private:
			SOCKET m_socket{ INVALID_SOCKET };

			std::atomic<bool> m_isRunning{ true };

			std::mutex m_mutex_receive;
			std::thread m_thread_receive;

			std::mutex m_mutex_send;
			std::condition_variable m_condition_send;
			std::thread m_thread_send;

			enum : uint32_t
			{
				eClientPacketBufferSize = eMaxPacketSize * 4,
			};
			packet::PacketBuffer<eClientPacketBufferSize> m_receivePacketBuffer;
			packet::PacketBuffer<eClientPacketBufferSize> m_sendPacketBuffer;
		};
	}
}