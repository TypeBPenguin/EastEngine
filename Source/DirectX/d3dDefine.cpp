#include "stdafx.h"
#include "d3dDefine.h"

namespace EastEngine
{
	namespace Graphics
	{
		SamplerStateDesc::SamplerStateDesc()
			: CD3D11_SAMPLER_DESC(CD3D11_DEFAULT())
		{
		}

		SamplerStateDesc::SamplerStateDesc(const D3D11_SAMPLER_DESC& desc)
		{
			Memory::Copy(this, sizeof(SamplerStateDesc), &desc);
		}

		SamplerStateKey SamplerStateDesc::GetKey() const
		{
			String::StringID strKey;
			strKey.Format("%lu_%lu_%lu_%lu_%x_%lu_%lu_%x_%x_%x_%x_%x_%x",
				Filter, AddressU, AddressV, AddressW,
				*(reinterpret_cast<const uint32_t*>(&MipLODBias)),
				MaxAnisotropy, ComparisonFunc,
				*(reinterpret_cast<const uint32_t*>(&BorderColor[0])),
				*(reinterpret_cast<const uint32_t*>(&BorderColor[1])),
				*(reinterpret_cast<const uint32_t*>(&BorderColor[2])),
				*(reinterpret_cast<const uint32_t*>(&BorderColor[3])),
				*(reinterpret_cast<const uint32_t*>(&MinLOD)),
				*(reinterpret_cast<const uint32_t*>(&MaxLOD)));

			return SamplerStateKey(strKey.Key());
		}

		BlendStateDesc::BlendStateDesc()
			: CD3D11_BLEND_DESC(CD3D11_DEFAULT())
		{
		}

		BlendStateKey BlendStateDesc::GetKey() const
		{
			String::StringID strKey;
			strKey.Format("%d_%d_{%d_%ld_%ld_%ld_%ld_%ld_%ld_%d}_{%d_%ld_%ld_%ld_%ld_%ld_%ld_%d}_{%d_%ld_%ld_%ld_%ld_%ld_%ld_%d}_{%d_%ld_%ld_%ld_%ld_%ld_%ld_%d}_{%d_%ld_%ld_%ld_%ld_%ld_%ld_%d}_{%d_%ld_%ld_%ld_%ld_%ld_%ld_%d}_{%d_%ld_%ld_%ld_%ld_%ld_%ld_%d}_{%d_%ld_%ld_%ld_%ld_%ld_%ld_%d}",
				static_cast<int>(AlphaToCoverageEnable), static_cast<int>(IndependentBlendEnable),
				static_cast<int>(RenderTarget[0].BlendEnable), RenderTarget[0].SrcBlend, RenderTarget[0].DestBlend, RenderTarget[0].BlendOp,
				RenderTarget[0].SrcBlendAlpha, RenderTarget[0].DestBlendAlpha, RenderTarget[0].BlendOpAlpha, static_cast<int>(RenderTarget[0].RenderTargetWriteMask),
				static_cast<int>(RenderTarget[1].BlendEnable), RenderTarget[1].SrcBlend, RenderTarget[1].DestBlend, RenderTarget[1].BlendOp,
				RenderTarget[1].SrcBlendAlpha, RenderTarget[1].DestBlendAlpha, RenderTarget[1].BlendOpAlpha, static_cast<int>(RenderTarget[1].RenderTargetWriteMask),
				static_cast<int>(RenderTarget[2].BlendEnable), RenderTarget[2].SrcBlend, RenderTarget[2].DestBlend, RenderTarget[2].BlendOp,
				RenderTarget[2].SrcBlendAlpha, RenderTarget[2].DestBlendAlpha, RenderTarget[2].BlendOpAlpha, static_cast<int>(RenderTarget[2].RenderTargetWriteMask),
				static_cast<int>(RenderTarget[3].BlendEnable), RenderTarget[3].SrcBlend, RenderTarget[3].DestBlend, RenderTarget[3].BlendOp,
				RenderTarget[3].SrcBlendAlpha, RenderTarget[3].DestBlendAlpha, RenderTarget[3].BlendOpAlpha, static_cast<int>(RenderTarget[3].RenderTargetWriteMask),
				static_cast<int>(RenderTarget[4].BlendEnable), RenderTarget[4].SrcBlend, RenderTarget[4].DestBlend, RenderTarget[4].BlendOp,
				RenderTarget[4].SrcBlendAlpha, RenderTarget[4].DestBlendAlpha, RenderTarget[4].BlendOpAlpha, static_cast<int>(RenderTarget[4].RenderTargetWriteMask),
				static_cast<int>(RenderTarget[5].BlendEnable), RenderTarget[5].SrcBlend, RenderTarget[5].DestBlend, RenderTarget[5].BlendOp,
				RenderTarget[5].SrcBlendAlpha, RenderTarget[5].DestBlendAlpha, RenderTarget[5].BlendOpAlpha, static_cast<int>(RenderTarget[5].RenderTargetWriteMask),
				static_cast<int>(RenderTarget[6].BlendEnable), RenderTarget[6].SrcBlend, RenderTarget[6].DestBlend, RenderTarget[6].BlendOp,
				RenderTarget[6].SrcBlendAlpha, RenderTarget[6].DestBlendAlpha, RenderTarget[6].BlendOpAlpha, static_cast<int>(RenderTarget[6].RenderTargetWriteMask),
				static_cast<int>(RenderTarget[7].BlendEnable), RenderTarget[7].SrcBlend, RenderTarget[7].DestBlend, RenderTarget[7].BlendOp,
				RenderTarget[7].SrcBlendAlpha, RenderTarget[7].DestBlendAlpha, RenderTarget[7].BlendOpAlpha, static_cast<int>(RenderTarget[7].RenderTargetWriteMask));

			return BlendStateKey(strKey.Key());
		}

