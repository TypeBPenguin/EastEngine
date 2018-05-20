#pragma once

namespace eastengine
{
	namespace graphics
	{
		enum ThreadType
		{
			eUpdate = 0,
			eRender,

			ThreadCount,

			eImmediate = 3,
		};

		namespace EmGBuffer
		{
			enum Type
			{
				eNormals = 0,
				eColors,
				eDisneyBRDF,

				Count,
			};
		}

		//////////////////
		// SamplerState //
		//////////////////
		struct tSamplerStateKey {};
		using SamplerStateKey = PhantomType<tSamplerStateKey, String::StringKey>;

		struct SamplerStateDesc : public CD3D11_SAMPLER_DESC
		{
			SamplerStateDesc();
			SamplerStateDesc(const D3D11_SAMPLER_DESC& desc);

			SamplerStateKey GetKey() const;

			static const SamplerStateDesc& DefaultDesc()
			{
				static SamplerStateDesc defaultDesc;
				return defaultDesc;
			}
		};

		////////////////
		// BlendState //
		////////////////
		struct tBlendStateKeyT {};
		using BlendStateKey = PhantomType<tBlendStateKeyT, String::StringKey>;

		struct BlendStateDesc : public CD3D11_BLEND_DESC
		{
			BlendStateDesc();

			BlendStateKey GetKey() const;

			static const BlendStateDesc& DefaultDesc()
			{
				static BlendStateDesc defaultDesc;
				return defaultDesc;
			}
		};

		///////////////////////
		// DepthStencilState //
		///////////////////////
		struct tDepthStencilStateKey {};
		using DepthStencilStateKey = PhantomType<tDepthStencilStateKey, String::StringKey>;

		struct DepthStencilStateDesc : public CD3D11_DEPTH_STENCIL_DESC
		{
			DepthStencilStateDesc();

			DepthStencilStateKey GetKey() const;

			static const DepthStencilStateDesc& DefaultDesc()
			{
				static DepthStencilStateDesc defaultDesc;
				return defaultDesc;
			}
		};

		/////////////////////
		// RasterizerState //
		/////////////////////
		struct tRasterizerStateKeyT {};
		using RasterizerStateKey = PhantomType<tRasterizerStateKeyT, String::StringKey>;

		struct RasterizerStateDesc : public CD3D11_RASTERIZER_DESC
		{
			RasterizerStateDesc();

			RasterizerStateKey GetKey() const;

			static const RasterizerStateDesc& DefaultDesc()
			{
				static RasterizerStateDesc defaultDesc;
				return defaultDesc;
			}
		};

		/////////////
		// Texture //
		/////////////
		struct TextureDesc1D : public CD3D11_TEXTURE1D_DESC
		{
		private:
			CD3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;

		public:
			TextureDesc1D();
			TextureDesc1D(DXGI_FORMAT format,
				uint32_t width,
				uint32_t arraySize = 1,
				uint32_t mipLevels = 1,
				uint32_t bindFlags = D3D11_BIND_SHADER_RESOURCE,
				D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
				uint32_t cpuaccessFlags = 0,
				uint32_t miscFlags = 0);
			TextureDesc1D(const D3D11_TEXTURE1D_DESC& desc);

			virtual void Build() { SetSRVDesc(); }

			void SetSRVDesc(uint32_t nMostDetailedMip = 0, // FirstElement for BUFFER
				uint32_t nFirstArraySlice = 0, // First2DArrayFace for TEXTURECUBEARRAY
				uint32_t nFlags = 0);

			const CD3D11_SHADER_RESOURCE_VIEW_DESC* GetSRVDescPtr() const { return &SRVDesc; }
		};

		struct TextureDesc2D : public CD3D11_TEXTURE2D_DESC
		{
		private:
			CD3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;

		public:
			TextureDesc2D();
			TextureDesc2D(DXGI_FORMAT format,
				uint32_t width,
				uint32_t height,
				uint32_t arraySize = 1,
				uint32_t mipLevels = 1,
				uint32_t bindFlags = D3D11_BIND_SHADER_RESOURCE,
				D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
				uint32_t cpuaccessFlags = 0,
				uint32_t sampleCount = 1,
				uint32_t sampleQuality = 0,
				uint32_t miscFlags = 0);
			TextureDesc2D(const CD3D11_TEXTURE2D_DESC& desc);

