#pragma once

#include "Resource.h"
#include "Define.h"
#include "Light.h"
#include "Camera.h"

namespace sid
{
	RegisterStringID(VertexBuffer);
	RegisterStringID(IndexBuffer);
	RegisterStringID(Texture);
	RegisterStringID(Material);
}

namespace est
{
	namespace graphics
	{
		struct DisplayModeDesc
		{
			uint32_t width{ 0 };
			uint32_t height{ 0 };
			uint32_t refreshRate_numerator{ 0 };
			uint32_t refreshRate_denominator{ 0 };
		};

		struct TextureDesc
		{
			string::StringID name;

			uint32_t Width{ 0 };
			uint32_t Height{ 0 };

			ResourceFormat resourceFormat{ eRF_UNKNOWN };
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
			bool OnCollisionVisible{ false };

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
			bool OnSSR{ false };
			bool OnMotionBlur{ false };

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
				math::float3 colorGuide{ 1.f, 0.588f, 0.529f };
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
				float threshold{ 0.2f };
				float strengthMultiplier{ 1.f };
				bool isEnableLuminance{ false };
			};
			BloomFilterConfig bloomFilterConfig;

			struct SSSConfig
			{
				float width{ 1.f };
			};
			SSSConfig sssConfig;

			struct SSRConfig
			{
				int sampleCount{ 16 };
				float blurSigma{ 0.5f };
			};
			SSRConfig ssrConfig;

			struct MotionBlurConfig
			{
				enum Mode
				{
					eDepthBuffer_4Samples = 0,
					eDepthBuffer_8Samples,
					eDepthBuffer_12Samples,

					eVelocityBuffer_4Samples,
					eVelocityBuffer_8Samples,
					eVelocityBuffer_12Samples,

					eDualVelocityBuffer_4Samples,
					eDualVelocityBuffer_8Samples,
					eDualVelocityBuffer_12Samples,

					ModeCount,
				};

				Mode emMode{ eVelocityBuffer_4Samples };
				float blurAmount{ 1.f };

				bool IsDepthMotionBlur() const { return eDepthBuffer_4Samples <= emMode && emMode <= eDepthBuffer_12Samples; }
				bool IsVelocityMotionBlur() const { return eVelocityBuffer_4Samples <= emMode && emMode <= eDualVelocityBuffer_12Samples; }
			};
			MotionBlurConfig motionBlurConfig;
		};

		struct DebugInfo
		{
			bool isEnableCollection{ false };

			struct OcclusionCulling
			{
				std::atomic<uint32_t> renderTryCount{ 0 };
				std::atomic<uint32_t> renderCompleteCount{ 0 };
				std::atomic<uint32_t> visibleCount{ 0 };
				std::atomic<uint32_t> occludedCount{ 0 };
				std::atomic<uint32_t> viewCulledCount{ 0 };

				OcclusionCulling& operator = (const OcclusionCulling& source)
				{
					renderTryCount.store(source.renderTryCount);
					renderCompleteCount.store(source.renderCompleteCount);
					visibleCount.store(source.visibleCount);
					occludedCount.store(source.occludedCount);
					viewCulledCount.store(source.viewCulledCount);
					return *this;
				}
			};
			OcclusionCulling occlusionCulling;
		};

		class IVertexBuffer : public IResource
		{
			GraphicsResource(IVertexBuffer);
		protected:
			IVertexBuffer() = default;
			virtual ~IVertexBuffer() = default;

		public:
			virtual const string::StringID& GetResourceType() const override { return sid::VertexBuffer; }

		public:
			virtual uint32_t GetVertexCount() const = 0;
			virtual uint32_t GetFormatSize() const = 0;

			virtual bool Map(void** ppData) = 0;
			virtual void Unmap() = 0;
		};
		using VertexBufferPtr = std::shared_ptr<IVertexBuffer>;

		class IIndexBuffer : public IResource
		{
			GraphicsResource(IIndexBuffer);
		protected:
			IIndexBuffer() = default;
			virtual ~IIndexBuffer() = default;

		public:
			virtual const string::StringID& GetResourceType() const override { return sid::IndexBuffer; }

		public:
			virtual uint32_t GetIndexCount() const = 0;

			virtual bool Map(void** ppData) = 0;
			virtual void Unmap() = 0;
		};
		using IndexBufferPtr = std::shared_ptr<IIndexBuffer>;

		class ITexture : public IResource
		{
			GraphicsResource(ITexture);
		protected:
			ITexture() = default;
			virtual ~ITexture() = default;

		public:
			virtual const string::StringID& GetResourceType() const override { return sid::Texture; }

		private:
			struct tKey { string::StringID DefaultValue() { return L""; } };

		public:
			using Key = PhantomType<tKey, string::StringID>;

		public:
			virtual const Key& GetKey() const = 0;
			virtual const string::StringID& GetName() const = 0;

		public:
			virtual const math::uint2& GetSize() const = 0;
			virtual const std::wstring& GetPath() const = 0;
		};
		using TexturePtr = std::shared_ptr<ITexture>;

		class IMaterial : public IResource
		{
			GraphicsResource(IMaterial);
		protected:
			IMaterial() = default;
			virtual ~IMaterial() = default;

		public:
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

			struct Data
			{
				string::StringID name;
				std::wstring path;

				math::Color colorAlbedo{ math::Color::White };
				math::Color colorEmissive{ math::Color::Transparent };

				math::float4 paddingRoughMetEmi;
				math::float4 surSpecTintAniso;
				math::float4 sheenTintClearcoatGloss;

				float stippleTransparencyFactor{ 0.f };
				float tessellationFactor{ 256.f };
				bool isAlbedoAlphaChannelMaskMap{ false };
				bool isVisible{ true };
				bool isAsyncTextureLoad{ true };

