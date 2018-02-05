#pragma once

#include "CommonLib/Singleton.h"

#include "D3DInterface.h"

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
			bool Initialize();
			void Release();

			bool Allocate(size_t nMatrixCount, Math::Matrix** ppDest_Out, size_t& nVTFID_Out);

			void Flush();

			const std::shared_ptr<ITexture>& GetTexture();

		private:
			bool m_isInit;

			std::mutex m_mutex;

			struct VTFInstance
			{
				size_t nAllocatedCount{ 0 };

				std::shared_ptr<ITexture> pVTF;
				std::vector<Math::Matrix> buffer;
			};

			std::array<VTFInstance, ThreadCount> m_vtfInstances;
		};
	}
}