#pragma once

#include "CommonLib/Lock.h"
#include "GraphicsInterface/GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class VTFManager : public IVTFManager
			{
			public:
				VTFManager();
				virtual ~VTFManager();

			public:
				virtual bool Allocate(uint32_t nMatrixCount, math::Matrix** ppDest_Out, uint32_t& nVTFID_Out) override;

			public:
				bool Bake();

			public:
				ITexture* GetTexture() const { return m_vtfInstance.pVTF; }

			private:
				thread::Lock m_lock;

				struct VTFInstance
				{
					uint32_t nAllocatedCount{ 0 };

					ITexture* pVTF{ nullptr };
					std::vector<math::Matrix> buffer;
				};
				VTFInstance m_vtfInstance;
			};
		}
	}
}