				std::array<string::StringID, IMaterial::TypeCount> textureNameArray;

				SamplerState::Type samplerState{ SamplerState::eMinMagMipLinearWrap };
				BlendState::Type blendState{ BlendState::eOff };
				RasterizerState::Type rasterizerState{ RasterizerState::eSolidCCW };
				DepthStencilState::Type depthStencilState{ DepthStencilState::eRead_Write_On };
			};

		public:
			virtual const string::StringID& GetResourceType() const override { return sid::Material; }

		public:
			virtual bool SaveFile(const wchar_t* filePath) const = 0;

		public:
			virtual void LoadTexture() = 0;

			virtual const string::StringID& GetName() const = 0;
			virtual void SetName(const string::StringID& strName) = 0;

			virtual const std::wstring& GetPath() const = 0;
			virtual void SetPath(const std::wstring& strPath) = 0;

			virtual const math::Color& GetAlbedoColor() const = 0;
			virtual void SetAlbedoColor(const math::Color& color) = 0;

			virtual const math::Color& GetEmissiveColor() const = 0;
			virtual void SetEmissiveColor(const math::Color& color) = 0;

			virtual const string::StringID& GetTextureName(IMaterial::Type emType) const = 0;
			virtual void SetTextureName(IMaterial::Type emType, const string::StringID& strName) = 0;

			virtual TexturePtr GetTexture(IMaterial::Type emType) const = 0;
			virtual void SetTexture(IMaterial::Type emType, const TexturePtr& pTexture) = 0;

			virtual SamplerState::Type GetSamplerState() const = 0;
			virtual void SetSamplerState(SamplerState::Type samplerState) = 0;

			virtual BlendState::Type GetBlendState() const = 0;
			virtual void SetBlendState(BlendState::Type blendState) = 0;

			virtual RasterizerState::Type GetRasterizerState() const = 0;
			virtual void SetRasterizerState(RasterizerState::Type rasterizerState) = 0;

			virtual DepthStencilState::Type GetDepthStencilState() const = 0;
			virtual void SetDepthStencilState(DepthStencilState::Type depthStencilState) = 0;

			virtual const math::float4& GetPaddingRoughMetEmi() const = 0;
			virtual void SetPaddingRoughMetEmi(const math::float4& paddingRoughMetEmi) = 0;
			virtual const math::float4& GetSurSpecTintAniso() const = 0;
			virtual void SetSurSpecTintAniso(const math::float4& surSpecTintAniso) = 0;
			virtual const math::float4& GetSheenTintClearcoatGloss() const = 0;
			virtual void SetSheenTintClearcoatGloss(const math::float4& sheenTintClearcoatGloss) = 0;

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
			virtual void SetStippleTransparencyFactor(float stippleTransparencyFactor) = 0;

			virtual float GetTessellationFactor() const = 0;
			virtual void SetTessellationFactor(float tessellationFactor) = 0;

			virtual bool IsAlbedoAlphaChannelMaskMap() const = 0;
			virtual void SetAlbedoAlphaChannelMaskMap(bool isAlbedoAlphaChannelMaskMap) = 0;

			virtual bool IsVisible() const = 0;
			virtual void SetVisible(bool isVisible) = 0;

			virtual bool IsLoadComplete() const = 0;
		};
		using MaterialPtr = std::shared_ptr<IMaterial>;

		class IImageBasedLight
		{
		public:
			IImageBasedLight() = default;

		protected:
			virtual ~IImageBasedLight() = default;

		public:
			virtual TexturePtr GetEnvironmentHDR() const = 0;
			virtual void SetEnvironmentHDR(const TexturePtr& pEnvironmentHDR) = 0;

			virtual TexturePtr GetDiffuseHDR() const = 0;
			virtual void SetDiffuseHDR(const TexturePtr& pDiffuseHDR) = 0;

			virtual TexturePtr GetSpecularHDR() const = 0;
			virtual void SetSpecularHDR(const TexturePtr& pSpecularHDR) = 0;

			virtual TexturePtr GetSpecularBRDF() const = 0;
			virtual void SetSpecularBRDF(const TexturePtr& pSpecularBRDF) = 0;

			virtual VertexBufferPtr GetEnvironmentSphereVB() const = 0;
			virtual IndexBufferPtr GetEnvironmentSphereIB() const = 0;
			virtual void SetEnvironmentSphere(const VertexBufferPtr& pEnvironmentSphereVB, const IndexBufferPtr& pEnvironmentSphereIB) = 0;
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

		class ICursor
		{
		public:
			ICursor() = default;
			virtual ~ICursor() = default;

		public:
			virtual void Update(float elapsedTime) = 0;

		public:
			struct ImageData
			{
				const char* pData{ nullptr };
				uint32_t dataSize{ 0 };
			};
			virtual bool LoadCursor(const string::StringID& key, const std::vector<ImageData>& imageDatas, uint32_t hotSpotX, uint32_t hotSpotY, float playTime, bool isLoop) = 0;
			virtual bool IsHasCursor(const string::StringID& key) const = 0;
			virtual void ChangeCursor(const string::StringID& key) = 0;
			virtual void SetVisible(bool isVisible) = 0;
			virtual bool IsVisible() = 0;

		public:
			virtual void SetDefaultCursor(const string::StringID& key) = 0;
		};

		Options& GetOptions();
		Options& GetPrevOptions();
		DebugInfo& GetDebugInfo();
		DebugInfo& GetPrevDebugInfo();
		Camera& GetCamera();

		ICursor* GetCursor();
	}
}

namespace std
{
	template <>
	struct hash<est::graphics::ITexture::Key>
	{
		const size_t operator()(const est::graphics::ITexture::Key& key) const
		{
			return reinterpret_cast<size_t>(key.Value().Key());
		}
	};
}