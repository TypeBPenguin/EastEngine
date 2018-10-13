#pragma once

#include "Resource.h"
#include "Define.h"
#include "Light.h"

namespace StrID
{
	RegisterStringID(VertexBuffer);
	RegisterStringID(IndexBuffer);
	RegisterStringID(Texture);
	RegisterStringID(Material);
}

namespace eastengine
{
	namespace graphics
	{
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

		struct TextureDesc
		{
			String::StringID name;

			uint32_t Width;
			uint32_t Height;

			ResourceFormat resourceFormat;
			bool isDynamic{ false };

			struct SubResourceData
			{
				const void* pSysMem{ nullptr };
				uint32_t SysMemPitch{ 0 };
				uint32_t SysMemSlicePitch{ 0 };

				size_t MemSize{ 0 };
			};
			SubResourceData subResourceData;
		};

		struct Options
		{
			bool OnVSync{ false };
			bool OnOcclusionCulling{ false };

			bool OnShadow{ false };
			bool OnTessellation{ false };
			bool OnWireframe{ false };

			// PostProcessing
			bool OnHDR{ false };
			bool OnFXAA{ false };
			bool OnDOF{ false };
			bool OnASSAO{ false };
			bool OnColorGrading{ false };
			bool OnBloomFilter{ false };
			bool OnSSS{ false };

			struct HDRConfig
			{
				enum ToneMappingType
				{
					eNone = 0,
					eLogarithmic,
					eExponential,
					eDragoLogarithmic,
					eReinhard,
					eReinhardModified,
					eFilmicALU,
					eFilmicUncharted,
					eACES,

					NumToneMappingTypes,
				};

				enum AutoExposureType
				{
					eManual = 0,
					eGeometricMean = 1,
					eGeometricMeanAutoKey = 2,

					NumAutoExposureTypes,
				};

				float BloomThreshold{ 2.f };		// 0.f ~ 10.f
				float BloomMagnitude{ 0.f };		// 0.f ~ 2.f
				float BloomBlurSigma{ 0.8f };		// 0.5f ~ 1.5f
				float Tau{ 1.25f };					// 0.f ~ 4.f
				float Exposure{ 0.f };				// -10.f ~ 10.f
				float KeyValue{ 0.18f };			// 0.f ~ 1.f
				float WhiteLevel{ 5.f };			// 0.f ~ 25.f
				float ShoulderStrength{ 0.22f };	// 0.f ~ 2.f
				float LinearStrength{ 0.3f };		// 0.f ~ 5.f
				float LinearAngle{ 0.1f };			// 0.f ~ 1.f
				float ToeStrength{ 0.2f };			// 0.f ~ 2.f
				float ToeNumerator{ 0.01f };		// 0.f ~ 0.5f
				float ToeDenominator{ 0.3f };		// 0.f ~ 2.f
				float LinearWhite{ 11.2f };			// 0.f ~ 20.f
				float LuminanceSaturation{ 1.f };	// 0.f ~ 4.f
				float Bias{ 0.5f };					// 0.f ~ 1.f

				int LumMapMipLevel{ 10 };		// 0 ~ 10
				float TimeDelta{ 0.f };

				ToneMappingType emToneMappingType{ ToneMappingType::eNone };
				AutoExposureType emAutoExposureType{ AutoExposureType::eManual };
			};
			HDRConfig hdrConfig;

			struct DepthOfFieldConfig
			{
				float FocalDistnace{ 5.f };
				float FocalWidth{ 20.f };
			};
			DepthOfFieldConfig depthOfFieldConfig;

			struct AssaoConfig
			{
				float Radius{ 1.2f };								// [0.0,  ~ ] World (view) space size of the occlusion sphere.
				float ShadowMultiplier{ 1.f };						// [0.0, 5.0] Effect strength linear multiplier
				float ShadowPower{ 1.5f };							// [0.5, 5.0] Effect strength pow modifier
				float ShadowClamp{ 0.98f };							// [0.0, 1.0] Effect max limit (applied after multiplier but before blur)
				float HorizonAngleThreshold{ 0.06f };				// [0.0, 0.2] Limits self-shadowing (makes the sampling area less of a hemisphere, more of a spherical cone, to avoid self-shadowing and various artifacts due to low tessellation and depth buffer imprecision, etc.)
				float FadeOutFrom{ 50.f };							// [0.0,  ~ ] Distance to start start fading out the effect.
				float FadeOutTo{ 300.f };							// [0.0,  ~ ] Distance at which the effect is faded out.
				float AdaptiveQualityLimit{ 0.45f };				// [0.0, 1.0] (only for Quality Level 3)
				int QualityLevel{ 2 };								// [ -1,  3 ] Effect quality{}; -1 - lowest (low, half res checkerboard), 0 - low, 1 - medium, 2 - high, 3 - very high / adaptive{}; each quality level is roughly 2x more costly than the previous, except the q3 which is variable but, in general, above q2.
				int BlurPassCount{ 2 };								// [  0,   6] Number of edge-sensitive smart blur passes to apply. Quality 0 is an exception with only one 'dumb' blur pass used.
				float Sharpness{ 0.98f };							// [0.0, 1.0] (How much to bleed over edges{}; 1: not at all, 0.5: half-half{}; 0.0: completely ignore edges)
				float TemporalSupersamplingAngleOffset{ 0.f };		// [0.0,  PI] Used to rotate sampling kernel{}; If using temporal AA / supersampling, suggested to rotate by ( (frame%3)/3.0*PI ) or similar. Kernel is already symmetrical, which is why we use PI and not 2*PI.
				float TemporalSupersamplingRadiusOffset{ 1.f };		// [0.0, 2.0] Used to scale sampling kernel{}; If using temporal AA / supersampling, suggested to scale by ( 1.0f + (((frame%3)-1.0)/3.0)*0.1 ) or similar.
				float DetailShadowStrength{ 0.5f };					// [0.0, 5.0] Used for high-res detail AO using neighboring depth pixels: adds a lot of detail but also reduces temporal stability (adds aliasing).
			};
			AssaoConfig assaoConfig;

