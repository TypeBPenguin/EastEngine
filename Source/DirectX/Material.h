#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;

		class Material : public IMaterial
		{
		public:
			Material();
			virtual ~Material();

			bool Init(const MaterialInfo* pInfo);
			bool Init(const String::StringID& strName);
			bool Init(const IMaterial* pSource);

		public:
			virtual void LoadTexture() override;

			virtual const String::StringID& GetName() const override { return m_info.strName; }
			virtual void SetName(const String::StringID& strName) override { m_info.strName = strName; }

			virtual const std::string& GetPath() const override { return m_info.strPath; }
			virtual void SetPath(const std::string& strPath) override { m_info.strPath = strPath; }

			virtual const Math::Color& GetAlbedoColor() const override { return m_info.colorAlbedo; }
			virtual void SetAlbedoColor(const Math::Color& color) override { m_info.colorAlbedo = color; }

			virtual const Math::Color& GetEmissiveColor() const override { return m_info.colorEmissive; }
			virtual void SetEmissiveColor(const Math::Color& color) override { m_info.colorEmissive = color; }

			virtual float GetTessellationFactor() const override { return m_info.fTessellationFactor; }
			virtual void SetTessellationFactor(float fTessellationFactor) override { m_info.fTessellationFactor = fTessellationFactor; }

			virtual const String::StringID& GetTextureName(EmMaterial::Type emType) const override { return m_info.strTextureNameArray[emType]; }
			virtual void SetTextureName(EmMaterial::Type emType, const String::StringID& strName) override { m_info.strTextureNameArray[emType] = strName; }

			virtual const std::shared_ptr<ITexture>& GetTexture(EmMaterial::Type emType) const override { return m_pTextureArray[emType]; }
			virtual void SetTexture(EmMaterial::Type emType, const std::shared_ptr<ITexture>& pTexture) override { m_pTextureArray[emType] = pTexture; }

			virtual EmSamplerState::Type GetSamplerState() const override { return m_info.emSamplerState; }
			virtual void SetSamplerState(EmSamplerState::Type emSamplerState) override { m_info.emSamplerState = emSamplerState; }

			virtual EmBlendState::Type GetBlendState() const override { return m_info.emBlendState; }
			virtual void SetBlendState(EmBlendState::Type emBlendState) override { m_info.emBlendState = emBlendState; }

			virtual EmRasterizerState::Type GetRasterizerState() const override { return m_info.emRasterizerState; }
			virtual void SetRasterizerState(EmRasterizerState::Type emRasterizerState) override { m_info.emRasterizerState = emRasterizerState; }

			virtual EmDepthStencilState::Type GetDepthStencilState() const override { return m_info.emDepthStencilState; }
			virtual void SetDepthStencilState(EmDepthStencilState::Type emDepthStencilState) override { m_info.emDepthStencilState = emDepthStencilState; }

			virtual const Math::Vector4& GetDisRoughMetEmi() const override { return m_info.f4DisRoughMetEmi; }
			virtual void SetDisRoughMetEmi(const Math::Vector4& f4DisRoughMetEmi) override { m_info.f4DisRoughMetEmi = f4DisRoughMetEmi; }
			virtual const Math::Vector4& GetSurSpecTintAniso() const override { return m_info.f4SurSpecTintAniso; }
			virtual void SetSurSpecTintAniso(const Math::Vector4& f4SurSpecTintAniso) override { m_info.f4SurSpecTintAniso = f4SurSpecTintAniso; }
			virtual const Math::Vector4& GetSheenTintClearcoatGloss() const override { return m_info.f4SheenTintClearcoatGloss; }
			virtual void SetSheenTintClearcoatGloss(const Math::Vector4& f4SheenTintClearcoatGloss) override { m_info.f4SheenTintClearcoatGloss = f4SheenTintClearcoatGloss; }

			virtual float GetDisplacement() const override { return m_info.f4DisRoughMetEmi.x; }
			virtual void SetDisplacement(float fDisplacement) override { m_info.f4DisRoughMetEmi.x = fDisplacement; }

			virtual float GetRoughness() const override { return m_info.f4DisRoughMetEmi.y; }
			virtual void SetRoughness(float fRoughness) override { m_info.f4DisRoughMetEmi.y = fRoughness; }
			virtual float GetMetallic() const override { return m_info.f4DisRoughMetEmi.z; }
			virtual void SetMetallic(float fMetallic) override { m_info.f4DisRoughMetEmi.z = fMetallic; }
			virtual float GetEmissive() const override { return m_info.f4DisRoughMetEmi.w; }
			virtual void SetEmissive(float fEmissive) override { m_info.f4DisRoughMetEmi.w = fEmissive; }

			virtual float GetSurface() const override { return m_info.f4SurSpecTintAniso.x; }
			virtual void SetSurface(float fSurface) override { m_info.f4SurSpecTintAniso.x = fSurface; }
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

			virtual bool IsAlbedoAlphaChannelMaskMap() const override { return m_info.isAlbedoAlphaChannelMaskMap; }
			virtual void SetAlbedoAlphaChannelMaskMap(bool isAlbedoAlphaChannelMaskMap) override { m_info.isAlbedoAlphaChannelMaskMap = isAlbedoAlphaChannelMaskMap; }

			virtual bool IsLoadComplete() const override;

		public:
			virtual int GetReferenceCount() const override { return m_nReferenceCount; }
			virtual int IncreaseReference() override
			{
				++m_nReferenceCount;
				return m_nReferenceCount;
			}
			virtual int DecreaseReference() override { return --m_nReferenceCount; }

		private:
			int m_nReferenceCount;

			MaterialInfo m_info;

			std::array<std::shared_ptr<ITexture>, EmMaterial::TypeCount> m_pTextureArray;
		};
	}
}