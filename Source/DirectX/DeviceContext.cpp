#include "stdafx.h"
#include "DeviceContext.h"

namespace eastengine
{
	namespace graphics
	{
		DeviceContext::DeviceContext(ID3D11DeviceContext* pDeviceContext)
			: m_pDeviceContext(pDeviceContext)
		{
			HRESULT hr = m_pDeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&m_pUserDefineAnnotation));
			if (FAILED(hr))
			{
				assert(false);
			}
		}

		DeviceContext::~DeviceContext()
		{
			ClearState();
			SafeRelease(m_pUserDefineAnnotation);
			SafeRelease(m_pDeviceContext);
		}

		void DeviceContext::ClearState()
		{
			m_pDeviceContext->ClearState();
		}

		void DeviceContext::ClearRenderTargetView(IRenderTarget* pRenderTarget, const math::Color& color)
		{
			if (pRenderTarget == nullptr)
				return;

			m_pDeviceContext->ClearRenderTargetView(pRenderTarget->GetRenderTargetView(), &color.r);
		}

		void DeviceContext::ClearDepthStencilView(IDepthStencil* pDepthStencil, uint32_t clearFlag)
		{
			if (pDepthStencil == nullptr)
				return;

			m_pDeviceContext->ClearDepthStencilView(pDepthStencil->GetDepthStencilView(), clearFlag, 1.f, 0);
		}

		void DeviceContext::ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView* pUnorderedAccessView, const math::uint4& n4Uint)
		{
			if (pUnorderedAccessView == nullptr)
				return;

			m_pDeviceContext->ClearUnorderedAccessViewUint(pUnorderedAccessView, &n4Uint.x);
		}

		void DeviceContext::DrawAuto()
		{
			m_pDeviceContext->DrawAuto();
		}

		void DeviceContext::Draw(uint32_t nVertexCount, uint32_t nStartVertexLocation)
		{
			m_pDeviceContext->Draw(nVertexCount, nStartVertexLocation);
		}

		void DeviceContext::DrawInstanced(uint32_t nVertexCountPerInstance, uint32_t nInstanceCount, uint32_t nStartVertexLocation, uint32_t nStartInstanceLocation)
		{
			m_pDeviceContext->DrawInstanced(nVertexCountPerInstance, nInstanceCount, nStartVertexLocation, nStartInstanceLocation);
		}

		void DeviceContext::DrawIndexed(uint32_t nIndexCount, uint32_t nStartIndexLocation, uint32_t nBaseVertexLocation)
		{
			m_pDeviceContext->DrawIndexed(nIndexCount, nStartIndexLocation, nBaseVertexLocation);
		}

		void DeviceContext::DrawIndexedInstanced(uint32_t nIndexCountPerInstance, uint32_t nInstanceCount, uint32_t nStartIndexLocation, int nBaseVertexLocation, uint32_t nStartInstanceLocation)
		{
			m_pDeviceContext->DrawIndexedInstanced(nIndexCountPerInstance, nInstanceCount, nStartIndexLocation, nBaseVertexLocation, nStartInstanceLocation);
		}

		void DeviceContext::Dispatch(uint32_t nThreadGroupCountX, uint32_t nThreadGroupCountY, uint32_t nThreadGroupCountZ)
		{
			m_pDeviceContext->Dispatch(nThreadGroupCountX, nThreadGroupCountY, nThreadGroupCountZ);
		}

		bool DeviceContext::SetInputLayout(EmVertexFormat::Type emVertexFormat)
		{
			ID3D11InputLayout* pInputLayout = GetDevice()->GetInputLayout(emVertexFormat);
			return SetInputLayout(pInputLayout);
		}

		bool DeviceContext::SetInputLayout(ID3D11InputLayout* pInputLayout)
		{
			if (pInputLayout == nullptr)
				return false;

			m_pDeviceContext->IASetInputLayout(pInputLayout);

			return true;
		}

		void DeviceContext::SetBlendState(EmBlendState::Type emBlendState, const math::float4& f4BlendFactor, uint32_t nSimpleMask)
		{
			IBlendState* pBlendState = GetDevice()->GetBlendState(emBlendState);
			SetBlendState(pBlendState, f4BlendFactor, nSimpleMask);
		}

		void DeviceContext::SetBlendState(const IBlendState* pBlendState, const math::float4& f4BlendFactor, uint32_t nSimpleMask)
		{
			if (pBlendState == nullptr)
				return;

			m_pDeviceContext->OMSetBlendState(pBlendState->GetInterface(), &f4BlendFactor.x, nSimpleMask);
		}

		void DeviceContext::SetDepthStencilState(EmDepthStencilState::Type emDepthStencil, uint32_t nStencilRef)
		{
			IDepthStencilState* pDepthStencilState = GetDevice()->GetDepthStencilState(emDepthStencil);
			SetDepthStencilState(pDepthStencilState, nStencilRef);
		}

		void DeviceContext::SetDepthStencilState(const IDepthStencilState* pDepthStencilState, uint32_t nStencilRef)
		{
			if (pDepthStencilState == nullptr)
				return;

			m_pDeviceContext->OMSetDepthStencilState(pDepthStencilState->GetInterface(), nStencilRef);
		}

		void DeviceContext::SetRasterizerState(EmRasterizerState::Type emRasterizerState)
		{
			IRasterizerState* pRasterizerState = GetDevice()->GetRasterizerState(emRasterizerState);
			SetRasterizerState(pRasterizerState);
		}

		void DeviceContext::SetRasterizerState(const IRasterizerState* pRasterizerState)
		{
			if (pRasterizerState == nullptr)
				return;

			m_pDeviceContext->RSSetState(pRasterizerState->GetInterface());
		}

		void DeviceContext::SetVertexBuffers(const IVertexBuffer* pVertexBuffer, uint32_t stride, uint32_t offset, uint32_t nStartSlot, uint32_t numBuffers)
		{
			if (pVertexBuffer != nullptr)
			{
				m_pDeviceContext->IASetVertexBuffers(nStartSlot, numBuffers, pVertexBuffer->GetBufferPtr(), &stride, &offset);
			}
			else
			{
				m_pDeviceContext->IASetVertexBuffers(0, 0, nullptr, 0, 0);
			}
		}

		void DeviceContext::SetIndexBuffer(const IIndexBuffer* pIndexBuffer, uint32_t offset)
		{
			if (pIndexBuffer != nullptr)
			{
				m_pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), static_cast<DXGI_FORMAT>(pIndexBuffer->GetFormat()), offset);
			}
			else
			{
				m_pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
			}
		}

		void DeviceContext::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology)
		{
			m_pDeviceContext->IASetPrimitiveTopology(primitiveTopology);
		}

		void DeviceContext::SetScissorRects(math::Rect* pRects, uint32_t nNumRects)
		{
			m_pDeviceContext->RSSetScissorRects(nNumRects, pRects);
		}

		void DeviceContext::SetRenderTargets(const RenderTargetDesc2D& renderTargetInfo, const IDepthStencil* pDepthStencil)
		{
			IRenderTarget* pRenderTarget = GetDevice()->GetRenderTarget(renderTargetInfo);
			SetRenderTargets(&pRenderTarget, 1, pDepthStencil);
		}

		void DeviceContext::SetRenderTargets(IRenderTarget** ppRenderTarget, uint32_t nSize, const IDepthStencil* pDepthStencil)
		{
			std::vector<ID3D11RenderTargetView*> vecRenderTarget(nSize);
			for (uint32_t i = 0; i < nSize; ++i)
			{
				if (ppRenderTarget != nullptr && ppRenderTarget[i] != nullptr)
				{
					vecRenderTarget[i] = ppRenderTarget[i]->GetRenderTargetView();
				}
			}

			ID3D11DepthStencilView* pDepthStencilView = pDepthStencil == nullptr ? nullptr : pDepthStencil->GetDepthStencilView();
			m_pDeviceContext->OMSetRenderTargets(nSize, &vecRenderTarget.front(), pDepthStencilView);
		}

		void DeviceContext::SetRenderTargetsAndUnorderedAccessViews(IRenderTarget** ppRenderTarget, uint32_t nSize, IDepthStencil* pDepthStencil, uint32_t nUAVStartSlot, uint32_t nUAVCount, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const uint32_t* pUAVInitialCounts)
		{
			std::vector<ID3D11RenderTargetView*> vecRenderTarget(nSize);
			for (uint32_t i = 0; i < nSize; ++i)
			{
				if (ppRenderTarget[i] != nullptr)
				{
					vecRenderTarget[i] = ppRenderTarget[i]->GetRenderTargetView();
				}
			}

			ID3D11DepthStencilView* pDepthStencilView = pDepthStencil == nullptr ? nullptr : pDepthStencil->GetDepthStencilView();
			m_pDeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(nSize, &vecRenderTarget.front(), pDepthStencilView, nUAVStartSlot, nUAVCount, ppUnorderedAccessViews, pUAVInitialCounts);
		}

		void DeviceContext::SetViewport(const math::Viewport& viewport)
		{
			m_pDeviceContext->RSSetViewports(1, viewport.Get11());
		}

		void DeviceContext::SetViewport(const math::Viewport* pViewports, uint32_t nCount)
		{
			m_pDeviceContext->RSSetViewports(nCount, reinterpret_cast<const D3D11_VIEWPORT*>(pViewports));
		}

		void DeviceContext::SetDefaultViewport()
		{
			const math::Viewport& viewport = GetDevice()->GetViewport();

			m_pDeviceContext->RSSetViewports(1, viewport.Get11());
		}

		void DeviceContext::CopySubresourceRegion(ID3D11Resource* pDstResource, uint32_t DstSubresource, uint32_t DstX, uint32_t DstY, uint32_t DstZ, ID3D11Resource* pSrcResource, uint32_t SrcSubresource, const D3D11_BOX* pSrcBox)
		{
			m_pDeviceContext->CopySubresourceRegion(pDstResource, DstSubresource, DstX, DstY, DstZ, pSrcResource, SrcSubresource, pSrcBox);
		}

		void DeviceContext::UpdateSubresource(ID3D11Resource* pDstResource, uint32_t DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, uint32_t SrcRowPitch, uint32_t SrcDepthPitch)
		{
			m_pDeviceContext->UpdateSubresource(pDstResource, DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
		}

		HRESULT DeviceContext::Map(ID3D11Resource* pResource, uint32_t Subresource, D3D11_MAP MapType, uint32_t MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource)
		{
			return m_pDeviceContext->Map(pResource, Subresource, MapType, MapFlags, pMappedResource);
		}

		void DeviceContext::Unmap(ID3D11Resource* pResource, uint32_t Subresource)
		{
			m_pDeviceContext->Unmap(pResource, Subresource);
		}

		void DeviceContext::GenerateMips(ID3D11ShaderResourceView* pShaderResourceView)
		{
			m_pDeviceContext->GenerateMips(pShaderResourceView);
		}
	}
}