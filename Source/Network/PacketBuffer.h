#pragma once

#include "NetworkInterface.h"

namespace est
{
	namespace network
	{
		namespace packet
		{
			struct ReceivedPacket
			{
				IUserSession* pUserSession{ nullptr };
				const Header* pHeader{ nullptr };
				bool isFromBuffer{ false };

				ReceivedPacket(IUserSession* pUserSession, const Header* pHeader, bool isFromBuffer)
					: pUserSession(pUserSession)
					, pHeader(pHeader)
					, isFromBuffer(isFromBuffer)
				{
				}

				~ReceivedPacket()
				{
					if (isFromBuffer == false)
					{
						delete reinterpret_cast<const char*>(pHeader);
					}
				}
			};

			template <uint32_t BufferSize>
			class PacketBuffer
			{
			public:
				PacketBuffer()
				{
					m_buffer.resize(BufferSize);
				}
				~PacketBuffer() = default;

			public:
				void Push(IUserSession* pUserSession, const Header* pNewPacket)
				{
					if (pNewPacket->length > BufferSize)
						return;

					uint32_t position = m_position;

					if (position + pNewPacket->length > BufferSize)
					{
						if (m_packets.empty() == false)
						{
							const Header* pPacket = m_packetsFromBuffer.front();
							const uint32_t packetPosition = static_cast<uint32_t>(reinterpret_cast<const char*>(pPacket) - m_buffer.data());

							if (pNewPacket->length > packetPosition)
							{
								LOG_WARNING(L"failed to push packet because it exceeds packet buffer size. instead, used a temporary buffer. this is not a fatal problem, but it slows performance.");

								char* pEmergencyBuffer = new char[pNewPacket->length];
								memcpy_s(pEmergencyBuffer, pNewPacket->length, pNewPacket, pNewPacket->length);
								std::unique_ptr<ReceivedPacket> pEmergencyPacket = std::make_unique<ReceivedPacket>(pUserSession, reinterpret_cast<Header*>(pEmergencyBuffer), false);
								m_packets.emplace(std::move(pEmergencyPacket));
								return;
							}
						}

						position = 0;
					}

					memcpy_s(&m_buffer[position], BufferSize - position, pNewPacket, pNewPacket->length);

					std::unique_ptr<ReceivedPacket> pReceivedPacket = std::make_unique<ReceivedPacket>(pUserSession, reinterpret_cast<Header*>(&m_buffer[position]), true);
					m_packets.emplace(std::move(pReceivedPacket));

					m_packetsFromBuffer.emplace(reinterpret_cast<Header*>(&m_buffer[position]));

					m_position = position + pNewPacket->length;
				}

				std::unique_ptr<ReceivedPacket> Front()
				{
					m_isPopPacketFromBuffer = m_packets.front()->isFromBuffer;

					return std::move(m_packets.front());
				}

				void Pop()
				{
					if (m_isPopPacketFromBuffer == true)
					{
						m_packetsFromBuffer.pop();
					}
					m_packets.pop();
				}

				bool IsEmpty() const { return m_packets.empty(); }

			private:
				uint32_t m_position{ 0 };

				std::vector<char> m_buffer{};
				std::queue<std::unique_ptr<ReceivedPacket>> m_packets;
				std::queue<const packet::Header*> m_packetsFromBuffer;
				bool m_isPopPacketFromBuffer{ false };
			};
		}
	}
}