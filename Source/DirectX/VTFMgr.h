#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;

		enum : uint32_t
		{
			eInvalidVTFID = UINT32_MAX
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

			bool Allocate(uint32_t nMatrixCount, Math::Matrix** ppDest_Out, uint32_t& nVTFID_Out)
			{
				if (m_nAllocatedCount + nMatrixCount >= eBufferCapacity)
				{
					*ppDest_Out = nullptr;
					nVTFID_Out = eInvalidVTFID;
					return false;
				}

				*ppDest_Out = &m_pVTFBuffer[m_nAllocatedCount];
				nVTFID_Out = m_nAllocatedCount;

				m_nAllocatedCount += nMatrixCount;

				return true;
			}

			void Flush() { m_nAllocatedCount = 0; }

			const std::shared_ptr<ITexture>& GetTexture() { return m_pVTF; }

		private:
			bool m_bInit;

			uint32_t m_nAllocatedCount;

			std::shared_ptr<ITexture> m_pVTF;
			Math::Matrix* m_pVTFBuffer;
		};
	}
}