			virtual void Build() { SetSRVDesc(); }

			void SetSRVDesc(uint32_t nMostDetailedMip = 0, // FirstElement for BUFFER
				uint32_t nFirstArraySlice = 0, // First2DArrayFace for TEXTURECUBEARRAY
				uint32_t nFlags = 0);

			void SetSRVFormat(DXGI_FORMAT format) { SRVDesc.Format = format; }

			const CD3D11_SHADER_RESOURCE_VIEW_DESC* GetSRVDescPtr() const { return &SRVDesc; }
		};

		struct TextureDesc3D : public CD3D11_TEXTURE3D_DESC
		{
		private:
			CD3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;

		public:
			TextureDesc3D();
			TextureDesc3D(DXGI_FORMAT format,
				uint32_t width,
				uint32_t height,
				uint32_t depth,
				uint32_t mipLevels = 1,
				uint32_t bindFlags = D3D11_BIND_SHADER_RESOURCE,
				D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
				uint32_t cpuaccessFlags = 0,
				uint32_t miscFlags = 0);
			TextureDesc3D(const CD3D11_TEXTURE3D_DESC& desc);

			virtual void Build() { SetSRVDesc(); }

			void SetSRVDesc(uint32_t nMostDetailedMip = 0, // FirstElement for BUFFER
				uint32_t nFirstArraySlice = 0, // First2DArrayFace for TEXTURECUBEARRAY
				uint32_t nFlags = 0);

			const CD3D11_SHADER_RESOURCE_VIEW_DESC* GetSRVDescPtr() const { return &SRVDesc; }
		};

		//////////////////
		// RenderTarget //
		//////////////////
		struct tRenderTargetKeyT {};
		using RenderTargetKey = PhantomType<tRenderTargetKeyT, String::StringKey>;

		struct RenderTargetDesc1D : public TextureDesc1D
		{
		private:
			std::vector<CD3D11_RENDER_TARGET_VIEW_DESC> vecRTVDesc;
			std::vector<CD3D11_UNORDERED_ACCESS_VIEW_DESC> vecUAVDesc;

			void init()
			{
				if (BindFlags & D3D11_BIND_RENDER_TARGET)
				{
					vecRTVDesc.resize(MipLevels);
					for (uint32_t i = 0; i < MipLevels; ++i)
					{
						vecRTVDesc[i] = CD3D11_RENDER_TARGET_VIEW_DESC(ArraySize == 1 ? D3D11_RTV_DIMENSION_TEXTURE1D : D3D11_RTV_DIMENSION_TEXTURE1DARRAY, Format, i);
					}
				}

				if (BindFlags & D3D11_BIND_UNORDERED_ACCESS)
				{
					vecUAVDesc.resize(MipLevels);
					for (uint32_t i = 0; i < MipLevels; ++i)
					{
						vecUAVDesc[i] = CD3D11_UNORDERED_ACCESS_VIEW_DESC(ArraySize == 1 ? D3D11_UAV_DIMENSION_TEXTURE1D : D3D11_UAV_DIMENSION_TEXTURE1DARRAY, Format, i);
					}
				}
			}

		public:
			RenderTargetDesc1D();
			RenderTargetDesc1D(DXGI_FORMAT format,
				uint32_t width,
				uint32_t arraySize = 1,
				uint32_t mipLevels = 1,
				uint32_t bindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
				D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
				uint32_t cpuaccessFlags = 0,
				uint32_t miscFlags = 0);
			RenderTargetDesc1D(const D3D11_TEXTURE1D_DESC& desc);

			RenderTargetKey GetKey() const;

			virtual void Build()
			{
				TextureDesc1D::Build();

				init();
			}

