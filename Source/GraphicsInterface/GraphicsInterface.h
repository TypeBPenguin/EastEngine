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
			using Key = PhantomType<tKey, const String::StringKey>;

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

		class ImageBasedLight
		{
		public:
			ImageBasedLight() = default;
			virtual ~ImageBasedLight();

		private:
			void SetTexture(ITexture** ppDest, ITexture* pSource)
			{
				if (*ppDest != nullptr)
				{
					(*ppDest)->DecreaseReference();
				}

				*ppDest = pSource;

				if (*ppDest != nullptr)
				{
					(*ppDest)->IncreaseReference();
				}
			}

		public:
			ITexture* GetEnvHDR() const { return m_pEnvHDR; }
			void SetEnvHDR(ITexture* pEnvHDR) { SetTexture(&m_pEnvHDR, pEnvHDR); }

			ITexture* GetDiffuseHDR() const { return m_pDiffuseHDR; }
			void SetDiffuseHDR(ITexture* pDiffuseHDR) { SetTexture(&m_pDiffuseHDR, pDiffuseHDR); }

			ITexture* GetSpecularHDR() const { return m_pSpecularHDR; }
			void SetSpecularHDR(ITexture* pSpecularHDR) { SetTexture(&m_pSpecularHDR, pSpecularHDR); }

			ITexture* GetSpecularBRDF() const { return m_pSpecularBRDF; }
			void SetSpecularBRDF(ITexture* pSpecularBRDF) { SetTexture(&m_pSpecularBRDF, pSpecularBRDF); }

		public:
			ITexture* m_pEnvHDR{ nullptr };
			ITexture* m_pDiffuseHDR{ nullptr };
			ITexture* m_pSpecularHDR{ nullptr };
			ITexture* m_pSpecularBRDF{ nullptr };
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
		std::uint64_t operator()(const eastengine::graphics::ITexture::Key& key) const
		{
			return key.value.value;
		}
	};
}