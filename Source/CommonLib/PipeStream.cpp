#include "stdafx.h"
#include "PipeStream.h"

namespace EastEngine
{
	namespace PipeNetwork
	{
		SPipeStream::SPipeStream()
			: m_bInit(false)
			, m_bStop(false)
			, m_nMaxInstanceCount(0)
			, m_hPipeServer(INVALID_HANDLE_VALUE)
		{
		}

		SPipeStream::~SPipeStream()
		{
			Release();
		}

		bool SPipeStream::InitServer(const char* strPipeName, uint32_t nMaxInstanceCount)
		{
			if (m_bInit == true)
				return true;

			m_bInit = true;

			std::string strPipeNameServer = "\\\\.\\pipe\\";
			strPipeNameServer += strPipeName;

			std::thread thServerPipeProcess([&, strPipeNameServer]()
			{
				m_hPipeServer = ::CreateNamedPipe(strPipeNameServer.c_str(),
					PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
					PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
					nMaxInstanceCount,
					65536, // 출력 버퍼
					65536, // 입력 버퍼
					NMPWAIT_WAIT_FOREVER, // 타임아웃
					NULL);// 보안속성

				while (true)
				{
					if (IsRunning() == false)
						break;

					if (m_hPipeServer == INVALID_HANDLE_VALUE)
						break;

					//::OVERLAPPED overlap;
					ConnectNamedPipe(m_hPipeServer, nullptr);

					PipeDataHeader header;

					uint32_t nSize = 0;
					if (read(nSize, &header, sizeof(PipeDataHeader)) == false)
					{
						DWORD errorCode = GetLastError();
						if (errorCode == ERROR_SUCCESS)
							continue;

						if (errorCode == ERROR_PIPE_CONNECTED)
						{
							::FlushFileBuffers(m_hPipeServer);
							continue;
						}

						if (errorCode != ERROR_MORE_DATA)
						{
							if (errorCode == ERROR_BROKEN_PIPE || errorCode == ERROR_NO_DATA)
							{
								::FlushFileBuffers(m_hPipeServer);
								::DisconnectNamedPipe(m_hPipeServer);
								continue;
							}
							else
							{
								printf("[PipeStream_S(0)] : Can't read : %d\n", errorCode);
								::FlushFileBuffers(m_hPipeServer);
								continue;
							}
						}
					}

					CPipeData binPack(header.nDataType, header.nDataSize);
					if (0 < header.nDataSize)
					{
						nSize = 0;
						if (read(nSize, binPack.GetBuffer(), header.nDataSize) == false)
						{
							printf("[PipeStream_S(1)] : Can't read : %d\n", GetLastError());
							::FlushFileBuffers(m_hPipeServer);
							continue;
						}
					}

					m_conQueueReceiveMsg.push(binPack);
				}
			});

			thServerPipeProcess.detach();

			return true;
		}

		bool SPipeStream::InitClient(const char* strPipeName)
		{
			std::string strPipeNameClient = "\\\\.\\pipe\\";
			strPipeNameClient += strPipeName;

			if (!::WaitNamedPipe(strPipeNameClient.c_str(), 1000))      //여분의 파이프를 생성할수 없을때 잠시대기
			{
				printf("[PipeStream_C(1)] : Can't connect : %d\n", GetLastError());
				return false;
			}

			m_hPipeClient = ::CreateFile(strPipeNameClient.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			if (m_hPipeClient == INVALID_HANDLE_VALUE)
				return false;

			return true;
		}

		void SPipeStream::Release()
		{
			if (m_bInit == false)
				return;

			disconnect();

			m_bInit = false;
		}

		void SPipeStream::CloseClient()
		{
			if (m_hPipeClient != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(m_hPipeClient);
			}
		}

		bool SPipeStream::PushMessage(CPipeData& pipeData)
		{
			if (pipeData.GetHeader().nDataType == -1)
				return false;

			pipeData.Packing();

			uint32_t nSize = 0;
			std::vector<char> vecFinBuffer;
			vecFinBuffer.resize(pipeData.GetDataSize() + sizeof(PipeDataHeader));
			CopyMemory(&vecFinBuffer[0], &pipeData.GetHeader(), sizeof(PipeDataHeader));

			if (pipeData.GetDataSize() != 0)
			{
				CopyMemory(&vecFinBuffer[sizeof(PipeDataHeader)], pipeData.GetData(), pipeData.GetDataSize());
			}

			bool bRet = write(nSize, &vecFinBuffer.front(), vecFinBuffer.size());
			if (bRet == false)
				printf("[PipeStream_S(0)] : Can't write : %d\n", GetLastError());

			return bRet;
		}

		bool SPipeStream::PopMessage(CPipeData& pipeData)
		{
			return m_conQueueReceiveMsg.try_pop(pipeData);
		}

		bool SPipeStream::write(uint32_t& nBytesWritten, const void* pWriteData, const uint32_t nWriteDataSize)
		{
			if (m_hPipeClient == INVALID_HANDLE_VALUE)
				return false;

			if (WriteFile(m_hPipeClient, pWriteData, nWriteDataSize, reinterpret_cast<DWORD*>(&nBytesWritten), nullptr) == false)
				return false;

			if (nWriteDataSize != nBytesWritten)
				return false;

			return true;
		}

		bool SPipeStream::read(uint32_t& nBytesRead, void* pReadBuffer, const uint32_t nReadBufferSize)
		{
			if (m_hPipeServer == INVALID_HANDLE_VALUE)
				return false;

			if (ReadFile(m_hPipeServer, pReadBuffer, nReadBufferSize, reinterpret_cast<DWORD*>(&nBytesRead), nullptr) == false)
				return false;

			if (nBytesRead == 0)
				return false;

			return true;
		}

		void SPipeStream::disconnect()
		{
			m_bStop = false;

			CloseClient();

			if (m_hPipeServer != INVALID_HANDLE_VALUE)
			{
				::FlushFileBuffers(m_hPipeServer);
				::DisconnectNamedPipe(m_hPipeServer);

				::CloseHandle(m_hPipeServer);
			}
		}
	}
}