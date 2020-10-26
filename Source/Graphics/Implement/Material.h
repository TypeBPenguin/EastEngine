#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		class Material : public IMaterial
		{
		public:
			Material();
			virtual ~Material();

		public:
			static std::shared_ptr<Material> Create(const IMaterial::Data* pMaterialData);
			static std::shared_ptr<Material> Create(const wchar_t* fileName, const wchar_t* filePath);

			static std::shared_ptr<Material> Clone(const Material* pSource);

		public:
			virtual bool SaveFile(const wchar_t* filePath) const override;

		public:
			virtual void LoadTexture() override;

			virtual const string::StringID& GetName() const override { return m_data.name; }
			virtual void SetName(const string::StringID& strName) override { m_data.name = strName; }

			virtual const std::wstring& GetPath() const override { return m_data.path; }
			virtual void SetPath(const std::wstring& path) override { m_data.path = path; }

			virtual const math::Color& GetAlbedoColor() const override { return m_data.colorAlbedo; }
			virtual void SetAlbedoColor(const math::Color& color) override { m_data.colorAlbedo = color; }

			virtual const math::Color& GetEmissiveColor() const override { return m_data.colorEmissive; }
			virtual void SetEmissiveColor(const math::Color& color) override { m_data.colorEmissive = color; }

			virtual const string::StringID& GetTextureName(IMaterial::Type emType) const override { return m_data.textureNameArray[emType]; }
			virtual void SetTextureName(IMaterial::Type emType, const string::StringID& strName) override { m_data.textureNameArray[emType] = strName; }

			virtual TexturePtr GetTexture(IMaterial::Type emType) const override { return m_pTextures[emType]; }
			virtual void SetTexture(IMaterial::Type emType, const TexturePtr& pTexture) override;

			virtual EmSamplerState::Type GetSamplerState() const override { return m_data.emSamplerState; }
			virtual void SetSamplerState(EmSamplerState::Type emSamplerState) override { m_data.emSamplerState = emSamplerState; }

			virtual EmBlendState::Type GetBlendState() const override { return m_data.emBlendState; }
			virtual void SetBlendState(EmBlendState::Type emBlendState) override { m_data.emBlendState = emBlendState; }

			virtual EmRasterizerState::Type GetRasterizerState() const override { return m_data.emRasterizerState; }
			virtual void SetRasterizerState(EmRasterizerState::Type emRasterizerState) override { m_data.emRasterizerState = emRasterizerState; }

			virtual EmDepthStencilState::Type GetDepthStencilState() const override { return m_data.emDepthStencilState; }
			virtual void SetDepthStencilState(EmDepthStencilState::Type emDepthStencilState) override { m_data.emDepthStencilState = emDepthStencilState; }

			virtual const math::float4& GetPaddingRoughMetEmi() const override { return m_data.f4PaddingRoughMetEmi; }
			virtual void SetPaddingRoughMetEmi(const math::float4& f4PaddingRoughMetEmi) override { m_data.f4PaddingRoughMetEmi = f4PaddingRoughMetEmi; }
			virtual const math::float4& GetSurSpecTintAniso() const override { return m_data.f4SurSpecTintAniso; }
			virtual void SetSurSpecTintAniso(const math::float4& f4SurSpecTintAniso) override { m_data.f4SurSpecTintAniso = f4SurSpecTintAniso; }
			virtual const math::float4& GetSheenTintClearcoatGloss() const override { return m_data.f4SheenTintClearcoatGloss; }
			virtual void SetSheenTintClearcoatGloss(const math::float4& f4SheenTintClearcoatGloss) override { m_data.f4SheenTintClearcoatGloss = f4SheenTintClearcoatGloss; }

			virtual float GetDisplacement() const override { return m_data.f4PaddingRoughMetEmi.x; }
			virtual void SetDisplacement(float fDisplacement) override { m_data.f4PaddingRoughMetEmi.x = fDisplacement; }

			virtual float GetRoughness() const override { return m_data.f4PaddingRoughMetEmi.y; }
			virtual void SetRoughness(float fRoughness) override { m_data.f4PaddingRoughMetEmi.y = fRoughness; }
			virtual float GetMetallic() const override { return m_data.f4PaddingRoughMetEmi.z; }
			virtual void SetMetallic(float fMetallic) override { m_data.f4PaddingRoughMetEmi.z = fMetallic; }
			virtual float GetEmissive() const override { return m_data.f4PaddingRoughMetEmi.w; }
			virtual void SetEmissive(float fEmissive) override { m_data.f4PaddingRoughMetEmi.w = fEmissive; }

			virtual float GetSubsurface() const override { return m_data.f4SurSpecTintAniso.x; }
			virtual void SetSubsurface(float fSurface) override { m_data.f4SurSpecTintAniso.x = fSurface; }
			virtual float GetSpecular() const override { return m_data.f4SurSpecTintAniso.y; }
			virtual void SetSpecular(float fSpecular) override { m_data.f4SurSpecTintAniso.y = fSpecular; }
			virtual float GetSpecularTint() const override { return m_data.f4SurSpecTintAniso.z; }
			virtual void SetSpecularTint(float fSpecularTint) override { m_data.f4SurSpecTintAniso.z = fSpecularTint; }
			virtual float GetAnisotropic() const override { return m_data.f4SurSpecTintAniso.w; }
			virtual void SetAnisotropic(float fAnisotropic) override { m_data.f4SurSpecTintAniso.w = fAnisotropic; }

			virtual float GetSheen() const override { return m_data.f4SheenTintClearcoatGloss.x; }
			virtual void SetSheen(float fSheen) override { m_data.f4SheenTintClearcoatGloss.x = fSheen; }
			virtual float GetSheenTint() const override { return m_data.f4SheenTintClearcoatGloss.y; }
			virtual void SetSheenTint(float fSheenTint) override { m_data.f4SheenTintClearcoatGloss.y = fSheenTint; }
			virtual float GetClearcoat() const override { return m_data.f4SheenTintClearcoatGloss.z; }
			virtual void SetClearcoat(float fClearcoat) override { m_data.f4SheenTintClearcoatGloss.z = fClearcoat; }
			virtual float GetClearcoatGloss() const override { return m_data.f4SheenTintClearcoatGloss.w; }
			virtual void SetClearcoatGloss(float fClearcoatGloss) override { m_data.f4SheenTintClearcoatGloss.w = fClearcoatGloss; }

			virtual float GetStippleTransparencyFactor() const override { return m_data.stippleTransparencyFactor; }
			virtual void SetStippleTransparencyFactor(float stippleTransparencyFactor) override { m_data.stippleTransparencyFactor = stippleTransparencyFactor; }

			virtual float GetTessellationFactor() const override { return m_data.tessellationFactor; }
			virtual void SetTessellationFactor(float tessellationFactor) override { m_data.tessellationFactor = tessellationFactor; }

			virtual bool IsAlbedoAlphaChannelMaskMap() const override { return m_data.isAlbedoAlphaChannelMaskMap; }
			virtual void SetAlbedoAlphaChannelMaskMap(bool isAlbedoAlphaChannelMaskMap) override { m_data.isAlbedoAlphaChannelMaskMap = isAlbedoAlphaChannelMaskMap; }

			virtual bool IsVisible() const override { return m_data.isVisible; }
			virtual void SetVisible(bool isVisible) override { m_data.isVisible = isVisible; }

			virtual bool IsLoadComplete() const override;

		private:
			Data m_data;

			std::array<TexturePtr, IMaterial::TypeCount> m_pTextures{ nullptr };
		};
	}
}