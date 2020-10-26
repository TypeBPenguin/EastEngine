#pragma once

#include "CommonLib/PhantomType.h"
#include "Vertex.h"

namespace est
{
	namespace graphics
	{
		enum APIs : uint8_t
		{
			eDX11 = 0,
			eDX12,
			eVulkan,
		};

		enum ResourceFormat
		{
			eRF_UNKNOWN = 0,
			eRF_R32G32B32A32_TYPELESS = 1,
			eRF_R32G32B32A32_FLOAT = 2,
			eRF_R32G32B32A32_UINT = 3,
			eRF_R32G32B32A32_SINT = 4,
			eRF_R32G32B32_TYPELESS = 5,
			eRF_R32G32B32_FLOAT = 6,
			eRF_R32G32B32_UINT = 7,
			eRF_R32G32B32_SINT = 8,
			eRF_R16G16B16A16_TYPELESS = 9,
			eRF_R16G16B16A16_FLOAT = 10,
			eRF_R16G16B16A16_UNORM = 11,
			eRF_R16G16B16A16_UINT = 12,
			eRF_R16G16B16A16_SNORM = 13,
			eRF_R16G16B16A16_SINT = 14,
			eRF_R32G32_TYPELESS = 15,
			eRF_R32G32_FLOAT = 16,
			eRF_R32G32_UINT = 17,
			eRF_R32G32_SINT = 18,
			eRF_R32G8X24_TYPELESS = 19,
			eRF_D32_FLOAT_S8X24_UINT = 20,
			eRF_R32_FLOAT_X8X24_TYPELESS = 21,
			eRF_X32_TYPELESS_G8X24_UINT = 22,
			eRF_R10G10B10A2_TYPELESS = 23,
			eRF_R10G10B10A2_UNORM = 24,
			eRF_R10G10B10A2_UINT = 25,
			eRF_R11G11B10_FLOAT = 26,
			eRF_R8G8B8A8_TYPELESS = 27,
			eRF_R8G8B8A8_UNORM = 28,
			eRF_R8G8B8A8_UNORM_SRGB = 29,
			eRF_R8G8B8A8_UINT = 30,
			eRF_R8G8B8A8_SNORM = 31,
			eRF_R8G8B8A8_SINT = 32,
			eRF_R16G16_TYPELESS = 33,
			eRF_R16G16_FLOAT = 34,
			eRF_R16G16_UNORM = 35,
			eRF_R16G16_UINT = 36,
			eRF_R16G16_SNORM = 37,
			eRF_R16G16_SINT = 38,
			eRF_R32_TYPELESS = 39,
			eRF_D32_FLOAT = 40,
			eRF_R32_FLOAT = 41,
			eRF_R32_UINT = 42,
			eRF_R32_SINT = 43,
			eRF_R24G8_TYPELESS = 44,
			eRF_D24_UNORM_S8_UINT = 45,
			eRF_R24_UNORM_X8_TYPELESS = 46,
			eRF_X24_TYPELESS_G8_UINT = 47,
			eRF_R8G8_TYPELESS = 48,
			eRF_R8G8_UNORM = 49,
			eRF_R8G8_UINT = 50,
			eRF_R8G8_SNORM = 51,
			eRF_R8G8_SINT = 52,
			eRF_R16_TYPELESS = 53,
			eRF_R16_FLOAT = 54,
			eRF_D16_UNORM = 55,
			eRF_R16_UNORM = 56,
			eRF_R16_UINT = 57,
			eRF_R16_SNORM = 58,
			eRF_R16_SINT = 59,
			eRF_R8_TYPELESS = 60,
			eRF_R8_UNORM = 61,
			eRF_R8_UINT = 62,
			eRF_R8_SNORM = 63,
			eRF_R8_SINT = 64,
			eRF_A8_UNORM = 65,
			eRF_R1_UNORM = 66,
			eRF_R9G9B9E5_SHAREDEXP = 67,
			eRF_R8G8_B8G8_UNORM = 68,
			eRF_G8R8_G8B8_UNORM = 69,
			eRF_BC1_TYPELESS = 70,
			eRF_BC1_UNORM = 71,
			eRF_BC1_UNORM_SRGB = 72,
			eRF_BC2_TYPELESS = 73,
			eRF_BC2_UNORM = 74,
			eRF_BC2_UNORM_SRGB = 75,
			eRF_BC3_TYPELESS = 76,
			eRF_BC3_UNORM = 77,
			eRF_BC3_UNORM_SRGB = 78,
			eRF_BC4_TYPELESS = 79,
			eRF_BC4_UNORM = 80,
			eRF_BC4_SNORM = 81,
			eRF_BC5_TYPELESS = 82,
			eRF_BC5_UNORM = 83,
			eRF_BC5_SNORM = 84,
			eRF_B5G6R5_UNORM = 85,
			eRF_B5G5R5A1_UNORM = 86,
			eRF_B8G8R8A8_UNORM = 87,
			eRF_B8G8R8X8_UNORM = 88,
			eRF_R10G10B10_XR_BIAS_A2_UNORM = 89,
			eRF_B8G8R8A8_TYPELESS = 90,
			eRF_B8G8R8A8_UNORM_SRGB = 91,
			eRF_B8G8R8X8_TYPELESS = 92,
			eRF_B8G8R8X8_UNORM_SRGB = 93,
			eRF_BC6H_TYPELESS = 94,
			eRF_BC6H_UF16 = 95,
			eRF_BC6H_SF16 = 96,
			eRF_BC7_TYPELESS = 97,
			eRF_BC7_UNORM = 98,
			eRF_BC7_UNORM_SRGB = 99,
			eRF_AYUV = 100,
			eRF_Y410 = 101,
			eRF_Y416 = 102,
			eRF_NV12 = 103,
			eRF_P010 = 104,
			eRF_P016 = 105,
			eRF_420_OPAQUE = 106,
			eRF_YUY2 = 107,
			eRF_Y210 = 108,
			eRF_Y216 = 109,
			eRF_NV11 = 110,
			eRF_AI44 = 111,
			eRF_IA44 = 112,
			eRF_P8 = 113,
			eRF_A8P8 = 114,
			eRF_B4G4R4A4_UNORM = 115,
			eRF_P208 = 130,
			eRF_V208 = 131,
			eRF_V408 = 132,
			eRF_FORCE_UINT = 0xffffffff,
		};