			struct ColorGradingConfig
			{
				math::Vector3 colorGuide{ 1.f, 0.588f, 0.529f };
			};
			ColorGradingConfig colorGradingConfig;

			struct BloomFilterConfig
			{
				enum Presets
				{
					eWide = 0,
					eFocussed,
					eSmall,
					eSuperWide,
					eCheap,
					eOne,

					PresetCount,
				};

				Presets emPreset{ Presets::eWide };
				float fThreshold{ 0.2f };
				float fStrengthMultiplier{ 1.f };
				bool isEnableLuminance{ false };
			};
			BloomFilterConfig bloomFilterConfig;

			struct SSSConfig
			{
				float fWidth{ 1.f };
			};
			SSSConfig sssConfig;
		};

		const Options& GetOptions();
		void SetOptions(const Options& options);

		class IVertexBuffer : public IResource
		{
		protected:
			IVertexBuffer() = default;
			virtual ~IVertexBuffer() = default;

		public:
			virtual const String::StringID& GetResourceType() const override { return StrID::VertexBuffer; }

		public:
			virtual uint32_t GetVertexCount() const = 0;
			virtual uint32_t GetFormatSize() const = 0;

			virtual bool Map(void** ppData) = 0;
			virtual void Unmap() = 0;
		};

		class IIndexBuffer : public IResource
		{
		protected:
			IIndexBuffer() = default;
			virtual ~IIndexBuffer() = default;

		public:
			virtual const String::StringID& GetResourceType() const override { return StrID::IndexBuffer; }

		public:
			virtual uint32_t GetIndexCount() const = 0;

			virtual bool Map(void** ppData) = 0;
			virtual void Unmap() = 0;
		};

		class ITexture : public IResource
		{
		protected:
			ITexture() = default;
			virtual ~ITexture() = default;

		public:
			virtual const String::StringID& GetResourceType() const override { return StrID::Texture; }

		private:
			struct tKey {};

		public:
			using Key = PhantomType<tKey, const String::StringID>;

		public:
			virtual const Key& GetKey() const = 0;
			virtual const String::StringID& GetName() const = 0;

		public:
			virtual const math::UInt2& GetSize() const = 0;
			virtual const std::string& GetPath() const = 0;
		};

		class IMaterial : public IResource
		{
		protected:
			IMaterial() = default;
			virtual ~IMaterial() = default;

		public:
			virtual const String::StringID& GetResourceType() const override { return StrID::Material; }

		public:
			virtual bool SaveToFile(const char* strFilePath) const = 0;

		public:
			virtual void LoadTexture() = 0;

			virtual const String::StringID& GetName() const = 0;
			virtual void SetName(const String::StringID& strName) = 0;

			virtual const std::string& GetPath() const = 0;
			virtual void SetPath(const std::string& strPath) = 0;

			virtual const math::Color& GetAlbedoColor() const = 0;
			virtual void SetAlbedoColor(const math::Color& color) = 0;

			virtual const math::Color& GetEmissiveColor() const = 0;
			virtual void SetEmissiveColor(const math::Color& color) = 0;

			virtual const String::StringID& GetTextureName(EmMaterial::Type emType) const = 0;
			virtual void SetTextureName(EmMaterial::Type emType, const String::StringID& strName) = 0;

			virtual ITexture* GetTexture(EmMaterial::Type emType) const = 0;
			virtual void SetTexture(EmMaterial::Type emType, ITexture* pTexture) = 0;

			virtual EmSamplerState::Type GetSamplerState() const = 0;
			virtual void SetSamplerState(EmSamplerState::Type emSamplerState) = 0;

			virtual EmBlendState::Type GetBlendState() const = 0;
			virtual void SetBlendState(EmBlendState::Type emBlendState) = 0;

