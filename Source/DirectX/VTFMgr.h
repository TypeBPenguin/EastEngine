#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;

		enum : size_t
		{
			eInvalidVTFID = std::numeric_limits<size_t>::max()
		};

		class VTFManager : public Singleton<VTFManager>
		{
			friend Singleton<VTFManager>;
		private:
			VTFManager();
			virtual ~VTFManager();

		public:
			enum
			{
				eTextureWidth = 1024,
				eBufferCapacity = eTextureWidth * eTextureWidth / 4,
			};

		public:
			bool Init();
			void Release();

			bool Process();

			bool Allocate(size_t nMatrixCount, Math::Matrix** ppDest_Out, size_t& nVTFID_Out);

			void Flush() { m_nAllocatedCount = 0; }

			const std::shared_ptr<ITexture>& GetTexture() { return m_pVTF; }

		private:
			bool m_isInit;

			std::mutex m_mutex;

			size_t m_nAllocatedCount;

			std::shared_ptr<ITexture> m_pVTF;
			Math::Matrix* m_pVTFBuffer;
		};
	}
}