#include "stdafx.h"
#include "ModelRendererDX12.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Lock.h"
#include "CommonLib/ThreadPool.h"
#include "CommonLib/Timer.h"

#include "GraphicsInterface/Instancing.h"
#include "GraphicsInterface/Camera.h"
#include "GraphicsInterface/LightManager.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "GBufferDX12.h"
#include "VTFManagerDX12.h"
#include "DescriptorHeapDX12.h"

#include "VertexBufferDX12.h"
#include "IndexBufferDX12.h"
#include "TextureDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				enum
				{
					eMaxJobCount = 1 << 16,
					eMaxInstancingJobCount = 1024,
				};

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

					math::Vector4 f4PaddingRoughMetEmi;
					math::Vector4 f4SurSpecTintAniso;
					math::Vector4 f4SheenTintClearcoatGloss;

					float fStippleTransparencyFactor{ 0.f };
					uint32_t nVTFID{ 0 };

					math::Vector2 f2Padding;
				};

				struct VSConstantsBuffer
				{
					math::Matrix matViewProj;

					uint32_t nTexVTFIndex;
					math::Vector3 padding;
				};

				struct SRVIndexConstants
				{
					uint32_t nTexAlbedoIndex{ 0 };
					uint32_t nTexMaskIndex{ 0 };
					uint32_t nTexNormalMapIndex{ 0 };
					uint32_t nTexRoughnessIndex{ 0 };
					uint32_t nTexMetallicIndex{ 0 };
					uint32_t nTexEmissiveIndex{ 0 };
					uint32_t nTexEmissiveColorIndex{ 0 };
					uint32_t nTexSubsurfaceIndex{ 0 };
					uint32_t nTexSpecularIndex{ 0 };
					uint32_t nTexSpecularTintIndex{ 0 };
					uint32_t nTexAnisotropicIndex{ 0 };
					uint32_t nTexSheenIndex{ 0 };
					uint32_t nTexSheenTintIndex{ 0 };
					uint32_t nTexClearcoatIndex{ 0 };
					uint32_t nTexClearcoatGlossIndex{ 0 };
					uint32_t nSamplerStateIndex{ 0 };
				};

				struct CommonContents
				{
					math::Vector3 f3CameraPos;
					int nEnableShadowCount{ 0 };

					uint32_t nTexDiffuseHDRIndex{ 0 };
					uint32_t nTexSpecularHDRIndex{ 0 };
					uint32_t nTexSpecularBRDFIndex{ 0 };
					uint32_t nTexShadowMapIndex{ 0 };

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

					eCB_SRVIndex,

					eCB_CommonContents = 5,
				};

				enum Pass
				{
					ePass_Deferred = 0,
					ePass_AlphaBlend_Pre,
					ePass_AlphaBlend_Post,
				};

				enum Mask : uint32_t
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

				const char* GetMaskName(uint32_t nMask)
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
						"USE_ALPHABLENDING",

						"USE_WRITEDEPTH",
						"USE_CUBEMAP",
						"USE_TESSELLATION",
						"USE_ALBEDO_ALPHA_IS_MASK_MAP",
					};

					return s_strMaskName[nMask].c_str();
				}

				std::vector<D3D_SHADER_MACRO> GetMacros(uint32_t nMask)
				{
					std::vector<D3D_SHADER_MACRO> vecMacros;
					vecMacros.push_back({ "DX12", "1" });
					for (uint32_t i = 0; i < shader::MaskCount; ++i)
					{
						if ((nMask & (1llu << i)) != 0)
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
				};

				uint32_t GetMaterialMask(const IMaterial* pMaterial)
				{
					uint32_t nMask = 0;
					if (pMaterial != nullptr)
					{
						nMask |= IsValidTexture(pMaterial, EmMaterial::eAlbedo) ? eUseTexAlbedo : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eMask) ? eUseTexMask : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eNormal) ? eUseTexNormal : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eRoughness) ? eUseTexRoughness : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eMetallic) ? eUseTexMetallic : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eEmissive) ? eUseTexEmissive : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eEmissiveColor) ? eUseTexEmissiveColor : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eSubsurface) ? eUseTexSubsurface : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eSpecular) ? eUseTexSpecular : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eSpecularTint) ? eUseTexSpecularTint : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eAnisotropic) ? eUseTexAnisotropic : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eSheen) ? eUseTexSheen : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eSheenTint) ? eUseTexSheenTint : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eClearcoat) ? eUseTexClearcoat : 0;
						nMask |= IsValidTexture(pMaterial, EmMaterial::eClearcoatGloss) ? eUseTexClearcoatGloss : 0;
					}

					return nMask;
				}

				PSOKey GetPSOKey(ModelRenderer::Group emGroup, Pass emPass, uint32_t nMask, const IMaterial* pMaterial)
				{
					EmRasterizerState::Type emRasterizerState = EmRasterizerState::eSolidCCW;
					EmBlendState::Type emBlendState = EmBlendState::eOff;
					EmDepthStencilState::Type emDepthStencilState = EmDepthStencilState::eRead_Write_On;

					if (pMaterial != nullptr)
					{
						emBlendState = pMaterial->GetBlendState();
						emRasterizerState = pMaterial->GetRasterizerState();
						emDepthStencilState = pMaterial->GetDepthStencilState();

						if (emGroup == ModelRenderer::eAlphaBlend &&
							emPass == shader::ePass_AlphaBlend_Pre)
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
					}

					return { nMask, emRasterizerState, emBlendState, emDepthStencilState };
				}

				void SetObjectData(ObjectDataBuffer* pObjectDataBuffer,
					const IMaterial* pMaterial, const math::Matrix& matWorld, uint32_t nVTFID = 0)
				{
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

						pObjectDataBuffer->f4PaddingRoughMetEmi = math::Vector4::Zero;
						pObjectDataBuffer->f4SurSpecTintAniso = math::Vector4::Zero;
						pObjectDataBuffer->f4SheenTintClearcoatGloss = math::Vector4::Zero;

						pObjectDataBuffer->fStippleTransparencyFactor = 0.f;
						pObjectDataBuffer->nVTFID = nVTFID;

						pObjectDataBuffer->f2Padding = {};
					}
				}

				void SetSRVIndex(const IMaterial* pMaterial, EmMaterial::Type emType, uint32_t* pSRVIndex_out)
				{
					if (IsValidTexture(pMaterial, emType) == true)
					{
						Texture* pTexture = static_cast<Texture*>(pMaterial->GetTexture(emType));
						*pSRVIndex_out = pTexture->GetDescriptorIndex();
					}
					else
					{
						*pSRVIndex_out = eInvalidDescriptorIndex;
					}
				}

				void SetMaterial(SRVIndexConstants* pSRVIndexContantBuffer, const IMaterial* pMaterial)
				{
					SetSRVIndex(pMaterial, EmMaterial::eAlbedo, &pSRVIndexContantBuffer->nTexAlbedoIndex);
					SetSRVIndex(pMaterial, EmMaterial::eMask, &pSRVIndexContantBuffer->nTexMaskIndex);
					SetSRVIndex(pMaterial, EmMaterial::eNormal, &pSRVIndexContantBuffer->nTexNormalMapIndex);
					SetSRVIndex(pMaterial, EmMaterial::eRoughness, &pSRVIndexContantBuffer->nTexRoughnessIndex);
					SetSRVIndex(pMaterial, EmMaterial::eMetallic, &pSRVIndexContantBuffer->nTexMetallicIndex);
					SetSRVIndex(pMaterial, EmMaterial::eEmissive, &pSRVIndexContantBuffer->nTexEmissiveIndex);
					SetSRVIndex(pMaterial, EmMaterial::eEmissiveColor, &pSRVIndexContantBuffer->nTexEmissiveColorIndex);
					SetSRVIndex(pMaterial, EmMaterial::eSubsurface, &pSRVIndexContantBuffer->nTexSubsurfaceIndex);
					SetSRVIndex(pMaterial, EmMaterial::eSpecular, &pSRVIndexContantBuffer->nTexSpecularIndex);
					SetSRVIndex(pMaterial, EmMaterial::eSpecularTint, &pSRVIndexContantBuffer->nTexSpecularTintIndex);
					SetSRVIndex(pMaterial, EmMaterial::eAnisotropic, &pSRVIndexContantBuffer->nTexAnisotropicIndex);
					SetSRVIndex(pMaterial, EmMaterial::eSheen, &pSRVIndexContantBuffer->nTexSheenIndex);
					SetSRVIndex(pMaterial, EmMaterial::eSheenTint, &pSRVIndexContantBuffer->nTexSheenTintIndex);
					SetSRVIndex(pMaterial, EmMaterial::eClearcoat, &pSRVIndexContantBuffer->nTexClearcoatIndex);
					SetSRVIndex(pMaterial, EmMaterial::eClearcoatGloss, &pSRVIndexContantBuffer->nTexClearcoatGlossIndex);

					if (pMaterial != nullptr)
					{
						pSRVIndexContantBuffer->nSamplerStateIndex = pMaterial->GetSamplerState();
					}
					else
					{
						pSRVIndexContantBuffer->nSamplerStateIndex = EmSamplerState::eMinMagMipLinearWrap;
					}
				}

				void SetCommonContents_ForAlpha(CommonContents* pCommonContents,
					const IImageBasedLight* pImageBasedLight, const LightManager* pLightManager, const math::Vector3& f3CameraPos, int nEnableShadowCount)
				{
					pCommonContents->f3CameraPos = f3CameraPos;
					pCommonContents->nEnableShadowCount = nEnableShadowCount;

					Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR());
					pCommonContents->nTexDiffuseHDRIndex = pDiffuseHDR->GetDescriptorIndex();

					Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR());
					pCommonContents->nTexSpecularHDRIndex = pSpecularHDR->GetDescriptorIndex();

					Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF());
					pCommonContents->nTexSpecularBRDFIndex = pSpecularBRDF->GetDescriptorIndex();

					const DirectionalLightData* pDirectionalLightData = nullptr;
					pLightManager->GetDirectionalLightData(&pDirectionalLightData, &pCommonContents->nDirectionalLightCount);
					Memory::Copy(pCommonContents->lightDirectional, pDirectionalLightData, sizeof(DirectionalLightData) * pCommonContents->nDirectionalLightCount);

					const PointLightData* pPointLightData = nullptr;
					pLightManager->GetPointLightData(&pPointLightData, &pCommonContents->nPointLightCount);
					Memory::Copy(pCommonContents->lightPoint, pPointLightData, sizeof(PointLightData) * pCommonContents->nPointLightCount);

					const SpotLightData* pSpotLightData = nullptr;
					pLightManager->GetSpotLightData(&pSpotLightData, &pCommonContents->nSpotLightCount);
					Memory::Copy(pCommonContents->lightSpot, pSpotLightData, sizeof(SpotLightData) * pCommonContents->nSpotLightCount);
				}
			}

			class ModelRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(Camera* pCamera, Group emGroup);
				void Flush();

			public:
				void PushJob(const RenderJobStatic& job);
				void PushJob(const RenderJobSkinned& job);

			private:
				void RenderStaticElement(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, Group emGroup, shader::Pass emPass, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t nRTVHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, std::unordered_map<const IMaterial*, uint32_t>& umapMaterialMask);
				void RenderSkinnedElement(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, Group emGroup, shader::Pass emPass, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t nRTVHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, std::unordered_map<const IMaterial*, uint32_t>& umapMaterialMask);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,
					eRP_SamplerStates,
					eRP_SRVIndicesCB,

					eRP_VSConstantsCB,
					eRP_ObjectDataCB,
					eRP_SkinningInstancingDataCB,
					eRP_StaticInstancingDataCB,

					eRP_CommonContentsCB,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				struct RenderPipeline
				{
					ID3D12PipelineState* pPipelineState{ nullptr };
					ID3D12RootSignature* pRootSignature{ nullptr };

					std::array<uint32_t, eRP_Count> nRootParameterIndex;
				};
				const RenderPipeline* GetRenderPipeline(ID3D12Device* pDevice, const shader::PSOKey& psoKey);

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice, uint32_t nMask, std::array<uint32_t, eRP_Count>& nRootParameterIndex_out);
				void CreatePipelineState(ID3D12Device* pDevice, uint32_t nMask, EmRasterizerState::Type emRasterizerState, EmBlendState::Type emBlendState, EmDepthStencilState::Type emDepthStencilState);

			private:
				thread::SRWLock m_job_srwLock;
				thread::SRWLock m_pipelines_srwLock;
				thread::SRWLock m_logic_srwLock;

				std::string m_strShaderPath;
				ID3DBlob* m_pShaderBlob{ nullptr };
				
				std::unordered_map<shader::PSOKey, RenderPipeline> m_umapRenderPipelines;

				std::atomic<size_t> m_nSkinningBufferIndex{ 0 };
				std::atomic<size_t> m_nStaticBufferIndex{ 0 };
				std::atomic<size_t> m_nObjectBufferIndex{ 0 };
				std::atomic<size_t> m_nSrvBufferIndex{ 0 };

				ConstantBuffer<shader::SkinningInstancingDataBuffer> m_skinningInstancingDataBuffer;
				ConstantBuffer<shader::StaticInstancingDataBuffer> m_staticInstancingDataBuffer;
				ConstantBuffer<shader::ObjectDataBuffer> m_objectDataBuffer;
				ConstantBuffer<shader::SRVIndexConstants> m_srvIndexConstantsBuffer;

				ConstantBuffer<shader::VSConstantsBuffer> m_vsConstantsBuffer;
				ConstantBuffer<shader::CommonContents> m_commonContentsBuffer;
				
				struct JobStatic
				{
					std::pair<const void*, const IMaterial*> pairKey;
					RenderJobStatic data;
					bool isCulling{ false };

					void Set(const RenderJobStatic& source)
					{
						pairKey = std::make_pair(source.pKey, source.pMaterial);
						data = source;
						isCulling = false;
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

				std::array<std::array<JobStatic, shader::eMaxJobCount>, GroupCount> m_vecJobStatics;
				std::array<size_t, GroupCount> m_nJobStaticCount;

				using UMapJobStaticBatch = std::unordered_map<const void*, JobStaticBatch>;
				using UMapJobStaticMaterialBatch = std::unordered_map<const IMaterial*, UMapJobStaticBatch>;
				UMapJobStaticMaterialBatch m_umapJobStaticMasterBatchs;

				struct JobSkinned
				{
					std::pair<const void*, const IMaterial*> pairKey;
					RenderJobSkinned data;
					bool isCulling{ false };

					void Set(const RenderJobSkinned& source)
					{
						pairKey = std::make_pair(source.pKey, source.pMaterial);
						data = source;
						isCulling = false;
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

				std::array<std::array<JobSkinned, shader::eMaxJobCount>, GroupCount> m_vecJobSkinneds;
				std::array<size_t, GroupCount> m_nJobSkinnedCount;

				using UMapJobSkinnedBatch = std::unordered_map<const void*, JobSkinnedBatch>;
				using UMapJobSkinnedMaterialBatch = std::unordered_map<const IMaterial*, UMapJobSkinnedBatch>;
				UMapJobSkinnedMaterialBatch m_umapJobSkinnedMasterBatchs;
			};

			ModelRenderer::Impl::Impl()
			{
				m_strShaderPath = file::GetPath(file::eFx);
				m_strShaderPath.append("Model\\Model.hlsl");

				if (FAILED(D3DReadFileToBlob(String::MultiToWide(m_strShaderPath).c_str(), &m_pShaderBlob)))
				{
					throw_line("failed to read shader file : Model.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreatePipelineState(pDevice, 0, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
				CreatePipelineState(pDevice, shader::eUseInstancing, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
				CreatePipelineState(pDevice, shader::eUseAlphaBlending, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
				CreatePipelineState(pDevice, shader::eUseInstancing | shader::eUseAlphaBlending, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);

				CreatePipelineState(pDevice, shader::eUseSkinning, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
				CreatePipelineState(pDevice, shader::eUseSkinning | shader::eUseInstancing, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
				CreatePipelineState(pDevice, shader::eUseSkinning | shader::eUseAlphaBlending, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
				CreatePipelineState(pDevice, shader::eUseSkinning | shader::eUseInstancing | shader::eUseAlphaBlending, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);

				m_skinningInstancingDataBuffer.Create(pDevice, shader::eMaxInstancingJobCount, "SkinningInstancingDataBuffer");
				m_staticInstancingDataBuffer.Create(pDevice, shader::eMaxInstancingJobCount, "StaticInstancingDataBuffer");
				m_objectDataBuffer.Create(pDevice, shader::eMaxJobCount, "ObjectDataBuffer");
				m_srvIndexConstantsBuffer.Create(pDevice, shader::eMaxJobCount, "SRVIndexConstantsBuffer");
				m_vsConstantsBuffer.Create(pDevice, 1, "VSConstantsBuffer");
				m_commonContentsBuffer.Create(pDevice, 1, "CommonContentsBuffer");

				m_umapJobStaticMasterBatchs.rehash(512);
				m_umapJobSkinnedMasterBatchs.rehash(128);
			}

			ModelRenderer::Impl::~Impl()
			{
				m_skinningInstancingDataBuffer.Destroy();
				m_staticInstancingDataBuffer.Destroy();
				m_objectDataBuffer.Destroy();
				m_vsConstantsBuffer.Destroy();
				m_srvIndexConstantsBuffer.Destroy();
				m_commonContentsBuffer.Destroy();

				std::for_each(m_umapRenderPipelines.begin(), m_umapRenderPipelines.end(), [](std::pair<const shader::PSOKey, RenderPipeline>& iter)
				{
					RenderPipeline& renderPipeline = iter.second;
					SafeRelease(renderPipeline.pPipelineState);
					SafeRelease(renderPipeline.pRootSignature);
				});
				m_umapRenderPipelines.clear();

				SafeRelease(m_pShaderBlob);
			}

			void ModelRenderer::Impl::Render(Camera* pCamera, Group emGroup)
			{
				const bool isAlphaBlend = emGroup == Group::eAlphaBlend;

				if (isAlphaBlend == true && m_nJobStaticCount[emGroup] == 0 && m_nJobSkinnedCount[emGroup] == 0)
					return;

				Device* pDeviceInstance = Device::GetInstance();
				ID3D12Device* pDevice = pDeviceInstance->GetInterface();

				int nFrameIndex = pDeviceInstance->GetFrameIndex();
				GBuffer* pGBuffer = pDeviceInstance->GetGBuffer(nFrameIndex);

				DepthStencil* pDepthStencil = pGBuffer->GetDepthStencil();

				RenderTarget* pRenderTarget = nullptr;
				std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> vecRTVHandles;
				if (isAlphaBlend == true)
				{
					D3D12_RESOURCE_DESC desc = pDeviceInstance->GetSwapChainRenderTarget(nFrameIndex)->GetDesc();

					if (GetOptions().OnHDR == true)
					{
						desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					}

					pRenderTarget = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent, true);

					vecRTVHandles.emplace_back(pRenderTarget->GetCPUHandle());
				}
				else
				{
					const RenderTarget* pNormalsRT = pGBuffer->GetRenderTarget(EmGBuffer::eNormals);
					const RenderTarget* pColorsRT = pGBuffer->GetRenderTarget(EmGBuffer::eColors);
					const RenderTarget* pDisneyBRDFRT = pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF);

					vecRTVHandles = 
					{
						pNormalsRT->GetCPUHandle(),
						pColorsRT->GetCPUHandle(),
						pDisneyBRDFRT->GetCPUHandle(),
					};
				}
				const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = pDepthStencil->GetCPUHandle();

				{
					VTFManager* pVTFManager = pDeviceInstance->GetVTFManager();
					Texture* pVTFTexture = pVTFManager->GetTexture();

					shader::VSConstantsBuffer* pVSConstantsBuffer = m_vsConstantsBuffer.Cast(nFrameIndex);
					pVSConstantsBuffer->matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();
					pVSConstantsBuffer->matViewProj = pVSConstantsBuffer->matViewProj.Transpose();

					pVSConstantsBuffer->nTexVTFIndex = pVTFTexture->GetDescriptorIndex();
				}

				std::unordered_map<const IMaterial*, uint32_t> umapMaterialMask;
				umapMaterialMask.rehash(m_umapJobStaticMasterBatchs.size() + m_umapJobSkinnedMasterBatchs.size());

				if (isAlphaBlend == true)
				{
					const LightManager* pLightManager = LightManager::GetInstance();
					const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();

					shader::CommonContents* pCommonContents = m_commonContentsBuffer.Cast(nFrameIndex);
					shader::SetCommonContents_ForAlpha(pCommonContents, pImageBasedLight, pLightManager, pCamera->GetPosition(), 0);

					if (pRenderTarget->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET ||
						pDepthStencil->GetResourceState() != D3D12_RESOURCE_STATE_DEPTH_WRITE)
					{
						ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
						pDeviceInstance->ResetCommandList(0, nullptr);

						if (pRenderTarget->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
						{
							const D3D12_RESOURCE_BARRIER transition[] =
							{
								pRenderTarget->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET),
							};
							pCommandList->ResourceBarrier(_countof(transition), transition);
						}

						if (pDepthStencil->GetResourceState() != D3D12_RESOURCE_STATE_DEPTH_WRITE)
						{
							const D3D12_RESOURCE_BARRIER transition[] =
							{
								pDepthStencil->Transition(D3D12_RESOURCE_STATE_DEPTH_WRITE)
							};
							pCommandList->ResourceBarrier(_countof(transition), transition);
						}

						pCommandList->Close();
						pDeviceInstance->ExecuteCommandList(pCommandList);
					}

					RenderStaticElement(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_AlphaBlend_Pre, vecRTVHandles.data(), vecRTVHandles.size(), &dsvHandle, umapMaterialMask);
					RenderSkinnedElement(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_AlphaBlend_Pre, vecRTVHandles.data(), vecRTVHandles.size(), &dsvHandle, umapMaterialMask);

					RenderStaticElement(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_AlphaBlend_Post, vecRTVHandles.data(), vecRTVHandles.size(), &dsvHandle, umapMaterialMask);
					RenderSkinnedElement(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_AlphaBlend_Post, vecRTVHandles.data(), vecRTVHandles.size(), &dsvHandle, umapMaterialMask);

					{
						ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
						pDeviceInstance->ResetCommandList(0, nullptr);

						const D3D12_RESOURCE_BARRIER transition[] =
						{
							pDepthStencil->Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
						};
						pCommandList->ResourceBarrier(_countof(transition), transition);

						pCommandList->Close();
						pDeviceInstance->ExecuteCommandList(pCommandList);
					}
				}
				else
				{
					RenderStaticElement(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_Deferred, vecRTVHandles.data(), vecRTVHandles.size(), &dsvHandle, umapMaterialMask);
					RenderSkinnedElement(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_Deferred, vecRTVHandles.data(), vecRTVHandles.size(), &dsvHandle, umapMaterialMask);
				}

				if (isAlphaBlend == true)
				{
					pDeviceInstance->ReleaseRenderTargets(&pRenderTarget, 1);
				}
			}

			void ModelRenderer::Impl::Flush()
			{
				thread::SRWWriteLock writeLock(&m_job_srwLock);

				m_nJobStaticCount.fill(0);
				m_nJobSkinnedCount.fill(0);

				m_nSkinningBufferIndex = 0;
				m_nStaticBufferIndex = 0;
				m_nObjectBufferIndex = 0;
				m_nSrvBufferIndex = 0;
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

				thread::SRWWriteLock writeLock(&m_job_srwLock);

				if (m_nJobStaticCount[emGroup] >= shader::eMaxJobCount)
				{
					assert(false);
				}
				else
				{
					const size_t nIndex = m_nJobStaticCount[emGroup];
					m_vecJobStatics[emGroup][nIndex].Set(job);
					++m_nJobStaticCount[emGroup];
				}
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

				thread::SRWWriteLock writeLock(&m_job_srwLock);

				if (m_nJobSkinnedCount[emGroup] >= shader::eMaxJobCount)
				{
					assert(false);
				}
				else
				{
					const size_t nIndex = m_nJobSkinnedCount[emGroup];
					m_vecJobSkinneds[emGroup][nIndex].Set(job);
					++m_nJobSkinnedCount[emGroup];
				}
			}

			const ModelRenderer::Impl::RenderPipeline* ModelRenderer::Impl::GetRenderPipeline(ID3D12Device* pDevice, const shader::PSOKey& psoKey)
			{
				{
					thread::SRWReadLock readLock(&m_pipelines_srwLock);

					auto iter = m_umapRenderPipelines.find(psoKey);
					if (iter != m_umapRenderPipelines.end())
						return &iter->second;
				}

				const RenderPipeline* pRenderPipeline = nullptr;
				bool isRequestCreatePipeline = false;
				{
					thread::SRWWriteLock writeLock(&m_pipelines_srwLock);

					auto iter = m_umapRenderPipelines.find(psoKey);
					if (iter != m_umapRenderPipelines.end())
					{
						pRenderPipeline = &iter->second;
					}
					else
					{
						RenderPipeline& renderPipeline = m_umapRenderPipelines[psoKey];
						renderPipeline.pPipelineState = nullptr;
						renderPipeline.pRootSignature = nullptr;
						renderPipeline.nRootParameterIndex.fill(0);

						pRenderPipeline = &renderPipeline;

						isRequestCreatePipeline = true;
					}
				}

				if (isRequestCreatePipeline == true)
				{
					//thread::CreateTask([&, psoKey = psoKey]()
					//{
						Stopwatch stopwatch;
						stopwatch.Start();
					
						CreatePipelineState(Device::GetInstance()->GetInterface(), psoKey.nMask, psoKey.emRasterizerState, psoKey.emBlendState, psoKey.emDepthStencilState);
					
						stopwatch.Stop();
						LOG_MESSAGE("CreatePipelineState[%u_%d_%d_%d] : ElapsedTime[%lf]", psoKey.nMask, psoKey.emRasterizerState, psoKey.emBlendState, psoKey.emDepthStencilState, stopwatch.Elapsed());
					//});
				}

				return pRenderPipeline;
			}

			void ModelRenderer::Impl::RenderStaticElement(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, Group emGroup, shader::Pass emPass, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t nRTVHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, std::unordered_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				int nFrameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();
				DescriptorHeap* pSamplerDescriptorHeap = pDeviceInstance->GetSamplerDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				m_umapJobStaticMasterBatchs.clear();

				if (m_nJobStaticCount[emGroup] > 0)
				{
					for (size_t i = 0; i < m_nJobStaticCount[emGroup]; ++i)
					{
						const JobStatic& job = m_vecJobStatics[emGroup][i];

						if (job.isCulling == true)
							continue;

						//if (frustum.Contains(job.data.boundingSphere) == Collision::EmContainment::eDisjoint)
						//	continue;

						UMapJobStaticBatch& umapJobStaticBatch = m_umapJobStaticMasterBatchs[job.data.pMaterial];

						auto iter = umapJobStaticBatch.find(job.data.pKey);
						if (iter != umapJobStaticBatch.end())
						{
							iter->second.vecInstanceData.emplace_back(job.data.matWorld.Transpose());
						}
						else
						{
							umapJobStaticBatch.emplace(job.data.pKey, JobStaticBatch(&job, job.data.matWorld.Transpose()));
						}
					}

					std::unordered_map<shader::PSOKey, std::vector<const JobStaticBatch*>> umapJobStaticMaskBatch;
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

							shader::PSOKey maskKey = GetPSOKey(emGroup, emPass, nMask, jobBatch.pJob->data.pMaterial);
							umapJobStaticMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					const size_t nCommandListCount = pDeviceInstance->GetCommandListCount();
					std::vector<ID3D12GraphicsCommandList2*> vecCommandLists(nCommandListCount);
					pDeviceInstance->GetCommandLists(vecCommandLists.data());

					auto iter = umapJobStaticMaskBatch.begin();

					Concurrency::parallel_for(0llu, vecCommandLists.size(), [&](const size_t nIndex)
					{
						const IVertexBuffer* pPrevVertexBuffer = nullptr;
						const IIndexBuffer* pPrevIndexBuffer = nullptr;

						ID3D12GraphicsCommandList2* pCommandList = vecCommandLists[nIndex];
						pDeviceInstance->ResetCommandList(nIndex, nullptr);

						ID3D12DescriptorHeap* pDescriptorHeaps[] =
						{
							pSRVDescriptorHeap->GetHeap(),
							pSamplerDescriptorHeap->GetHeap(),
						};
						pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

						pCommandList->RSSetViewports(1, pViewport);
						pCommandList->RSSetScissorRects(1, pScissorRect);
						pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

						pCommandList->OMSetRenderTargets(static_cast<uint32_t>(nRTVHandleCount), pRTVHandles, FALSE, pDSVHandle);

						while (true)
						{
							const RenderPipeline* pRenderPipeline = nullptr;

							const shader::PSOKey* pPSOKey = nullptr;
							std::vector<const JobStaticBatch*>* pJobBatchs = nullptr;
							{
								thread::SRWWriteLock writeLock(&m_logic_srwLock);

								if (iter == umapJobStaticMaskBatch.end())
									break;

								pPSOKey = &iter->first;
								pJobBatchs = &iter->second;
								++iter;

								pRenderPipeline = GetRenderPipeline(pDevice, *pPSOKey);

								if (pRenderPipeline == nullptr || pRenderPipeline->pPipelineState == nullptr || pRenderPipeline->pRootSignature == nullptr)
								{
									const uint32_t nMask = (pPSOKey->nMask & shader::eUseAlphaBlending) | (pPSOKey->nMask & shader::eUseInstancing);

									const shader::PSOKey psoDefaultStaticKey(nMask, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
									pRenderPipeline = GetRenderPipeline(pDevice, psoDefaultStaticKey);
									if (pRenderPipeline == nullptr || pRenderPipeline->pPipelineState == nullptr || pRenderPipeline->pRootSignature == nullptr)
									{
										throw_line("invalid default static pipeline state");
										continue;
									}
								}
							}

							pCommandList->SetPipelineState(pRenderPipeline->pPipelineState);
							pCommandList->SetGraphicsRootSignature(pRenderPipeline->pRootSignature);

							if (pPSOKey->emBlendState != EmBlendState::eOff)
							{
								pCommandList->OMSetBlendFactor(&math::Vector4::Zero.x);
							}

							if (pRenderPipeline->nRootParameterIndex[eRP_StandardDescriptor] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootDescriptorTable(
									pRenderPipeline->nRootParameterIndex[eRP_StandardDescriptor],
									pSRVDescriptorHeap->GetStartGPUHandle(nFrameIndex));
							}

							if (pRenderPipeline->nRootParameterIndex[eRP_SamplerStates] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootDescriptorTable(
									pRenderPipeline->nRootParameterIndex[eRP_SamplerStates],
									pSamplerDescriptorHeap->GetStartGPUHandle(nFrameIndex));
							}

							if (pRenderPipeline->nRootParameterIndex[eRP_VSConstantsCB] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootConstantBufferView(
									pRenderPipeline->nRootParameterIndex[eRP_VSConstantsCB],
									m_vsConstantsBuffer.GPUAddress(nFrameIndex));
							}

							if (pRenderPipeline->nRootParameterIndex[eRP_CommonContentsCB] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootConstantBufferView(
									pRenderPipeline->nRootParameterIndex[eRP_CommonContentsCB],
									m_commonContentsBuffer.GPUAddress(nFrameIndex));
							}

							if ((pPSOKey->nMask & shader::eUseInstancing) == shader::eUseInstancing)
							{
								for (auto& pJobBatch : (*pJobBatchs))
								{
									const RenderJobStatic& job = pJobBatch->pJob->data;

									if (pRenderPipeline->nRootParameterIndex[eRP_ObjectDataCB] != eRP_InvalidIndex)
									{
										const size_t nObjectBufferIndex = m_nObjectBufferIndex++;

										shader::ObjectDataBuffer* pBuffer = m_objectDataBuffer.Cast(nFrameIndex, nObjectBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_objectDataBuffer.GPUAddress(nFrameIndex, nObjectBufferIndex);

										shader::SetObjectData(pBuffer, job.pMaterial, math::Matrix::Identity);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_ObjectDataCB], gpuAddress);
									}

									if (pRenderPipeline->nRootParameterIndex[eRP_SRVIndicesCB] != eRP_InvalidIndex)
									{
										const size_t nSrvBufferIndex = m_nSrvBufferIndex++;

										shader::SRVIndexConstants* pBuffer = m_srvIndexConstantsBuffer.Cast(nFrameIndex, nSrvBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_srvIndexConstantsBuffer.GPUAddress(nFrameIndex, nSrvBufferIndex);

										shader::SetMaterial(pBuffer, job.pMaterial);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_SRVIndicesCB], gpuAddress);
									}
									
									const math::Matrix* pInstanceData = pJobBatch->vecInstanceData.data();
									const size_t nInstanceCount = pJobBatch->vecInstanceData.size();

									const size_t nLoopCount = nInstanceCount / eMaxInstancingCount + 1;
									for (size_t i = 0; i < nLoopCount; ++i)
									{
										const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), nInstanceCount);
										const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

										if (nDrawInstanceCount <= 0)
											break;

										const size_t nStaticBufferIndex = m_nStaticBufferIndex++;

										shader::StaticInstancingDataBuffer* pStaticInstancingData = m_staticInstancingDataBuffer.Cast(nFrameIndex, nStaticBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_staticInstancingDataBuffer.GPUAddress(nFrameIndex, nStaticBufferIndex);

										Memory::Copy(pStaticInstancingData->data.data(), sizeof(pStaticInstancingData->data),
											&pInstanceData[i * eMaxInstancingCount], sizeof(math::Matrix) * nDrawInstanceCount);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_StaticInstancingDataCB], gpuAddress);

										if (pPrevVertexBuffer != job.pVertexBuffer)
										{
											const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(job.pVertexBuffer);
											pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

											pPrevVertexBuffer = job.pVertexBuffer;
										}

										if (job.pIndexBuffer != nullptr)
										{
											if (pPrevIndexBuffer != job.pIndexBuffer)
											{
												const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(job.pIndexBuffer);
												pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());

												pPrevIndexBuffer = job.pIndexBuffer;
											}

											pCommandList->DrawIndexedInstanced(job.nIndexCount, static_cast<uint32_t>(nDrawInstanceCount), job.nStartIndex, 0, 0);
										}
										else
										{
											pCommandList->IASetIndexBuffer(nullptr);
											pPrevIndexBuffer = nullptr;

											pCommandList->DrawInstanced(job.nIndexCount, static_cast<uint32_t>(nDrawInstanceCount), 0, 0);
										}
									}
								}
							}
							else
							{
								std::sort(pJobBatchs->begin(), pJobBatchs->end(), [](const JobStaticBatch* a, const JobStaticBatch* b)
								{
									return a->pJob->data.fDepth < b->pJob->data.fDepth;
								});

								for (auto& pJobBatch : (*pJobBatchs))
								{
									const RenderJobStatic& job = pJobBatch->pJob->data;

									if (pRenderPipeline->nRootParameterIndex[eRP_SRVIndicesCB] != eRP_InvalidIndex)
									{
										const size_t nSrvBufferIndex = m_nSrvBufferIndex++;

										shader::SRVIndexConstants* pBuffer = m_srvIndexConstantsBuffer.Cast(nFrameIndex, nSrvBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_srvIndexConstantsBuffer.GPUAddress(nFrameIndex, nSrvBufferIndex);

										shader::SetMaterial(pBuffer, job.pMaterial);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_SRVIndicesCB], gpuAddress);
									}

									if (pRenderPipeline->nRootParameterIndex[eRP_ObjectDataCB] != eRP_InvalidIndex)
									{
										const size_t nObjectBufferIndex = m_nObjectBufferIndex++;

										shader::ObjectDataBuffer* pBuffer = m_objectDataBuffer.Cast(nFrameIndex, nObjectBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_objectDataBuffer.GPUAddress(nFrameIndex, nObjectBufferIndex);

										shader::SetObjectData(pBuffer, job.pMaterial, job.matWorld);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_ObjectDataCB], gpuAddress);
									}

									if (pPrevVertexBuffer != job.pVertexBuffer)
									{
										const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(job.pVertexBuffer);
										pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

										pPrevVertexBuffer = job.pVertexBuffer;
									}

									if (job.pIndexBuffer != nullptr)
									{
										if (pPrevIndexBuffer != job.pIndexBuffer)
										{
											const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(job.pIndexBuffer);
											pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());

											pPrevIndexBuffer = job.pIndexBuffer;
										}

										pCommandList->DrawIndexedInstanced(job.nIndexCount, 1, job.nStartIndex, 0, 0);
									}
									else
									{
										pCommandList->IASetIndexBuffer(nullptr);
										pPrevIndexBuffer = nullptr;

										pCommandList->DrawInstanced(job.nIndexCount, 1, 0, 0);
									}
								}
							}
						}

						HRESULT hr = pCommandList->Close();
						if (FAILED(hr))
						{
							throw_line("failed to close command list");
						}
					});

					pDeviceInstance->ExecuteCommandLists(vecCommandLists.data(), vecCommandLists.size());
				}

				m_umapJobStaticMasterBatchs.clear();
			}

			void ModelRenderer::Impl::RenderSkinnedElement(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, Group emGroup, shader::Pass emPass, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t nRTVHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, std::unordered_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				int nFrameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();
				DescriptorHeap* pSamplerDescriptorHeap = pDeviceInstance->GetSamplerDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				m_umapJobSkinnedMasterBatchs.clear();

				if (m_nJobSkinnedCount[emGroup] > 0)
				{
					for (size_t i = 0; i < m_nJobSkinnedCount[emGroup]; ++i)
					{
						const JobSkinned& job = m_vecJobSkinneds[emGroup][i];

						if (job.isCulling == true)
							continue;

						//if (frustum.Contains(job.data.boundingSphere) == Collision::EmContainment::eDisjoint)
						//	continue;

						UMapJobSkinnedBatch& umapJobSkinnedBatch = m_umapJobSkinnedMasterBatchs[job.data.pMaterial];

						auto iter = umapJobSkinnedBatch.find(job.data.pKey);
						if (iter != umapJobSkinnedBatch.end())
						{
							iter->second.vecInstanceData.emplace_back(job.data.matWorld, job.data.nVTFID);
						}
						else
						{
							umapJobSkinnedBatch.emplace(job.data.pKey, JobSkinnedBatch(&job, job.data.matWorld, job.data.nVTFID));
						}
					}

					std::unordered_map<shader::PSOKey, std::vector<const JobSkinnedBatch*>> umapJobSkinnedMaskBatch;
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

							nMask |= shader::eUseSkinning;

							shader::PSOKey maskKey = GetPSOKey(emGroup, emPass, nMask, jobBatch.pJob->data.pMaterial);
							umapJobSkinnedMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					const size_t nCommandListCount = pDeviceInstance->GetCommandListCount();
					std::vector<ID3D12GraphicsCommandList2*> vecCommandLists(nCommandListCount);
					pDeviceInstance->GetCommandLists(vecCommandLists.data());

					auto iter = umapJobSkinnedMaskBatch.begin();

					Concurrency::parallel_for(0llu, vecCommandLists.size(), [&](const size_t nIndex)
					{
						const IVertexBuffer* pPrevVertexBuffer = nullptr;
						const IIndexBuffer* pPrevIndexBuffer = nullptr;

						ID3D12GraphicsCommandList2* pCommandList = vecCommandLists[nIndex];
						pDeviceInstance->ResetCommandList(nIndex, nullptr);

						ID3D12DescriptorHeap* pDescriptorHeaps[] =
						{
							pSRVDescriptorHeap->GetHeap(),
							pSamplerDescriptorHeap->GetHeap(),
						};
						pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

						pCommandList->RSSetViewports(1, pViewport);
						pCommandList->RSSetScissorRects(1, pScissorRect);
						pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

						pCommandList->OMSetRenderTargets(static_cast<uint32_t>(nRTVHandleCount), pRTVHandles, FALSE, pDSVHandle);

						while (true)
						{
							const RenderPipeline* pRenderPipeline = nullptr;

							const shader::PSOKey* pPSOKey = nullptr;
							std::vector<const JobSkinnedBatch*>* pJobBatchs = nullptr;
							{
								thread::SRWWriteLock writeLock(&m_logic_srwLock);

								if (iter == umapJobSkinnedMaskBatch.end())
									break;

								pPSOKey = &iter->first;
								pJobBatchs = &iter->second;
								++iter;

								pRenderPipeline = GetRenderPipeline(pDevice, *pPSOKey);

								if (pRenderPipeline == nullptr || pRenderPipeline->pPipelineState == nullptr || pRenderPipeline->pRootSignature == nullptr)
								{
									const uint32_t nMask = shader::eUseSkinning | (pPSOKey->nMask & shader::eUseAlphaBlending) | (pPSOKey->nMask & shader::eUseInstancing);

									const shader::PSOKey psoDefaultSkinnedKey(nMask, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
									pRenderPipeline = GetRenderPipeline(pDevice, psoDefaultSkinnedKey);
									if (pRenderPipeline == nullptr || pRenderPipeline->pPipelineState == nullptr || pRenderPipeline->pRootSignature == nullptr)
									{
										throw_line("invalid default skinned pipeline state");
										continue;
									}
								}
							}

							pCommandList->SetPipelineState(pRenderPipeline->pPipelineState);
							pCommandList->SetGraphicsRootSignature(pRenderPipeline->pRootSignature);

							if (pPSOKey->emBlendState != EmBlendState::eOff)
							{
								pCommandList->OMSetBlendFactor(&math::Vector4::Zero.x);
							}

							if (pRenderPipeline->nRootParameterIndex[eRP_StandardDescriptor] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootDescriptorTable(
									pRenderPipeline->nRootParameterIndex[eRP_StandardDescriptor],
									pSRVDescriptorHeap->GetStartGPUHandle(nFrameIndex));
							}

							if (pRenderPipeline->nRootParameterIndex[eRP_SamplerStates] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootDescriptorTable(
									pRenderPipeline->nRootParameterIndex[eRP_SamplerStates],
									pSamplerDescriptorHeap->GetStartGPUHandle(nFrameIndex));
							}

							if (pRenderPipeline->nRootParameterIndex[eRP_VSConstantsCB] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootConstantBufferView(
									pRenderPipeline->nRootParameterIndex[eRP_VSConstantsCB],
									m_vsConstantsBuffer.GPUAddress(nFrameIndex));
							}

							if (pRenderPipeline->nRootParameterIndex[eRP_CommonContentsCB] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootConstantBufferView(
									pRenderPipeline->nRootParameterIndex[eRP_CommonContentsCB],
									m_commonContentsBuffer.GPUAddress(nFrameIndex));
							}

							if ((pPSOKey->nMask & shader::eUseInstancing) == shader::eUseInstancing)
							{
								for (auto& pJobBatch : (*pJobBatchs))
								{
									const RenderJobSkinned& job = pJobBatch->pJob->data;

									if (pRenderPipeline->nRootParameterIndex[eRP_ObjectDataCB] != eRP_InvalidIndex)
									{
										const size_t nObjectBufferIndex = m_nObjectBufferIndex++;

										shader::ObjectDataBuffer* pBuffer = m_objectDataBuffer.Cast(nFrameIndex, nObjectBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_objectDataBuffer.GPUAddress(nFrameIndex, nObjectBufferIndex);

										shader::SetObjectData(pBuffer, job.pMaterial, math::Matrix::Identity);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_ObjectDataCB], gpuAddress);
									}

									if (pRenderPipeline->nRootParameterIndex[eRP_SRVIndicesCB] != eRP_InvalidIndex)
									{
										const size_t nSrvBufferIndex = m_nSrvBufferIndex++;

										shader::SRVIndexConstants* pBuffer = m_srvIndexConstantsBuffer.Cast(nFrameIndex, nSrvBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_srvIndexConstantsBuffer.GPUAddress(nFrameIndex, nSrvBufferIndex);

										shader::SetMaterial(pBuffer, job.pMaterial);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_SRVIndicesCB], gpuAddress);
									}

									const SkinningInstancingData* pInstanceData = pJobBatch->vecInstanceData.data();
									const size_t nInstanceCount = pJobBatch->vecInstanceData.size();

									const size_t nLoopCount = nInstanceCount / eMaxInstancingCount + 1;
									for (size_t i = 0; i < nLoopCount; ++i)
									{
										const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), nInstanceCount);
										const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

										if (nDrawInstanceCount <= 0)
											break;

										const size_t nSkinningBufferIndex = m_nSkinningBufferIndex++;

										shader::SkinningInstancingDataBuffer* pSkinnedInstancingData = m_skinningInstancingDataBuffer.Cast(nFrameIndex, nSkinningBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_skinningInstancingDataBuffer.GPUAddress(nFrameIndex, nSkinningBufferIndex);

										Memory::Copy(pSkinnedInstancingData->data.data(), sizeof(pSkinnedInstancingData->data),
											&pInstanceData[i * eMaxInstancingCount], sizeof(SkinningInstancingData) * nDrawInstanceCount);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_SkinningInstancingDataCB], gpuAddress);

										if (pPrevVertexBuffer != job.pVertexBuffer)
										{
											const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(job.pVertexBuffer);
											pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

											pPrevVertexBuffer = job.pVertexBuffer;
										}

										if (job.pIndexBuffer != nullptr)
										{
											if (pPrevIndexBuffer != job.pIndexBuffer)
											{
												const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(job.pIndexBuffer);
												pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());

												pPrevIndexBuffer = job.pIndexBuffer;
											}

											pCommandList->DrawIndexedInstanced(job.nIndexCount, static_cast<uint32_t>(nDrawInstanceCount), job.nStartIndex, 0, 0);
										}
										else
										{
											pCommandList->IASetIndexBuffer(nullptr);
											pPrevIndexBuffer = nullptr;

											pCommandList->DrawInstanced(job.nIndexCount, static_cast<uint32_t>(nDrawInstanceCount), 0, 0);
										}
									}
								}
							}
							else
							{
								std::sort(pJobBatchs->begin(), pJobBatchs->end(), [](const JobSkinnedBatch* a, const JobSkinnedBatch* b)
								{
									return a->pJob->data.fDepth < b->pJob->data.fDepth;
								});

								for (auto& pJobBatch : (*pJobBatchs))
								{
									const RenderJobSkinned& job = pJobBatch->pJob->data;

									if (pRenderPipeline->nRootParameterIndex[eRP_SRVIndicesCB] != eRP_InvalidIndex)
									{
										const size_t nSrvBufferIndex = m_nSrvBufferIndex++;

										shader::SRVIndexConstants* pBuffer = m_srvIndexConstantsBuffer.Cast(nFrameIndex, nSrvBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_srvIndexConstantsBuffer.GPUAddress(nFrameIndex, nSrvBufferIndex);

										shader::SetMaterial(pBuffer, job.pMaterial);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_SRVIndicesCB], gpuAddress);
									}

									if (pRenderPipeline->nRootParameterIndex[eRP_ObjectDataCB] != eRP_InvalidIndex)
									{
										const size_t nObjectBufferIndex = m_nObjectBufferIndex++;

										shader::ObjectDataBuffer* pBuffer = m_objectDataBuffer.Cast(nFrameIndex, nObjectBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_objectDataBuffer.GPUAddress(nFrameIndex, nObjectBufferIndex);

										shader::SetObjectData(pBuffer, job.pMaterial, job.matWorld, job.nVTFID);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->nRootParameterIndex[eRP_ObjectDataCB], gpuAddress);
									}

									if (pPrevVertexBuffer != job.pVertexBuffer)
									{
										const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(job.pVertexBuffer);
										pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

										pPrevVertexBuffer = job.pVertexBuffer;
									}

									if (job.pIndexBuffer != nullptr)
									{
										if (pPrevIndexBuffer != job.pIndexBuffer)
										{
											const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(job.pIndexBuffer);
											pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());

											pPrevIndexBuffer = job.pIndexBuffer;
										}

										pCommandList->DrawIndexedInstanced(job.nIndexCount, 1, job.nStartIndex, 0, 0);
									}
									else
									{
										pCommandList->IASetIndexBuffer(nullptr);
										pPrevIndexBuffer = nullptr;

										pCommandList->DrawInstanced(job.nIndexCount, 1, 0, 0);
									}
								}
							}
						}

						HRESULT hr = pCommandList->Close();
						if (FAILED(hr))
						{
							throw_line("failed to close command list");
						}
					});

					pDeviceInstance->ExecuteCommandLists(vecCommandLists.data(), vecCommandLists.size());
				}

				m_umapJobSkinnedMasterBatchs.clear();
			}

			ID3D12RootSignature* ModelRenderer::Impl::CreateRootSignature(ID3D12Device* pDevice, uint32_t nMask, std::array<uint32_t, eRP_Count>& nRootParameterIndex_out)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_ALL);
				nRootParameterIndex_out[eRP_StandardDescriptor] = static_cast<uint32_t>(vecRootParameters.size() - 1);

				D3D12_DESCRIPTOR_RANGE samplerRange{};
				samplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				samplerRange.NumDescriptors = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				samplerRange.BaseShaderRegister = 0;
				samplerRange.RegisterSpace = 0;
				samplerRange.OffsetInDescriptorsFromTableStart = 0;

				CD3DX12_ROOT_PARAMETER& samplerDescriptorTable = vecRootParameters.emplace_back();
				samplerDescriptorTable.InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_PIXEL);
				nRootParameterIndex_out[eRP_SamplerStates] = static_cast<uint32_t>(vecRootParameters.size() - 1);

				CD3DX12_ROOT_PARAMETER& srvIndexParameter = vecRootParameters.emplace_back();
				srvIndexParameter.InitAsConstantBufferView(shader::eCB_SRVIndex, 0, D3D12_SHADER_VISIBILITY_PIXEL);
				nRootParameterIndex_out[eRP_SRVIndicesCB] = static_cast<uint32_t>(vecRootParameters.size() - 1);

				CD3DX12_ROOT_PARAMETER& matrixParameter = vecRootParameters.emplace_back();
				matrixParameter.InitAsConstantBufferView(shader::eCB_VSConstants, 0, D3D12_SHADER_VISIBILITY_VERTEX);
				nRootParameterIndex_out[eRP_VSConstantsCB] = static_cast<uint32_t>(vecRootParameters.size() - 1);

				CD3DX12_ROOT_PARAMETER& objectParameter = vecRootParameters.emplace_back();
				objectParameter.InitAsConstantBufferView(shader::eCB_ObjectData, 0, D3D12_SHADER_VISIBILITY_ALL);
				nRootParameterIndex_out[eRP_ObjectDataCB] = static_cast<uint32_t>(vecRootParameters.size() - 1);

				if ((nMask & shader::eUseInstancing) == shader::eUseInstancing)
				{
					CD3DX12_ROOT_PARAMETER& instancingParameter = vecRootParameters.emplace_back();
					if ((nMask & shader::eUseSkinning) == shader::eUseSkinning)
					{
						instancingParameter.InitAsConstantBufferView(shader::eCB_SkinningInstancingData, 0, D3D12_SHADER_VISIBILITY_VERTEX);
						nRootParameterIndex_out[eRP_SkinningInstancingDataCB] = static_cast<uint32_t>(vecRootParameters.size() - 1);
					}
					else
					{
						instancingParameter.InitAsConstantBufferView(shader::eCB_StaticInstancingData, 0, D3D12_SHADER_VISIBILITY_VERTEX);
						nRootParameterIndex_out[eRP_StaticInstancingDataCB] = static_cast<uint32_t>(vecRootParameters.size() - 1);
					}
				}

				std::vector<D3D12_STATIC_SAMPLER_DESC> vecStaticSamplers;
				if ((nMask & shader::eUseAlphaBlending) == shader::eUseAlphaBlending)
				{
					CD3DX12_ROOT_PARAMETER& commonContentsParameter = vecRootParameters.emplace_back();
					commonContentsParameter.InitAsConstantBufferView(shader::eCB_CommonContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);
					nRootParameterIndex_out[eRP_CommonContentsCB] = static_cast<uint32_t>(vecRootParameters.size() - 1);

					vecStaticSamplers = 
					{
						util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipPointClamp, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
						util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipLinearClamp, 1, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					};
				}

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					static_cast<uint32_t>(vecStaticSamplers.size()), vecStaticSamplers.data(),
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void ModelRenderer::Impl::CreatePipelineState(ID3D12Device* pDevice, uint32_t nMask, EmRasterizerState::Type emRasterizerState, EmBlendState::Type emBlendState, EmDepthStencilState::Type emDepthStencilState)
			{
				std::vector<D3D_SHADER_MACRO> vecMacros = shader::GetMacros(nMask);

				ID3DBlob* pVertexShaderBlob = nullptr;
				bool isSuccess = util::CompileShader(m_pShaderBlob, vecMacros.data(), m_strShaderPath.c_str(), "VS", "vs_5_1", &pVertexShaderBlob);
				if (isSuccess == false)
				{
					throw_line("failed to compile vertex shader");
				}

				ID3DBlob* pPixelShaderBlob = nullptr;
				isSuccess = util::CompileShader(m_pShaderBlob, vecMacros.data(), m_strShaderPath.c_str(), "PS", "ps_5_1", &pPixelShaderBlob);
				if (isSuccess == false)
				{
					throw_line("failed to compile pixel shader");
				}

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.VS.BytecodeLength = pVertexShaderBlob->GetBufferSize();
				psoDesc.VS.pShaderBytecode = pVertexShaderBlob->GetBufferPointer();

				psoDesc.PS.BytecodeLength = pPixelShaderBlob->GetBufferSize();
				psoDesc.PS.pShaderBytecode = pPixelShaderBlob->GetBufferPointer();

				const D3D12_INPUT_ELEMENT_DESC* pInputElements = nullptr;
				size_t nElementCount = 0;
				if ((nMask & shader::eUseSkinning) == shader::eUseSkinning)
				{
					util::GetInputElementDesc(VertexPosTexNorWeiIdx::Format(), &pInputElements, &nElementCount);
				}
				else
				{
					util::GetInputElementDesc(VertexPosTexNor::Format(), &pInputElements, &nElementCount);
				}

				psoDesc.InputLayout.NumElements = static_cast<uint32_t>(nElementCount);
				psoDesc.InputLayout.pInputElementDescs = pInputElements;

				psoDesc.SampleDesc.Count = 1;

				std::array<uint32_t, eRP_Count> nRootParameterIndex;
				nRootParameterIndex.fill(eRP_InvalidIndex);
				ID3D12RootSignature* pRootSignature = CreateRootSignature(pDevice, nMask, nRootParameterIndex);

				shader::PSOKey key(nMask, emRasterizerState, emBlendState, emDepthStencilState);

				std::string strName = String::Format("%u_%d_%d_%d", key.nMask, key.emRasterizerState, key.emBlendState, key.emDepthStencilState);
				std::wstring wstrName = String::MultiToWide(strName);
				pRootSignature->SetName(wstrName.c_str());

				psoDesc.pRootSignature = pRootSignature;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.SampleMask = 0xffffffff;
				psoDesc.RasterizerState = util::GetRasterizerDesc(emRasterizerState);
				psoDesc.BlendState = util::GetBlendDesc(emBlendState);

				GBuffer* pGBuffer = Device::GetInstance()->GetGBuffer(0);

				if ((nMask & shader::eUseAlphaBlending) == shader::eUseAlphaBlending)
				{
					psoDesc.NumRenderTargets = 1;

					if (GetOptions().OnHDR == true)
					{
						psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
					}
					else
					{
						RenderTarget* pRenderTarget = Device::GetInstance()->GetSwapChainRenderTarget(0);
						psoDesc.RTVFormats[0] = pRenderTarget->GetDesc().Format;
					}
				}
				else
				{
					psoDesc.NumRenderTargets = EmGBuffer::Count;
					psoDesc.RTVFormats[EmGBuffer::eNormals] = pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetDesc().Format;
					psoDesc.RTVFormats[EmGBuffer::eColors] = pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetDesc().Format;
					psoDesc.RTVFormats[EmGBuffer::eDisneyBRDF] = pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetDesc().Format;
				}

				psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(emDepthStencilState);

				ID3D12PipelineState* pPipelineState = nullptr;
				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				pPipelineState->SetName(wstrName.c_str());

				{
					thread::SRWWriteLock writeLock(&m_pipelines_srwLock);

					RenderPipeline& renderPipeline = m_umapRenderPipelines[key];
					renderPipeline.pPipelineState = pPipelineState;
					renderPipeline.pRootSignature = pRootSignature;
					renderPipeline.nRootParameterIndex = nRootParameterIndex;
				}

				SafeRelease(pVertexShaderBlob);
				SafeRelease(pPixelShaderBlob);
			}

			ModelRenderer::ModelRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			ModelRenderer::~ModelRenderer()
			{
			}

			void ModelRenderer::Render(Camera* pCamera, Group emGroup)
			{
				m_pImpl->Render(pCamera, emGroup);
			}

			void ModelRenderer::Flush()
			{
				m_pImpl->Flush();
			}
			
			void ModelRenderer::PushJob(const RenderJobStatic& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}

			void ModelRenderer::PushJob(const RenderJobSkinned& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}
		}
	}
}