			virtual EmRasterizerState::Type GetRasterizerState() const = 0;
			virtual void SetRasterizerState(EmRasterizerState::Type emRasterizerState) = 0;

			virtual EmDepthStencilState::Type GetDepthStencilState() const = 0;
			virtual void SetDepthStencilState(EmDepthStencilState::Type emDepthStencilState) = 0;

			virtual const math::Vector4& GetPaddingRoughMetEmi() const = 0;
			virtual void SetPaddingRoughMetEmi(const math::Vector4& f4PaddingRoughMetEmi) = 0;
			virtual const math::Vector4& GetSurSpecTintAniso() const = 0;
			virtual void SetSurSpecTintAniso(const math::Vector4& f4SurSpecTintAniso) = 0;
			virtual const math::Vector4& GetSheenTintClearcoatGloss() const = 0;
			virtual void SetSheenTintClearcoatGloss(const math::Vector4& f4SheenTintClearcoatGloss) = 0;

			virtual float GetDisplacement() const = 0;
			virtual void SetDisplacement(float fDisplacement) = 0;

			virtual float GetRoughness() const = 0;
			virtual void SetRoughness(float fRoughness) = 0;
			virtual float GetMetallic() const = 0;
			virtual void SetMetallic(float fMetallic) = 0;
			virtual float GetEmissive() const = 0;
			virtual void SetEmissive(float fEmissive) = 0;

			virtual float GetSubsurface() const = 0;
			virtual void SetSubsurface(float fSurface) = 0;
			virtual float GetSpecular() const = 0;
			virtual void SetSpecular(float fSpecular) = 0;
			virtual float GetSpecularTint() const = 0;
			virtual void SetSpecularTint(float fSpecularTint) = 0;
			virtual float GetAnisotropic() const = 0;
			virtual void SetAnisotropic(float fAnisotropic) = 0;

			virtual float GetSheen() const = 0;
			virtual void SetSheen(float fSheen) = 0;
			virtual float GetSheenTint() const = 0;
			virtual void SetSheenTint(float fSheenTint) = 0;
			virtual float GetClearcoat() const = 0;
			virtual void SetClearcoat(float fClearcoat) = 0;
			virtual float GetClearcoatGloss() const = 0;
			virtual void SetClearcoatGloss(float fClearcoatGloss) = 0;

			virtual float GetStippleTransparencyFactor() const = 0;
			virtual void SetStippleTransparencyFactor(float fStippleTransparencyFactor) = 0;

			virtual float GetTessellationFactor() const = 0;
			virtual void SetTessellationFactor(float fTessellationFactor) = 0;

			virtual bool IsAlbedoAlphaChannelMaskMap() const = 0;
			virtual void SetAlbedoAlphaChannelMaskMap(bool isAlbedoAlphaChannelMaskMap) = 0;

			virtual bool IsVisible() const = 0;
			virtual void SetVisible(bool isVisible) = 0;

			virtual bool IsLoadComplete() const = 0;
		};

		class IImageBasedLight
		{
		public:
			IImageBasedLight() = default;

		protected:
			virtual ~IImageBasedLight() = default;

		public:
			virtual ITexture* GetEnvironmentHDR() const = 0;
			virtual void SetEnvironmentHDR(ITexture* pEnvironmentHDR) = 0;

			virtual ITexture* GetDiffuseHDR() const = 0;
			virtual void SetDiffuseHDR(ITexture* pDiffuseHDR) = 0;

			virtual ITexture* GetSpecularHDR() const = 0;
			virtual void SetSpecularHDR(ITexture* pSpecularHDR) = 0;

			virtual ITexture* GetSpecularBRDF() const = 0;
			virtual void SetSpecularBRDF(ITexture* pSpecularBRDF) = 0;

			virtual IVertexBuffer* GetEnvironmentSphereVB() const = 0;
			virtual IIndexBuffer* GetEnvironmentSphereIB() const = 0;
			virtual void SetEnvironmentSphere(IVertexBuffer* pEnvironmentSphereVB, IIndexBuffer* pEnvironmentSphereIB) = 0;
		};

		class IVTFManager
		{
		public:
			IVTFManager() = default;
			virtual ~IVTFManager() = default;

		public:
			enum : uint32_t
			{
				eTextureWidth = 1024,
				eBufferCapacity = eTextureWidth * eTextureWidth / 4,
				eInvalidVTFID = std::numeric_limits<uint32_t>::max(),
			};

		public:
			virtual bool Allocate(uint32_t nMatrixCount, math::Matrix** ppDest_Out, uint32_t& nVTFID_Out) = 0;
		};
	}
}

namespace std
{
	template <>
	struct hash<eastengine::graphics::ITexture::Key>
	{
		const eastengine::String::StringData* operator()(const eastengine::graphics::ITexture::Key& key) const
		{
			return key.Value().Key();
		}
	};
}