		DepthStencilStateDesc::DepthStencilStateDesc()
			: CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT())
		{
		}

		DepthStencilStateKey DepthStencilStateDesc::GetKey() const
		{
			String::StringID strKey;
			strKey.Format("%d_%u_%u_%d_%d_%d_%u_%u_%u_%u_%u_%u_%u_%u",
				static_cast<int>(DepthEnable), DepthWriteMask, DepthFunc,
				static_cast<int>(StencilEnable), StencilReadMask, StencilWriteMask,
				FrontFace.StencilDepthFailOp,
				FrontFace.StencilFailOp,
				FrontFace.StencilFunc,
				FrontFace.StencilPassOp,
				BackFace.StencilDepthFailOp,
				BackFace.StencilFailOp,
				BackFace.StencilFunc,
				BackFace.StencilPassOp);

			return DepthStencilStateKey(strKey.Key());
		}

		RasterizerStateDesc::RasterizerStateDesc()
			: CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT())
		{
		}

		RasterizerStateKey RasterizerStateDesc::GetKey() const
		{
			String::StringID strKey;
			strKey.Format("%u_%u_%d_%d_%x_%x_%d_%d_%d_%d",
				FillMode, CullMode, FrontCounterClockwise, DepthBias,
				*(reinterpret_cast<const uint32_t*>(&DepthBiasClamp)),
				*(reinterpret_cast<const uint32_t*>(&SlopeScaledDepthBias)),
				DepthClipEnable,
				ScissorEnable,
				MultisampleEnable,
				AntialiasedLineEnable);

			return RasterizerStateKey(strKey.Key());
		}

		TextureDesc1D::TextureDesc1D()
			: CD3D11_TEXTURE1D_DESC(DXGI_FORMAT_R8G8B8A8_UINT, 0)
			, SRVDesc(D3D11_SRV_DIMENSION_TEXTURE1D, DXGI_FORMAT_R8G8B8A8_UINT)
		{
		}

		TextureDesc1D::TextureDesc1D(DXGI_FORMAT format,
			uint32_t width,
			uint32_t arraySize,
			uint32_t mipLevels,
			uint32_t bindFlags,
			D3D11_USAGE usage,
			uint32_t cpuaccessFlags,
			uint32_t miscFlags)
			: CD3D11_TEXTURE1D_DESC(format, width, arraySize, mipLevels, bindFlags, usage, cpuaccessFlags, miscFlags)
			, SRVDesc(arraySize == 1 ? D3D11_SRV_DIMENSION_TEXTURE1D : D3D11_SRV_DIMENSION_TEXTURE1DARRAY, format)
		{
		}

		TextureDesc1D::TextureDesc1D(const D3D11_TEXTURE1D_DESC& desc)
			: CD3D11_TEXTURE1D_DESC(desc)
			, SRVDesc(desc.ArraySize == 1 ? D3D11_SRV_DIMENSION_TEXTURE1D : D3D11_SRV_DIMENSION_TEXTURE1DARRAY, desc.Format)
		{
		}

		void TextureDesc1D::SetSRVDesc(uint32_t nMostDetailedMip, uint32_t nFirstArraySlice, uint32_t nFlags)
		{
			SRVDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(ArraySize == 1 ? D3D11_SRV_DIMENSION_TEXTURE1D : D3D11_SRV_DIMENSION_TEXTURE1DARRAY, Format, nMostDetailedMip, MipLevels, nFirstArraySlice, ArraySize, nFlags);
		}

		TextureDesc2D::TextureDesc2D()
			: CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R8G8B8A8_UINT, 0, 0)
			, SRVDesc(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UINT)
		{
		}

		TextureDesc2D::TextureDesc2D(DXGI_FORMAT format,
			uint32_t width,
			uint32_t height,
			uint32_t arraySize,
			uint32_t mipLevels,
			uint32_t bindFlags,
			D3D11_USAGE usage,
			uint32_t cpuaccessFlags,
			uint32_t sampleCount,
			uint32_t sampleQuality,
			uint32_t miscFlags)
			: CD3D11_TEXTURE2D_DESC(format, width, height, arraySize, mipLevels, bindFlags, usage, cpuaccessFlags, sampleCount, sampleQuality, miscFlags)
			, SRVDesc(arraySize == 1 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DARRAY, format)
		{
		}

		TextureDesc2D::TextureDesc2D(const CD3D11_TEXTURE2D_DESC& desc)
			: CD3D11_TEXTURE2D_DESC(desc)
			, SRVDesc(desc.ArraySize == 1 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DARRAY, desc.Format)
		{
		}

		void TextureDesc2D::SetSRVDesc(uint32_t nMostDetailedMip, // FirstElement for BUFFER
			uint32_t nFirstArraySlice, // First2DArrayFace for TEXTURECUBEARRAY
			uint32_t nFlags) // BUFFEREX only)
		{
			SRVDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(ArraySize == 1 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DARRAY, Format, nMostDetailedMip, MipLevels, nFirstArraySlice, ArraySize, nFlags);
		}

		TextureDesc3D::TextureDesc3D()
			: CD3D11_TEXTURE3D_DESC(DXGI_FORMAT_R8G8B8A8_UINT, 0, 0, 0)
			, SRVDesc(D3D11_SRV_DIMENSION_TEXTURE3D, DXGI_FORMAT_R8G8B8A8_UINT)
		{
		}

		TextureDesc3D::TextureDesc3D(DXGI_FORMAT format,
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			uint32_t mipLevels,
			uint32_t bindFlags,
			D3D11_USAGE usage,
			uint32_t cpuaccessFlags,
			uint32_t miscFlags)
			: CD3D11_TEXTURE3D_DESC(format, width, height, depth, mipLevels, bindFlags, usage, cpuaccessFlags, miscFlags)
			, SRVDesc(D3D11_SRV_DIMENSION_TEXTURE3D, format)
		{
		}

		TextureDesc3D::TextureDesc3D(const CD3D11_TEXTURE3D_DESC& desc)
			: CD3D11_TEXTURE3D_DESC(desc)
			, SRVDesc(D3D11_SRV_DIMENSION_TEXTURE3D, desc.Format)
		{
		}

		void TextureDesc3D::SetSRVDesc(uint32_t nMostDetailedMip, uint32_t nFirstArraySlice, uint32_t nFlags)
		{
			SRVDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE3D, Format, nMostDetailedMip, MipLevels, nFirstArraySlice, 1, nFlags);
		}

		RenderTargetDesc1D::RenderTargetDesc1D()
			: TextureDesc1D()
		{
			init();
		}

		RenderTargetDesc1D::RenderTargetDesc1D(DXGI_FORMAT format,
			uint32_t width,
			uint32_t arraySize,
			uint32_t mipLevels,
			uint32_t bindFlags,
			D3D11_USAGE usage,
			uint32_t cpuaccessFlags,
			uint32_t miscFlags)
			: TextureDesc1D(format, width, arraySize, mipLevels, bindFlags, usage, cpuaccessFlags, miscFlags)
		{
		}

		RenderTargetDesc1D::RenderTargetDesc1D(const D3D11_TEXTURE1D_DESC& desc)
			: TextureDesc1D(desc)
		{
		}

		RenderTargetKey RenderTargetDesc1D::GetKey() const
		{
			String::StringID strKey;
			strKey.Format("1D_%u_%u_%u_%u_%u_%u_%u_%u",
				Width, MipLevels, ArraySize, Format, Usage, BindFlags, CPUAccessFlags, MiscFlags);

			return RenderTargetKey(strKey.Key());
		}

		RenderTargetDesc2D::RenderTargetDesc2D()
			: TextureDesc2D()
		{
			init();
		}

		RenderTargetDesc2D::RenderTargetDesc2D(DXGI_FORMAT format,
			uint32_t width,
			uint32_t height,
			uint32_t arraySize,
			uint32_t mipLevels,
			uint32_t bindFlags,
			D3D11_USAGE usage,
			uint32_t cpuaccessFlags,
			uint32_t sampleCount,
			uint32_t sampleQuality,
			uint32_t miscFlags)
			: TextureDesc2D(format, width, height, arraySize, mipLevels, bindFlags, usage, cpuaccessFlags, sampleCount, sampleQuality, miscFlags)
		{
			init();
		}

		RenderTargetDesc2D::RenderTargetDesc2D(const CD3D11_TEXTURE2D_DESC& desc)
			: TextureDesc2D(desc)
		{
			init();
		}

		RenderTargetKey RenderTargetDesc2D::GetKey() const
		{
			String::StringID strKey;
			strKey.Format("2D_%u_%u_%u_%u_%u_%u_%u_%u_%u_%u_%u",
				Width,
				Height,
				MipLevels,
				ArraySize,
				Format,
				SampleDesc.Count,
				SampleDesc.Quality,
				Usage,
				BindFlags,
				CPUAccessFlags,
				MiscFlags);

			return RenderTargetKey(strKey.Key());
		}

		DepthStencilDesc::DepthStencilDesc(DXGI_FORMAT format,
			uint32_t width,
			uint32_t height,
			uint32_t arraySize,
			uint32_t mipLevels,
			uint32_t bindFlags,
			D3D11_USAGE usage,
			uint32_t cpuaccessFlags,
			uint32_t sampleCount,
			uint32_t sampleQuality,
			uint32_t miscFlags)
			: TextureDesc2D(format, width, height, arraySize, mipLevels, bindFlags, usage, cpuaccessFlags, sampleCount, sampleQuality, miscFlags)
			, DSVDesc(arraySize == 1 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DARRAY, format)
		{
		}

		DepthStencilDesc::DepthStencilDesc(const CD3D11_TEXTURE2D_DESC& desc)
			: TextureDesc2D(desc)
			, DSVDesc(desc.ArraySize == 1 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DARRAY, desc.Format)
		{
		}
	}
}