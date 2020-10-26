#pragma once

namespace est
{
	namespace network
	{
		enum : size_t
		{
			eMaxPacketSize = (1 << 14),
			eMaxReceiveIOBufferSize = eMaxPacketSize * 4,
			eMaxSendIOBufferSize = eMaxPacketSize,

			eMaxUserSession = (1 << 13),
			eUserSessionReceiveBufferSize = (1 << 10),

			eInvaliUserSessionID = std::numeric_limits<size_t>::max(),
		};

		enum OperationType
		{
			eSend = 0,
			eReceive,
		};

		namespace packet
		{
#pragma pack(1)
			struct Header
			{
				uint32_t length{ 0 };
				uint32_t type{ 0 };

				Header(uint32_t length, uint32_t type)
					: length(length)
					, type(type)
				{
				}
			};
#pragma pack()
		}

		class IIOBuffer
		{
		public:
			IIOBuffer() = default;
			virtual ~IIOBuffer() = default;

		public:
			virtual OperationType GetOperationType() const = 0;

		public:
			virtual const packet::Header* GetPacket() const = 0;
			virtual char* GetBuffer() = 0;
			virtual uint32_t Length() const = 0;

		public:
			void IncreaseReferenceCount() { ++m_referenceCount; }
			void DecreaseReferenceCount() { --m_referenceCount; }
			uint32_t GetReferenceCount() const { return m_referenceCount; }

		private:
			std::atomic<uint32_t> m_referenceCount{ 0 };
		};

		class IUserSession
		{
		public:
			IUserSession() = default;
			~IUserSession() = default;

		public:
			virtual void Disconnect() = 0;
			virtual bool IsConnected() const = 0;

		public:
			virtual void Send(const packet::Header* pPacket) = 0;
			virtual void Send(IIOBuffer* pIOBuffer) = 0;

		public:
			virtual size_t GetID() const = 0;
			virtual const char* GetIPAddress() const = 0;

			virtual void* GetUserData() = 0;
			virtual void SetUserData(void* pUserData) = 0;
		};

		class INetServer
		{
		public:
			INetServer() = default;
			virtual ~INetServer() = default;

		public:
			virtual bool IsValid() const = 0;
			virtual size_t GetFreeUserSessionCount() const = 0;

		public:
			virtual void Broadcast(const packet::Header* pPacket) = 0;

		public:
			virtual void ProcessConnectUserSessions(std::function<void(IUserSession*)> func) = 0;
			virtual void ProcessDisconnectUserSessions(std::function<void(IUserSession*)> func) = 0;
			virtual void ProcessReceivedPacket(std::function<void(IUserSession* pUserSession, const packet::Header*)> func) = 0;
		};

		class INetClient
		{
		public:
			INetClient() = default;
			virtual ~INetClient() = default;

		public:
			virtual bool Connect(const char* ipAddress, uint16_t port, std::function<void(int)> errorCallback) = 0;
			virtual void Disconnect() = 0;

			virtual bool IsConnected() const = 0;

		public:
			virtual void Send(const packet::Header* pPacket) = 0;
			virtual bool IsEmptySendBuffer() = 0;

		public:
			virtual void ProcessReceivedPacket(std::function<void(const packet::Header*)> func) = 0;
		};

		void Destroy();

		std::unique_ptr<INetServer> CreateServer(uint16_t port);
		IIOBuffer* CreateIOBuffer(const packet::Header* pPacket);

		std::unique_ptr<INetClient> ConnectToServer(const char* ipAddress, uint16_t port, std::function<void(int)> errorCallback);

		void LogError(const wchar_t* log, int error, std::wstring* pResult_out = nullptr);
	}
}