#pragma once

#include "Singleton.h"

#include "PipeData.h"

namespace EastEngine
{
	namespace PipeNetwork
	{
		class SPipeStream : public Singleton<SPipeStream>
		{
			friend Singleton<SPipeStream>;
		private:
			SPipeStream();
			virtual ~SPipeStream();

		public:
			bool InitServer(const char* strPipeName = "EastEngine_A", uint32_t nMaxInstanceCount = 10);
			bool InitClient(const char* strPipeName = "EastEngine_S");
			void Release();

			void CloseClient();

			bool PushMessage(CPipeData& pipeData);
			bool PopMessage(CPipeData& pipeData);

			bool IsRunning() { return m_bStop == false; }

		private:
			bool write(uint32_t& nBytesWritten, const void* pWriteData, const uint32_t nWriteDataSize);
			bool read(uint32_t& nBytesRead, void* pReadBuffer, const uint32_t nReadBufferSize);

		private:
			void disconnect();

		private:
			bool m_bInit;
			bool m_bStop;

			uint32_t m_nMaxInstanceCount;

			HANDLE m_hPipeServer;
			HANDLE m_hPipeClient;

			Concurrency::concurrent_queue<CPipeData> m_conQueueReceiveMsg;
		};
	}
}