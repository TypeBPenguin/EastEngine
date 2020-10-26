#pragma once

#include "NetworkUserSession.h"
#include "PacketBuffer.h"

namespace est
{
	namespace network
	{
		class NetServer : public INetServer
		{
		public:
			NetServer();
			virtual ~NetServer();

		public:
			bool Initialize(uint16_t port);
			void Release();

		public:
			virtual bool IsValid() const override { return m_handleIOCP != INVALID_HANDLE_VALUE && m_handleIOCP != NULL; }
			virtual size_t GetFreeUserSessionCount() const override { return m_userSessionAllocator.size(); }

		public:
			virtual void Broadcast(const packet::Header* pPacket) override;

		public:
			virtual void ProcessConnectUserSessions(std::function<void(IUserSession*)> func) override;
			virtual void ProcessDisconnectUserSessions(std::function<void(IUserSession*)> func) override;
			virtual void ProcessReceivedPacket(std::function<void(IUserSession*, const packet::Header*)> func) override;

		private:
			char m_hostName[256]{};
			char m_ipAddress[32]{};

			HANDLE m_handleIOCP{ INVALID_HANDLE_VALUE };
			SOCKET m_socket{ INVALID_SOCKET };

			std::atomic<bool> m_isRunning{ true };
			std::vector<std::thread> m_workerThreads;
			std::thread m_acceptThread;
			std::mutex m_mutex;

			std::array<UserSession, eMaxUserSession> m_userSessions;
			std::queue<size_t> m_userSessionAllocator;

			std::unordered_map<size_t, UserSession*> m_activeUserSessions;

			enum : uint32_t
			{
				ePacketBufferSize = (eMaxPacketSize * 2 / 10) * (eMaxUserSession * 2 / 10),
			};
			packet::PacketBuffer<ePacketBufferSize> m_receivePacketBuffer;

			std::mutex m_mutex_connectUserSession;
			std::queue<UserSession*> m_connectUserSessions;

			std::mutex m_mutex_disconnectUserSession;
			std::vector<UserSession*> m_disconnectUserSessions;
		};
	}
}