			void SetRTVDesc(uint32_t nTargetMipLevel,
				uint32_t nMipSlice = 0, // FirstElement for BUFFER
				uint32_t nFirstArraySlice = 0, // NumElements for BUFFER, FirstWSlice for TEXTURE3D
				uint32_t nArraySize = -1) // WSize for TEXTURE3D
			{
				vecRTVDesc[nTargetMipLevel] = CD3D11_RENDER_TARGET_VIEW_DESC(ArraySize == 1 ? D3D11_RTV_DIMENSION_TEXTURE1D : D3D11_RTV_DIMENSION_TEXTURE1DARRAY, Format, nMipSlice, nFirstArraySlice, nArraySize);
			}

			void SetUAVDesc(uint32_t nTargetMipLevel,
				uint32_t nMipSlice = 0, // FirstElement for BUFFER
				uint32_t nFirstArraySlice = 0, // NumElements for BUFFER, FirstWSlice for TEXTURE3D
				uint32_t nArraySize = -1, // WSize for TEXTURE3D
				uint32_t nFlags = 0) // BUFFER only
			{
				vecUAVDesc[nTargetMipLevel] = CD3D11_UNORDERED_ACCESS_VIEW_DESC(ArraySize == 1 ? D3D11_UAV_DIMENSION_TEXTURE1D : D3D11_UAV_DIMENSION_TEXTURE1DARRAY, Format, nMipSlice, nFirstArraySlice, nArraySize, nFlags);
			}

			const CD3D11_RENDER_TARGET_VIEW_DESC* GetRTVDescPtr(uint32_t nTargetMipLevel) { return &vecRTVDesc[nTargetMipLevel]; }
			const CD3D11_UNORDERED_ACCESS_VIEW_DESC* GetUAVDescPtr(uint32_t nTargetMipLevel) { return &vecUAVDesc[nTargetMipLevel]; }
		};

		struct RenderTargetDesc2D : public TextureDesc2D
		{
		private:
			std::vector<CD3D11_RENDER_TARGET_VIEW_DESC> vecRTVDesc;
			std::vector<CD3D11_UNORDERED_ACCESS_VIEW_DESC> vecUAVDesc;

			void init()
			{
				if (MipLevels > 1 && (D3D11_BIND_RENDER_TARGET & BindFlags) != 0)
				{
					MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
				}

				if (BindFlags & D3D11_BIND_RENDER_TARGET)
				{
					vecRTVDesc.clear();
					vecRTVDesc.resize(MipLevels);
					for (uint32_t i = 0; i < MipLevels; ++i)
					{
						SetRTVDesc(i);
					}
				}

				if (BindFlags & D3D11_BIND_UNORDERED_ACCESS)
				{
					vecUAVDesc.clear();
					vecUAVDesc.resize(MipLevels);
					for (uint32_t i = 0; i < MipLevels; ++i)
					{
						SetUAVDesc(i);
					}
				}
			}

		public:
			RenderTargetDesc2D();
			RenderTargetDesc2D(DXGI_FORMAT format,
				uint32_t width,
				uint32_t height,
				uint32_t arraySize = 1,
				uint32_t mipLevels = 1,
				uint32_t bindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
				D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
				uint32_t cpuaccessFlags = 0,
				uint32_t sampleCount = 1,
				uint32_t sampleQuality = 0,
				uint32_t miscFlags = 0);
			RenderTargetDesc2D(const CD3D11_TEXTURE2D_DESC& desc);

			RenderTargetKey GetKey() const;

			virtual void Build()
			{
				TextureDesc2D::Build();

				init();
			}

			void SetRTVDesc(uint32_t nTargetMipLevel,
				uint32_t nMipSlice = 0, // FirstElement for BUFFER
				uint32_t nFirstArraySlice = 0, // NumElements for BUFFER, FirstWSlice for TEXTURE3D
				uint32_t nArraySize = -1) // WSize for TEXTURE3D
			{
				vecRTVDesc[nTargetMipLevel] = CD3D11_RENDER_TARGET_VIEW_DESC(ArraySize == 1 ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DARRAY, Format, nMipSlice, nFirstArraySlice, nArraySize);
			}

