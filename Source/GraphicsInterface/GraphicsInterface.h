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
		struct Options
		{
			bool OnHDR{ false };
			bool OnFXAA{ false };
			bool OnDOF{ false };
			bool OnASSAO{ false };
			bool OnColorGrading{ false };
			bool OnBloomFilter{ false };
			bool OnSSS{ false };

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
			return key.value.Key();
		}
	};
}