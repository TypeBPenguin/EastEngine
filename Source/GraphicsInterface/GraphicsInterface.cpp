#include "stdafx.h"
#include "GraphicsInterface.h"

#include <dxgiformat.h>

namespace eastengine
{
	namespace graphics
	{
		static_assert(eRF_UNKNOWN == DXGI_FORMAT_UNKNOWN, "not equil dxgi format");
		static_assert(eRF_R32G32B32A32_TYPELESS == DXGI_FORMAT_R32G32B32A32_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R32G32B32A32_FLOAT == DXGI_FORMAT_R32G32B32A32_FLOAT, "not equil dxgi format");
		static_assert(eRF_R32G32B32A32_UINT == DXGI_FORMAT_R32G32B32A32_UINT, "not equil dxgi format");
		static_assert(eRF_R32G32B32A32_SINT == DXGI_FORMAT_R32G32B32A32_SINT, "not equil dxgi format");
		static_assert(eRF_R32G32B32_TYPELESS == DXGI_FORMAT_R32G32B32_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R32G32B32_FLOAT == DXGI_FORMAT_R32G32B32_FLOAT, "not equil dxgi format");
		static_assert(eRF_R32G32B32_UINT == DXGI_FORMAT_R32G32B32_UINT, "not equil dxgi format");
		static_assert(eRF_R32G32B32_SINT == DXGI_FORMAT_R32G32B32_SINT, "not equil dxgi format");
		static_assert(eRF_R16G16B16A16_TYPELESS == DXGI_FORMAT_R16G16B16A16_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R16G16B16A16_FLOAT == DXGI_FORMAT_R16G16B16A16_FLOAT, "not equil dxgi format");
		static_assert(eRF_R16G16B16A16_UNORM == DXGI_FORMAT_R16G16B16A16_UNORM, "not equil dxgi format");
		static_assert(eRF_R16G16B16A16_UINT == DXGI_FORMAT_R16G16B16A16_UINT, "not equil dxgi format");
		static_assert(eRF_R16G16B16A16_SNORM == DXGI_FORMAT_R16G16B16A16_SNORM, "not equil dxgi format");
		static_assert(eRF_R16G16B16A16_SINT == DXGI_FORMAT_R16G16B16A16_SINT, "not equil dxgi format");
		static_assert(eRF_R32G32_TYPELESS == DXGI_FORMAT_R32G32_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R32G32_FLOAT == DXGI_FORMAT_R32G32_FLOAT, "not equil dxgi format");
		static_assert(eRF_R32G32_UINT == DXGI_FORMAT_R32G32_UINT, "not equil dxgi format");
		static_assert(eRF_R32G32_SINT == DXGI_FORMAT_R32G32_SINT, "not equil dxgi format");
		static_assert(eRF_R32G8X24_TYPELESS == DXGI_FORMAT_R32G8X24_TYPELESS, "not equil dxgi format");
		static_assert(eRF_D32_FLOAT_S8X24_UINT == DXGI_FORMAT_D32_FLOAT_S8X24_UINT, "not equil dxgi format");
		static_assert(eRF_R32_FLOAT_X8X24_TYPELESS == DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, "not equil dxgi format");
		static_assert(eRF_X32_TYPELESS_G8X24_UINT == DXGI_FORMAT_X32_TYPELESS_G8X24_UINT, "not equil dxgi format");
		static_assert(eRF_R10G10B10A2_TYPELESS == DXGI_FORMAT_R10G10B10A2_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R10G10B10A2_UNORM == DXGI_FORMAT_R10G10B10A2_UNORM, "not equil dxgi format");
		static_assert(eRF_R10G10B10A2_UINT == DXGI_FORMAT_R10G10B10A2_UINT, "not equil dxgi format");
		static_assert(eRF_R11G11B10_FLOAT == DXGI_FORMAT_R11G11B10_FLOAT, "not equil dxgi format");
		static_assert(eRF_R8G8B8A8_TYPELESS == DXGI_FORMAT_R8G8B8A8_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R8G8B8A8_UNORM == DXGI_FORMAT_R8G8B8A8_UNORM, "not equil dxgi format");
		static_assert(eRF_R8G8B8A8_UNORM_SRGB == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, "not equil dxgi format");
		static_assert(eRF_R8G8B8A8_UINT == DXGI_FORMAT_R8G8B8A8_UINT, "not equil dxgi format");
		static_assert(eRF_R8G8B8A8_SNORM == DXGI_FORMAT_R8G8B8A8_SNORM, "not equil dxgi format");
		static_assert(eRF_R8G8B8A8_SINT == DXGI_FORMAT_R8G8B8A8_SINT, "not equil dxgi format");
		static_assert(eRF_R16G16_TYPELESS == DXGI_FORMAT_R16G16_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R16G16_FLOAT == DXGI_FORMAT_R16G16_FLOAT, "not equil dxgi format");
		static_assert(eRF_R16G16_UNORM == DXGI_FORMAT_R16G16_UNORM, "not equil dxgi format");
		static_assert(eRF_R16G16_UINT == DXGI_FORMAT_R16G16_UINT, "not equil dxgi format");
		static_assert(eRF_R16G16_SNORM == DXGI_FORMAT_R16G16_SNORM, "not equil dxgi format");
		static_assert(eRF_R16G16_SINT == DXGI_FORMAT_R16G16_SINT, "not equil dxgi format");
		static_assert(eRF_R32_TYPELESS == DXGI_FORMAT_R32_TYPELESS, "not equil dxgi format");
		static_assert(eRF_D32_FLOAT == DXGI_FORMAT_D32_FLOAT, "not equil dxgi format");
		static_assert(eRF_R32_FLOAT == DXGI_FORMAT_R32_FLOAT, "not equil dxgi format");
		static_assert(eRF_R32_UINT == DXGI_FORMAT_R32_UINT, "not equil dxgi format");
		static_assert(eRF_R32_SINT == DXGI_FORMAT_R32_SINT, "not equil dxgi format");
		static_assert(eRF_R24G8_TYPELESS == DXGI_FORMAT_R24G8_TYPELESS, "not equil dxgi format");
		static_assert(eRF_D24_UNORM_S8_UINT == DXGI_FORMAT_D24_UNORM_S8_UINT, "not equil dxgi format");
		static_assert(eRF_R24_UNORM_X8_TYPELESS == DXGI_FORMAT_R24_UNORM_X8_TYPELESS, "not equil dxgi format");
		static_assert(eRF_X24_TYPELESS_G8_UINT == DXGI_FORMAT_X24_TYPELESS_G8_UINT, "not equil dxgi format");
		static_assert(eRF_R8G8_TYPELESS == DXGI_FORMAT_R8G8_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R8G8_UNORM == DXGI_FORMAT_R8G8_UNORM, "not equil dxgi format");
		static_assert(eRF_R8G8_UINT == DXGI_FORMAT_R8G8_UINT, "not equil dxgi format");
		static_assert(eRF_R8G8_SNORM == DXGI_FORMAT_R8G8_SNORM, "not equil dxgi format");
		static_assert(eRF_R8G8_SINT == DXGI_FORMAT_R8G8_SINT, "not equil dxgi format");
		static_assert(eRF_R16_TYPELESS == DXGI_FORMAT_R16_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R16_FLOAT == DXGI_FORMAT_R16_FLOAT, "not equil dxgi format");
		static_assert(eRF_D16_UNORM == DXGI_FORMAT_D16_UNORM, "not equil dxgi format");
		static_assert(eRF_R16_UNORM == DXGI_FORMAT_R16_UNORM, "not equil dxgi format");
		static_assert(eRF_R16_UINT == DXGI_FORMAT_R16_UINT, "not equil dxgi format");
		static_assert(eRF_R16_SNORM == DXGI_FORMAT_R16_SNORM, "not equil dxgi format");
		static_assert(eRF_R16_SINT == DXGI_FORMAT_R16_SINT, "not equil dxgi format");
		static_assert(eRF_R8_TYPELESS == DXGI_FORMAT_R8_TYPELESS, "not equil dxgi format");
		static_assert(eRF_R8_UNORM == DXGI_FORMAT_R8_UNORM, "not equil dxgi format");
		static_assert(eRF_R8_UINT == DXGI_FORMAT_R8_UINT, "not equil dxgi format");
		static_assert(eRF_R8_SNORM == DXGI_FORMAT_R8_SNORM, "not equil dxgi format");
		static_assert(eRF_R8_SINT == DXGI_FORMAT_R8_SINT, "not equil dxgi format");
		static_assert(eRF_A8_UNORM == DXGI_FORMAT_A8_UNORM, "not equil dxgi format");
		static_assert(eRF_R1_UNORM == DXGI_FORMAT_R1_UNORM, "not equil dxgi format");
		static_assert(eRF_R9G9B9E5_SHAREDEXP == DXGI_FORMAT_R9G9B9E5_SHAREDEXP, "not equil dxgi format");
		static_assert(eRF_R8G8_B8G8_UNORM == DXGI_FORMAT_R8G8_B8G8_UNORM, "not equil dxgi format");
		static_assert(eRF_G8R8_G8B8_UNORM == DXGI_FORMAT_G8R8_G8B8_UNORM, "not equil dxgi format");
		static_assert(eRF_BC1_TYPELESS == DXGI_FORMAT_BC1_TYPELESS, "not equil dxgi format");
		static_assert(eRF_BC1_UNORM == DXGI_FORMAT_BC1_UNORM, "not equil dxgi format");
		static_assert(eRF_BC1_UNORM_SRGB == DXGI_FORMAT_BC1_UNORM_SRGB, "not equil dxgi format");
		static_assert(eRF_BC2_TYPELESS == DXGI_FORMAT_BC2_TYPELESS, "not equil dxgi format");
		static_assert(eRF_BC2_UNORM == DXGI_FORMAT_BC2_UNORM, "not equil dxgi format");
		static_assert(eRF_BC2_UNORM_SRGB == DXGI_FORMAT_BC2_UNORM_SRGB, "not equil dxgi format");
		static_assert(eRF_BC3_TYPELESS == DXGI_FORMAT_BC3_TYPELESS, "not equil dxgi format");
		static_assert(eRF_BC3_UNORM == DXGI_FORMAT_BC3_UNORM, "not equil dxgi format");
		static_assert(eRF_BC3_UNORM_SRGB == DXGI_FORMAT_BC3_UNORM_SRGB, "not equil dxgi format");
		static_assert(eRF_BC4_TYPELESS == DXGI_FORMAT_BC4_TYPELESS, "not equil dxgi format");
		static_assert(eRF_BC4_UNORM == DXGI_FORMAT_BC4_UNORM, "not equil dxgi format");
		static_assert(eRF_BC4_SNORM == DXGI_FORMAT_BC4_SNORM, "not equil dxgi format");
		static_assert(eRF_BC5_TYPELESS == DXGI_FORMAT_BC5_TYPELESS, "not equil dxgi format");
		static_assert(eRF_BC5_UNORM == DXGI_FORMAT_BC5_UNORM, "not equil dxgi format");
		static_assert(eRF_BC5_SNORM == DXGI_FORMAT_BC5_SNORM, "not equil dxgi format");
		static_assert(eRF_B5G6R5_UNORM == DXGI_FORMAT_B5G6R5_UNORM, "not equil dxgi format");
		static_assert(eRF_B5G5R5A1_UNORM == DXGI_FORMAT_B5G5R5A1_UNORM, "not equil dxgi format");
		static_assert(eRF_B8G8R8A8_UNORM == DXGI_FORMAT_B8G8R8A8_UNORM, "not equil dxgi format");
		static_assert(eRF_B8G8R8X8_UNORM == DXGI_FORMAT_B8G8R8X8_UNORM, "not equil dxgi format");
		static_assert(eRF_R10G10B10_XR_BIAS_A2_UNORM == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM, "not equil dxgi format");
		static_assert(eRF_B8G8R8A8_TYPELESS == DXGI_FORMAT_B8G8R8A8_TYPELESS, "not equil dxgi format");
		static_assert(eRF_B8G8R8A8_UNORM_SRGB == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, "not equil dxgi format");
		static_assert(eRF_B8G8R8X8_TYPELESS == DXGI_FORMAT_B8G8R8X8_TYPELESS, "not equil dxgi format");
		static_assert(eRF_B8G8R8X8_UNORM_SRGB == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB, "not equil dxgi format");
		static_assert(eRF_BC6H_TYPELESS == DXGI_FORMAT_BC6H_TYPELESS, "not equil dxgi format");
		static_assert(eRF_BC6H_UF16 == DXGI_FORMAT_BC6H_UF16, "not equil dxgi format");
		static_assert(eRF_BC6H_SF16 == DXGI_FORMAT_BC6H_SF16, "not equil dxgi format");
		static_assert(eRF_BC7_TYPELESS == DXGI_FORMAT_BC7_TYPELESS, "not equil dxgi format");
		static_assert(eRF_BC7_UNORM == DXGI_FORMAT_BC7_UNORM, "not equil dxgi format");
		static_assert(eRF_BC7_UNORM_SRGB == DXGI_FORMAT_BC7_UNORM_SRGB, "not equil dxgi format");
		static_assert(eRF_AYUV == DXGI_FORMAT_AYUV, "not equil dxgi format");
		static_assert(eRF_Y410 == DXGI_FORMAT_Y410, "not equil dxgi format");
		static_assert(eRF_Y416 == DXGI_FORMAT_Y416, "not equil dxgi format");
		static_assert(eRF_NV12 == DXGI_FORMAT_NV12, "not equil dxgi format");
		static_assert(eRF_P010 == DXGI_FORMAT_P010, "not equil dxgi format");
		static_assert(eRF_P016 == DXGI_FORMAT_P016, "not equil dxgi format");
		static_assert(eRF_420_OPAQUE == DXGI_FORMAT_420_OPAQUE, "not equil dxgi format");
		static_assert(eRF_YUY2 == DXGI_FORMAT_YUY2, "not equil dxgi format");
		static_assert(eRF_Y210 == DXGI_FORMAT_Y210, "not equil dxgi format");
		static_assert(eRF_Y216 == DXGI_FORMAT_Y216, "not equil dxgi format");
		static_assert(eRF_NV11 == DXGI_FORMAT_NV11, "not equil dxgi format");
		static_assert(eRF_AI44 == DXGI_FORMAT_AI44, "not equil dxgi format");
		static_assert(eRF_IA44 == DXGI_FORMAT_IA44, "not equil dxgi format");
		static_assert(eRF_P8 == DXGI_FORMAT_P8, "not equil dxgi format");
		static_assert(eRF_A8P8 == DXGI_FORMAT_A8P8, "not equil dxgi format");
		static_assert(eRF_B4G4R4A4_UNORM == DXGI_FORMAT_B4G4R4A4_UNORM, "not equil dxgi format");
		static_assert(eRF_P208 == DXGI_FORMAT_P208, "not equil dxgi format");
		static_assert(eRF_V208 == DXGI_FORMAT_V208, "not equil dxgi format");
		static_assert(eRF_V408 == DXGI_FORMAT_V408, "not equil dxgi format");
		static_assert(eRF_FORCE_UINT == DXGI_FORMAT_FORCE_UINT, "not equil dxgi format");

		static Options s_options;

		Options& GetOptions()
		{
			return s_options;
		}

		void SetOptions(const Options& options)
		{
			s_options = options;
		}
	}
}