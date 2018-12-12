#include "stdafx.h"
#include "ModelRendererDX11.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Lock.h"
#include "CommonLib/Timer.h"

#include "GraphicsInterface/Instancing.h"
#include "GraphicsInterface/Camera.h"
#include "GraphicsInterface/LightManager.h"
#include "GraphicsInterface/OcclusionCulling.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "GBufferDX11.h"
#include "VTFManagerDX11.h"

#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"
#include "TextureDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct SkinningInstancingDataBuffer
				{
					std::array<SkinningInstancingData, eMaxInstancingCount> data;
				};

				struct StaticInstancingDataBuffer
				{
					std::array<math::Matrix, eMaxInstancingCount> data;
				};

				struct ObjectDataBuffer
				{
					math::Matrix matWorld;

					math::Color f4AlbedoColor;
					math::Color f4EmissiveColor;

					math::float4 f4PaddingRoughMetEmi;
					math::float4 f4SurSpecTintAniso;
					math::float4 f4SheenTintClearcoatGloss;

					float fStippleTransparencyFactor{ 0.f };
					uint32_t nVTFID{ 0 };

					math::float2 f2Padding;
				};

				struct VSConstants
				{
					math::Matrix matViewProj;
				};

				struct CommonContents
				{
					math::float3 f3CameraPos;
					int nEnableShadowCount{ 0 };

					uint32_t nDirectionalLightCount{ 0 };
					uint32_t nPointLightCount{ 0 };
					uint32_t nSpotLightCount{ 0 };
					uint32_t padding{ 0 };

					std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
					std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
					std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};
				};

				enum CBSlot
				{
					eCB_SkinningInstancingData = 0,
					eCB_StaticInstancingData,
					eCB_ObjectData,
					eCB_VSConstants,

					eCB_CommonContents = 5,
				};

				enum SamplerSlot
				{
					eSampler_Material = 0,
					eSampler_PointClamp = 1,
					eSampler_Clamp = 2,
				};

				enum SRVSlot
				{
					eSRV_Albedo = 0,
					eSRV_Mask,
					eSRV_Normal,
					eSRV_Roughness,
					eSRV_Metallic,
					eSRV_Emissive,
					eSRV_EmissiveColor,
					eSRV_Subsurface,
					eSRV_Specular,
					eSRV_SpecularTint,
					eSRV_Anisotropic,
					eSRV_Sheen,
					eSRV_SheenTint,
					eSRV_Clearcoat,
					eSRV_ClearcoatGloss,

					eSRV_VTF = 15,

					eSRV_DiffuseHDR = 16,
					eSRV_SpecularHDR = 17,
					eSRV_SpecularBRDF = 18,
					eSRV_ShadowMap = 19,

					SRVSlotCount,
				};

				enum Pass
				{
					ePass_Deferred = 0,
					ePass_AlphaBlend_Pre,
					ePass_AlphaBlend_Post,
				};

				enum Mask : uint64_t
				{
					eUseTexAlbedo = 1 << 0,
					eUseTexMask = 1 << 1,
					eUseTexNormal = 1 << 2,
					eUseTexRoughness = 1 << 3,
					eUseTexMetallic = 1 << 4,
					eUseTexEmissive = 1 << 5,
					eUseTexEmissiveColor = 1 << 6,
					eUseTexSubsurface = 1 << 7,
					eUseTexSpecular = 1 << 8,
					eUseTexSpecularTint = 1 << 9,
					eUseTexAnisotropic = 1 << 10,
					eUseTexSheen = 1 << 11,
					eUseTexSheenTint = 1 << 12,
					eUseTexClearcoat = 1 << 13,
					eUseTexClearcoatGloss = 1 << 14,

					eUseInstancing = 1 << 15,
					eUseSkinning = 1 << 16,
					eUseAlphaBlending = 1 << 17,

					MaskCount = 18,
				};

				const char* GetMaskName(uint32_t nMaskBit)
				{
					static const std::string s_strMaskName[] =
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
						"USE_ALPHABLENDING",

						"USE_WRITEDEPTH",
						"USE_CUBEMAP",
						"USE_TESSELLATION",
						"USE_ALBEDO_ALPHA_IS_MASK_MAP",
					};

					return s_strMaskName[nMaskBit].c_str();
				}

				std::vector<D3D_SHADER_MACRO> GetMacros(const MaskKey& maskKey)
				{
					std::vector<D3D_SHADER_MACRO> vecMacros;
					vecMacros.push_back({ "DX11", "1" });
					for (uint32_t i = 0; i < shader::MaskCount; ++i)
					{
						if ((maskKey & (1 << i)) != 0)
						{
							vecMacros.push_back({ GetMaskName(i), "1" });
						}
					}
					vecMacros.push_back({ nullptr, nullptr });

					return vecMacros;
				}

				bool IsValidTexture(const IMaterial* pMaterial, EmMaterial::Type emType)
				{
					if (pMaterial == nullptr)
						return false;

					ITexture* pTexture = pMaterial->GetTexture(emType);
					if (pTexture == nullptr)
						return false;

					return pTexture->GetState() == IResource::eComplete;
				}

				void SetPSSRV(ID3D11DeviceContext* pDeviceContext, const IMaterial* pMaterial, EmMaterial::Type emType, shader::SRVSlot srvSlot)
				{
					if (IsValidTexture(pMaterial, emType) == true)
					{
						Texture* pTexture = static_cast<Texture*>(pMaterial->GetTexture(emType));
						ID3D11ShaderResourceView* pSRVs[] =
						{
							pTexture->GetShaderResourceView(),
						};
						pDeviceContext->PSSetShaderResources(srvSlot, _countof(pSRVs), pSRVs);
					}
					else
					{
						pDeviceContext->PSSetShaderResources(srvSlot, 0, nullptr);
					}
				};

				uint32_t GetMaterialMask(const IMaterial* pMaterial)
				{
					uint32_t nMask = 0;
					if (pMaterial != nullptr)
					{
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eAlbedo) ? shader::eUseTexAlbedo : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eMask) ? shader::eUseTexMask : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eNormal) ? shader::eUseTexNormal : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eRoughness) ? shader::eUseTexRoughness : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eMetallic) ? shader::eUseTexMetallic : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eEmissive) ? shader::eUseTexEmissive : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eEmissiveColor) ? shader::eUseTexEmissiveColor : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eSubsurface) ? shader::eUseTexSubsurface : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eSpecular) ? shader::eUseTexSpecular : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eSpecularTint) ? shader::eUseTexSpecularTint : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eAnisotropic) ? shader::eUseTexAnisotropic : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eSheen) ? shader::eUseTexSheen : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eSheenTint) ? shader::eUseTexSheenTint : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eClearcoat) ? shader::eUseTexClearcoat : 0;
						nMask |= shader::IsValidTexture(pMaterial, EmMaterial::eClearcoatGloss) ? shader::eUseTexClearcoatGloss : 0;
					}

					return nMask;
				}

				void SetMaterial(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext, const IMaterial* pMaterial, ModelRenderer::Group emGroup, Pass emPass)
				{
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eAlbedo, shader::eSRV_Albedo);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eMask, shader::eSRV_Mask);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eNormal, shader::eSRV_Normal);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eRoughness, shader::eSRV_Roughness);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eMetallic, shader::eSRV_Metallic);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eEmissive, shader::eSRV_Emissive);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eEmissiveColor, shader::eSRV_EmissiveColor);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eSubsurface, shader::eSRV_Subsurface);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eSpecular, shader::eSRV_Specular);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eSpecularTint, shader::eSRV_SpecularTint);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eAnisotropic, shader::eSRV_Anisotropic);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eSheen, shader::eSRV_Sheen);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eSheenTint, shader::eSRV_SheenTint);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eClearcoat, shader::eSRV_Clearcoat);
					SetPSSRV(pDeviceContext, pMaterial, EmMaterial::eClearcoatGloss, shader::eSRV_ClearcoatGloss);

					if (pMaterial != nullptr)
					{
						ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(pMaterial->GetBlendState());
						pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

						ID3D11SamplerState* pSamplerStates[] =
						{
							pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagLinearMipPointWrap),
						};
						pDeviceContext->PSSetSamplers(eSampler_Material, _countof(pSamplerStates), pSamplerStates);

						EmDepthStencilState::Type emDepthStencilState = pMaterial->GetDepthStencilState();
						EmRasterizerState::Type emRasterizerState = pMaterial->GetRasterizerState();
						if (emGroup == ModelRenderer::eAlphaBlend &&
							emPass == ePass_AlphaBlend_Pre)
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

						ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(emRasterizerState);
						pDeviceContext->RSSetState(pRasterizerState);

						ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(emDepthStencilState);
						pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);
					}
					else
					{
						ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
						pDeviceContext->RSSetState(pRasterizerState);

						ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOff);
						pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

						ID3D11SamplerState* pSamplerStates[] =
						{
							pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagLinearMipPointWrap),
						};
						pDeviceContext->PSSetSamplers(eSampler_Material, _countof(pSamplerStates), pSamplerStates);

						ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_On);
						pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);
					}
				}

				void SetObjectData(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<ObjectDataBuffer>* pCB_ObjectDataBuffer,
					const IMaterial* pMaterial, const math::Matrix& matWorld, uint32_t nVTFID)
				{
					ObjectDataBuffer* pObjectDataBuffer = pCB_ObjectDataBuffer->Map(pDeviceContext);
					pObjectDataBuffer->matWorld = matWorld.Transpose();

					if (pMaterial != nullptr)
					{
						pObjectDataBuffer->f4AlbedoColor = pMaterial->GetAlbedoColor();
						pObjectDataBuffer->f4EmissiveColor = pMaterial->GetEmissiveColor();

						pObjectDataBuffer->f4PaddingRoughMetEmi = pMaterial->GetPaddingRoughMetEmi();
						pObjectDataBuffer->f4SurSpecTintAniso = pMaterial->GetSurSpecTintAniso();
						pObjectDataBuffer->f4SheenTintClearcoatGloss = pMaterial->GetSheenTintClearcoatGloss();

						pObjectDataBuffer->fStippleTransparencyFactor = pMaterial->GetStippleTransparencyFactor();
						pObjectDataBuffer->nVTFID = nVTFID;

						pObjectDataBuffer->f2Padding = {};
					}
					else
					{
						pObjectDataBuffer->f4AlbedoColor = math::Color::White;
						pObjectDataBuffer->f4EmissiveColor = math::Color::Black;

						pObjectDataBuffer->f4PaddingRoughMetEmi = math::float4::Zero;
						pObjectDataBuffer->f4SurSpecTintAniso = math::float4::Zero;
						pObjectDataBuffer->f4SheenTintClearcoatGloss = math::float4::Zero;

						pObjectDataBuffer->fStippleTransparencyFactor = 0.f;
						pObjectDataBuffer->nVTFID = nVTFID;

						pObjectDataBuffer->f2Padding = {};
					}
					pCB_ObjectDataBuffer->Unmap(pDeviceContext);
				}

				void SetImageBasedLight(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext, const IImageBasedLight* pImageBasedLight)
				{
					Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR());
					ID3D11ShaderResourceView* pSRV = pDiffuseHDR->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(eSRV_DiffuseHDR, 1, &pSRV);

					Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR());
					pSRV = pSpecularHDR->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(eSRV_SpecularHDR, 1, &pSRV);

					Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF());
					pSRV = pSpecularBRDF->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(eSRV_SpecularBRDF, 1, &pSRV);

					ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointClamp);
					pDeviceContext->PSSetSamplers(eSampler_PointClamp, 1, &pSamplerPointClamp);

					ID3D11SamplerState* pSamplerClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipLinearClamp);
					pDeviceContext->PSSetSamplers(eSampler_Clamp, 1, &pSamplerClamp);
				}

				void SetCommonContents_ForAlpha(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<CommonContents>* pCB_CommonContents,
					const LightManager* pLightManager, const math::float3& f3CameraPos, int nEnableShadowCount)
				{
					CommonContents* pCommonContents = pCB_CommonContents->Map(pDeviceContext);

					pCommonContents->f3CameraPos = f3CameraPos;
					pCommonContents->nEnableShadowCount = nEnableShadowCount;

					const DirectionalLightData* pDirectionalLightData = nullptr;
					pLightManager->GetDirectionalLightData(&pDirectionalLightData, &pCommonContents->nDirectionalLightCount);
					memory::Copy(pCommonContents->lightDirectional, pDirectionalLightData, sizeof(DirectionalLightData) * pCommonContents->nDirectionalLightCount);

					const PointLightData* pPointLightData = nullptr;
					pLightManager->GetPointLightData(&pPointLightData, &pCommonContents->nPointLightCount);
					memory::Copy(pCommonContents->lightPoint, pPointLightData, sizeof(PointLightData) * pCommonContents->nPointLightCount);

					const SpotLightData* pSpotLightData = nullptr;
					pLightManager->GetSpotLightData(&pSpotLightData, &pCommonContents->nSpotLightCount);
					memory::Copy(pCommonContents->lightSpot, pSpotLightData, sizeof(SpotLightData) * pCommonContents->nSpotLightCount);

					pCB_CommonContents->Unmap(pDeviceContext);
				}

				void Draw(ID3D11DeviceContext* pDeviceContext,
					const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer,
					uint32_t nStartIndex, uint32_t nIndexCount)
				{
					ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
					const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
					const uint32_t nOffsets[] = { 0, };
					pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);

					if (pIndexBuffer != nullptr)
					{
						pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
						pDeviceContext->DrawIndexed(nIndexCount, nStartIndex, 0);
					}
					else
					{
						pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
						pDeviceContext->Draw(nIndexCount, nStartIndex);
					}
				}

				void DrawInstance(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<StaticInstancingDataBuffer>* pCB_StaticInstancingDataBuffer,
					const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer,
					uint32_t nStartIndex, uint32_t nIndexCount,
					const math::Matrix* pInstanceData, size_t nInstanceCount)
				{
					ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
					const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
					const uint32_t nOffsets[] = { 0, };
					pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);

					if (pIndexBuffer != nullptr)
					{
						pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
					}
					else
					{
						pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
					}

					const size_t loopCount = nInstanceCount / eMaxInstancingCount + 1;
					for (size_t i = 0; i < loopCount; ++i)
					{
						const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), nInstanceCount);
						const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

						if (nDrawInstanceCount <= 0)
							break;

						StaticInstancingDataBuffer* pStaticInstancingData = pCB_StaticInstancingDataBuffer->Map(pDeviceContext);
						memory::Copy(pStaticInstancingData->data.data(), sizeof(pStaticInstancingData->data), &pInstanceData[i * eMaxInstancingCount], sizeof(math::Matrix) * nDrawInstanceCount);
						pCB_StaticInstancingDataBuffer->Unmap(pDeviceContext);

						if (pIndexBuffer != nullptr)
						{
							pDeviceContext->DrawIndexedInstanced(nIndexCount, static_cast<uint32_t>(nDrawInstanceCount), nStartIndex, 0, 0);
						}
						else
						{
							pDeviceContext->DrawInstanced(nIndexCount, static_cast<uint32_t>(nDrawInstanceCount), nStartIndex, 0);
						}
					}
				}

				void DrawInstance(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<SkinningInstancingDataBuffer>* pCB_SkinningInstancingDataBuffer,
					const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer,
					uint32_t nStartIndex, uint32_t nIndexCount,
					const SkinningInstancingData* pInstanceData, size_t nInstanceCount)
				{
					ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
					const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
					const uint32_t nOffsets[] = { 0, };
					pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);

					if (pIndexBuffer != nullptr)
					{
						pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
					}
					else
					{
						pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
					}

					const size_t loopCount = nInstanceCount / eMaxInstancingCount + 1;
					for (size_t i = 0; i < loopCount; ++i)
					{
						const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), nInstanceCount);
						const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

						if (nDrawInstanceCount <= 0)
							break;

						SkinningInstancingDataBuffer* pSkinningInstancingData = pCB_SkinningInstancingDataBuffer->Map(pDeviceContext);
						memory::Copy(pSkinningInstancingData->data.data(), sizeof(pSkinningInstancingData->data), &pInstanceData[i * eMaxInstancingCount], sizeof(SkinningInstancingData) * nDrawInstanceCount);
						pCB_SkinningInstancingDataBuffer->Unmap(pDeviceContext);

						if (pIndexBuffer != nullptr)
						{
							pDeviceContext->DrawIndexedInstanced(nIndexCount, static_cast<uint32_t>(nDrawInstanceCount), nStartIndex, 0, 0);
						}
						else
						{
							pDeviceContext->DrawInstanced(nIndexCount, static_cast<uint32_t>(nDrawInstanceCount), nStartIndex, 0);
						}
					}
				}
			}

			class ModelRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera, Group emGroup);
				void Cleanup();

			public:
				void PushJob(const RenderJobStatic& job);
				void PushJob(const RenderJobSkinned& job);

			private:
				void DeferredGroupSetup(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext);
				RenderTarget* AlphaBlendGroupSetup(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext);

				void RenderStaticElement(Device* pDeviceInstance, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera, Group emGroup, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask);
				void RenderSkinnedElement(Device* pDeviceInstance, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera, Group emGroup, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask);

			private:
				struct RenderState
				{
					ID3D11VertexShader* pVertexShader{ nullptr };
					ID3D11PixelShader* pPixelShader{ nullptr };
					ID3D11InputLayout* pInputLayout{ nullptr };
				};
				const RenderState* GetRenderState(ID3D11Device* pDevice, const shader::MaskKey& nMask);

				void CreateVertexShader(ID3D11Device* pDevice, const shader::MaskKey& nMask, const char* pFunctionName);
				void CreatePixelShader(ID3D11Device* pDevice, const shader::MaskKey& nMask, const char* pFunctionName);

			private:
				thread::SRWLock m_srwLock;
				std::string m_strShaderPath;
				ID3DBlob* m_pShaderBlob{ nullptr };

				ConstantBuffer<shader::SkinningInstancingDataBuffer> m_skinningInstancingDataBuffer;
				ConstantBuffer<shader::StaticInstancingDataBuffer> m_staticInstancingDataBuffer;
				ConstantBuffer<shader::ObjectDataBuffer> m_objectDataBuffer;
				ConstantBuffer<shader::VSConstants> m_vsConstants;

				// for alphablend, common.hlsl
				ConstantBuffer<shader::CommonContents> m_commonContentsBuffer;

				tsl::robin_map<shader::MaskKey, RenderState> m_umapRenderStates;

				struct JobStatic
				{
					RenderJobStatic data;
					bool isCulled{ false };

					void Set(const RenderJobStatic& source)
					{
						data = source;
						isCulled = false;
					}
				};

				struct JobStaticBatch
				{
					const JobStatic* pJob{ nullptr };
					std::vector<math::Matrix> vecInstanceData;

					JobStaticBatch(const JobStatic* pJob, const math::Matrix& matWorld)
						: pJob(pJob)
					{
						vecInstanceData.emplace_back(matWorld);
					}
				};

				std::array<std::vector<JobStatic>, GroupCount> m_vecJobStatics;
				std::array<size_t, GroupCount> m_nJobStaticCount;

				using UMapJobStaticBatch = tsl::robin_map<const void*, JobStaticBatch>;
				using UMapJobStaticMaterialBatch = tsl::robin_map<const IMaterial*, UMapJobStaticBatch>;
				UMapJobStaticMaterialBatch m_umapJobStaticMasterBatchs;

				struct JobSkinned
				{
					RenderJobSkinned data;
					bool isCulled{ false };

					void Set(const RenderJobSkinned& source)
					{
						data = source;
						isCulled = false;
					}
				};

				struct JobSkinnedBatch
				{
					const JobSkinned* pJob{ nullptr };
					std::vector<SkinningInstancingData> vecInstanceData;

					JobSkinnedBatch(const JobSkinned* pJob, const math::Matrix& matWorld, uint32_t nVTFID)
						: pJob(pJob)
					{
						vecInstanceData.emplace_back(matWorld, nVTFID);
					}
				};

				std::array<std::vector<JobSkinned>, GroupCount> m_vecJobSkinneds;
				std::array<size_t, GroupCount> m_nJobSkinnedCount;

				using UMapJobSkinnedBatch = tsl::robin_map<const void*, JobSkinnedBatch>;
				using UMapJobSkinnedMaterialBatch = tsl::robin_map<const IMaterial*, UMapJobSkinnedBatch>;
				UMapJobSkinnedMaterialBatch m_umapJobSkinnedMasterBatchs;
			};

			ModelRenderer::Impl::Impl()
			{
				m_strShaderPath = file::GetPath(file::eFx);
				m_strShaderPath.append("Model\\Model.hlsl");

				if (FAILED(D3DReadFileToBlob(string::MultiToWide(m_strShaderPath).c_str(), &m_pShaderBlob)))
				{
					throw_line("failed to read shader file : Model.hlsl");
				}

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				CreateVertexShader(pDevice, 0, "VS");
				CreateVertexShader(pDevice, shader::eUseSkinning, "VS");

				CreatePixelShader(pDevice, 0, "PS");
				CreatePixelShader(pDevice, shader::eUseSkinning, "PS");

				m_skinningInstancingDataBuffer.Create(pDevice, "SkinningInstancingDataBuffer");
				m_staticInstancingDataBuffer.Create(pDevice, "StaticInstancingDataBuffer");
				m_objectDataBuffer.Create(pDevice, "ObjectDataBuffer");
				m_vsConstants.Create(pDevice, "VSConstants");
				m_commonContentsBuffer.Create(pDevice, "CommonContents");

				for (int i = 0; i < GroupCount; ++i)
				{
					m_vecJobStatics[i].resize(512);
					m_vecJobSkinneds[i].resize(128);
				}

				m_umapJobStaticMasterBatchs.rehash(512);
				m_umapJobSkinnedMasterBatchs.rehash(128);
			}

			ModelRenderer::Impl::~Impl()
			{
				m_skinningInstancingDataBuffer.Destroy();
				m_staticInstancingDataBuffer.Destroy();
				m_objectDataBuffer.Destroy();
				m_vsConstants.Destroy();
				m_commonContentsBuffer.Destroy();

				for (auto iter = m_umapRenderStates.begin(); iter != m_umapRenderStates.end(); ++iter)
				{
					RenderState& renderState = iter.value();
					SafeRelease(renderState.pVertexShader);
					SafeRelease(renderState.pPixelShader);
					SafeRelease(renderState.pInputLayout);
				}
				m_umapRenderStates.clear();

				SafeRelease(m_pShaderBlob);
			}

			void ModelRenderer::Impl::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera, Group emGroup)
			{
				TRACER_EVENT(__FUNCTION__);
				DX_PROFILING(ModelRenderer);

				const bool isAlphaBlend = emGroup == Group::eAlphaBlend;
				if (m_nJobStaticCount[emGroup] == 0 && m_nJobSkinnedCount[emGroup] == 0)
					return;

				const OcclusionCulling* pOcclusionCulling = OcclusionCulling::GetInstance();
				if (emGroup == Group::eDeferred)
				{
					TRACER_EVENT("Culling");
					DX_PROFILING(Culling);

					const Collision::Frustum& frustum = pCamera->GetFrustum();
					concurrency::parallel_for(0llu, m_nJobStaticCount[emGroup], [&](size_t i)
					{
						JobStatic& job = m_vecJobStatics[emGroup][i];
						const OcclusionCullingData& occlusionCullingData = job.data.occlusionCullingData;
						//if (frustum.Contains(occlusionCullingData.aabb) == Collision::EmContainment::eDisjoint)
						//{
						//	job.isCulled = true;
						//	return;
						//}

						if (pOcclusionCulling->TestRect(occlusionCullingData.aabb) != OcclusionCulling::eVisible)
						{

							job.isCulled = true;
							return;
						}
					});

					concurrency::parallel_for(0llu, m_nJobSkinnedCount[emGroup], [&](size_t i)
					{
						JobSkinned& job = m_vecJobSkinneds[emGroup][i];
						const OcclusionCullingData& occlusionCullingData = job.data.occlusionCullingData;
						//if (frustum.Contains(occlusionCullingData.aabb) == Collision::EmContainment::eDisjoint)
						//{
						//	job.isCulled = true;
						//	return;
						//}

						if (pOcclusionCulling->TestRect(occlusionCullingData.aabb) != OcclusionCulling::eVisible)
						{
							job.isCulled = true;
							return;
						}
					});
				}

				Device* pDeviceInstance = Device::GetInstance();

				pDeviceContext->ClearState();

				const D3D11_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				pDeviceContext->RSSetViewports(1, pViewport);

				RenderTarget* pRenderTarget = nullptr;
				if (isAlphaBlend == true)
				{
					pRenderTarget = AlphaBlendGroupSetup(pDeviceInstance, pDeviceContext);
				}
				else
				{
					DeferredGroupSetup(pDeviceInstance, pDeviceContext);
				}

				{
					shader::VSConstants* pVSConstants = m_vsConstants.Map(pDeviceContext);
					pVSConstants->matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();
					pVSConstants->matViewProj = pVSConstants->matViewProj.Transpose();
					m_vsConstants.Unmap(pDeviceContext);
				}

				pDeviceContext->VSSetConstantBuffers(shader::eCB_VSConstants, 1, &m_vsConstants.pBuffer);

				pDeviceContext->VSSetConstantBuffers(shader::eCB_ObjectData, 1, &m_objectDataBuffer.pBuffer);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_ObjectData, 1, &m_objectDataBuffer.pBuffer);

				if (isAlphaBlend == true)
				{
					const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();
					shader::SetImageBasedLight(pDeviceInstance, pDeviceContext, pImageBasedLight);

					LightManager* pLightManager = LightManager::GetInstance();
					shader::SetCommonContents_ForAlpha(pDeviceContext, &m_commonContentsBuffer, pLightManager, pCamera->GetPosition(), 0);
					pDeviceContext->PSSetConstantBuffers(shader::eCB_CommonContents, 1, &m_commonContentsBuffer.pBuffer);

					ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointClamp);
					pDeviceContext->PSSetSamplers(shader::eSampler_PointClamp, 1, &pSamplerPointClamp);

					ID3D11SamplerState* pSamplerClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipLinearClamp);
					pDeviceContext->PSSetSamplers(shader::eSampler_Clamp, 1, &pSamplerClamp);
				}

				tsl::robin_map<const IMaterial*, uint32_t> umapMaterialMask;
				umapMaterialMask.rehash(m_umapJobStaticMasterBatchs.size() + m_umapJobSkinnedMasterBatchs.size());

				RenderStaticElement(pDeviceInstance, pDevice, pDeviceContext, pCamera, emGroup, umapMaterialMask);
				RenderSkinnedElement(pDeviceInstance, pDevice, pDeviceContext, pCamera, emGroup, umapMaterialMask);

				if (isAlphaBlend == true)
				{
					pDeviceInstance->ReleaseRenderTargets(&pRenderTarget, 1);
				}
			}

			void ModelRenderer::Impl::Cleanup()
			{
				m_nJobStaticCount.fill(0);
				m_nJobSkinnedCount.fill(0);

				m_umapJobStaticMasterBatchs.clear();
				m_umapJobSkinnedMasterBatchs.clear();
			}

			void ModelRenderer::Impl::PushJob(const RenderJobStatic& job)
			{
				Group emGroup;

				const IMaterial* pMaterial = job.pMaterial;
				if (pMaterial == nullptr || pMaterial->GetBlendState() == EmBlendState::eOff)
				{
					emGroup = eDeferred;
				}
				else
				{
					emGroup = eAlphaBlend;
				}

				const thread::SRWWriteLock writeLock(&m_srwLock);

				const size_t nIndex = m_nJobStaticCount[emGroup];
				if (nIndex >= m_vecJobStatics[emGroup].size())
				{
					m_vecJobStatics[emGroup].resize(m_vecJobStatics[emGroup].size() * 2);
				}

				m_vecJobStatics[emGroup][nIndex].Set(job);
				++m_nJobStaticCount[emGroup];
			}

			void ModelRenderer::Impl::PushJob(const RenderJobSkinned& job)
			{
				Group emGroup;

				const IMaterial* pMaterial = job.pMaterial;
				if (pMaterial == nullptr || pMaterial->GetBlendState() == EmBlendState::eOff)
				{
					emGroup = eDeferred;
				}
				else
				{
					emGroup = eAlphaBlend;
				}

				thread::SRWWriteLock writeLock(&m_srwLock);

				const size_t nIndex = m_nJobSkinnedCount[emGroup];
				if (nIndex >= m_vecJobSkinneds[emGroup].size())
				{
					m_vecJobSkinneds[emGroup].resize(m_vecJobSkinneds[emGroup].size() * 2);
				}

				m_vecJobSkinneds[emGroup][nIndex].Set(job);
				++m_nJobSkinnedCount[emGroup];
			}

			void ModelRenderer::Impl::DeferredGroupSetup(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext)
			{
				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();
				ID3D11RenderTargetView* pRTV[] =
				{
					pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetRenderTargetView(),
					pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetRenderTargetView(),
					pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetRenderTargetView(),
				};
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, pGBuffer->GetDepthStencil()->GetDepthStencilView());
			}

			RenderTarget* ModelRenderer::Impl::AlphaBlendGroupSetup(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext)
			{
				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();

				D3D11_TEXTURE2D_DESC desc{};
				pDeviceInstance->GetSwapChainRenderTarget()->GetDesc2D(&desc);
				desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				if (GetOptions().OnHDR == true)
				{
					desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}

				RenderTarget* pRenderTarget = pDeviceInstance->GetLastUsedRenderTarget();
				if (pRenderTarget != nullptr)
				{
					const RenderTarget::Key key = RenderTarget::BuildKey(&desc);
					if (pRenderTarget->GetKey() != key)
					{
						pRenderTarget = nullptr;
					}
				}

				if (pRenderTarget == nullptr)
				{
					pRenderTarget = pDeviceInstance->GetRenderTarget(&desc);
				}

				ID3D11RenderTargetView* pRTV[] =
				{
					pRenderTarget->GetRenderTargetView(),
				};
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, pGBuffer->GetDepthStencil()->GetDepthStencilView());

				return pRenderTarget;
			}

			void ModelRenderer::Impl::RenderStaticElement(Device* pDeviceInstance, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera, Group emGroup, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				DX_PROFILING(RenderStaticElement);

				m_umapJobStaticMasterBatchs.clear();
				{
					for (size_t i = 0; i < m_nJobStaticCount[emGroup]; ++i)
					{
						JobStatic& job = m_vecJobStatics[emGroup][i];
						if (job.isCulled == true)
							continue;

						UMapJobStaticBatch& umapJobStaticBatch = m_umapJobStaticMasterBatchs[job.data.pMaterial];

						auto iter = umapJobStaticBatch.find(job.data.pKey);
						if (iter != umapJobStaticBatch.end())
						{
							iter.value().vecInstanceData.emplace_back(job.data.matWorld.Transpose());
						}
						else
						{
							umapJobStaticBatch.emplace(job.data.pKey, JobStaticBatch(&job, job.data.matWorld.Transpose()));
						}
					}

					tsl::robin_map<shader::MaskKey, std::vector<const JobStaticBatch*>> umapJobStaticMaskBatch;
					umapJobStaticMaskBatch.rehash(m_umapJobStaticMasterBatchs.size());

					for (auto& iter_master : m_umapJobStaticMasterBatchs)
					{
						const UMapJobStaticBatch& umapJobStaticBatch = iter_master.second;

						for (auto& iter : umapJobStaticBatch)
						{
							const JobStaticBatch& jobBatch = iter.second;

							uint32_t nMask = 0;
							auto iter_find = umapMaterialMask.find(jobBatch.pJob->data.pMaterial);
							if (iter_find != umapMaterialMask.end())
							{
								nMask = iter_find->second;
							}
							else
							{
								nMask = shader::GetMaterialMask(jobBatch.pJob->data.pMaterial);
								umapMaterialMask.emplace(jobBatch.pJob->data.pMaterial, nMask);
							}

							if (jobBatch.vecInstanceData.size() > 1)
							{
								nMask |= shader::eUseInstancing;
							}

							if (emGroup == Group::eAlphaBlend)
							{
								nMask |= shader::eUseAlphaBlending;
							}

							shader::MaskKey maskKey(nMask);
							umapJobStaticMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					for (auto iter = umapJobStaticMaskBatch.begin(); iter != umapJobStaticMaskBatch.end(); ++iter)
					{
						std::vector<const JobStaticBatch*>& vecJobBatch = iter.value();

						const RenderState* pRenderState = GetRenderState(pDevice, iter->first);
						if (pRenderState == nullptr)
							continue;

						pDeviceContext->VSSetShader(pRenderState->pVertexShader, nullptr, 0);
						pDeviceContext->PSSetShader(pRenderState->pPixelShader, nullptr, 0);

						pDeviceContext->IASetInputLayout(pRenderState->pInputLayout);
						pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

						if ((iter->first & shader::eUseInstancing) == shader::eUseInstancing)
						{
							pDeviceContext->VSSetConstantBuffers(shader::eCB_StaticInstancingData, 1, &m_staticInstancingDataBuffer.pBuffer);

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobStatic& job = pJobBatch->pJob->data;

								shader::SetObjectData(pDeviceContext, &m_objectDataBuffer, job.pMaterial, math::Matrix::Identity, 0);

								if (emGroup == eDeferred)
								{
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_Deferred);

									shader::DrawInstance(pDeviceContext,
										&m_staticInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount,
										pJobBatch->vecInstanceData.data(), pJobBatch->vecInstanceData.size());
								}
								else if (emGroup == eAlphaBlend)
								{
									// Pre
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_AlphaBlend_Pre);

									shader::DrawInstance(pDeviceContext,
										&m_staticInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount,
										pJobBatch->vecInstanceData.data(), pJobBatch->vecInstanceData.size());

									// Post
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_AlphaBlend_Post);

									shader::DrawInstance(pDeviceContext,
										&m_staticInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount,
										pJobBatch->vecInstanceData.data(), pJobBatch->vecInstanceData.size());
								}
							}
						}
						else
						{
							std::sort(vecJobBatch.begin(), vecJobBatch.end(), [](const JobStaticBatch* a, const JobStaticBatch* b)
							{
								return a->pJob->data.fDepth < b->pJob->data.fDepth;
							});

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobStatic& job = pJobBatch->pJob->data;

								shader::SetObjectData(pDeviceContext, &m_objectDataBuffer, job.pMaterial, job.matWorld, 0);

								if (emGroup == eDeferred)
								{
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_Deferred);

									shader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount);
								}
								else if (emGroup == eAlphaBlend)
								{
									// Pre
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_AlphaBlend_Pre);

									shader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount);

									// Post
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_AlphaBlend_Post);

									shader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount);
								}
							}
						}
					}
				}
				m_umapJobStaticMasterBatchs.clear();
			}

			void ModelRenderer::Impl::RenderSkinnedElement(Device* pDeviceInstance, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera, Group emGroup, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				DX_PROFILING(RenderSkinnedElement);

				VTFManager* pVTFManager = pDeviceInstance->GetVTFManager();
				Texture* pVTFTexture = pVTFManager->GetTexture();

				m_umapJobSkinnedMasterBatchs.clear();
				if (pVTFTexture != nullptr)
				{
					for (size_t i = 0; i < m_nJobSkinnedCount[emGroup]; ++i)
					{
						JobSkinned& job = m_vecJobSkinneds[emGroup][i];
						if (job.isCulled == true)
							continue;

						UMapJobSkinnedBatch& umapJobSkinnedBatch = m_umapJobSkinnedMasterBatchs[job.data.pMaterial];

						auto iter = umapJobSkinnedBatch.find(job.data.pKey);
						if (iter != umapJobSkinnedBatch.end())
						{
							iter.value().vecInstanceData.emplace_back(job.data.matWorld, job.data.nVTFID);
						}
						else
						{
							umapJobSkinnedBatch.emplace(job.data.pKey, JobSkinnedBatch(&job, job.data.matWorld, job.data.nVTFID));
						}
					}

					tsl::robin_map<shader::MaskKey, std::vector<const JobSkinnedBatch*>> umapJobSkinnedMaskBatch;
					umapJobSkinnedMaskBatch.rehash(m_umapJobSkinnedMasterBatchs.size());

					for (auto& iter_master : m_umapJobSkinnedMasterBatchs)
					{
						const UMapJobSkinnedBatch& umapJobSkinnedBatch = iter_master.second;

						for (auto& iter : umapJobSkinnedBatch)
						{
							const JobSkinnedBatch& jobBatch = iter.second;

							uint32_t nMask = 0;
							auto iter_find = umapMaterialMask.find(jobBatch.pJob->data.pMaterial);
							if (iter_find != umapMaterialMask.end())
							{
								nMask = iter_find->second;
							}
							else
							{
								shader::MaskKey maskKey = shader::GetMaterialMask(jobBatch.pJob->data.pMaterial);
								umapMaterialMask.emplace(jobBatch.pJob->data.pMaterial, maskKey);
								nMask = maskKey;
							}

							if (jobBatch.vecInstanceData.size() > 1)
							{
								nMask |= shader::eUseInstancing;
							}

							if (emGroup == Group::eAlphaBlend)
							{
								nMask |= shader::eUseAlphaBlending;
							}

							nMask |= shader::eUseSkinning;

							shader::MaskKey maskKey(nMask);
							umapJobSkinnedMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					ID3D11ShaderResourceView* pSRVs[] =
					{
						pVTFTexture->GetShaderResourceView(),
					};
					pDeviceContext->VSSetShaderResources(shader::eSRV_VTF, _countof(pSRVs), pSRVs);

					for (auto iter = umapJobSkinnedMaskBatch.begin(); iter != umapJobSkinnedMaskBatch.end(); ++iter)
					{
						std::vector<const JobSkinnedBatch*>& vecJobBatch = iter.value();

						const RenderState* pRenderState = GetRenderState(pDevice, iter->first);
						if (pRenderState == nullptr)
							continue;

						pDeviceContext->VSSetShader(pRenderState->pVertexShader, nullptr, 0);
						pDeviceContext->PSSetShader(pRenderState->pPixelShader, nullptr, 0);

						pDeviceContext->IASetInputLayout(pRenderState->pInputLayout);
						pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

						if ((iter->first & shader::eUseInstancing) == shader::eUseInstancing)
						{
							pDeviceContext->VSSetConstantBuffers(shader::eCB_SkinningInstancingData, 1, &m_skinningInstancingDataBuffer.pBuffer);

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobSkinned& job = pJobBatch->pJob->data;

								shader::SetObjectData(pDeviceContext, &m_objectDataBuffer, job.pMaterial, math::Matrix::Identity, 0);

								if (emGroup == eDeferred)
								{
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_Deferred);

									shader::DrawInstance(pDeviceContext,
										&m_skinningInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount,
										pJobBatch->vecInstanceData.data(), pJobBatch->vecInstanceData.size());
								}
								else if (emGroup == eAlphaBlend)
								{
									// Pre
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_AlphaBlend_Pre);

									shader::DrawInstance(pDeviceContext,
										&m_skinningInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount,
										pJobBatch->vecInstanceData.data(), pJobBatch->vecInstanceData.size());

									// Post
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_AlphaBlend_Post);

									shader::DrawInstance(pDeviceContext,
										&m_skinningInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount,
										pJobBatch->vecInstanceData.data(), pJobBatch->vecInstanceData.size());
								}
							}
						}
						else
						{
							std::sort(vecJobBatch.begin(), vecJobBatch.end(), [](const JobSkinnedBatch* a, const JobSkinnedBatch* b)
							{
								return a->pJob->data.fDepth < b->pJob->data.fDepth;
							});

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobSkinned& job = pJobBatch->pJob->data;

								shader::SetObjectData(pDeviceContext, &m_objectDataBuffer, job.pMaterial, job.matWorld, job.nVTFID);

								if (emGroup == eDeferred)
								{
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_Deferred);

									shader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount);
								}
								else if (emGroup == eAlphaBlend)
								{
									// Pre
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_AlphaBlend_Pre);

									shader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount);

									// Post
									shader::SetMaterial(pDeviceInstance, pDeviceContext, job.pMaterial, emGroup, shader::ePass_AlphaBlend_Post);

									shader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer),
										static_cast<const IndexBuffer*>(job.pIndexBuffer),
										job.nStartIndex, job.nIndexCount);
								}
							}
						}
					}
				}
				m_umapJobSkinnedMasterBatchs.clear();
			}

			const ModelRenderer::Impl::RenderState* ModelRenderer::Impl::GetRenderState(ID3D11Device* pDevice, const shader::MaskKey& maskKey)
			{
				const RenderState* pRenderState = nullptr;

				auto iter = m_umapRenderStates.find(maskKey);
				if (iter != m_umapRenderStates.end())
				{
					pRenderState = &iter->second;
				}
				else
				{
					CreateVertexShader(pDevice, maskKey, "VS");
					CreatePixelShader(pDevice, maskKey, "PS");

					iter = m_umapRenderStates.find(maskKey);
					if (iter != m_umapRenderStates.end())
					{
						pRenderState = &iter->second;
					}
				}

				if (pRenderState == nullptr || pRenderState->pInputLayout == nullptr || pRenderState->pVertexShader == nullptr || pRenderState->pPixelShader == nullptr)
					return nullptr;

				return pRenderState;
			}

			void ModelRenderer::Impl::CreateVertexShader(ID3D11Device* pDevice, const shader::MaskKey& maskKey, const char* pFunctionName)
			{
				const std::vector<D3D_SHADER_MACRO> vecMacros = shader::GetMacros(maskKey);

				const D3D11_INPUT_ELEMENT_DESC* pInputElements = nullptr;
				size_t nElementCount = 0;

				if ((maskKey & shader::eUseSkinning) == shader::eUseSkinning)
				{
					util::GetInputElementDesc(VertexPosTexNorWeiIdx::Format(), &pInputElements, &nElementCount);
				}
				else
				{
					util::GetInputElementDesc(VertexPosTexNor::Format(), &pInputElements, &nElementCount);
				}

				if (pInputElements == nullptr || nElementCount == 0)
				{
					throw_line("invalid vertex shader input elements");
				}

				const std::string strDebugName = string::Format("ModelVertexShader : %u", maskKey);

				ID3D11VertexShader* pVertexShader = nullptr;
				ID3D11InputLayout* pInputLayout = nullptr;

				if (util::CreateVertexShader(pDevice, m_pShaderBlob, vecMacros.data(), m_strShaderPath.c_str(), pFunctionName, shader::VS_CompileVersion, &pVertexShader, pInputElements, nElementCount, &pInputLayout, strDebugName.c_str()) == false)
				{
					LOG_ERROR("failed to create vertex shader : %u", maskKey);
					return;
				}

				m_umapRenderStates[maskKey].pVertexShader = pVertexShader;
				m_umapRenderStates[maskKey].pInputLayout = pInputLayout;
			}

			void ModelRenderer::Impl::CreatePixelShader(ID3D11Device* pDevice, const shader::MaskKey& maskKey, const char* pFunctionName)
			{
				const std::vector<D3D_SHADER_MACRO> vecMacros = shader::GetMacros(maskKey);

				const std::string strDebugName = string::Format("ModelPixelShader : %u", maskKey);

				ID3D11PixelShader* pPixelShader = nullptr;
				if (util::CreatePixelShader(pDevice, m_pShaderBlob, vecMacros.data(), m_strShaderPath.c_str(), pFunctionName, shader::PS_CompileVersion, &pPixelShader, strDebugName.c_str()) == false)
				{
					LOG_ERROR("failed to create pixel shader : %u", maskKey);
					return;
				}

				m_umapRenderStates[maskKey].pPixelShader = pPixelShader;
			}

			ModelRenderer::ModelRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			ModelRenderer::~ModelRenderer()
			{
			}

			void ModelRenderer::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera, Group emGroup)
			{
				m_pImpl->Render(pDevice, pDeviceContext, pCamera, emGroup);
			}

			void ModelRenderer::Cleanup()
			{
				m_pImpl->Cleanup();
			}

			void ModelRenderer::PushJob(const RenderJobStatic& job)
			{
				m_pImpl->PushJob(job);
			}

			void ModelRenderer::PushJob(const RenderJobSkinned& job)
			{
				m_pImpl->PushJob(job);
			}
		}
	}
}