#include "stdafx.h"
#include "ModelRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Performance.h"
#include "CommonLib/Config.h"

#include "DirectX/CameraManager.h"
#include "DirectX/LightMgr.h"
#include "DirectX/OcclusionCulling.h"
#include "DirectX/VTFMgr.h"

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

	RegisterStringID(g_f4AlbedoColor);
	RegisterStringID(g_f4EmissiveColor);
	RegisterStringID(g_f4DisRoughMetEmi);
	RegisterStringID(g_f4SurSpecTintAniso);
	RegisterStringID(g_f4SheenTintClearcoatGloss);
	RegisterStringID(g_texAlbedo);
	RegisterStringID(g_texMask);
	RegisterStringID(g_texNormalMap);
	RegisterStringID(g_texDisplaceMap);
	RegisterStringID(g_texSpecularColor);
	RegisterStringID(g_texRoughness);
	RegisterStringID(g_texMetallic);
	RegisterStringID(g_texEmissive);
	RegisterStringID(g_texSurface);
	RegisterStringID(g_texSpecular);
	RegisterStringID(g_texSpecularTint);
	RegisterStringID(g_texAnisotropic);
	RegisterStringID(g_texSheen);
	RegisterStringID(g_texSheenTint);
	RegisterStringID(g_texClearcoat);
	RegisterStringID(g_texClearcoatGloss);
	RegisterStringID(g_samplerState);
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
				eUseTexDisplacement,
				eUseTexSpecularColor,
				eUseTexRoughness,
				eUseTexMetallic,
				eUseTexEmissive,
				eUseTexSurface,
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

				MaskCount,
			};

			const char* GetMaskName(uint64_t nMask)
			{
				static std::string s_strMaskName[] =
				{
					"USE_TEX_ALBEDO",
					"USE_TEX_MASK",
					"USE_TEX_NORMAL",
					"USE_TEX_DISPLACEMENT",
					"USE_TEX_SPECULARCOLOR",
					"USE_TEX_ROUGHNESS",
					"USE_TEX_METALLIC",
					"USE_TEX_EMISSIVE",
					"USE_TEX_SURFACE",
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

			pEffect = IEffect::Compile(strName, strPath.c_str(), &macros);
			if (pEffect == nullptr)
				return nullptr;

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

			return pEffect;
		}

		ModelRenderer::ModelRenderer()
			: m_nStaticIndex(0)
			, m_nSkinnedIndex(0)
		{
		}

		ModelRenderer::~ModelRenderer()
		{
		}

		bool ModelRenderer::Init(const Math::Viewport& viewport)
		{
			m_vecStaticSubsets.resize(512);
			m_vecSkinnedSubsets.resize(128);

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
		};

		void ClearEffect(IDeviceContext* pDeviceContext, IEffect* pEffect, IEffectTech* pEffectTech)
		{
			pEffect->SetTexture(StrID::g_texAlbedo, nullptr);
			pEffect->SetTexture(StrID::g_texMask, nullptr);
			pEffect->SetTexture(StrID::g_texNormalMap, nullptr);
			pEffect->SetTexture(StrID::g_texDisplaceMap, nullptr);
			pEffect->SetTexture(StrID::g_texSpecularColor, nullptr);
			pEffect->SetTexture(StrID::g_texRoughness, nullptr);
			pEffect->SetTexture(StrID::g_texMetallic, nullptr);
			pEffect->SetTexture(StrID::g_texEmissive, nullptr);
			pEffect->SetTexture(StrID::g_texSurface, nullptr);
			pEffect->SetTexture(StrID::g_texSpecular, nullptr);
			pEffect->SetTexture(StrID::g_texSpecularTint, nullptr);
			pEffect->SetTexture(StrID::g_texAnisotropic, nullptr);
			pEffect->SetTexture(StrID::g_texSheen, nullptr);
			pEffect->SetTexture(StrID::g_texSheenTint, nullptr);
			pEffect->SetTexture(StrID::g_texClearcoat, nullptr);
			pEffect->SetTexture(StrID::g_texClearcoatGloss, nullptr);

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
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eDisplacement) == true ? EmModelShader::eUseTexDisplacement : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSpecularColor) == true ? EmModelShader::eUseTexSpecularColor : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eRoughness) == true ? EmModelShader::eUseTexRoughness : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eMetallic) == true ? EmModelShader::eUseTexMetallic : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eEmissive) == true ? EmModelShader::eUseTexEmissive : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSurface) == true ? EmModelShader::eUseTexSurface : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSpecular) == true ? EmModelShader::eUseTexSpecular : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSpecularTint) == true ? EmModelShader::eUseTexSpecularTint : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eAnisotropic) == true ? EmModelShader::eUseTexAnisotropic : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSheen) == true ? EmModelShader::eUseTexSheen : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eSheenTint) == true ? EmModelShader::eUseTexSheenTint : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eClearcoat) == true ? EmModelShader::eUseTexClearcoat : -1);
				SetBitMask64(nMask, IsValidTexture(EmMaterial::eClearcoatGloss) == true ? EmModelShader::eUseTexClearcoatGloss : -1);

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
			if (pEffect == nullptr)
				return;

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
					const float fTessellationFactor = 128.f;
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

					pEffect->SetVector(StrID::g_f4DisRoughMetEmi, pMaterial->GetDisRoughMetEmi());
					pEffect->SetVector(StrID::g_f4SurSpecTintAniso, pMaterial->GetSurSpecTintAniso());
					pEffect->SetVector(StrID::g_f4SheenTintClearcoatGloss, pMaterial->GetSheenTintClearcoatGloss());

					pEffect->SetTexture(StrID::g_texAlbedo, pMaterial->GetTexture(EmMaterial::eAlbedo));
					pEffect->SetTexture(StrID::g_texMask, pMaterial->GetTexture(EmMaterial::eMask));
					pEffect->SetTexture(StrID::g_texNormalMap, pMaterial->GetTexture(EmMaterial::eNormal));
					pEffect->SetTexture(StrID::g_texDisplaceMap, pMaterial->GetTexture(EmMaterial::eDisplacement));
					pEffect->SetTexture(StrID::g_texSpecularColor, pMaterial->GetTexture(EmMaterial::eSpecularColor));
					pEffect->SetTexture(StrID::g_texRoughness, pMaterial->GetTexture(EmMaterial::eRoughness));
					pEffect->SetTexture(StrID::g_texMetallic, pMaterial->GetTexture(EmMaterial::eMetallic));
					pEffect->SetTexture(StrID::g_texEmissive, pMaterial->GetTexture(EmMaterial::eEmissive));
					pEffect->SetTexture(StrID::g_texSurface, pMaterial->GetTexture(EmMaterial::eSurface));
					pEffect->SetTexture(StrID::g_texSpecular, pMaterial->GetTexture(EmMaterial::eSpecular));
					pEffect->SetTexture(StrID::g_texSpecularTint, pMaterial->GetTexture(EmMaterial::eSpecularTint));
					pEffect->SetTexture(StrID::g_texAnisotropic, pMaterial->GetTexture(EmMaterial::eAnisotropic));
					pEffect->SetTexture(StrID::g_texSheen, pMaterial->GetTexture(EmMaterial::eSheen));
					pEffect->SetTexture(StrID::g_texSheenTint, pMaterial->GetTexture(EmMaterial::eSheenTint));
					pEffect->SetTexture(StrID::g_texClearcoat, pMaterial->GetTexture(EmMaterial::eClearcoat));
					pEffect->SetTexture(StrID::g_texClearcoatGloss, pMaterial->GetTexture(EmMaterial::eClearcoatGloss));

					pEffect->SetSamplerState(StrID::g_samplerState, pMaterial->GetSamplerState(), 0);

					pDeviceContext->SetBlendState(pMaterial->GetBlendState());

					if (isWriteDepth == false)
					{
						pDeviceContext->SetDepthStencilState(pMaterial->GetDepthStencilState());

						if (Config::IsEnable("Wireframe"_s) == true)
						{
							pDeviceContext->SetRasterizerState(EmRasterizerState::eWireframeCullNone);
						}
						else
						{
							pDeviceContext->SetRasterizerState(pMaterial->GetRasterizerState());
						}
					}
				}
				else
				{
					pEffect->SetVector(StrID::g_f4AlbedoColor, reinterpret_cast<const Math::Vector4&>(Math::Color::White));
					pEffect->SetVector(StrID::g_f4EmissiveColor, reinterpret_cast<const Math::Vector4&>(Math::Color::Black));

					pEffect->SetVector(StrID::g_f4DisRoughMetEmi, reinterpret_cast<const Math::Vector4&>(Math::Color::Transparent));
					pEffect->SetVector(StrID::g_f4SurSpecTintAniso, reinterpret_cast<const Math::Vector4&>(Math::Color::Transparent));
					pEffect->SetVector(StrID::g_f4SheenTintClearcoatGloss, reinterpret_cast<const Math::Vector4&>(Math::Color::Transparent));

					ClearEffect(pDeviceContext, pEffect, pEffectTech);

					if (isWriteDepth == false)
					{
						pDeviceContext->SetBlendState(pDevice->GetBlendState(EmBlendState::eOff));
						pDeviceContext->SetDepthStencilState(pDevice->GetDepthStencilState(EmDepthStencilState::eOn));

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

		void ModelRenderer::Render(uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(ModelRenderer);

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				LOG_ERROR("ModelRenderer::Render() : Not Exist Main Camera !!");
				return;
			}

			/*const Math::Vector3& f3CameraPos = pCamera->GetPosition();
			{
				D3D_PROFILING(SortStaticModel);
				std::sort(m_vecStaticSubsets.begin(), m_vecStaticSubsets.end(), [&f3CameraPos](const StaticSubset& a, const StaticSubset& b) -> bool
				{
					return a.fDepth < b.fDepth;
				});
			}*/

			//{
			//	std::vector<int> check, check2;
			//	check.resize(3);
			//	check2.resize(3);

			//	Performance::Counter counter1;

			//	//uint32_t nVertexTotalCount = 0;
			//	//uint32_t nIndexTotalCount = 0;

			//	static const Math::Matrix matViewport
			//	(
			//		1.0f, 0.0f, 0.0f, 0.0f,
			//		0.0f, -1.0f, 0.0f, 0.0f,
			//		0.0f, 0.0f, 1.0f, 0.0f,
			//		0.0f, 0.0f, 0.0f, 1.0f
			//	);

			//	Math::Matrix matViewProjPort = pCamera->GetViewMatrix() * pCamera->GetProjMatrix() * matViewport;

			//	counter1.Start();
			//	{
			//		D3D_PROFILING(OcclusionCulling_Render);

			//		if (Config::IsEnable(String::StringKey("OcclusionCulling"_s)) == true)
			//		{
			//			SOcclusionCulling::GetInstance()->Start();
			//			std::for_each(m_vecStaticSubsets.begin(), m_vecStaticSubsets.end(), [&](StaticSubset& subset)
			//			{
			//				const RenderSubsetStatic& renderSubset = *subset.pRenderSubset;
			//			
			//				const std::vector<VertexClipSpace>& vecClipSpace = renderSubset.pVertexBuffer->GetVertexClipSpaceVector();
			//				const VertexPos* pVertexPos = renderSubset.pVertexBuffer->GetVertexPosPtr();
			//				if (pVertexPos != nullptr)
			//				{
			//					nVertexTotalCount += renderSubset.pVertexBuffer->GetVertexNum();
			//					nIndexTotalCount += renderSubset.pIndexBuffer->GetIndexNum();
			//			
			//					Math::Matrix matClipSpace = subset.matWorld * matViewProjPort;
			//			
			//					const uint32_t* pIndexData = reinterpret_cast<const uint32_t*>(renderSubset.pIndexBuffer->GetRawValuePtr());
			//					EmOcclusionCulling::Result result = SOcclusionCulling::GetInstance()->RenderTriangles(&vecClipSpace.front(), pIndexData, renderSubset.pIndexBuffer->GetIndexNum(), &matClipSpace);
			//			
			//					check[result]++;
			//			
			//					if (result != EmOcclusionCulling::eVisible)
			//					{
			//						subset.isCulling = true;
			//					}
			//				}
			//			});
			//			SOcclusionCulling::GetInstance()->Flush();
			//			SOcclusionCulling::GetInstance()->End();
			//		}
			//	}
			//	counter1.End();

			//	Performance::Counter counter2;
			//	counter2.Start();
			//	{
			//		D3D_PROFILING(OcclusionCulling_Test);

			//		if (Config::IsEnable(String::StringKey("OcclusionCulling"_s)) == true)
			//		{
			//			//Concurrency::parallel_for_each(m_vecStaticSubsets.begin(), m_vecStaticSubsets.end(), [&](RenderSubset& subset)
			//			std::for_each(m_vecStaticSubsets.begin(), m_vecStaticSubsets.end(), [&](StaticSubset& subset)
			//			{
			//				const RenderSubsetStatic& renderSubset = *subset.pRenderSubset;
			//				if (subset.isCulling == false)
			//				{
			//					const std::vector<VertexClipSpace>& vecClipSpace = renderSubset.pVertexBuffer->GetVertexClipSpaceVector();
			//					const VertexPos* pVertexPos = renderSubset.pVertexBuffer->GetVertexPosPtr();
			//					if (pVertexPos != nullptr)
			//					{
			//						Math::Matrix matClipSpace = subset.matWorld * matViewProjPort;
			//			
			//						const uint32_t* pIndexData = reinterpret_cast<const uint32_t*>(renderSubset.pIndexBuffer->GetRawValuePtr());
			//						EmOcclusionCulling::Result result = SOcclusionCulling::GetInstance()->TestTriangles(&vecClipSpace.front(), pIndexData, renderSubset.pIndexBuffer->GetIndexNum(), &matClipSpace);
			//			
			//						check2[result]++;
			//			
			//						if (result != EmOcclusionCulling::eVisible)
			//						{
			//							subset.isCulling = true;
			//						}
			//					}
			//				}
			//			});
			//		}
			//	}
			//	counter2.End();
			//}

			IDevice* pDevice = GetDevice();
			IDeviceContext* pDeviceContext = GetDeviceContext();

			IGBuffers* pGBuffers = GetGBuffers();

			{
				D3D_PROFILING(Ready);
				pDeviceContext->ClearState();

				pDeviceContext->SetDefaultViewport();

				pDeviceContext->SetRenderTargets(pGBuffers->GetGBuffers(), EmGBuffer::Count, pDevice->GetMainDepthStencil());
			}

			renderStaticModel(pDevice, pCamera);
			renderSkinnedModel(pDevice, pCamera);

			if (Config::IsEnable("Shadow"_s) == true)
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

										renderStaticModel_Shadow(pDevice, pDeviceContext, pCamera, &matView, matProj, frustum, false);
										renderSkinnedModel_Shadow(pDevice, pDeviceContext, pCamera, &matView, matProj, frustum, false);
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

									renderStaticModel_Shadow(pDevice, pDeviceContext, pCamera, matViews.data(), matProj, frustum, true);
									renderSkinnedModel_Shadow(pDevice, pDeviceContext, pCamera, matViews.data(), matProj, frustum, true);
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

									renderStaticModel_Shadow(pDevice, pDeviceContext, pCamera, &matView, matProj, frustum, false);
									renderSkinnedModel_Shadow(pDevice, pDeviceContext, pCamera, &matView, matProj, frustum, false);
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
			m_nStaticIndex = 0;
			m_nSkinnedIndex = 0;
		}

		void ModelRenderer::renderStaticModel(IDevice* pDevice, Camera* pCamera)
		{
			IDeviceContext* pDeviceContext = GetDeviceContext();

			D3D_PROFILING(StaticModel);
			{
				const Collision::Frustum& frustum = pCamera->GetFrustum();

				std::map<std::pair<void*, IMaterial*>, RenderSubsetStaticBatch> mapStatic;
				{
					D3D_PROFILING(Ready);

					for (size_t i = 0; i < m_nStaticIndex; ++i)
					{
						auto& subset = m_vecStaticSubsets[i];

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
				vecSubsetStatic.reserve(m_nStaticIndex);
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
							RenderModel(EmRenderType::eStatic | EmRenderType::eInstancing,
								pDevice, pDeviceContext,
								&pCamera->GetViewMatrix(), pCamera->GetProjMatrix(), pCamera->GetPosition(),
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

						RenderModel(EmRenderType::eStatic,
							pDevice, pDeviceContext,
							&pCamera->GetViewMatrix(), pCamera->GetProjMatrix(), pCamera->GetPosition(),
							renderSubset.pVertexBuffer, renderSubset.pIndexBuffer, renderSubset.pMaterial, renderSubset.nIndexCount, renderSubset.nStartIndex,
							&pSubset->data.matWorld);
					}
				}

				mapStatic.clear();
			}
		}

		void ModelRenderer::renderSkinnedModel(IDevice* pDevice, Camera* pCamera)
		{
			IDeviceContext* pDeviceContext = GetDeviceContext();

			D3D_PROFILING(SkinnedModel);
			{
				std::map<std::pair<void*, IMaterial*>, RenderSubsetSkinnedBatch> mapSkinned;
				{
					D3D_PROFILING(Ready);

					for (uint32_t i = 0; i < m_nSkinnedIndex; ++i)
					{
						auto& subset = m_vecSkinnedSubsets[i];
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
							RenderModel(EmRenderType::eSkinned,
								pDevice, pDeviceContext,
								&pCamera->GetViewMatrix(), pCamera->GetProjMatrix(), pCamera->GetPosition(),
								subset.pVertexBuffer, subset.pIndexBuffer, subset.pMaterial, subset.nIndexCount, subset.nStartIndex,
								&subset.matWorld, subset.nVTFID);
						}
						else
						{
							const RenderSubsetSkinned& subset = renderSubsetBatch.pSubset->data;
							RenderModel(EmRenderType::eSkinned | EmRenderType::eInstancing,
								pDevice, pDeviceContext,
								&pCamera->GetViewMatrix(), pCamera->GetProjMatrix(), pCamera->GetPosition(),
								subset.pVertexBuffer, subset.pIndexBuffer, subset.pMaterial, subset.nIndexCount, subset.nStartIndex,
								&renderSubsetBatch.vecInstData);
						}
					}
				}

				mapSkinned.clear();
			}
		}
		void ModelRenderer::renderStaticModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap)
		{
			D3D_PROFILING(StaticModel_ShadowDepth);

			std::map<std::pair<void*, IMaterial*>, RenderSubsetStaticBatch> mapStatic;
			for (size_t i = 0; i < m_nStaticIndex; ++i)
			{
				auto& subset = m_vecStaticSubsets[i];

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
			vecSubsetStatic.reserve(m_nStaticIndex);

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

		void ModelRenderer::renderSkinnedModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap)
		{
			D3D_PROFILING(SkinnedModel_ShadowDepth);

			std::map<std::pair<void*, IMaterial*>, RenderSubsetSkinnedBatch> mapSkinned;
			{
				D3D_PROFILING(Ready);

				for (uint32_t i = 0; i < m_nSkinnedIndex; ++i)
				{
					auto& subset = m_vecSkinnedSubsets[i];
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