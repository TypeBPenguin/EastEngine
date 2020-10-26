#pragma once

#include "CommonLib/Lock.h"
#include "Graphics/Interface/GraphicsInterface.h"

namespace est
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
				void Cleanup();

			public:
				Texture* GetTexture() const { return m_vtfInstance.pVTF.get(); }
				Texture* GetPrevTexture() const { return m_pPrevVTF.get(); }

			private:
				thread::SRWLock m_srwLock;

				struct VTFInstance
				{
					uint32_t allocatedCount{ 0 };

					std::unique_ptr<Texture> pVTF{ nullptr };
					std::vector<math::Matrix> buffer;
				};
				VTFInstance m_vtfInstance;
				std::unique_ptr<Texture> m_pPrevVTF{ nullptr };
			};
		}
	}
}