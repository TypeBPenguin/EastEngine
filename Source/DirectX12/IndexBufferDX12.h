#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

struct ID3D12Resource;
struct D3D12_INDEX_BUFFER_VIEW;

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class IndexBuffer : public IIndexBuffer
			{
			public:
				IndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize);
				virtual ~IndexBuffer();

			public:
				virtual uint32_t GetIndexCount() const override;

				virtual bool Map(void** ppData) override;
				virtual void Unmap() override;

			public:
				ID3D12Resource* GetBuffer() const;
				const D3D12_INDEX_BUFFER_VIEW* GetView() const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}