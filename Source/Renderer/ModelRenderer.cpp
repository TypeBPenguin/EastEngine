#include "stdafx.h"
#include "ModelRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Performance.h"
#include "CommonLib/Config.h"

#include "DirectX/Camera.h"
#include "DirectX/LightMgr.h"
#include "DirectX/OcclusionCulling.h"
#include "DirectX/VTFManager.h"

namespace StrID
{
	RegisterStringID(ModelStatic);
	RegisterStringID(ModelSkinned);

	RegisterStringID(ModelStatic_Tessellation);
	RegisterStringID(ModelSkinned_Tessellation);

	RegisterStringID(g_Instances);
	RegisterStringID(g_matViewProj);
	RegisterStringID(g_matView);
	RegisterStringID(g_matViewCM);
	RegisterStringID(g_matProj);
	RegisterStringID(g_matWorld);
	RegisterStringID(g_texVTF);
	RegisterStringID(g_nVTFID);
	RegisterStringID(g_fFarClip);
	RegisterStringID(g_FrustumOrigin);
	RegisterStringID(g_FrustumNormals);
	RegisterStringID(g_fTessellationFactor);
	RegisterStringID(g_fStippleTransparencyFactor);

	RegisterStringID(g_f4AlbedoColor);
	RegisterStringID(g_f4EmissiveColor);
	RegisterStringID(g_f4PaddingRoughMetEmi);
	RegisterStringID(g_f4SurSpecTintAniso);
	RegisterStringID(g_f4SheenTintClearcoatGloss);
	RegisterStringID(g_texAlbedo);
	RegisterStringID(g_texMask);
	RegisterStringID(g_texNormalMap);
	RegisterStringID(g_texRoughness);
	RegisterStringID(g_texMetallic);
	RegisterStringID(g_texEmissive);
	RegisterStringID(g_texEmissiveColor);
	RegisterStringID(g_texSurface);
	RegisterStringID(g_texSpecular);
	RegisterStringID(g_texSpecularTint);
	RegisterStringID(g_texAnisotropic);
	RegisterStringID(g_texSheen);
	RegisterStringID(g_texSheenTint);
	RegisterStringID(g_texClearcoat);
	RegisterStringID(g_texClearcoatGloss);
	RegisterStringID(g_samplerState);

	RegisterStringID(g_f3CameraPos);

	RegisterStringID(g_texIBLMap);
	RegisterStringID(g_texDiffuseHDR);

	RegisterStringID(g_texSpecularHDR);
	RegisterStringID(g_texSpecularBRDF);

	RegisterStringID(g_lightDirectional);
	RegisterStringID(g_lightPoint);
	RegisterStringID(g_lightSpot);

	RegisterStringID(g_nDirectionalLightCount);
	RegisterStringID(g_nPointLightCount);
	RegisterStringID(g_nSpotLightCount);
}

namespace EastEngine
{
	namespace Graphics
	{
		namespace EmModelShader
		{
			enum Mask : uint64_t
			{
				eUseTexAlbedo = 0,
				eUseTexMask,
				eUseTexNormal,
				eUseTexRoughness,
				eUseTexMetallic,
				eUseTexEmissive,
				eUseTexEmissiveColor,
				eUseTexSubsurface,
				eUseTexSpecular,
				eUseTexSpecularTint,
				eUseTexAnisotropic,
				eUseTexSheen,
				eUseTexSheenTint,
				eUseTexClearcoat,
				eUseTexClearcoatGloss,
				eUseInstancing,
				eUseSkinning,
				eUseWriteDepth,
				eUseCubeMap,
				eUseTessellation,
				eUseAlbedoAlphaIsMaskMap,
				eUseAlphaBlending,

				MaskCount,
			};

			const char* GetMaskName(uint64_t nMask)
			{
				static std::string s_strMaskName[] =
				{
					"USE_TEX_ALBEDO",
					"USE_TEX_MASK",
					"USE_TEX_NORMAL",
					"USE_TEX_ROUGHNESS",
					"USE_TEX_METALLIC",
					"USE_TEX_EMISSIVE",
					"USE_TEX_EMISSIVECOLOR",
					"USE_TEX_SUBSURFACE",
					"USE_TEX_SPECULAR",
					"USE_TEX_SPECULARTINT",
					"USE_TEX_ANISOTROPIC",
					"USE_TEX_SHEEN",
					"USE_TEX_SHEENTINT",
					"USE_TEX_CLEARCOAT",
					"USE_TEX_CLEARCOATGLOSS",
					"USE_INSTANCING",
					"USE_SKINNING",
					"USE_WRITEDEPTH",
					"USE_CUBEMAP",
					"USE_TESSELLATION",
					"USE_ALBEDO_ALPHA_IS_MASK_MAP",
					"USE_ALPHABLENDING",
				};

				return s_strMaskName[nMask].c_str();
			}
		}

		IEffect* GetEffect(uint64_t nMask)
		{
			enum TechType
			{
				eModelStatic = 0,
				eModelSkinned,
				eModelStatic_Tessellation,
				eModelSkinned_Tessellation,
			};

			String::StringID strName;
			strName.Format("EffectModel_%lld", nMask);

			IEffect* pEffect = ShaderManager::GetInstance()->GetEffect(strName);
			if (pEffect != nullptr)
				return pEffect;

			ShaderMacros macros;
			for (int i = 0; i < EmModelShader::MaskCount; ++i)
			{
				if (GetBitMask64(nMask, i) == true)
				{
					macros.AddMacro(EmModelShader::GetMaskName(i), "1");
				}
			}
			macros.EndSet();

			TechType emTechType = eModelStatic;
			if (GetBitMask64(nMask, EmModelShader::eUseSkinning))
			{
				if (GetBitMask64(nMask, EmModelShader::eUseTessellation))
				{
					emTechType = eModelSkinned_Tessellation;
				}
				else
				{
					emTechType = eModelSkinned;
				}
			}
			else
			{
				if (GetBitMask64(nMask, EmModelShader::eUseTessellation))
				{
					emTechType = eModelStatic_Tessellation;
				}
			}

			static std::string strPath;
			if (strPath.empty() == true)
			{
				strPath.append(File::GetPath(File::EmPath::eFx));
				strPath.append("Model\\Model.fx");
			}

			pEffect = IEffect::CompileAsync(strName, strPath.c_str(), &macros, [emTechType](IEffect* pEffect, bool isSuccess)
			{
				if (isSuccess == true)
				{
					switch (emTechType)
					{
					case eModelStatic:
						pEffect->CreateTechnique(StrID::ModelStatic, EmVertexFormat::ePosTexNor);
						break;
					case eModelSkinned:
						pEffect->CreateTechnique(StrID::ModelSkinned, EmVertexFormat::ePosTexNorWeiIdx);
						break;
					case eModelStatic_Tessellation:
						pEffect->CreateTechnique(StrID::ModelStatic_Tessellation, EmVertexFormat::ePosTexNor);
						break;
					case eModelSkinned_Tessellation:
						pEffect->CreateTechnique(StrID::ModelSkinned_Tessellation, EmVertexFormat::ePosTexNorWeiIdx);
						break;
					}
				}
			});

			if (pEffect == nullptr)
				return nullptr;

			return pEffect;
		}

