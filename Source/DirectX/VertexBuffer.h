#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class VertexBuffer : public IVertexBuffer
		{
		public:
			VertexBuffer();
			virtual ~VertexBuffer();

			bool Init(EmVertexFormat::Type emVertexFormat, size_t nElementCount, const void* pData, D3D11_USAGE emUsage = D3D11_USAGE_DEFAULT, uint32_t nOptions = IVertexBuffer::Option::eNone);

		public:
			virtual ID3D11Buffer* const* GetBufferPtr() const override { return &m_pVertexBuffer; }
			virtual uint32_t GetFormatSize() const override { return m_nFormatSize; }
			virtual EmVertexFormat::Type GetFormat() const override { return m_emVertexFormat; }
			virtual uint32_t GetVertexNum() const override { return m_nVertexNum; }

			virtual bool Map(ThreadType emThreadID, uint32_t Subresource, D3D11_MAP emMap, void** ppData) const override;
			virtual void Unmap(ThreadType emThreadID, uint32_t Subresource) const override;

			virtual const void* GetRawValuePtr() const override
			{
				if (m_vecRawValue.empty() == true)
					return nullptr;

				return &m_vecRawValue.front();
			}

			virtual const VertexPos* GetVertexPosPtr() const override
			{
				if (m_vecVertexPos.empty() == true)
					return nullptr;

				return &m_vecVertexPos.front();
			}

			/*virtual const VertexClipSpace* GetVertexClipSpace() const override
			{
				if (m_vecVertexClipSpace.empty() == true)
					return nullptr;

				return &m_vecVertexClipSpace.front();
			}*/

		private:
			ID3D11Buffer* m_pVertexBuffer;
			uint32_t m_nFormatSize;
			EmVertexFormat::Type m_emVertexFormat;
			uint32_t m_nVertexNum;

			std::vector<byte> m_vecRawValue;
			std::vector<VertexPos> m_vecVertexPos;
			//std::vector<VertexClipSpace> m_vecVertexClipSpace;
		};
	}
}