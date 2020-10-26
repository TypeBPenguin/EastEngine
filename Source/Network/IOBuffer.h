#pragma once

#include "NetworkInterface.h"

namespace est
{
	namespace network
	{
		class IOBufferSend;

		struct IOverlapped : OVERLAPPED
		{
			const OperationType operationType{};
			WSABUF wsaBuf{};

			IOverlapped(OperationType operationType)
				: OVERLAPPED{}
				, operationType(operationType)
			{
			}
		};

		struct OverlappedReceive : public IOverlapped
		{
			std::array<char, eUserSessionReceiveBufferSize> buffer{};

			OverlappedReceive()
				: IOverlapped(OperationType::eReceive)
			{
			}
		};

		struct OverlappedSend : public IOverlapped
		{
			IOBufferSend* pIOBuffer{ nullptr };

			OverlappedSend()
				: IOverlapped(OperationType::eSend)
			{
			}
		};

		class IOBufferReceive : public IIOBuffer
		{
		public:
			IOBufferReceive() = default;
			virtual ~IOBufferReceive() = default;

		public:
			OperationType GetOperationType() const override { return OperationType::eReceive; }

		public:
			virtual const packet::Header* GetPacket() const override { return reinterpret_cast<const packet::Header*>(m_buffer.data()); }
			virtual char* GetBuffer() override { return m_buffer.data(); }
			virtual uint32_t Length() const override { return GetPacket()->length; }

		public:
			void Receive(uint32_t startIndex, const char* pSource, uint32_t length) { memcpy_s(&m_buffer[startIndex], m_buffer.size() - startIndex, pSource, length); }
			void PopReceivedPacket(uint32_t remainLength) { memmove(m_buffer.data(), m_buffer.data() + GetPacket()->length, remainLength); }

		private:
			std::array<char, eMaxReceiveIOBufferSize> m_buffer{};
		};

		class IOBufferSend : public IIOBuffer
		{
		public:
			IOBufferSend() = default;
			virtual ~IOBufferSend() = default;

		public:
			OperationType GetOperationType() const override { return OperationType::eSend; }

		public:
			virtual const packet::Header* GetPacket() const override { return reinterpret_cast<const packet::Header*>(m_buffer.data()); }
			virtual char* GetBuffer() override { return m_buffer.data(); }
			virtual uint32_t Length() const override { return GetPacket()->length; }

		public:
			void Send(const packet::Header* pPacket) { memcpy_s(m_buffer.data(), m_buffer.size(), pPacket, pPacket->length); }

		private:
			std::array<char, eMaxSendIOBufferSize> m_buffer{};
		};
	}
}