		enum GBufferType : uint8_t
		{
			eNormals = 0,
			eColors,
			eDisneyBRDF,
			eVelocity,

			GBufferTypeCount,
		};

		constexpr ResourceFormat GBufferFormat(GBufferType emGBuffer)
		{
			switch (emGBuffer)
			{
			case GBufferType::eNormals:
				return eRF_R16G16B16A16_FLOAT;
			case GBufferType::eColors:
				return eRF_R32G32B32A32_FLOAT;
			case GBufferType::eDisneyBRDF:
				return eRF_R32G32B32A32_FLOAT;
			case GBufferType::eVelocity:
				return eRF_R16G16_FLOAT;
			default:
				return eRF_UNKNOWN;
			}
		}

		namespace EmRasterizerState
		{
			enum Type : uint8_t
			{
				eSolidCCW = 0,
				eSolidCW,
				eSolidCullNone,
				eWireframeCCW,
				eWireframeCW,
				eWireframeCullNone,

				TypeCount,
			};
		}

		namespace EmDepthStencilState
		{
			enum Type : uint8_t
			{
				eRead_Write_On = 0,
				eRead_Write_Off,
				eRead_On_Write_Off,
				eRead_Off_Write_On,

				TypeCount,
			};
		}

		namespace EmBlendState
		{
			enum Type : uint8_t
			{
				eOff = 0,			// 알파 OFF
				eLinear,			// 선형합성
				eAdditive,			// 가산합성
				eSubTractive,		// 감산합성
				eMultiplicative,	// 곱셈합성
				eSquared,			// 제곱합성
				eNegative,			// 반전합성
				eOpacity,			// 불투명합성
				eAlphaBlend,

				TypeCount,
			};
		};

		namespace EmSamplerState
		{
			enum Type : uint8_t
			{
				eMinMagMipLinearWrap = 0,
				eMinMagMipLinearClamp,
				eMinMagMipLinearBorder,
				eMinMagMipLinearMirror,
				eMinMagMipLinearMirrorOnce,
				eMinMagLinearMipPointWrap,
				eMinMagLinearMipPointClamp,
				eMinMagLinearMipPointBorder,
				eMinMagLinearMipPointMirror,
				eMinMagLinearMipPointMirrorOnce,
				eAnisotropicWrap,
				eAnisotropicClamp,
				eAnisotropicBorder,
				eAnisotropicMirror,
				eAnisotropicMirrorOnce,
				eMinMagMipPointWrap,
				eMinMagMipPointClamp,
				eMinMagMipPointBorder,
				eMinMagMipPointMirror,
				eMinMagMipPointMirrorOnce,

				TypeCount,
			};
		};

		namespace EmMaterial
		{
			enum Type : uint8_t
			{
				eAlbedo = 0,
				eMask,

				eNormal,

				// 기존 데이터와의 호환성을 위해, 제거가 아닌 빈공간으로 처리
				// 나중에 수정할것
				ePadding1,
				ePadding2,

				eRoughness,
				eMetallic,
				eEmissive,
				eEmissiveColor,

				eSubsurface,
				eSpecular,
				eSpecularTint,
				eAnisotropic,

				eSheen,
				eSheenTint,
				eClearcoat,
				eClearcoatGloss,

				TypeCount,
			};
		}

		enum Primitive : uint8_t
		{
			eTriangleList = 0,
			eTriangleStrip,
			eLineList,
			eLineStrip,
			ePointList,
			eTriangleListAdj,
			eTriangleStripAdj,
			eLineListAdj,
			eLineStripAdj,
			eQuadPatchList,
			eTrianglePatchList,
		};
	}
}