			void SetUAVDesc(uint32_t nTargetMipLevel,
				uint32_t nMipSlice = 0, // FirstElement for BUFFER
				uint32_t nFirstArraySlice = 0, // NumElements for BUFFER, FirstWSlice for TEXTURE3D
				uint32_t nArraySize = -1, // WSize for TEXTURE3D
				uint32_t nFlags = 0) // BUFFER only
			{
				vecUAVDesc[nTargetMipLevel] = CD3D11_UNORDERED_ACCESS_VIEW_DESC(ArraySize == 1 ? D3D11_UAV_DIMENSION_TEXTURE2D : D3D11_UAV_DIMENSION_TEXTURE2DARRAY, Format, nMipSlice, nFirstArraySlice, nArraySize, nFlags);
			}

			const CD3D11_RENDER_TARGET_VIEW_DESC* GetRTVDescPtr(uint32_t nTargetMipLevel) { return &vecRTVDesc[nTargetMipLevel]; }
			const CD3D11_UNORDERED_ACCESS_VIEW_DESC* GetUAVDescPtr(uint32_t nTargetMipLevel) { return &vecUAVDesc[nTargetMipLevel]; }
		};

		//////////////////
		// DepthStencil //
		//////////////////
		struct DepthStencilDesc : public TextureDesc2D
		{
		private:
			CD3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;

		public:
			DepthStencilDesc(DXGI_FORMAT format,
				uint32_t width,
				uint32_t height,
				uint32_t arraySize = 1,
				uint32_t mipLevels = 1,
				uint32_t bindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,
				D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
				uint32_t cpuaccessFlags = 0,
				uint32_t sampleCount = 1,
				uint32_t sampleQuality = 0,
				uint32_t miscFlags = 0);
			DepthStencilDesc(const CD3D11_TEXTURE2D_DESC& desc);

			virtual void Build()
			{
				TextureDesc2D::Build();

				DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN;
				switch (Format)
				{
				case DXGI_FORMAT_R32_TYPELESS:
					DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
					srvFormat = DXGI_FORMAT_R32_FLOAT;
					break;
				case DXGI_FORMAT_D24_UNORM_S8_UINT:
					DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
					srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					break;
				case DXGI_FORMAT_R24G8_TYPELESS:
					DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
					srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					break;
				case DXGI_FORMAT_D32_FLOAT:
					Format = DXGI_FORMAT_R32_TYPELESS;
					DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
					srvFormat = DXGI_FORMAT_R32_FLOAT;
					break;
				case DXGI_FORMAT_R16_TYPELESS:
					srvFormat = DXGI_FORMAT_R16_UNORM;
					DSVDesc.Format = DXGI_FORMAT_D16_UNORM;
					break;
				case DXGI_FORMAT_R8_TYPELESS:
					srvFormat = DXGI_FORMAT_R8_UNORM;
					DSVDesc.Format = DXGI_FORMAT_R8_UNORM;
					break;
				}

				BindFlags |= D3D11_BIND_DEPTH_STENCIL;
				SetSRVFormat(srvFormat);
			}

			const CD3D11_DEPTH_STENCIL_VIEW_DESC* GetDSVDescPtr() const { return &DSVDesc; }
		};
	}
}

namespace std
{
	template <>
	struct hash<eastengine::graphics::SamplerStateKey>
	{
		std::uint64_t operator()(const eastengine::graphics::SamplerStateKey& key) const
		{
			return key.value.value;
		}
	};

	template <>
	struct hash<eastengine::graphics::BlendStateKey>
	{
		std::uint64_t operator()(const eastengine::graphics::BlendStateKey& key) const
		{
			return key.value.value;
		}
	};

	template <>
	struct hash<eastengine::graphics::DepthStencilStateKey>
	{
		std::uint64_t operator()(const eastengine::graphics::DepthStencilStateKey& key) const
		{
			return key.value.value;
		}
	};

	template <>
	struct hash<eastengine::graphics::RasterizerStateKey>
	{
		std::uint64_t operator()(const eastengine::graphics::RasterizerStateKey& key) const
		{
			return key.value.value;
		}
	};

	template <>
	struct hash<eastengine::graphics::RenderTargetKey>
	{
		std::uint64_t operator()(const eastengine::graphics::RenderTargetKey& key) const
		{
			return key.value.value;
		}
	};
}