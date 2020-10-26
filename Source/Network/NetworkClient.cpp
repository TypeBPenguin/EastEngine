#include "stdafx.h"
#include "NetworkClient.h"

namespace est
{
	namespace network
	{
		NetClient::NetClient()
		{
		}

		NetClient::~NetClient()
		{
			Disconnect();

			if (m_thread_receive.joinable() == true)
			{
				m_thread_receive.join();
			}

			if (m_thread_send.joinable() == true)
			{
				m_thread_send.join();
			}
		}

		bool NetClient::Connect(const char* ipAddress, uint16_t port, std::function<void(int)> errorCallback)
		{
			if (IsConnected() == true)
				return true;

			if (m_thread_receive.joinable() == true)
			{
				m_thread_receive.join();
			}

			if (m_thread_send.joinable() == true)
			{
				m_thread_send.join();
			}

			WSADATA wsaData{};

			int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (error != 0)
			{
				if (errorCallback != nullptr)
				{
					errorCallback(error);
				}
				return false;
			}

			m_socket = socket(PF_INET, SOCK_STREAM, 0);
			if (m_socket == INVALID_SOCKET)
				return false;

			// send 처리를 별도의 스레드에서 하기 때문에 blocking 이어도 상관없다.
			//DWORD on = 1;
			//const int result = ioctlsocket(m_socket, FIONBIO, &on);
			//if (result == SOCKET_ERROR)
			//{
			//	LogError("ioctlsocket", WSAGetLastError());
			//	return false;
			//}

			SOCKADDR_IN serverAddress{};
			serverAddress.sin_family = AF_INET;
			serverAddress.sin_port = htons(port);
			inet_pton(AF_INET, ipAddress, &serverAddress.sin_addr.s_addr);

			if (connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
			{
				error = WSAGetLastError();
				LogError(L"socket connect", error);
				if (errorCallback != nullptr)
				{
					errorCallback(error);
				}
				return false;
			}

			m_thread_receive = std::thread([&, errorCallback]()
				{
					// 여기 수정하기, 버퍼 터짐

					std::vector<char> packetBuffer(eClientPacketBufferSize);

					std::vector<char> receivedPacketBuffer(eMaxPacketSize);
					uint32_t receivedPacketSize = 0;

					while (m_isRunning == true)
					{
						const int result = recv(m_socket, receivedPacketBuffer.data(), eMaxPacketSize, 0);
						if (result == SOCKET_ERROR)
						{
							const int error = WSAGetLastError();
							LogError(L"packet send", error);
							if (errorCallback != nullptr)
							{
								errorCallback(error);
							}
							Disconnect();
							break;
						}

						memcpy_s(packetBuffer.data() + receivedPacketSize, eClientPacketBufferSize - receivedPacketSize, receivedPacketBuffer.data(), result);
						receivedPacketSize += result;

						while (receivedPacketSize >= sizeof(packet::Header))
						{
							const packet::Header* pHeader = reinterpret_cast<const packet::Header*>(receivedPacketBuffer.data());
							if (receivedPacketSize < pHeader->length)
								break;

							{
								std::lock_guard<std::mutex> lock(m_mutex_receive);
								m_receivePacketBuffer.Push(nullptr, pHeader);
							}
							receivedPacketSize -= pHeader->length;
							memmove_s(packetBuffer.data(), eClientPacketBufferSize, packetBuffer.data() + pHeader->length, receivedPacketSize);
						}
					}
				});

			m_thread_send = std::thread([&]()
				{
					while (m_isRunning == true)
					{
						std::unique_lock<std::mutex> lock(m_mutex_send);
						m_condition_send.wait(lock, [&]()
							{
								return m_isRunning == false || m_sendPacketBuffer.IsEmpty() == false;
							});

						if (m_isRunning == false || m_sendPacketBuffer.IsEmpty() == true)
							return;

						std::unique_ptr<packet::ReceivedPacket> pReceivedPacket = m_sendPacketBuffer.Front();

						const packet::Header* pPacket = pReceivedPacket->pHeader;
						const int result = send(m_socket, reinterpret_cast<const char*>(pPacket), pPacket->length, 0);
						if (result == SOCKET_ERROR)
						{
							const int error = WSAGetLastError();
							LogError(L"packet send", error);
							if (errorCallback != nullptr)
							{
								errorCallback(error);
							}
						}

						m_sendPacketBuffer.Pop();
					}
				});

			return true;
		}

		void NetClient::Disconnect()
		{
			if (IsConnected() == false)
				return;

			m_isRunning = false;
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;

			m_condition_send.notify_all();

			WSACleanup();
		}

		void NetClient::Send(const packet::Header* pPacket)
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex_send);
				m_sendPacketBuffer.Push(nullptr, pPacket);
			}
			m_condition_send.notify_all();
		}

		bool NetClient::IsEmptySendBuffer()
		{
			std::lock_guard<std::mutex> lock(m_mutex_send);
			return m_sendPacketBuffer.IsEmpty();
		}

		void NetClient::ProcessReceivedPacket(std::function<void(const packet::Header*)> func)
		{
			std::lock_guard<std::mutex> lock(m_mutex_receive);
			while (m_receivePacketBuffer.IsEmpty() == false)
			{
				std::unique_ptr<packet::ReceivedPacket> pReceivedPacket = m_receivePacketBuffer.Front();
				{
					func(pReceivedPacket->pHeader);
				}
				m_receivePacketBuffer.Pop();
			}
		}
	}
}