		ModelRenderer::ModelRenderer()
		{
			for (int i = 0; i < ThreadCount; ++i)
			{
				m_nStaticIndex[i].fill(0);
				m_nSkinnedIndex[i].fill(0);
			}
		}

		ModelRenderer::~ModelRenderer()
		{
		}

		bool ModelRenderer::Init(const Math::Viewport& viewport)
		{
			for (int i = 0; i < ThreadCount; ++i)
			{
				m_vecStaticSubsets[i][eDeferred].resize(512);
				m_vecStaticSubsets[i][eAlphaBlend].resize(512);
				m_vecSkinnedSubsets[i][eDeferred].resize(128);
				m_vecSkinnedSubsets[i][eAlphaBlend].resize(128);
			}

			return true;
		}

		enum EmRenderType
		{
			eNone = 0,
			eStatic = 1 << 0,
			eSkinned = 1 << 1,

			eInstancing = 1 << 3,
			eWriteDepth = 1 << 4,
			eCubeMap = 1 << 5,
			eAlphaBlend_Pre = 1 << 6,
			eAlphaBlend_Post = 1 << 7,
		};

		void ClearEffect(IDeviceContext* pDeviceContext, IEffect* pEffect, IEffectTech* pEffectTech)
		{
			pEffect->SetTexture(StrID::g_texAlbedo, nullptr);
			pEffect->SetTexture(StrID::g_texMask, nullptr);
			pEffect->SetTexture(StrID::g_texNormalMap, nullptr);
			pEffect->SetTexture(StrID::g_texRoughness, nullptr);
			pEffect->SetTexture(StrID::g_texMetallic, nullptr);
			pEffect->SetTexture(StrID::g_texEmissive, nullptr);
			pEffect->SetTexture(StrID::g_texEmissiveColor, nullptr);
			pEffect->SetTexture(StrID::g_texSurface, nullptr);
			pEffect->SetTexture(StrID::g_texSpecular, nullptr);
			pEffect->SetTexture(StrID::g_texSpecularTint, nullptr);
			pEffect->SetTexture(StrID::g_texAnisotropic, nullptr);
			pEffect->SetTexture(StrID::g_texSheen, nullptr);
			pEffect->SetTexture(StrID::g_texSheenTint, nullptr);
			pEffect->SetTexture(StrID::g_texClearcoat, nullptr);
			pEffect->SetTexture(StrID::g_texClearcoatGloss, nullptr);

			pEffect->SetStructuredBuffer(StrID::g_lightDirectional, nullptr);
			pEffect->SetStructuredBuffer(StrID::g_lightPoint, nullptr);
			pEffect->SetStructuredBuffer(StrID::g_lightSpot, nullptr);

			//pEffect->SetTexture(StrID::g_texIBLMap, nullptr);
			pEffect->SetTexture(StrID::g_texDiffuseHDR, nullptr);

			pEffect->SetTexture(StrID::g_texSpecularHDR, nullptr);
			pEffect->SetTexture(StrID::g_texSpecularBRDF, nullptr);

			pEffect->UndoSamplerState(StrID::g_samplerState, 0);

			pEffect->ClearState(pDeviceContext, pEffectTech);
		}

