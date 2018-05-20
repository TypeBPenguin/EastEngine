#pragma once

#include "D3DInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class DeviceContext : public IDeviceContext
		{
		public:
			DeviceContext(ID3D11DeviceContext* pDeviceContext);
			virtual ~DeviceContext();

		public:
			virtual ID3D11DeviceContext* GetInterface() { return m_pDeviceContext; }

		public:
			virtual void ClearState() override;

			virtual void ClearRenderTargetView(IRenderTarget* pRenderTarget, const math::Color& color) override;
			virtual void ClearDepthStencilView(IDepthStencil* pDepthStencil, uint32_t clearFlag = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL) override;
			virtual void ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView* pUnorderedAccessView, const math::UInt4& n4Uint) override;

		public:
			virtual void DrawAuto() override;
			virtual void Draw(uint32_t nVertexCount, uint32_t nStartVertexLocation) override;
			virtual void DrawInstanced(uint32_t nVertexCountPerInstance, uint32_t nInstanceCount, uint32_t nStartVertexLocation, uint32_t nStartInstanceLocation) override;
			virtual void DrawIndexed(uint32_t nIndexCount, uint32_t nStartIndexLocation, uint32_t nBaseVertexLocation) override;
			virtual void DrawIndexedInstanced(uint32_t nIndexCountPerInstance, uint32_t nInstanceCount, uint32_t nStartIndexLocation, int nBaseVertexLocation, uint32_t nStartInstanceLocation) override;
			virtual void Dispatch(uint32_t nThreadGroupCountX, uint32_t nThreadGroupCountY, uint32_t nThreadGroupCountZ) override;

		public:
			virtual bool SetInputLayout(EmVertexFormat::Type emVertexFormat) override;
			virtual bool SetInputLayout(ID3D11InputLayout* pInputLayout) override;

			virtual void SetBlendState(EmBlendState::Type emBlendState, const math::Vector4& f4BlendFactor = math::Vector4::Zero, uint32_t nSimpleMask = 0xffffffff) override;
			virtual void SetBlendState(const IBlendState* pBlendState, const math::Vector4& f4BlendFactor = math::Vector4::Zero, uint32_t nSimpleMask = 0xffffffff) override;

			virtual void SetDepthStencilState(EmDepthStencilState::Type emDepthStencil, uint32_t nStencilRef = 1) override;
			virtual void SetDepthStencilState(const IDepthStencilState* pDepthStencilState, uint32_t nStencilRef = 1) override;

			virtual void SetRasterizerState(EmRasterizerState::Type emRasterizerState) override;
			virtual void SetRasterizerState(const IRasterizerState* pRasterizerState) override;

			virtual void SetVertexBuffers(const IVertexBuffer* pVertexBuffer, uint32_t stride, uint32_t offset, uint32_t nStartSlot = 0, uint32_t numBuffers = 1) override;
			virtual void SetIndexBuffer(const IIndexBuffer* pIndexBuffer, uint32_t offset) override;

			virtual void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology) override;
			virtual void SetScissorRects(math::Rect* pRects, uint32_t nNumRects) override;

			virtual void SetRenderTargets(const RenderTargetDesc2D& renderTargetInfo, const IDepthStencil* pDepthStencil = nullptr) override;
			virtual void SetRenderTargets(IRenderTarget** ppRenderTarget, uint32_t nSize, const IDepthStencil* pDepthStencil = nullptr) override;
			virtual void SetRenderTargetsAndUnorderedAccessViews(IRenderTarget** ppRenderTarget, uint32_t nSize, IDepthStencil* pDepthStencil, uint32_t nUAVStartSlot, uint32_t nUAVCount, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const uint32_t* pUAVInitialCounts) override;

			virtual void SetViewport(const math::Viewport& viewport) override;
			virtual void SetViewport(const math::Viewport* pViewports, uint32_t nCount) override;
			virtual void SetDefaultViewport() override;

		public:
			virtual void CopySubresourceRegion(ID3D11Resource* pDstResource, uint32_t DstSubresource, uint32_t DstX, uint32_t DstY, uint32_t DstZ, ID3D11Resource* pSrcResource, uint32_t SrcSubresource, const D3D11_BOX* pSrcBox) override;

			virtual void UpdateSubresource(ID3D11Resource* pDstResource, uint32_t DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, uint32_t SrcRowPitch, uint32_t SrcDepthPitch) override;

			virtual HRESULT Map(ID3D11Resource* pResource, uint32_t Subresource, D3D11_MAP MapType, uint32_t MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource) override;
			virtual void Unmap(ID3D11Resource* pResource, uint32_t Subresource) override;

			virtual void GenerateMips(ID3D11ShaderResourceView* pShaderResourceView) override;

		public:
			virtual ID3DUserDefinedAnnotation* GetUserDefineAnnotation() override { return m_pUserDefineAnnotation; };

		private:
			ID3D11DeviceContext* m_pDeviceContext{ nullptr };
			ID3DUserDefinedAnnotation* m_pUserDefineAnnotation{ nullptr };
		};
	}
}