#include "stdafx.h"
#include "NetworkServer.h"

#include "IOBufferManager.h"

namespace est
{
	namespace network
	{
		NetServer::NetServer()
		{
			
		}

		NetServer::~NetServer()
		{
			Release();
		}

		bool NetServer::Initialize(uint16_t port)
		{
			if (IsValid() == true)
				return true;

			WSADATA wsaData{};
			int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (error != 0)
			{
				LogError(L"WSAStartup", error);
				return false;
			}

			error = gethostname(m_hostName, sizeof(m_hostName));
			if (error != 0)
			{
				LogError(L"gethostname", error);
				return false;
			}

			ADDRINFO addressInfo{};
			addressInfo.ai_flags = AI_PASSIVE;
			addressInfo.ai_family = AF_UNSPEC;
			addressInfo.ai_socktype = SOCK_STREAM;

			ADDRINFO* pServerInfo = nullptr;
			if (getaddrinfo(nullptr, "9000", &addressInfo, &pServerInfo) != 0)
			{
				LogError(L"getaddrinfo", GetLastError());
				return false;
			}

			for (ADDRINFO* p = pServerInfo; p != nullptr; p = p->ai_next)
			{
				if (p->ai_family == AF_INET)
				{
				}
				else
				{
					sockaddr_in* pAddressIn = reinterpret_cast<sockaddr_in*>(pServerInfo->ai_addr);
					inet_ntop(AF_INET, &pAddressIn->sin_addr, m_ipAddress, sizeof(m_ipAddress));
				}
			}

			freeaddrinfo(pServerInfo);

			m_handleIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
			if (m_handleIOCP == NULL)
			{
				LogError(L"CreateIoCompletionPort", GetLastError());
				return false;
			}

			for (size_t i = 0; i < eMaxUserSession; ++i)
			{
				m_userSessionAllocator.emplace(i);
			}

			const uint32_t numCore = std::thread::hardware_concurrency();
			m_workerThreads.resize(numCore);

			for (uint32_t i = 0; i < numCore; ++i)
			{
				m_workerThreads[i] = std::thread([&]()
					{
						while (m_isRunning == true)
						{
							size_t id = eInvaliUserSessionID;

							DWORD numTransferredBytes{ 0 };
							IOverlapped* pUserOverlapped = nullptr;

							const BOOL result = GetQueuedCompletionStatus(m_handleIOCP, &numTransferredBytes, &id, reinterpret_cast<LPOVERLAPPED*>(&pUserOverlapped), INFINITE);
							if (result == FALSE || numTransferredBytes == 0)
							{
								if (result == FALSE)
								{
									LogError(L"GetQueuedCompletionStatus", GetLastError());
								}

								if (id != eInvaliUserSessionID)
								{
									std::lock_guard<std::mutex> lock(m_mutex_disconnectUserSession);
									m_disconnectUserSessions.emplace_back(&m_userSessions[id]);
								}
								continue;
							}

							switch (pUserOverlapped->operationType)
							{
							case OperationType::eSend:
							{
								OverlappedSend* pSendOverlapped = static_cast<OverlappedSend*>(pUserOverlapped);
								IOBufferManager::GetInstance()->DestroyIOBuffer(pSendOverlapped->pIOBuffer);
								pSendOverlapped->pIOBuffer = nullptr;

								IOBufferManager::GetInstance()->DestroyOverlapped(pSendOverlapped);
							}
							break;
							case OperationType::eReceive:
							{
								m_userSessions[id].Receive(numTransferredBytes);

								while (true)
								{
									const packet::Header* pReceivedPacket = m_userSessions[id].GetReceivedPacket();
									if (pReceivedPacket == nullptr)
										break;

									{
										std::lock_guard<std::mutex> lock(m_mutex);
										m_receivePacketBuffer.Push(&m_userSessions[id], pReceivedPacket);
									}
									m_userSessions[id].PopReceivedPacket();
								}

								m_userSessions[id].SetReceiveState();
							}
							break;
							default:
								// LOG_FATAL("unknown operation type : %d", userOverlapped.operationType);
								continue;
							}
						}
					});
			}

			m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (m_socket == INVALID_SOCKET)
			{
				LogError(L"WSASocket", WSAGetLastError());
				return false;
			}

			SOCKADDR_IN serverAddress{};
			serverAddress.sin_family = AF_INET;
			serverAddress.sin_port = htons(port);
			inet_pton(AF_INET, m_ipAddress, &serverAddress.sin_addr.s_addr);

			if (bind(m_socket, reinterpret_cast<const sockaddr*>(&serverAddress), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
			{
				LogError(L"socket bind", WSAGetLastError());
				return false;
			}

			if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
			{
				LogError(L"socket listen", WSAGetLastError());
				return false;
			}

			m_acceptThread = std::thread([&]()
				{
					while (m_isRunning == true)
					{
						SOCKADDR_IN clientAddress{};
						int addrLen = sizeof(clientAddress);

						SOCKET clientSocket = WSAAccept(m_socket, reinterpret_cast<SOCKADDR*>(&clientAddress), &addrLen, NULL, 0);
						if (clientSocket == INVALID_SOCKET)
						{
							LogError(L"WSAAccept", WSAGetLastError());
							continue;
						}

						size_t id = eInvaliUserSessionID;
						{
							std::lock_guard<std::mutex> lock(m_mutex);
							if (m_userSessionAllocator.empty() == true)
							{
								closesocket(clientSocket);
								continue;
							}

							id = m_userSessionAllocator.front();
							m_userSessionAllocator.pop();
						}

						if (id >= eMaxUserSession)
							continue;

						if (m_userSessions[id].IsConnected() == true)
						{
							// LOG_FATAL("already connected usersession, please check this logic");
							continue;
						}

						CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), m_handleIOCP, id, 0);

						char clientIP[32]{};
						inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, 32);

						m_userSessions[id].Connect(clientSocket, id, clientIP);
						m_userSessions[id].SetReceiveState();

						{
							std::lock_guard<std::mutex> lock(m_mutex);
							m_activeUserSessions[id] = &m_userSessions[id];
						}

						{
							std::lock_guard<std::mutex> lock(m_mutex_connectUserSession);
							m_connectUserSessions.emplace(&m_userSessions[id]);
						}
					}
				});