		void RenderModel(uint32_t nRenderType, IDevice* pDevice, IDeviceContext* pDeviceContext,
			const Math::Matrix* pMatViews, const Math::Matrix& matProj, const Math::Vector3& f3CameraPos,
			const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const IMaterial* pMaterial,
			uint32_t nIndexCount, uint32_t nStartIndex,
			const void* pInstanceData, uint32_t nVTFID = 0)
		{
			if (pVertexBuffer == nullptr || pIndexBuffer == nullptr)
				return;

			const bool isSkinnedModel = (nRenderType & EmRenderType::eSkinned) != 0;
			const bool isInstancing = (nRenderType & EmRenderType::eInstancing) != 0;
			const bool isWriteDepth = (nRenderType & EmRenderType::eWriteDepth) != 0;
			const bool isCubeMap = (nRenderType & EmRenderType::eCubeMap) != 0;
			const bool isAlphaBlend_Pre = (nRenderType & EmRenderType::eAlphaBlend_Pre) != 0;
			const bool isAlphaBlend_Post = (nRenderType & EmRenderType::eAlphaBlend_Post) != 0;

			int64_t nMask = 0;
			if (isInstancing == true)
			{
				SetBitMask64(nMask, EmModelShader::eUseInstancing);
			}

			if (isSkinnedModel == true)
			{
				SetBitMask64(nMask, EmModelShader::eUseSkinning);
			}

			if (isWriteDepth == true)
			{
				SetBitMask64(nMask, EmModelShader::eUseWriteDepth);
			}

			if (isCubeMap == true)
			{
				SetBitMask64(nMask, EmModelShader::eUseCubeMap);
			}

			if (isAlphaBlend_Pre == true || isAlphaBlend_Post == true)
			{
				SetBitMask64(nMask, EmModelShader::eUseAlphaBlending);
			}

			bool isEnableTessellation = false;

			if (pMaterial != nullptr && isWriteDepth == false)
			{
				auto IsValidTexture = [&](EmMaterial::Type emType) -> bool
				{
					const std::shared_ptr<ITexture>& pTexture = pMaterial->GetTexture(emType);
					if (pTexture == nullptr)
						return false;

					return pTexture->GetLoadState() == EmLoadState::eComplete;
				};

				SetBitMask64(nMask, IsValidTexture(EmMaterial::eAlbedo) == true ? EmModelShader::eUseTexAlbedo : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eMask) == true ? EmModelShader::eUseTexMask : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eNormal) == true ? EmModelShader::eUseTexNormal : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eRoughness) == true ? EmModelShader::eUseTexRoughness : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eMetallic) == true ? EmModelShader::eUseTexMetallic : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eEmissive) == true ? EmModelShader::eUseTexEmissive : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eEmissiveColor) == true ? EmModelShader::eUseTexEmissiveColor : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSubsurface) == true ? EmModelShader::eUseTexSubsurface : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSpecular) == true ? EmModelShader::eUseTexSpecular : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSpecularTint) == true ? EmModelShader::eUseTexSpecularTint : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eAnisotropic) == true ? EmModelShader::eUseTexAnisotropic : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSheen) == true ? EmModelShader::eUseTexSheen : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSheenTint) == true ? EmModelShader::eUseTexSheenTint : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eClearcoat) == true ? EmModelShader::eUseTexClearcoat : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eClearcoatGloss) == true ? EmModelShader::eUseTexClearcoatGloss : -1);

				if (isAlphaBlend_Post == true)
				{
					if (pMaterial->GetDepthStencilState() == EmDepthStencilState::eRead_Write_Off ||
						pMaterial->GetDepthStencilState() == EmDepthStencilState::eRead_On_Write_Off)
						return;

					SetBitMask64(nMask, EmModelShader::eUseAlbedoAlphaIsMaskMap);
				}
				else
				{
					SetBitMask64(nMask, pMaterial->IsAlbedoAlphaChannelMaskMap() == true ? EmModelShader::eUseAlbedoAlphaIsMaskMap : -1);
				}

				if (Config::IsEnable("Tessellation"_s) == true && Math::IsZero(pMaterial->GetTessellationFactor() - 1.f) == false)
				{
					SetBitMask64(nMask, EmModelShader::eUseTessellation);
					isEnableTessellation = true;
				}
			}
			else
			{
				if (Config::IsEnable("Tessellation"_s) == true)
				{
					SetBitMask64(nMask, EmModelShader::eUseTessellation);
					isEnableTessellation = true;
				}
			}

			IEffect* pEffect = GetEffect(nMask);
			if (pEffect == nullptr || pEffect->IsValid() == false)
			{
				int64_t nDefaultMask = 0;
				SetBitMask64(nDefaultMask, isInstancing == true ? EmModelShader::eUseInstancing : -1);
				SetBitMask64(nDefaultMask, isSkinnedModel == true ? EmModelShader::eUseSkinning : -1);
				SetBitMask64(nDefaultMask, isWriteDepth == true ? EmModelShader::eUseWriteDepth : -1);
				SetBitMask64(nDefaultMask, isCubeMap == true ? EmModelShader::eUseCubeMap : -1);
				SetBitMask64(nDefaultMask, isEnableTessellation == true ? EmModelShader::eUseTessellation : -1);
				SetBitMask64(nDefaultMask, (isAlphaBlend_Pre == true || isAlphaBlend_Post == true) ? EmModelShader::eUseAlphaBlending : -1);

				pEffect = GetEffect(nDefaultMask);
				if (pEffect == nullptr || pEffect->IsValid() == false)
					return;
			}

			String::StringID strTechName = StrID::ModelStatic;
			if (isSkinnedModel == true)
			{
				strTechName = isEnableTessellation == true ? StrID::ModelSkinned_Tessellation : StrID::ModelSkinned;
			}
			else
			{
				strTechName = isEnableTessellation == true ? StrID::ModelStatic_Tessellation : StrID::ModelStatic;
			}

			IEffectTech* pEffectTech = pEffect->GetTechnique(strTechName);
			if (pEffectTech == nullptr)
				return;

			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			if (isAlphaBlend_Pre == true || isAlphaBlend_Post == true)
			{
				IImageBasedLight* pIBL = GetImageBasedLight();

				pEffect->SetVector(StrID::g_f3CameraPos, f3CameraPos);

				pEffect->SetTexture(StrID::g_texDiffuseHDR, pIBL->GetDiffuseHDR());

				pEffect->SetTexture(StrID::g_texSpecularHDR, pIBL->GetSpecularHDR());
				pEffect->SetTexture(StrID::g_texSpecularBRDF, pIBL->GetSpecularBRDF());

				pEffect->SetStructuredBuffer(StrID::g_lightDirectional, LightManager::GetInstance()->GetLightBuffer(EmLight::eDirectional));
				pEffect->SetStructuredBuffer(StrID::g_lightPoint, LightManager::GetInstance()->GetLightBuffer(EmLight::ePoint));
				pEffect->SetStructuredBuffer(StrID::g_lightSpot, LightManager::GetInstance()->GetLightBuffer(EmLight::eSpot));

				pEffect->SetInt(StrID::g_nDirectionalLightCount, LightManager::GetInstance()->GetLightCountInView(EmLight::eDirectional));
				pEffect->SetInt(StrID::g_nPointLightCount, LightManager::GetInstance()->GetLightCountInView(EmLight::ePoint));
				pEffect->SetInt(StrID::g_nSpotLightCount, LightManager::GetInstance()->GetLightCountInView(EmLight::eSpot));
			}

			if (isEnableTessellation == true)
			{
				pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

				Math::Vector4 f4FrustumNormals[4];
				{
					float clipNear = -matProj._43 / matProj._33;
					float clipFar = clipNear * matProj._33 / (matProj._33 - 1.f);

					Math::Vector3 f3CameraFrustum[4];

					Math::Vector3 f3CenterFar = f3CameraPos + pMatViews->Forward() * clipFar;
					Math::Vector3 f3OffsetH = (clipFar / matProj._11) * pMatViews->Right();
					Math::Vector3 f3OffsetV = (clipFar / matProj._22) * pMatViews->Up();
					f3CameraFrustum[0] = f3CenterFar - f3OffsetV - f3OffsetH;
					f3CameraFrustum[1] = f3CenterFar + f3OffsetV - f3OffsetH;
					f3CameraFrustum[2] = f3CenterFar + f3OffsetV + f3OffsetH;
					f3CameraFrustum[3] = f3CenterFar - f3OffsetV + f3OffsetH;

					// left/top planes normals
					Math::Vector3 f3Normal;
					Math::Vector3 f3Temp = f3CameraFrustum[1] - f3CameraPos;
					f3Normal = f3Temp.Cross(pMatViews->Up());
					f3Normal.Normalize();
					f4FrustumNormals[0] = Math::Vector4(f3Normal.x, f3Normal.y, f3Normal.z, 0.f);

					f3Normal = f3Temp.Cross(pMatViews->Right());
					f3Normal.Normalize();
					f4FrustumNormals[1] = Math::Vector4(f3Normal.x, f3Normal.y, f3Normal.z, 0.f);

					// right/bottom planes normals
					f3Temp = f3CameraFrustum[3] - f3CameraPos;
					f3Normal = pMatViews->Up().Cross(f3Temp);
					f3Normal.Normalize();
					f4FrustumNormals[2] = Math::Vector4(f3Normal.x, f3Normal.y, f3Normal.z, 0.f);

					f3Normal = pMatViews->Right().Cross(f3Temp);
					f3Normal.Normalize();
					f4FrustumNormals[2] = Math::Vector4(f3Normal.x, f3Normal.y, f3Normal.z, 0.f);
				}
				pEffect->SetVector(StrID::g_FrustumOrigin, f3CameraPos);
				pEffect->SetVectorArray(StrID::g_FrustumNormals, f4FrustumNormals, 0, 4);

				if (pMaterial != nullptr)
				{
					const float fTessellationFactor = pMaterial->GetTessellationFactor();
					pEffect->SetFloat(StrID::g_fTessellationFactor, fTessellationFactor);
				}
				else
				{
					const float fTessellationFactor = 256.f;
					pEffect->SetFloat(StrID::g_fTessellationFactor, fTessellationFactor);
				}
			}
			else
			{
				pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}

			pDeviceContext->SetVertexBuffers(pVertexBuffer, pVertexBuffer->GetFormatSize(), 0);
			pDeviceContext->SetIndexBuffer(pIndexBuffer, 0);

			if (isSkinnedModel == true)
			{
				pEffect->SetTexture(StrID::g_texVTF, VTFManager::GetInstance()->GetTexture());
			}

			if (isCubeMap == true)
			{
				pEffect->SetMatrixArray(StrID::g_matViewCM, pMatViews, 0, 6);
			}
			else
			{
				pEffect->SetMatrix(StrID::g_matViewProj, *pMatViews * matProj);
				pEffect->SetMatrix(StrID::g_matView, *pMatViews);
			}
			pEffect->SetMatrix(StrID::g_matProj, matProj);

			if (isWriteDepth == false)
			{
				if (pMaterial != nullptr)
				{
					pEffect->SetVector(StrID::g_f4AlbedoColor, reinterpret_cast<const Math::Vector4&>(pMaterial->GetAlbedoColor()));
					pEffect->SetVector(StrID::g_f4EmissiveColor, reinterpret_cast<const Math::Vector4&>(pMaterial->GetEmissiveColor()));

					pEffect->SetVector(StrID::g_f4PaddingRoughMetEmi, pMaterial->GetPaddingRoughMetEmi());
					pEffect->SetVector(StrID::g_f4SurSpecTintAniso, pMaterial->GetSurSpecTintAniso());
					pEffect->SetVector(StrID::g_f4SheenTintClearcoatGloss, pMaterial->GetSheenTintClearcoatGloss());

					pEffect->SetTexture(StrID::g_texAlbedo, pMaterial->GetTexture(EmMaterial::eAlbedo));
					pEffect->SetTexture(StrID::g_texMask, pMaterial->GetTexture(EmMaterial::eMask));
					pEffect->SetTexture(StrID::g_texNormalMap, pMaterial->GetTexture(EmMaterial::eNormal));
					pEffect->SetTexture(StrID::g_texRoughness, pMaterial->GetTexture(EmMaterial::eRoughness));
					pEffect->SetTexture(StrID::g_texMetallic, pMaterial->GetTexture(EmMaterial::eMetallic));
					pEffect->SetTexture(StrID::g_texEmissive, pMaterial->GetTexture(EmMaterial::eEmissive));
					pEffect->SetTexture(StrID::g_texEmissiveColor, pMaterial->GetTexture(EmMaterial::eEmissiveColor));
					pEffect->SetTexture(StrID::g_texSurface, pMaterial->GetTexture(EmMaterial::eSubsurface));
					pEffect->SetTexture(StrID::g_texSpecular, pMaterial->GetTexture(EmMaterial::eSpecular));
					pEffect->SetTexture(StrID::g_texSpecularTint, pMaterial->GetTexture(EmMaterial::eSpecularTint));
					pEffect->SetTexture(StrID::g_texAnisotropic, pMaterial->GetTexture(EmMaterial::eAnisotropic));
					pEffect->SetTexture(StrID::g_texSheen, pMaterial->GetTexture(EmMaterial::eSheen));
					pEffect->SetTexture(StrID::g_texSheenTint, pMaterial->GetTexture(EmMaterial::eSheenTint));
					pEffect->SetTexture(StrID::g_texClearcoat, pMaterial->GetTexture(EmMaterial::eClearcoat));
					pEffect->SetTexture(StrID::g_texClearcoatGloss, pMaterial->GetTexture(EmMaterial::eClearcoatGloss));

					pEffect->SetFloat(StrID::g_fStippleTransparencyFactor, pMaterial->GetStippleTransparencyFactor());

					ISamplerState* pSamplerState = pDevice->GetSamplerState(pMaterial->GetSamplerState());
					pEffect->SetSamplerState(StrID::g_samplerState, pSamplerState, 0);

					pDeviceContext->SetBlendState(pMaterial->GetBlendState());

					//if (isForward == false)
					{
						if (isWriteDepth == false)
						{
							EmRasterizerState::Type emRasterizerState = pMaterial->GetRasterizerState();
							EmDepthStencilState::Type emDepthStencilState = pMaterial->GetDepthStencilState();

							if (isAlphaBlend_Pre == true)
							{
								if (pMaterial->GetDepthStencilState() == EmDepthStencilState::eRead_Write_On)
								{
									emRasterizerState = EmRasterizerState::eSolidCullNone;
									emDepthStencilState = EmDepthStencilState::eRead_On_Write_Off;
								}
								else if (pMaterial->GetDepthStencilState() == EmDepthStencilState::eRead_Off_Write_On)
								{
									emRasterizerState = EmRasterizerState::eSolidCullNone;
									emDepthStencilState = EmDepthStencilState::eRead_Write_Off;
								}
							}

							if (Config::IsEnable("Wireframe"_s) == true)
							{
								emRasterizerState = EmRasterizerState::eWireframeCullNone;
							}

							pDeviceContext->SetRasterizerState(emRasterizerState);
							pDeviceContext->SetDepthStencilState(emDepthStencilState);
						}
					}
				}
				else
				{
					pEffect->SetVector(StrID::g_f4AlbedoColor, reinterpret_cast<const Math::Vector4&>(Math::Color::White));
					pEffect->SetVector(StrID::g_f4EmissiveColor, reinterpret_cast<const Math::Vector4&>(Math::Color::Black));

					pEffect->SetVector(StrID::g_f4PaddingRoughMetEmi, reinterpret_cast<const Math::Vector4&>(Math::Color::Transparent));
					pEffect->SetVector(StrID::g_f4SurSpecTintAniso, reinterpret_cast<const Math::Vector4&>(Math::Color::Transparent));
					pEffect->SetVector(StrID::g_f4SheenTintClearcoatGloss, reinterpret_cast<const Math::Vector4&>(Math::Color::Transparent));

					pEffect->SetFloat(StrID::g_fStippleTransparencyFactor, 0.f);

					ClearEffect(pDeviceContext, pEffect, pEffectTech);

					if (isWriteDepth == false)
					{
						pDeviceContext->SetBlendState(pDevice->GetBlendState(EmBlendState::eOff));
						pDeviceContext->SetDepthStencilState(pDevice->GetDepthStencilState(EmDepthStencilState::eRead_Write_On));

						if (Config::IsEnable("Wireframe"_s) == true)
						{
							pDeviceContext->SetRasterizerState(EmRasterizerState::eWireframeCullNone);
						}
						else
						{
							pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
						}
					}
				}
			}

			if (isInstancing == true)
			{
				if (isSkinnedModel == true)
				{
					const std::vector<InstSkinnedData>& vecInstData = *static_cast<const std::vector<InstSkinnedData>*>(pInstanceData);
					size_t nInstanceSize = vecInstData.size();
					size_t nLoopCount = nInstanceSize / MAX_INSTANCE_NUM + 1;
					for (size_t j = 0; j < nLoopCount; ++j)
					{
						int nMax = std::min(MAX_INSTANCE_NUM * (j + 1), nInstanceSize);
						int nNum = nMax - j * MAX_INSTANCE_NUM;

						if (nNum <= 0)
							break;

						pEffect->SetRawValue(StrID::g_Instances, &vecInstData[j * MAX_INSTANCE_NUM], 0, nNum * sizeof(InstSkinnedData));

						uint32_t nPassCount = pEffectTech->GetPassCount();
						for (uint32_t p = 0; p < nPassCount; ++p)
						{
							pEffectTech->PassApply(p, pDeviceContext);

							if (pIndexBuffer != nullptr)
							{
								pDeviceContext->DrawIndexedInstanced(nIndexCount, nNum, nStartIndex, 0, 0);
							}
							else
							{
								pDeviceContext->DrawInstanced(pVertexBuffer->GetVertexNum(), nNum, 0, 0);
							}
						}
					}
				}
				else
				{
					const std::vector<InstStaticData>& vecInstData = *static_cast<const std::vector<InstStaticData>*>(pInstanceData);
					size_t nInstanceSize = vecInstData.size();
					size_t nLoopCount = nInstanceSize / MAX_INSTANCE_NUM + 1;
					for (size_t j = 0; j < nLoopCount; ++j)
					{
						int nMax = std::min(MAX_INSTANCE_NUM * (j + 1), nInstanceSize);
						int nNum = nMax - j * MAX_INSTANCE_NUM;

						if (nNum <= 0)
							break;

						pEffect->SetRawValue(StrID::g_Instances, &vecInstData[j * MAX_INSTANCE_NUM], 0, nNum * sizeof(InstStaticData));

						uint32_t nPassCount = pEffectTech->GetPassCount();
						for (uint32_t p = 0; p < nPassCount; ++p)
						{
							pEffectTech->PassApply(p, pDeviceContext);

							if (pIndexBuffer != nullptr)
							{
								pDeviceContext->DrawIndexedInstanced(nIndexCount, nNum, nStartIndex, 0, 0);
							}
							else
							{
								pDeviceContext->DrawInstanced(pVertexBuffer->GetVertexNum(), nNum, 0, 0);
							}
						}
					}
				}
			}
			else
			{
				const Math::Matrix& matWorld = *static_cast<const Math::Matrix*>(pInstanceData);
				pEffect->SetMatrix(StrID::g_matWorld, matWorld);

				if (isSkinnedModel == true)
				{
					pEffect->SetInt(StrID::g_nVTFID, nVTFID);
				}

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					if (pIndexBuffer != nullptr)
					{
						pDeviceContext->DrawIndexed(nIndexCount, nStartIndex, 0);
					}
					else
					{
						pDeviceContext->Draw(pVertexBuffer->GetVertexNum(), 0);
					}
				}
			}

			ClearEffect(pDeviceContext, pEffect, pEffectTech);
		}

		void ModelRenderer::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(ModelRenderer);

			OcclusionCulling(pCamera, nRenderGroupFlag);

			const bool isAlphaBlend = nRenderGroupFlag == eAlphaBlend;

			IGBuffers* pGBuffers = GetGBuffers();

			IRenderTarget** ppRenderTarget = nullptr;
			uint32_t nRenderTargetCount = 0;
			{
				D3D_PROFILING(Ready);
				pDeviceContext->ClearState();

				pDeviceContext->SetDefaultViewport();
				if (isAlphaBlend == true)
				{
					IRenderTarget* pRenderTarget = nullptr;
					if (Config::IsEnable("HDRFilter"_s) == true)
					{
						auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
						if (Config::IsEnable("HDRFilter"_s) == true)
						{
							desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
							desc.Build();
						}

						pRenderTarget = pDevice->GetRenderTarget(desc);
					}
					else
					{
						auto& desc = pDevice->GetMainRenderTarget()->GetDesc2D();
						pRenderTarget = pDevice->GetRenderTarget(desc);
					}

					ppRenderTarget = &pRenderTarget;
					nRenderTargetCount = 1;
				}
				else
				{
					ppRenderTarget = pGBuffers->GetGBuffers();
					nRenderTargetCount = EmGBuffer::Count;
				}

				pDeviceContext->SetRenderTargets(ppRenderTarget, nRenderTargetCount, pDevice->GetMainDepthStencil());
			}

			const uint32_t nForwardFlag = (isAlphaBlend ? EmRenderType::eAlphaBlend_Pre : EmRenderType::eNone);

			RenderStaticModel(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, nForwardFlag);
			RenderSkinnedModel(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, nForwardFlag);

			if (isAlphaBlend == true)
			{
				RenderStaticModel(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, EmRenderType::eAlphaBlend_Post);
				RenderSkinnedModel(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, EmRenderType::eAlphaBlend_Post);

				pDevice->ReleaseRenderTargets(ppRenderTarget, nRenderTargetCount);
			}

			if (Config::IsEnable("Shadow"_s) == true && nRenderGroupFlag == eDeferred)
			{
				for (int i = 0; i < EmLight::eCount; ++i)
				{
					EmLight::Type emType = static_cast<EmLight::Type>(i);
					uint32_t nCount = LightManager::GetInstance()->GetLightCount(emType);
					for (uint32_t j = 0; j < nCount; ++j)
					{
						ILight* pLight = LightManager::GetInstance()->GetLight(emType, j);
						if (pLight != nullptr && pLight->IsEnableShadow())
						{
							switch (pLight->GetType())
							{
							case EmLight::eDirectional:
							{
								IDirectionalLight* pDirectionalLight = static_cast<IDirectionalLight*>(pLight);

								ICascadedShadows* pCascadedShadows = pDirectionalLight->GetCascadedShadow();
								if (pCascadedShadows != nullptr)
								{
									pDeviceContext->ClearDepthStencilView(pCascadedShadows->GetDepthStencil(), D3D11_CLEAR_DEPTH);

									pDeviceContext->SetRenderTargets(nullptr, EmGBuffer::Count, pCascadedShadows->GetDepthStencil());
									pDeviceContext->SetRasterizerState(pCascadedShadows->GetRasterizerShadow());

									for (uint32_t nCascadeLevel = 0; nCascadeLevel < pCascadedShadows->GetCascadeLevel(); ++nCascadeLevel)
									{
										pDeviceContext->SetViewport(pCascadedShadows->GetViewport(nCascadeLevel));

										const Math::Matrix& matView = pCascadedShadows->GetViewMatrix(nCascadeLevel);
										const Math::Matrix& matProj = pCascadedShadows->GetProjectionMatrix(nCascadeLevel);

										const Collision::Frustum& frustum = pCascadedShadows->GetFrustum(nCascadeLevel);

										RenderStaticModel_Shadow(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, &matView, matProj, frustum, false);
										RenderSkinnedModel_Shadow(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, &matView, matProj, frustum, false);
									}
								}
							}
							break;
							case EmLight::ePoint:
							{
								IPointLight* pPointLight = static_cast<IPointLight*>(pLight);

								IShadowCubeMap* pShadowCubeMap = pPointLight->GetShadowCubeMap();
								if (pShadowCubeMap != nullptr)
								{
									pDeviceContext->ClearDepthStencilView(pShadowCubeMap->GetDepthStencil(), D3D11_CLEAR_DEPTH);

									pDeviceContext->SetRenderTargets(nullptr, EmGBuffer::Count, pShadowCubeMap->GetDepthStencil());
									pDeviceContext->SetRasterizerState(pShadowCubeMap->GetRasterizerShadow());

									pDeviceContext->SetViewport(pShadowCubeMap->GetViewport());

									std::array<Math::Matrix, IShadowCubeMap::DirectionCount> matViews;
									for (int k = 0; k < IShadowCubeMap::DirectionCount; ++k)
									{
										IShadowCubeMap::EmDirection emDirection = static_cast<IShadowCubeMap::EmDirection>(k);
										matViews[k] = pShadowCubeMap->GetViewMatrix(emDirection);
									}

									const Math::Matrix& matProj = pShadowCubeMap->GetProjectionMatrix();

									const Collision::Frustum& frustum = pShadowCubeMap->GetFrustum(IShadowCubeMap::EmDirection::eFront);

									RenderStaticModel_Shadow(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, matViews.data(), matProj, frustum, true);
									RenderSkinnedModel_Shadow(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, matViews.data(), matProj, frustum, true);
								}
							}
							break;
							case EmLight::eSpot:
							{
								ISpotLight* pSpotLight = static_cast<ISpotLight*>(pLight);

								IShadowMap* pShadowMap = pSpotLight->GetShadowMap();
								if (pShadowMap != nullptr)
								{
									pDeviceContext->ClearDepthStencilView(pShadowMap->GetDepthStencil(), D3D11_CLEAR_DEPTH);

									pDeviceContext->SetRenderTargets(nullptr, EmGBuffer::Count, pShadowMap->GetDepthStencil());
									pDeviceContext->SetRasterizerState(pShadowMap->GetRasterizerShadow());

									pDeviceContext->SetViewport(pShadowMap->GetViewport());

									const Math::Matrix& matView = pShadowMap->GetViewMatrix();
									const Math::Matrix& matProj = pShadowMap->GetProjectionMatrix();

									const Collision::Frustum& frustum = pShadowMap->GetFrustum();

									RenderStaticModel_Shadow(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, &matView, matProj, frustum, false);
									RenderSkinnedModel_Shadow(pDevice, pDeviceContext, pCamera, nRenderGroupFlag, &matView, matProj, frustum, false);
								}
							}
							break;
							default:
								continue;
							}
						}
					}
				}

				pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->SetRenderTargets(nullptr, EmGBuffer::Count, nullptr);
			}
		}

		void ModelRenderer::Flush()
		{
			int nThreadID = GetThreadID(ThreadType::eRender);
			m_nStaticIndex[nThreadID].fill(0);
			m_nSkinnedIndex[nThreadID].fill(0);
		}

		void ModelRenderer::AddRender(const RenderSubsetStatic& renderSubset)
		{
			Group group;

			IMaterial* pMaterial = renderSubset.pMaterial;
			if (pMaterial == nullptr || pMaterial->GetBlendState() == EmBlendState::eOff)
			{
				group = eDeferred;
			}
			else
			{
				group = eAlphaBlend;
			}

			int nThreadID = GetThreadID(ThreadType::eUpdate);
			size_t nIndex = m_nStaticIndex[nThreadID][group];
			if (nIndex >= m_vecStaticSubsets[nThreadID][group].size())
			{
				m_vecStaticSubsets[nThreadID][group].resize(m_vecStaticSubsets[nThreadID][group].size() * 2);
			}

			m_vecStaticSubsets[nThreadID][group][nIndex].Set(renderSubset);
			++m_nStaticIndex[nThreadID][group];
		}

		void ModelRenderer::AddRender(const RenderSubsetSkinned& renderSubset)
		{
			Group group;

			IMaterial* pMaterial = renderSubset.pMaterial;
			if (pMaterial == nullptr || pMaterial->GetBlendState() == EmBlendState::eOff)
			{
				group = eDeferred;
			}
			else
			{
				group = eAlphaBlend;
			}

			int nThreadID = GetThreadID(ThreadType::eUpdate);
			size_t nIndex = m_nSkinnedIndex[nThreadID][group];
			if (nIndex >= m_vecSkinnedSubsets[nThreadID][group].size())
			{
				m_vecSkinnedSubsets[nThreadID][group].resize(m_vecSkinnedSubsets[nThreadID][group].size() * 2);
			}

			m_vecSkinnedSubsets[nThreadID][group][nIndex].Set(renderSubset);
			++m_nSkinnedIndex[nThreadID][group];
		}

		void ModelRenderer::OcclusionCulling(Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			//if (Config::IsEnable("OcclusionCulling"_s) == true && nRenderGroupFlag == eDeferred)
			//{
			//	const Math::Matrix matViewport
			//	(
			//		1.f, 0.f, 0.f, 0.f,
			//		0.f, -1.f, 0.f, 0.f,
			//		0.f, 0.f, 1.f, 0.f,
			//		0.f, 0.f, 0.f, 1.f
			//	);

			//	const Math::Matrix matClipSpace = pCamera->GetViewMatrix() * pCamera->GetProjMatrix() * matViewport;

			//	const Collision::Frustum& frustum = pCamera->GetFrustum();

			//	OcclusionCulling::GetInstance()->Start();
			//	for (size_t i = 0; i < m_nStaticIndex[nRenderGroupFlag]; ++i)
			//	{
			//		StaticSubset& subset = m_vecStaticSubsets[nRenderGroupFlag][i];
			//		const RenderSubsetStatic& renderSubset = subset.data;

			//		if (frustum.Contains(subset.data.boundingSphere) == Collision::EmContainment::eDisjoint)
			//		{
			//			subset.isCulling = true;
			//			continue;
			//		}

			//		subset.vecVertexClipSpace.clear();
			//		subset.vecVertexClipSpace.resize(renderSubset.pVertexBuffer->GetVertexNum());

			//		OcclusionCulling::GetInstance()->TransformVertices(renderSubset.matWorld, renderSubset.pVertexBuffer->GetVertexPosPtr(), subset.vecVertexClipSpace.data(), subset.vecVertexClipSpace.size());

			//		//const VertexClipSpace* pVertexClipSpace = renderSubset.pVertexBuffer->GetVertexClipSpace();
			//		const uint32_t* pIndices = renderSubset.pIndexBuffer->GetRawValuePtr();
			//		const size_t nIndexCount = renderSubset.pIndexBuffer->GetIndexNum();
			//		//OcclusionCulling::GetInstance()->RenderTriangles(renderSubset.matWorld * matClipSpace, pVertexClipSpace, pIndices, nIndexCount);
			//		OcclusionCulling::GetInstance()->RenderTriangles(Math::Matrix::Identity, subset.vecVertexClipSpace.data(), pIndices, nIndexCount);
			//	}
			//	OcclusionCulling::GetInstance()->Flush();
			//	OcclusionCulling::GetInstance()->End();

			//	for (size_t i = 0; i < m_nStaticIndex[nRenderGroupFlag]; ++i)
			//	{
			//		StaticSubset& subset = m_vecStaticSubsets[nRenderGroupFlag][i];
			//		const RenderSubsetStatic& renderSubset = subset.data;

			//		if (subset.isCulling == true)
			//			continue;

			//		//const VertexClipSpace* pVertexClipSpace = renderSubset.pVertexBuffer->GetVertexClipSpace();
			//		const uint32_t* pIndices = renderSubset.pIndexBuffer->GetRawValuePtr();
			//		const size_t nIndexCount = renderSubset.pIndexBuffer->GetIndexNum();
			//		//OcclusionCulling::Result emResult = OcclusionCulling::GetInstance()->TestTriangles(renderSubset.matWorld * matClipSpace, pVertexClipSpace, pIndices, nIndexCount);
			//		OcclusionCulling::Result emResult = OcclusionCulling::GetInstance()->RenderTriangles(Math::Matrix::Identity, subset.vecVertexClipSpace.data(), pIndices, nIndexCount);
			//		if (emResult != OcclusionCulling::eVisible)
			//		{
			//			subset.isCulling = true;
			//		}
			//	}
			//}
		}

		void ModelRenderer::RenderStaticModel(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, uint32_t nRenderTypeFlag)
		{
			D3D_PROFILING(StaticModel);
			{
				int nThreadID = GetThreadID(ThreadType::eRender);
				const Math::Matrix& matView = pCamera->GetViewMatrix(nThreadID);
				const Math::Matrix& matProj = pCamera->GetProjMatrix(nThreadID);
				const Collision::Frustum& frustum = pCamera->GetFrustum(nThreadID);
				const Math::Vector3 f3Position = matView.Invert().Translation();

				std::map<std::pair<const void*, IMaterial*>, RenderSubsetStaticBatch> mapStatic;
				{
					D3D_PROFILING(Ready);

					for (size_t i = 0; i < m_nStaticIndex[nThreadID][nRenderGroupFlag]; ++i)
					{
						auto& subset = m_vecStaticSubsets[nThreadID][nRenderGroupFlag][i];

						if (subset.isCulling == true)
							continue;

						if (frustum.Contains(subset.data.boundingSphere) == Collision::EmContainment::eDisjoint)
							continue;

						auto iter = mapStatic.find(subset.pairKey);
						if (iter != mapStatic.end())
						{
							iter->second.vecInstData.emplace_back(subset.data.matWorld);
						}
						else
						{
							mapStatic.emplace(subset.pairKey, RenderSubsetStaticBatch(&subset, subset.data.matWorld));
						}
					}
				}

				std::vector<const StaticSubset*> vecSubsetStatic;
				vecSubsetStatic.reserve(m_nStaticIndex[nThreadID][nRenderGroupFlag]);
				{
					D3D_PROFILING(Render);

					for (auto& iter : mapStatic)
					{
						RenderSubsetStaticBatch& renderSubsetBatch = iter.second;

						if (renderSubsetBatch.vecInstData.size() == 1)
						{
							vecSubsetStatic.emplace_back(renderSubsetBatch.pSubset);
						}
						else
						{
							const RenderSubsetStatic& subset = renderSubsetBatch.pSubset->data;
							RenderModel(EmRenderType::eStatic | EmRenderType::eInstancing | nRenderTypeFlag,
								pDevice, pDeviceContext,
								&matView, matProj, f3Position,
								subset.pVertexBuffer, subset.pIndexBuffer, subset.pMaterial, subset.nIndexCount, subset.nStartIndex,
								&renderSubsetBatch.vecInstData);
						}
					}

					std::sort(vecSubsetStatic.begin(), vecSubsetStatic.end(), [](const StaticSubset* a, const StaticSubset* b) -> bool
					{
						return a->data.fDepth < b->data.fDepth;
					});

					for (auto& pSubset : vecSubsetStatic)
					{
						const RenderSubsetStatic& renderSubset = pSubset->data;

						RenderModel(EmRenderType::eStatic | nRenderTypeFlag,
							pDevice, pDeviceContext,
							&matView, matProj, f3Position,
							renderSubset.pVertexBuffer, renderSubset.pIndexBuffer, renderSubset.pMaterial, renderSubset.nIndexCount, renderSubset.nStartIndex,
							&pSubset->data.matWorld);
					}
				}

				mapStatic.clear();
			}
		}

		void ModelRenderer::RenderSkinnedModel(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, uint32_t nRenderTypeFlag)
		{
			D3D_PROFILING(SkinnedModel);
			{
				int nThreadID = GetThreadID(ThreadType::eRender);
				const Math::Matrix& matView = pCamera->GetViewMatrix(nThreadID);
				const Math::Matrix& matProj = pCamera->GetProjMatrix(nThreadID);
				const Collision::Frustum& frustum = pCamera->GetFrustum(nThreadID);
				const Math::Vector3 f3Position = matView.Invert().Translation();

				std::map<std::pair<const void*, IMaterial*>, RenderSubsetSkinnedBatch> mapSkinned;
				{
					D3D_PROFILING(Ready);

					for (uint32_t i = 0; i < m_nSkinnedIndex[nThreadID][nRenderGroupFlag]; ++i)
					{
						auto& subset = m_vecSkinnedSubsets[nThreadID][nRenderGroupFlag][i];
						if (subset.isCulling == true)
							continue;

						//if (renderSubset.pBoundingSphere != nullptr)
						//{
						//	if (pCamera->IsFrustumContains(subset.boundingSphere) == Collision::EmContainment::eDisjoint)
						//		continue;
						//}

						auto iter = mapSkinned.find(subset.pairKey);
						if (iter != mapSkinned.end())
						{
							iter->second.vecInstData.emplace_back(subset.data.matWorld, subset.data.nVTFID);
						}
						else
						{
							mapSkinned.emplace(subset.pairKey, RenderSubsetSkinnedBatch(&subset, subset.data.matWorld, subset.data.nVTFID));
						}
					}
				}

				{
					D3D_PROFILING(Render);

					for (auto& iter : mapSkinned)
					{
						RenderSubsetSkinnedBatch& renderSubsetBatch = iter.second;

						if (renderSubsetBatch.vecInstData.size() == 1)
						{
							const RenderSubsetSkinned& subset = renderSubsetBatch.pSubset->data;
							RenderModel(EmRenderType::eSkinned | nRenderTypeFlag,
								pDevice, pDeviceContext,
								&matView, matProj, f3Position,
								subset.pVertexBuffer, subset.pIndexBuffer, subset.pMaterial, subset.nIndexCount, subset.nStartIndex,
								&subset.matWorld, subset.nVTFID);
						}
						else
						{
							const RenderSubsetSkinned& subset = renderSubsetBatch.pSubset->data;
							RenderModel(EmRenderType::eSkinned | EmRenderType::eInstancing | nRenderTypeFlag,
								pDevice, pDeviceContext,
								&matView, matProj, f3Position,
								subset.pVertexBuffer, subset.pIndexBuffer, subset.pMaterial, subset.nIndexCount, subset.nStartIndex,
								&renderSubsetBatch.vecInstData);
						}
					}
				}

				mapSkinned.clear();
			}
		}
		void ModelRenderer::RenderStaticModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap)
		{
			D3D_PROFILING(StaticModel_ShadowDepth);

			int nThreadID = GetThreadID(ThreadType::eRender);

			std::map<std::pair<const void*, IMaterial*>, RenderSubsetStaticBatch> mapStatic;
			for (size_t i = 0; i < m_nStaticIndex[nThreadID][nRenderGroupFlag]; ++i)
			{
				auto& subset = m_vecStaticSubsets[nThreadID][nRenderGroupFlag][i];

				if (subset.isCulling == true)
					continue;

				if (isRenderCubeMap == false)
				{
					if (frustum.Contains(subset.data.boundingSphere) == Collision::EmContainment::eDisjoint)
						continue;
				}

				auto iter = mapStatic.find(subset.pairKey);
				if (iter != mapStatic.end())
				{
					iter->second.vecInstData.emplace_back(subset.data.matWorld);
				}
				else
				{
					mapStatic.emplace(subset.pairKey, RenderSubsetStaticBatch(&subset, subset.data.matWorld));
				}
			}

			std::vector<const RenderSubsetStatic*> vecSubsetStatic;
			vecSubsetStatic.reserve(m_nStaticIndex[nThreadID][nRenderGroupFlag]);

			for (auto& iter : mapStatic)
			{
				RenderSubsetStaticBatch& renderSubsetBatch = iter.second;

				if (renderSubsetBatch.vecInstData.size() == 1)
				{
					const RenderSubsetStatic& subset = renderSubsetBatch.pSubset->data;
					uint32_t nRenderType = EmRenderType::eStatic | EmRenderType::eWriteDepth;
					nRenderType |= isRenderCubeMap == true ? EmRenderType::eCubeMap : EmRenderType::eNone;

					RenderModel(nRenderType,
						pDevice, pDeviceContext,
						pMatView, matProj, pCamera->GetPosition(),
						subset.pVertexBuffer, subset.pIndexBuffer, subset.pMaterial, subset.nIndexCount, subset.nStartIndex,
						&subset.matWorld);
				}
				else
				{
					const RenderSubsetStatic& subset = renderSubsetBatch.pSubset->data;
					uint32_t nRenderType = EmRenderType::eStatic | EmRenderType::eInstancing | EmRenderType::eWriteDepth;
					nRenderType |= isRenderCubeMap == true ? EmRenderType::eCubeMap : EmRenderType::eNone;

					RenderModel(nRenderType,
						pDevice, pDeviceContext,
						pMatView, matProj, pCamera->GetPosition(),
						subset.pVertexBuffer, subset.pIndexBuffer, subset.pMaterial, subset.nIndexCount, subset.nStartIndex,
						&renderSubsetBatch.vecInstData);
				}
			}

			mapStatic.clear();
		}

		void ModelRenderer::RenderSkinnedModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap)
		{
			D3D_PROFILING(SkinnedModel_ShadowDepth);

			int nThreadID = GetThreadID(ThreadType::eRender);

			std::map<std::pair<const void*, IMaterial*>, RenderSubsetSkinnedBatch> mapSkinned;
			{
				D3D_PROFILING(Ready);

				for (uint32_t i = 0; i < m_nSkinnedIndex[nThreadID][nRenderGroupFlag]; ++i)
				{
					auto& subset = m_vecSkinnedSubsets[nThreadID][nRenderGroupFlag][i];
					if (subset.isCulling == true)
						continue;

					//if (isRenderCubeMap == false)
					//{
					//	if (renderSubset.pBoundingSphere != nullptr)
					//	{
					//		if (pCamera->IsFrustumContains(subset.boundingSphere) == Collision::EmContainment::eDisjoint)
					//			continue;
					//	}
					//}

					auto iter = mapSkinned.find(subset.pairKey);
					if (iter != mapSkinned.end())
					{
						iter->second.vecInstData.emplace_back(subset.data.matWorld, subset.data.nVTFID);
					}
					else
					{
						mapSkinned.emplace(subset.pairKey, RenderSubsetSkinnedBatch(&subset, subset.data.matWorld, subset.data.nVTFID));
					}
				}
			}

			{
				D3D_PROFILING(Render);

				for (auto& iter : mapSkinned)
				{
					RenderSubsetSkinnedBatch& renderSubsetBatch = iter.second;

					if (renderSubsetBatch.vecInstData.size() == 1)
					{
						const RenderSubsetSkinned& subset = renderSubsetBatch.pSubset->data;
						uint32_t nRenderType = EmRenderType::eSkinned | EmRenderType::eWriteDepth;
						nRenderType |= isRenderCubeMap == true ? EmRenderType::eCubeMap : EmRenderType::eNone;

						RenderModel(EmRenderType::eSkinned | EmRenderType::eWriteDepth,
							pDevice, pDeviceContext,
							pMatView, matProj, pCamera->GetPosition(),
							subset.pVertexBuffer, subset.pIndexBuffer, subset.pMaterial, subset.nIndexCount, subset.nStartIndex,
							&subset.matWorld, subset.nVTFID);
					}
					else
					{
						const RenderSubsetSkinned& subset = renderSubsetBatch.pSubset->data;
						uint32_t nRenderType = EmRenderType::eSkinned | EmRenderType::eInstancing | EmRenderType::eWriteDepth;
						nRenderType |= isRenderCubeMap == true ? EmRenderType::eCubeMap : EmRenderType::eNone;

						RenderModel(nRenderType,
							pDevice, pDeviceContext,
							pMatView, matProj, pCamera->GetPosition(),
							subset.pVertexBuffer, subset.pIndexBuffer, subset.pMaterial, subset.nIndexCount, subset.nStartIndex,
							&renderSubsetBatch.vecInstData);
					}
				}
			}

			mapSkinned.clear();
		}
	}
}