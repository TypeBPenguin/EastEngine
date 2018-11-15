#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class Material : public IMaterial
		{
		public:
			Material();
			virtual ~Material();

		public:
			static std::unique_ptr<Material> Create(const MaterialInfo* pInfo);
			static std::unique_ptr<Material> Create(const char* strFileName, const char* strFilePath);

			static std::unique_ptr<Material> Clone(const Material* pSource);

		public:
			virtual bool SaveToFile(const char* strFilePath) const override;

		public:
			virtual void LoadTexture() override;

			virtual const string::StringID& GetName() const override { return m_info.strName; }
			virtual void SetName(const string::StringID& strName) override { m_info.strName = strName; }

			virtual const std::string& GetPath() const override { return m_info.strPath; }
			virtual void SetPath(const std::string& strPath) override { m_info.strPath = strPath; }

			virtual const math::Color& GetAlbedoColor() const override { return m_info.colorAlbedo; }
			virtual void SetAlbedoColor(const math::Color& color) override { m_info.colorAlbedo = color; }

			virtual const math::Color& GetEmissiveColor() const override { return m_info.colorEmissive; }
			virtual void SetEmissiveColor(const math::Color& color) override { m_info.colorEmissive = color; }

			virtual const string::StringID& GetTextureName(EmMaterial::Type emType) const override { return m_info.strTextureNameArray[emType]; }
			virtual void SetTextureName(EmMaterial::Type emType, const string::StringID& strName) override { m_info.strTextureNameArray[emType] = strName; }

			virtual ITexture* GetTexture(EmMaterial::Type emType) const override { return m_pTextureArray[emType]; }
			virtual void SetTexture(EmMaterial::Type emType, ITexture* pTexture) override;

			virtual EmSamplerState::Type GetSamplerState() const override { return m_info.emSamplerState; }
			virtual void SetSamplerState(EmSamplerState::Type emSamplerState) override { m_info.emSamplerState = emSamplerState; }

			virtual EmBlendState::Type GetBlendState() const override { return m_info.emBlendState; }
			virtual void SetBlendState(EmBlendState::Type emBlendState) override { m_info.emBlendState = emBlendState; }

			virtual EmRasterizerState::Type GetRasterizerState() const override { return m_info.emRasterizerState; }
			virtual void SetRasterizerState(EmRasterizerState::Type emRasterizerState) override { m_info.emRasterizerState = emRasterizerState; }

			virtual EmDepthStencilState::Type GetDepthStencilState() const override { return m_info.emDepthStencilState; }
			virtual void SetDepthStencilState(EmDepthStencilState::Type emDepthStencilState) override { m_info.emDepthStencilState = emDepthStencilState; }

			virtual const math::float4& GetPaddingRoughMetEmi() const override { return m_info.f4PaddingRoughMetEmi; }
			virtual void SetPaddingRoughMetEmi(const math::float4& f4PaddingRoughMetEmi) override { m_info.f4PaddingRoughMetEmi = f4PaddingRoughMetEmi; }
			virtual const math::float4& GetSurSpecTintAniso() const override { return m_info.f4SurSpecTintAniso; }
			virtual void SetSurSpecTintAniso(const math::float4& f4SurSpecTintAniso) override { m_info.f4SurSpecTintAniso = f4SurSpecTintAniso; }
			virtual const math::float4& GetSheenTintClearcoatGloss() const override { return m_info.f4SheenTintClearcoatGloss; }
			virtual void SetSheenTintClearcoatGloss(const math::float4& f4SheenTintClearcoatGloss) override { m_info.f4SheenTintClearcoatGloss = f4SheenTintClearcoatGloss; }

			virtual float GetDisplacement() const override { return m_info.f4PaddingRoughMetEmi.x; }
			virtual void SetDisplacement(float fDisplacement) override { m_info.f4PaddingRoughMetEmi.x = fDisplacement; }

			virtual float GetRoughness() const override { return m_info.f4PaddingRoughMetEmi.y; }
			virtual void SetRoughness(float fRoughness) override { m_info.f4PaddingRoughMetEmi.y = fRoughness; }
			virtual float GetMetallic() const override { return m_info.f4PaddingRoughMetEmi.z; }
			virtual void SetMetallic(float fMetallic) override { m_info.f4PaddingRoughMetEmi.z = fMetallic; }
			virtual float GetEmissive() const override { return m_info.f4PaddingRoughMetEmi.w; }
			virtual void SetEmissive(float fEmissive) override { m_info.f4PaddingRoughMetEmi.w = fEmissive; }

			virtual float GetSubsurface() const override { return m_info.f4SurSpecTintAniso.x; }
			virtual void SetSubsurface(float fSurface) override { m_info.f4SurSpecTintAniso.x = fSurface; }
			virtual float GetSpecular() const override { return m_info.f4SurSpecTintAniso.y; }
			virtual void SetSpecular(float fSpecular) override { m_info.f4SurSpecTintAniso.y = fSpecular; }
			virtual float GetSpecularTint() const override { return m_info.f4SurSpecTintAniso.z; }
			virtual void SetSpecularTint(float fSpecularTint) override { m_info.f4SurSpecTintAniso.z = fSpecularTint; }
			virtual float GetAnisotropic() const override { return m_info.f4SurSpecTintAniso.w; }
			virtual void SetAnisotropic(float fAnisotropic) override { m_info.f4SurSpecTintAniso.w = fAnisotropic; }

			virtual float GetSheen() const override { return m_info.f4SheenTintClearcoatGloss.x; }
			virtual void SetSheen(float fSheen) override { m_info.f4SheenTintClearcoatGloss.x = fSheen; }
			virtual float GetSheenTint() const override { return m_info.f4SheenTintClearcoatGloss.y; }
			virtual void SetSheenTint(float fSheenTint) override { m_info.f4SheenTintClearcoatGloss.y = fSheenTint; }
			virtual float GetClearcoat() const override { return m_info.f4SheenTintClearcoatGloss.z; }
			virtual void SetClearcoat(float fClearcoat) override { m_info.f4SheenTintClearcoatGloss.z = fClearcoat; }
			virtual float GetClearcoatGloss() const override { return m_info.f4SheenTintClearcoatGloss.w; }
			virtual void SetClearcoatGloss(float fClearcoatGloss) override { m_info.f4SheenTintClearcoatGloss.w = fClearcoatGloss; }

			virtual float GetStippleTransparencyFactor() const override { return m_info.fStippleTransparencyFactor; }
			virtual void SetStippleTransparencyFactor(float fStippleTransparencyFactor) override { m_info.fStippleTransparencyFactor = fStippleTransparencyFactor; }

			virtual float GetTessellationFactor() const override { return m_info.fTessellationFactor; }
			virtual void SetTessellationFactor(float fTessellationFactor) override { m_info.fTessellationFactor = fTessellationFactor; }

			virtual bool IsAlbedoAlphaChannelMaskMap() const override { return m_info.isAlbedoAlphaChannelMaskMap; }
			virtual void SetAlbedoAlphaChannelMaskMap(bool isAlbedoAlphaChannelMaskMap) override { m_info.isAlbedoAlphaChannelMaskMap = isAlbedoAlphaChannelMaskMap; }

			virtual bool IsVisible() const override { return m_info.isVisible; }
			virtual void SetVisible(bool isVisible) override { m_info.isVisible = isVisible; }

			virtual bool IsLoadComplete() const override;

		private:
			MaterialInfo m_info;

			std::array<ITexture*, EmMaterial::TypeCount> m_pTextureArray{ nullptr };
		};
	}
}