			return true;
		}

		void NetServer::Release()
		{
			if (IsValid() == false)
				return;

			int error = WSACleanup();
			if (error != 0)
			{
				LogError(L"WSACleanup", error);
				return;
			}

			CloseHandle(m_handleIOCP);
			m_handleIOCP = INVALID_HANDLE_VALUE;

			for (auto& thread : m_workerThreads)
			{
				if (thread.joinable() == true)
				{
					thread.join();
				}
			}

			if (m_acceptThread.joinable() == true)
			{
				m_acceptThread.join();
			}
		}

		void NetServer::Broadcast(const packet::Header* pPacket)
		{
			if (pPacket == nullptr || pPacket->length == 0)
				return;

			if (pPacket->length > eMaxPacketSize)
			{
				//LOG_FATAL("failed to broadcast packet, packet size over : %u > %u", pPacket->length, eMaxPacketSize);
				return;
			}

			IIOBuffer* pIOBuffer = CreateIOBuffer(pPacket);

			for (auto& iter : m_activeUserSessions)
			{
				iter.second->Send(pIOBuffer);
			}

			IOBufferManager::GetInstance()->DestroyIOBuffer(pIOBuffer);
		}

		void NetServer::ProcessConnectUserSessions(std::function<void(IUserSession*)> func)
		{
			std::lock_guard<std::mutex> lock(m_mutex_connectUserSession);
			while (m_connectUserSessions.empty() == false)
			{
				UserSession* pUserSession = m_connectUserSessions.front();
				m_connectUserSessions.pop();

				func(pUserSession);
			}
		}

		void NetServer::ProcessDisconnectUserSessions(std::function<void(IUserSession*)> func)
		{
			std::vector<UserSession*> disconnectUserSessions;
			{
				std::lock_guard<std::mutex> lock(m_mutex_disconnectUserSession);
				std::copy(m_disconnectUserSessions.begin(), m_disconnectUserSessions.end(), std::back_inserter(disconnectUserSessions));

				for (auto& pUserSession : m_disconnectUserSessions)
				{
					func(pUserSession);
					pUserSession->Disconnect();
				}
				m_disconnectUserSessions.clear();
			}

			std::lock_guard<std::mutex> lock(m_mutex);
			for (auto& pUserSession : disconnectUserSessions)
			{
				m_userSessionAllocator.emplace(pUserSession->GetID());
				m_activeUserSessions.erase(pUserSession->GetID());
			}
		}

		void NetServer::ProcessReceivedPacket(std::function<void(IUserSession* pUserSession, const packet::Header*)> func)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			while (m_receivePacketBuffer.IsEmpty() == false)
			{
				std::unique_ptr<packet::ReceivedPacket> pReceivedPacket = m_receivePacketBuffer.Front();
				{
					func(pReceivedPacket->pUserSession, pReceivedPacket->pHeader);
				}
				m_receivePacketBuffer.Pop();
			}
		}
	}
}