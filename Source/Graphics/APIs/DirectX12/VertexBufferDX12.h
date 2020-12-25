#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

struct D3D12_VERTEX_BUFFER_VIEW;
struct ID3D12Resource;

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class VertexBuffer : public IVertexBuffer
			{
			public:
				VertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize, bool isDynamic);
				virtual ~VertexBuffer();

			public:
				virtual uint32_t GetVertexCount() const override;
				virtual uint32_t GetFormatSize() const override;

				virtual bool Map(MappedSubResourceData& mappedSubResourceData, bool isDiscard = true) override;
				virtual void Unmap() override;

			public:
				ID3D12Resource* GetBuffer() const;
				const D3D12_VERTEX_BUFFER_VIEW* GetView() const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}