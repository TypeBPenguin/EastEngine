#include "stdafx.h"
#include "NetworkInterface.h"

#include "NetworkServer.h"
#include "NetworkClient.h"
#include "IOBufferManager.h"

namespace est
{
	namespace network
	{
		void Destroy()
		{
			IOBufferManager::DestroyInstance();
		}

		std::unique_ptr<INetServer> CreateServer(uint16_t port)
		{
			std::unique_ptr<NetServer> pServer = std::make_unique<NetServer>();
			if (pServer->Initialize(port) == true)
				return pServer;

			return nullptr;
		}

		std::unique_ptr<INetClient> ConnectToServer(const char* ipAddress, uint16_t port, std::function<void(int)> errorCallback)
		{
			std::unique_ptr<NetClient> pClient = std::make_unique<NetClient>();
			if (pClient->Connect(ipAddress, port, errorCallback) == true)
				return pClient;

			return nullptr;
		}

		IIOBuffer* CreateIOBuffer(const packet::Header* pPacket)
		{
			IOBufferSend* pIOBuffer = static_cast<IOBufferSend*>(IOBufferManager::GetInstance()->AllocateIOBuffer(OperationType::eSend));
			pIOBuffer->Send(pPacket);
			return pIOBuffer;
		}

		void LogError(const wchar_t* log, int error, std::wstring* pResult_out)
		{
			wchar_t* message = nullptr;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<wchar_t*>(&message), 0, nullptr);
			if (pResult_out != nullptr)
			{
				*pResult_out = message;
			}
			LOG_ERROR(L"Error[%d] %s : %s", error, message, log);
			LocalFree(message);
		}
	}
}