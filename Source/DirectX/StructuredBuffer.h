#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class StructuredBuffer : public IStructuredBuffer
		{
		public:
			StructuredBuffer();
			virtual ~StructuredBuffer();

			bool Init(void* pData, uint32_t nNumElements, uint32_t nByteStride, bool isEnableCpuWrite = false, bool isEnableGpuWrite = true);

		public:
			virtual void UpdateSubresource(uint32_t DstSubresource, const void* pSrcData, uint32_t SrcRowPitch) override;

		public:
			virtual ID3D11Buffer* GetBuffer() override { return m_pBuffer; }
			virtual ID3D11UnorderedAccessView* GetUnorderedAccessView() override { return m_pUnorderedAccessView; }
			virtual ID3D11ShaderResourceView* GetShaderResourceView() override { return m_pShaderResourceView; }

		protected:
			ID3D11Buffer* m_pBuffer;
			ID3D11UnorderedAccessView* m_pUnorderedAccessView;
			ID3D11ShaderResourceView* m_pShaderResourceView;

			uint32_t m_nByteWidth;
		};
	}
}