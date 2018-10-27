#pragma once

#include "CommonLib/Lock.h"
#include "GraphicsInterface/GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class Texture;

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
				Texture* GetTexture() const { return m_vtfInstance.pVTF; }

			private:
				thread::SRWLock m_srwLock;

				struct VTFInstance
				{
					uint32_t nAllocatedCount{ 0 };

					Texture* pVTF{ nullptr };
					std::vector<math::Matrix> buffer;
				};
				VTFInstance m_vtfInstance;
			};
		}
	}
}