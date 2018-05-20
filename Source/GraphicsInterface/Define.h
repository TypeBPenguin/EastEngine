#pragma once

#include "CommonLib/PhantomType.h"
#include "Vertex.h"

namespace eastengine
{
	namespace graphics
	{
		enum APIs : uint8_t
		{
			eDX11 = 0,
			eDX12,
			eVulkan,
		};

		namespace EmGBuffer
		{
			enum Type : uint8_t
			{
				eNormals = 0,
				eColors,
				eDisneyBRDF,

				Count,
			};
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
				eOff = 0,			// ���� OFF
				eLinear,			// �����ռ�
				eAdditive,			// �����ռ�
				eSubTractive,		// �����ռ�
				eMultiplicative,	// �����ռ�
				eSquared,			// �����ռ�
				eNegative,			// �����ռ�
				eOpacity,			// �������ռ�
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

				// ���� �����Ϳ��� ȣȯ���� ����, ���Ű� �ƴ� ��������� ó��
				// ���߿� �����Ұ�
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

		namespace EmPrimitive
		{
			enum Type : uint8_t
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

		struct MaterialInfo
		{
			String::StringID strName;
			std::string strPath;

			math::Color colorAlbedo{ math::Color::White };
			math::Color colorEmissive{ math::Color::Transparent };

			math::Vector4 f4PaddingRoughMetEmi;
			math::Vector4 f4SurSpecTintAniso;
			math::Vector4 f4SheenTintClearcoatGloss;

			float fStippleTransparencyFactor{ 0.f };
			float fTessellationFactor{ 256.f };
			bool isAlbedoAlphaChannelMaskMap{ false };
			bool isVisible{ true };
			bool isIncludeTextureForder{ false };
			bool isAsyncTextureLoad{ true };

			std::array<String::StringID, EmMaterial::TypeCount> strTextureNameArray;

			EmSamplerState::Type emSamplerState{ EmSamplerState::eMinMagMipLinearWrap };
			EmBlendState::Type emBlendState{ EmBlendState::eOff };
			EmRasterizerState::Type emRasterizerState{ EmRasterizerState::eSolidCCW };
			EmDepthStencilState::Type emDepthStencilState{ EmDepthStencilState::eRead_Write_On };

			void Clear();
		};
	}
}