#include "stdafx.h"
#include "ModelRendererDX12.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Lock.h"
#include "CommonLib/ThreadPool.h"
#include "CommonLib/Timer.h"

#include "Graphics/Interface/Instancing.h"
#include "Graphics/Interface/Camera.h"
#include "Graphics/Interface/LightManager.h"
#include "Graphics/Interface/OcclusionCulling.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "GBufferDX12.h"
#include "VTFManagerDX12.h"
#include "DescriptorHeapDX12.h"

#include "VertexBufferDX12.h"
#include "IndexBufferDX12.h"
#include "TextureDX12.h"

namespace est
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
					std::array<SkinningInstancingData, eMaxInstancingCount> prevData;
				};

				struct StaticInstancingDataBuffer
				{
					std::array<math::Matrix, eMaxInstancingCount> data;
					std::array<math::Matrix, eMaxInstancingCount> prevData;
				};

				struct ObjectDataBuffer
				{
					math::Matrix matWorld;
					math::Matrix matPrevWorld;

					math::Color f4AlbedoColor;
					math::Color f4EmissiveColor;

					math::float4 f4PaddingRoughMetEmi;
					math::float4 f4SurSpecTintAniso;
					math::float4 f4SheenTintClearcoatGloss;

					float stippleTransparencyFactor{ 0.f };
					uint32_t VTFID{ 0 };
					uint32_t PrevVTFID{ 0 };

					float padding{ 0 };
				};

				struct VSConstantsBuffer
				{
					math::Matrix matViewProj;
					math::Matrix matPrevViewProj;

					uint32_t nTexVTFIndex{ 0 };
					uint32_t nTexPrevVTFIndex{ 0 };
					math::float2 padding;
				};

				struct CommonContents
				{
					math::float3 f3CameraPos;
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
					eUseInstancing = 1 << MaterialMaskCount,
					eUseSkinning = 1 << (MaterialMaskCount + 1),
					eUseAlphaBlending = 1 << (MaterialMaskCount + 2),
					eUseMotionBlur = 1 << (MaterialMaskCount + 3),

					MaskCount = MaterialMaskCount + 4,
				};

				const char* GetMaskName(uint32_t mask)
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
						"USE_MOTION_BLUR",

						"USE_WRITEDEPTH",
						"USE_CUBEMAP",
						"USE_TESSELLATION",
						"USE_ALBEDO_ALPHA_IS_MASK_MAP",
					};

					return s_strMaskName[mask].c_str();
				}

				std::vector<D3D_SHADER_MACRO> GetMacros(uint32_t mask)
				{
					std::vector<D3D_SHADER_MACRO> vecMacros;
					vecMacros.push_back({ "DX12", "1" });
					for (uint32_t i = 0; i < shader::MaskCount; ++i)
					{
						if ((mask & (1llu << i)) != 0)
						{
							vecMacros.push_back({ GetMaskName(i), "1" });
						}
					}
					vecMacros.push_back({ nullptr, nullptr });

					return vecMacros;
				}

				PSOKey GetPSOKey(ModelRenderer::Group emGroup, Pass emPass, uint32_t mask, const IMaterial* pMaterial)
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

					return { mask, emRasterizerState, emBlendState, emDepthStencilState };
				}

				void SetObjectData(ObjectDataBuffer* pObjectDataBuffer,
					const IMaterial* pMaterial, const math::Matrix& matWorld, const math::Matrix& matPrevWorld, uint32_t VTFID, uint32_t PrevVTFID)
				{
					pObjectDataBuffer->matWorld = matWorld.Transpose();
					pObjectDataBuffer->matPrevWorld = matPrevWorld.Transpose();

					if (pMaterial != nullptr)
					{
						pObjectDataBuffer->f4AlbedoColor = pMaterial->GetAlbedoColor();
						pObjectDataBuffer->f4EmissiveColor = pMaterial->GetEmissiveColor();

						pObjectDataBuffer->f4PaddingRoughMetEmi = pMaterial->GetPaddingRoughMetEmi();
						pObjectDataBuffer->f4SurSpecTintAniso = pMaterial->GetSurSpecTintAniso();
						pObjectDataBuffer->f4SheenTintClearcoatGloss = pMaterial->GetSheenTintClearcoatGloss();

						pObjectDataBuffer->stippleTransparencyFactor = pMaterial->GetStippleTransparencyFactor();
						pObjectDataBuffer->VTFID = VTFID;
						pObjectDataBuffer->PrevVTFID = PrevVTFID;

						pObjectDataBuffer->padding = 0.f;
					}
					else
					{
						pObjectDataBuffer->f4AlbedoColor = math::Color::White;
						pObjectDataBuffer->f4EmissiveColor = math::Color::Black;

						pObjectDataBuffer->f4PaddingRoughMetEmi = math::float4::Zero;
						pObjectDataBuffer->f4SurSpecTintAniso = math::float4::Zero;
						pObjectDataBuffer->f4SheenTintClearcoatGloss = math::float4::Zero;

						pObjectDataBuffer->stippleTransparencyFactor = 0.f;
						pObjectDataBuffer->VTFID = VTFID;
						pObjectDataBuffer->PrevVTFID = PrevVTFID;

						pObjectDataBuffer->padding = 0.f;
					}
				}

				void SetCommonContents_ForAlpha(CommonContents* pCommonContents,
					const IImageBasedLight* pImageBasedLight, const LightManager* pLightManager, const math::float3& f3CameraPos, int nEnableShadowCount)
				{
					pCommonContents->f3CameraPos = f3CameraPos;
					pCommonContents->nEnableShadowCount = nEnableShadowCount;

					Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR().get());
					pCommonContents->nTexDiffuseHDRIndex = pDiffuseHDR->GetDescriptorIndex();

					Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR().get());
					pCommonContents->nTexSpecularHDRIndex = pSpecularHDR->GetDescriptorIndex();

					Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF().get());
					pCommonContents->nTexSpecularBRDFIndex = pSpecularBRDF->GetDescriptorIndex();

					const DirectionalLightData* pDirectionalLightData = nullptr;
					pLightManager->GetDirectionalLightData(&pDirectionalLightData, &pCommonContents->nDirectionalLightCount);
					memory::Copy(pCommonContents->lightDirectional, pDirectionalLightData, sizeof(DirectionalLightData) * pCommonContents->nDirectionalLightCount);

					const PointLightData* pPointLightData = nullptr;
					pLightManager->GetPointLightData(&pPointLightData, &pCommonContents->nPointLightCount);
					memory::Copy(pCommonContents->lightPoint, pPointLightData, sizeof(PointLightData) * pCommonContents->nPointLightCount);

					const SpotLightData* pSpotLightData = nullptr;
					pLightManager->GetSpotLightData(&pSpotLightData, &pCommonContents->nSpotLightCount);
					memory::Copy(pCommonContents->lightSpot, pSpotLightData, sizeof(SpotLightData) * pCommonContents->nSpotLightCount);
				}
			}

			class ModelRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Render(const RenderElement& renderElement, Group emGroup, const math::Matrix& matPrevViewProjection);
				void Cleanup();

			public:
				void PushJob(const RenderJobStatic& job);
				void PushJob(const RenderJobSkinned& job);

			private:
				void RenderStaticModel(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, Group emGroup, shader::Pass emPass, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t nRTVHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask);
				void RenderSkinnedModel(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, Group emGroup, shader::Pass emPass, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t nRTVHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask);

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
					PSOCache psoCache;

					std::array<uint32_t, eRP_Count> rootParameterIndex{ eRP_InvalidIndex };
				};
				const RenderPipeline* CreateRenderPipeline(ID3D12Device* pDevice, const PSOKey& psoKey);

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice, uint32_t mask, std::array<uint32_t, eRP_Count>& rootParameterIndex_out);
				void CreatePipelineState(ID3D12Device* pDevice, const PSOKey& psoKey, RenderPipeline* pRenderPipeline);

			private:
				thread::SRWLock m_srwLock_jobStatic;
				thread::SRWLock m_srwLock_jobSkinned;

				thread::SRWLock m_srwLock_pipelines;
				thread::SRWLock m_srwLock_logic;

				std::wstring m_shaderPath;
				ID3DBlob* m_pShaderBlob{ nullptr };

				std::unordered_map<PSOKey, RenderPipeline> m_umapRenderPipelines;

				std::atomic<size_t> m_skinningBufferIndex{ 0 };
				std::atomic<size_t> m_staticBufferIndex{ 0 };
				std::atomic<size_t> m_objectBufferIndex{ 0 };
				std::atomic<size_t> m_srvBufferIndex{ 0 };

				ConstantBuffer<shader::SkinningInstancingDataBuffer> m_skinningInstancingDataBuffer;
				ConstantBuffer<shader::StaticInstancingDataBuffer> m_staticInstancingDataBuffer;
				ConstantBuffer<shader::ObjectDataBuffer> m_objectDataBuffer;
				ConstantBuffer<shader::MaterialSRVIndexConstants> m_materialSRVIndexConstantsBuffer;

				ConstantBuffer<shader::VSConstantsBuffer> m_vsConstantsBuffer;
				ConstantBuffer<shader::CommonContents> m_commonContentsBuffer;

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
					std::vector<math::Matrix> instanceData;
					std::vector<math::Matrix> prevInstanceData;

					JobStaticBatch(const JobStatic* pJob, const math::Matrix& matWorld, const math::Matrix& matPrevWorld)
						: pJob(pJob)
					{
						instanceData.emplace_back(matWorld);
						prevInstanceData.emplace_back(matPrevWorld);
					}
				};

				std::array<std::array<JobStatic, shader::eMaxJobCount>, GroupCount> m_jobStatics;
				std::array<size_t, GroupCount> m_jobStaticCount{ 0 };

				using UMapJobStaticBatch = tsl::robin_map<const void*, JobStaticBatch>;
				using UMapJobStaticMaterialBatch = tsl::robin_map<MaterialPtr, UMapJobStaticBatch>;
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
					std::vector<SkinningInstancingData> instanceData;
					std::vector<SkinningInstancingData> prevInstanceData;

					JobSkinnedBatch(const JobSkinned* pJob, const math::Matrix& matWorld, const math::Matrix& matPrevWorld, uint32_t VTFID, uint32_t PrevVTFID)
						: pJob(pJob)
					{
						instanceData.emplace_back(matWorld, VTFID);
						prevInstanceData.emplace_back(matPrevWorld, PrevVTFID);
					}
				};

				std::array<std::array<JobSkinned, shader::eMaxJobCount>, GroupCount> m_jobSkinneds;
				std::array<size_t, GroupCount> m_jobSkinnedCount{ 0 };

				using UMapJobSkinnedBatch = tsl::robin_map<const void*, JobSkinnedBatch>;
				using UMapJobSkinnedMaterialBatch = tsl::robin_map<MaterialPtr, UMapJobSkinnedBatch>;
				UMapJobSkinnedMaterialBatch m_umapJobSkinnedMasterBatchs;
			};

			ModelRenderer::Impl::Impl()
			{
				m_shaderPath = file::GetEngineDataPath();
				m_shaderPath.append(L"Fx\\Model\\Model.hlsl");

				if (FAILED(D3DReadFileToBlob(m_shaderPath.c_str(), &m_pShaderBlob)))
				{
					throw_line("failed to read shader file : Model.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreateRenderPipeline(pDevice, { 0, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseInstancing, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseAlphaBlending, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseInstancing | shader::eUseAlphaBlending, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On });

				CreateRenderPipeline(pDevice, { shader::eUseSkinning, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseSkinning | shader::eUseInstancing, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseSkinning | shader::eUseAlphaBlending, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseSkinning | shader::eUseInstancing | shader::eUseAlphaBlending, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On });

				m_skinningInstancingDataBuffer.Create(pDevice, shader::eMaxInstancingJobCount, "SkinningInstancingDataBuffer");
				m_staticInstancingDataBuffer.Create(pDevice, shader::eMaxInstancingJobCount, "StaticInstancingDataBuffer");
				m_objectDataBuffer.Create(pDevice, shader::eMaxJobCount, "ObjectDataBuffer");
				m_materialSRVIndexConstantsBuffer.Create(pDevice, shader::eMaxJobCount, "MaterialSRVIndexConstantsBuffer");
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
				m_materialSRVIndexConstantsBuffer.Destroy();
				m_commonContentsBuffer.Destroy();

				m_umapRenderPipelines.clear();

				SafeRelease(m_pShaderBlob);
			}

			void ModelRenderer::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				for (auto iter = m_umapRenderPipelines.begin(); iter != m_umapRenderPipelines.end(); ++iter)
				{
					const PSOKey& psoKey = iter->first;
					if ((psoKey.mask & shader::eUseAlphaBlending) == shader::eUseAlphaBlending)
					{
						CreatePipelineState(pDevice, psoKey, &iter->second);
					}
				}
			}

			void ModelRenderer::Impl::Render(const RenderElement& renderElement, Group emGroup, const math::Matrix& matPrevViewProjection)
			{
				const bool isAlphaBlend = emGroup == Group::eAlphaBlend;
				if (isAlphaBlend == true && m_jobStaticCount[emGroup] == 0 && m_jobSkinnedCount[emGroup] == 0)
					return;

				Camera* pCamera = renderElement.pCamera;

				TRACER_EVENT(__FUNCTIONW__);
				const OcclusionCulling* pOcclusionCulling = OcclusionCulling::GetInstance();
				if (emGroup == Group::eDeferred)
				{
					TRACER_EVENT(L"Culling");

					const collision::Frustum& frustum = pCamera->GetFrustum();
					jobsystem::ParallelFor(m_jobStaticCount[emGroup], [&](size_t i)
					{
						JobStatic& job = m_jobStatics[emGroup][i];
						const OcclusionCullingData& occlusionCullingData = job.data.occlusionCullingData;
						if (frustum.Contains(occlusionCullingData.aabb) == collision::EmContainment::eDisjoint)
						{
							job.isCulled = true;
							return;
						}

						if (pOcclusionCulling->TestRect(occlusionCullingData.aabb) != OcclusionCulling::eVisible)
						{
							job.isCulled = true;
							return;
						}
					});

					jobsystem::ParallelFor(m_jobSkinnedCount[emGroup], [&](size_t i)
					{
						JobSkinned& job = m_jobSkinneds[emGroup][i];
						const OcclusionCullingData& occlusionCullingData = job.data.occlusionCullingData;
						if (frustum.Contains(occlusionCullingData.aabb) == collision::EmContainment::eDisjoint)
						{
							job.isCulled = true;
							return;
						}

						if (pOcclusionCulling->TestRect(occlusionCullingData.aabb) != OcclusionCulling::eVisible)
						{
							job.isCulled = true;
							return;
						}
					});
				}

				Device* pDeviceInstance = Device::GetInstance();
				ID3D12Device* pDevice = pDeviceInstance->GetInterface();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();

				{
					VTFManager* pVTFManager = pDeviceInstance->GetVTFManager();
					Texture* pVTFTexture = pVTFManager->GetTexture();
					Texture* pPrevVTFTexture = pVTFManager->GetPrevTexture();

					shader::VSConstantsBuffer* pVSConstantsBuffer = m_vsConstantsBuffer.Cast(frameIndex);
					pVSConstantsBuffer->matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();
					pVSConstantsBuffer->matViewProj = pVSConstantsBuffer->matViewProj.Transpose();

					pVSConstantsBuffer->matPrevViewProj = matPrevViewProjection.Transpose();

					pVSConstantsBuffer->nTexVTFIndex = pVTFTexture->GetDescriptorIndex();
					pVSConstantsBuffer->nTexPrevVTFIndex = pPrevVTFTexture->GetDescriptorIndex();
				}

				tsl::robin_map<const IMaterial*, uint32_t> umapMaterialMask;
				umapMaterialMask.rehash(m_umapJobStaticMasterBatchs.size() + m_umapJobSkinnedMasterBatchs.size());

				if (isAlphaBlend == true)
				{
					const LightManager* pLightManager = LightManager::GetInstance();
					const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();

					shader::CommonContents* pCommonContents = m_commonContentsBuffer.Cast(frameIndex);
					shader::SetCommonContents_ForAlpha(pCommonContents, pImageBasedLight, pLightManager, pCamera->GetPosition(), 0);

					if (renderElement.pRTVs[0]->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET ||
						renderElement.pDepthStencil->GetResourceState() != D3D12_RESOURCE_STATE_DEPTH_WRITE)
					{
						ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
						pDeviceInstance->ResetCommandList(0, nullptr);

						util::ChangeResourceState(pCommandList, renderElement.pRTVs[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
						util::ChangeResourceState(pCommandList, renderElement.pDepthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);

						pCommandList->Close();
						pDeviceInstance->ExecuteCommandList(pCommandList);
					}

					RenderStaticModel(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_AlphaBlend_Pre, renderElement.rtvHandles, renderElement.rtvCount, renderElement.GetDSVHandle(), umapMaterialMask);
					RenderSkinnedModel(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_AlphaBlend_Pre, renderElement.rtvHandles, renderElement.rtvCount, renderElement.GetDSVHandle(), umapMaterialMask);

					RenderStaticModel(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_AlphaBlend_Post, renderElement.rtvHandles, renderElement.rtvCount, renderElement.GetDSVHandle(), umapMaterialMask);
					RenderSkinnedModel(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_AlphaBlend_Post, renderElement.rtvHandles, renderElement.rtvCount, renderElement.GetDSVHandle(), umapMaterialMask);

					{
						ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
						pDeviceInstance->ResetCommandList(0, nullptr);

						util::ChangeResourceState(pCommandList, renderElement.pDepthStencil, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

						pCommandList->Close();
						pDeviceInstance->ExecuteCommandList(pCommandList);
					}
				}
				else
				{
					RenderStaticModel(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_Deferred, renderElement.rtvHandles, renderElement.rtvCount, renderElement.GetDSVHandle(), umapMaterialMask);
					RenderSkinnedModel(pDeviceInstance, pDevice, pCamera, emGroup, shader::ePass_Deferred, renderElement.rtvHandles, renderElement.rtvCount, renderElement.GetDSVHandle(), umapMaterialMask);
				}
			}

			void ModelRenderer::Impl::Cleanup()
			{
				m_jobStaticCount.fill(0);
				m_jobSkinnedCount.fill(0);

				m_skinningBufferIndex = 0;
				m_staticBufferIndex = 0;
				m_objectBufferIndex = 0;
				m_srvBufferIndex = 0;
			}

			void ModelRenderer::Impl::PushJob(const RenderJobStatic& job)
			{
				Group emGroup;

				const MaterialPtr& pMaterial = job.pMaterial;
				if (pMaterial == nullptr || pMaterial->GetBlendState() == EmBlendState::eOff)
				{
					emGroup = eDeferred;
				}
				else
				{
					emGroup = eAlphaBlend;
				}

				thread::SRWWriteLock writeLock(&m_srwLock_jobStatic);

				if (m_jobStaticCount[emGroup] >= shader::eMaxJobCount)
				{
					assert(false);
				}
				else
				{
					const size_t index = m_jobStaticCount[emGroup];
					m_jobStatics[emGroup][index].Set(job);
					++m_jobStaticCount[emGroup];
				}
			}

			void ModelRenderer::Impl::PushJob(const RenderJobSkinned& job)
			{
				Group emGroup;

				const MaterialPtr& pMaterial = job.pMaterial;
				if (pMaterial == nullptr || pMaterial->GetBlendState() == EmBlendState::eOff)
				{
					emGroup = eDeferred;
				}
				else
				{
					emGroup = eAlphaBlend;
				}

				thread::SRWWriteLock writeLock(&m_srwLock_jobSkinned);

				if (m_jobSkinnedCount[emGroup] >= shader::eMaxJobCount)
				{
					assert(false);
				}
				else
				{
					const size_t index = m_jobSkinnedCount[emGroup];
					m_jobSkinneds[emGroup][index].Set(job);
					++m_jobSkinnedCount[emGroup];
				}
			}

			const ModelRenderer::Impl::RenderPipeline* ModelRenderer::Impl::CreateRenderPipeline(ID3D12Device* pDevice, const PSOKey& psoKey)
			{
				{
					thread::SRWReadLock readLock(&m_srwLock_pipelines);

					auto iter = m_umapRenderPipelines.find(psoKey);
					if (iter != m_umapRenderPipelines.end())
						return &iter->second;
				}

				RenderPipeline* pRenderPipeline = nullptr;
				bool isRequestCreatePipeline = false;
				{
					thread::SRWWriteLock writeLock(&m_srwLock_pipelines);

					auto iter = m_umapRenderPipelines.find(psoKey);
					if (iter != m_umapRenderPipelines.end())
					{
						pRenderPipeline = &iter->second;
					}
					else
					{
						RenderPipeline& renderPipeline = m_umapRenderPipelines[psoKey];
						renderPipeline.psoCache.pPipelineState = nullptr;
						renderPipeline.psoCache.pRootSignature = nullptr;
						renderPipeline.rootParameterIndex.fill(0);

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

					CreatePipelineState(pDevice, psoKey, pRenderPipeline);

					stopwatch.Stop();
					LOG_MESSAGE(L"CreatePipelineState[%u_%d_%d_%d] : ElapsedTime[%lf]", psoKey.mask, psoKey.emRasterizerState, psoKey.emBlendState, psoKey.emDepthStencilState, stopwatch.Elapsed());
					//});
				}

				return pRenderPipeline;
			}

			void ModelRenderer::Impl::RenderStaticModel(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, Group emGroup, shader::Pass emPass, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t nRTVHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();
				DescriptorHeap* pSamplerDescriptorHeap = pDeviceInstance->GetSamplerDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				m_umapJobStaticMasterBatchs.clear();

				if (m_jobStaticCount[emGroup] > 0)
				{
					bool isEnableVelocityMotionBlur = false;
					if (GetOptions().OnMotionBlur == true && GetOptions().motionBlurConfig.IsVelocityMotionBlur() == true)
					{
						isEnableVelocityMotionBlur = true;
					}

					for (size_t i = 0; i < m_jobStaticCount[emGroup]; ++i)
					{
						const JobStatic& job = m_jobStatics[emGroup][i];
						if (job.isCulled == true)
							continue;

						UMapJobStaticBatch& umapJobStaticBatch = m_umapJobStaticMasterBatchs[job.data.pMaterial];

						auto iter = umapJobStaticBatch.find(job.data.pKey);
						if (iter != umapJobStaticBatch.end())
						{
							iter.value().instanceData.emplace_back(job.data.matWorld.Transpose());
							iter.value().prevInstanceData.emplace_back(job.data.matWorld.Transpose());
						}
						else
						{
							umapJobStaticBatch.emplace(job.data.pKey, JobStaticBatch(&job, job.data.matWorld.Transpose(), job.data.matPrevWorld.Transpose()));
						}
					}

					tsl::robin_map<PSOKey, std::vector<const JobStaticBatch*>> umapJobStaticMaskBatch;
					umapJobStaticMaskBatch.rehash(m_umapJobStaticMasterBatchs.size());

					for (auto& iter_master : m_umapJobStaticMasterBatchs)
					{
						const UMapJobStaticBatch& umapJobStaticBatch = iter_master.second;

						for (auto& iter : umapJobStaticBatch)
						{
							const JobStaticBatch& jobBatch = iter.second;

							uint32_t mask = 0;
							auto iter_find = umapMaterialMask.find(jobBatch.pJob->data.pMaterial.get());
							if (iter_find != umapMaterialMask.end())
							{
								mask = iter_find->second;
							}
							else
							{
								mask = shader::GetMaterialMask(jobBatch.pJob->data.pMaterial.get());
								umapMaterialMask.emplace(jobBatch.pJob->data.pMaterial.get(), mask);
							}

							if (jobBatch.instanceData.size() > 1)
							{
								mask |= shader::eUseInstancing;
							}

							if (emGroup == Group::eAlphaBlend)
							{
								mask |= shader::eUseAlphaBlending;
							}

							if (isEnableVelocityMotionBlur == true)
							{
								mask |= shader::eUseMotionBlur;
							}

							PSOKey maskKey = GetPSOKey(emGroup, emPass, mask, jobBatch.pJob->data.pMaterial.get());
							umapJobStaticMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					const size_t nCommandListCount = pDeviceInstance->GetCommandListCount();
					std::vector<ID3D12GraphicsCommandList2*> vecCommandLists(nCommandListCount);
					pDeviceInstance->GetCommandLists(vecCommandLists.data());

					auto iter = umapJobStaticMaskBatch.begin();

					jobsystem::ParallelFor(vecCommandLists.size(), [&](const size_t index)
					{
						const IVertexBuffer* pPrevVertexBuffer = nullptr;
						const IIndexBuffer* pPrevIndexBuffer = nullptr;

						ID3D12GraphicsCommandList2* pCommandList = vecCommandLists[index];
						pDeviceInstance->ResetCommandList(index, nullptr);

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

							const PSOKey* pPSOKey = nullptr;
							std::vector<const JobStaticBatch*>* pJobBatchs = nullptr;
							{
								{
									thread::SRWWriteLock writeLock(&m_srwLock_logic);

									if (iter == umapJobStaticMaskBatch.end())
										break;

									pPSOKey = &iter->first;
									pJobBatchs = &iter.value();
									++iter;
								}

								pRenderPipeline = CreateRenderPipeline(pDevice, *pPSOKey);

								if (pRenderPipeline == nullptr || pRenderPipeline->psoCache.pPipelineState == nullptr || pRenderPipeline->psoCache.pRootSignature == nullptr)
								{
									const uint32_t mask = (pPSOKey->mask & shader::eUseAlphaBlending) | (pPSOKey->mask & shader::eUseInstancing);

									const PSOKey psoDefaultStaticKey(mask, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
									pRenderPipeline = CreateRenderPipeline(pDevice, psoDefaultStaticKey);
									if (pRenderPipeline == nullptr || pRenderPipeline->psoCache.pPipelineState == nullptr || pRenderPipeline->psoCache.pRootSignature == nullptr)
									{
										throw_line("invalid default static pipeline state");
										continue;
									}
								}
							}

							pCommandList->SetPipelineState(pRenderPipeline->psoCache.pPipelineState);
							pCommandList->SetGraphicsRootSignature(pRenderPipeline->psoCache.pRootSignature);

							if (pPSOKey->emBlendState != EmBlendState::eOff)
							{
								pCommandList->OMSetBlendFactor(&math::float4::Zero.x);
							}

							if (pRenderPipeline->rootParameterIndex[eRP_StandardDescriptor] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootDescriptorTable(
									pRenderPipeline->rootParameterIndex[eRP_StandardDescriptor],
									pSRVDescriptorHeap->GetStartGPUHandle());
							}

							if (pRenderPipeline->rootParameterIndex[eRP_SamplerStates] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootDescriptorTable(
									pRenderPipeline->rootParameterIndex[eRP_SamplerStates],
									pSamplerDescriptorHeap->GetStartGPUHandle());
							}

							if (pRenderPipeline->rootParameterIndex[eRP_VSConstantsCB] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootConstantBufferView(
									pRenderPipeline->rootParameterIndex[eRP_VSConstantsCB],
									m_vsConstantsBuffer.GPUAddress(frameIndex));
							}

							if (pRenderPipeline->rootParameterIndex[eRP_CommonContentsCB] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootConstantBufferView(
									pRenderPipeline->rootParameterIndex[eRP_CommonContentsCB],
									m_commonContentsBuffer.GPUAddress(frameIndex));
							}

							if ((pPSOKey->mask & shader::eUseInstancing) == shader::eUseInstancing)
							{
								for (auto& pJobBatch : (*pJobBatchs))
								{
									const RenderJobStatic& job = pJobBatch->pJob->data;

									if (pRenderPipeline->rootParameterIndex[eRP_ObjectDataCB] != eRP_InvalidIndex)
									{
										const size_t objectBufferIndex = m_objectBufferIndex++;

										shader::ObjectDataBuffer* pBuffer = m_objectDataBuffer.Cast(frameIndex, objectBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_objectDataBuffer.GPUAddress(frameIndex, objectBufferIndex);

										shader::SetObjectData(pBuffer, job.pMaterial.get(), math::Matrix::Identity, math::Matrix::Identity, 0, 0);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_ObjectDataCB], gpuAddress);
									}

									if (pRenderPipeline->rootParameterIndex[eRP_SRVIndicesCB] != eRP_InvalidIndex)
									{
										const size_t srvBufferIndex = m_srvBufferIndex++;

										shader::MaterialSRVIndexConstants* pBuffer = m_materialSRVIndexConstantsBuffer.Cast(frameIndex, srvBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_materialSRVIndexConstantsBuffer.GPUAddress(frameIndex, srvBufferIndex);

										shader::SetMaterial(pBuffer, job.pMaterial.get());

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_SRVIndicesCB], gpuAddress);
									}

									const math::Matrix* pInstanceData = pJobBatch->instanceData.data();
									const math::Matrix* pPrevInstanceData = pJobBatch->prevInstanceData.data();
									const size_t instanceCount = pJobBatch->instanceData.size();

									const size_t loopCount = instanceCount / eMaxInstancingCount + 1;
									for (size_t i = 0; i < loopCount; ++i)
									{
										const size_t enableDrawCount = std::min(eMaxInstancingCount * (i + 1), instanceCount);
										const size_t drawInstanceCount = enableDrawCount - i * eMaxInstancingCount;

										if (drawInstanceCount <= 0)
											break;

										const size_t staticBufferIndex = m_staticBufferIndex++;

										shader::StaticInstancingDataBuffer* pStaticInstancingData = m_staticInstancingDataBuffer.Cast(frameIndex, staticBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_staticInstancingDataBuffer.GPUAddress(frameIndex, staticBufferIndex);

										memory::Copy(pStaticInstancingData->data.data(), sizeof(pStaticInstancingData->data),
											&pInstanceData[i * eMaxInstancingCount], sizeof(math::Matrix) * drawInstanceCount);

										memory::Copy(pStaticInstancingData->prevData.data(), sizeof(pStaticInstancingData->prevData),
											&pPrevInstanceData[i * eMaxInstancingCount], sizeof(math::Matrix) * drawInstanceCount);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_StaticInstancingDataCB], gpuAddress);

										if (pPrevVertexBuffer != job.pVertexBuffer.get())
										{
											const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(job.pVertexBuffer.get());
											pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

											pPrevVertexBuffer = job.pVertexBuffer.get();
										}

										if (job.pIndexBuffer != nullptr)
										{
											if (pPrevIndexBuffer != job.pIndexBuffer.get())
											{
												const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(job.pIndexBuffer.get());
												pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());

												pPrevIndexBuffer = job.pIndexBuffer.get();
											}

											pCommandList->DrawIndexedInstanced(job.indexCount, static_cast<uint32_t>(drawInstanceCount), job.startIndex, 0, 0);
										}
										else
										{
											pCommandList->IASetIndexBuffer(nullptr);
											pPrevIndexBuffer = nullptr;

											pCommandList->DrawInstanced(job.indexCount, static_cast<uint32_t>(drawInstanceCount), 0, 0);
										}
									}
								}
							}
							else
							{
								std::sort(pJobBatchs->begin(), pJobBatchs->end(), [](const JobStaticBatch* a, const JobStaticBatch* b)
								{
									return a->pJob->data.depth < b->pJob->data.depth;
								});

								for (auto& pJobBatch : (*pJobBatchs))
								{
									const RenderJobStatic& job = pJobBatch->pJob->data;

									if (pRenderPipeline->rootParameterIndex[eRP_SRVIndicesCB] != eRP_InvalidIndex)
									{
										const size_t srvBufferIndex = m_srvBufferIndex++;

										shader::MaterialSRVIndexConstants* pBuffer = m_materialSRVIndexConstantsBuffer.Cast(frameIndex, srvBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_materialSRVIndexConstantsBuffer.GPUAddress(frameIndex, srvBufferIndex);

										shader::SetMaterial(pBuffer, job.pMaterial.get());

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_SRVIndicesCB], gpuAddress);
									}

									if (pRenderPipeline->rootParameterIndex[eRP_ObjectDataCB] != eRP_InvalidIndex)
									{
										const size_t objectBufferIndex = m_objectBufferIndex++;

										shader::ObjectDataBuffer* pBuffer = m_objectDataBuffer.Cast(frameIndex, objectBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_objectDataBuffer.GPUAddress(frameIndex, objectBufferIndex);

										shader::SetObjectData(pBuffer, job.pMaterial.get(), job.matWorld, job.matPrevWorld, 0, 0);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_ObjectDataCB], gpuAddress);
									}

									if (pPrevVertexBuffer != job.pVertexBuffer.get())
									{
										const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(job.pVertexBuffer.get());
										pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

										pPrevVertexBuffer = job.pVertexBuffer.get();
									}

									if (job.pIndexBuffer != nullptr)
									{
										if (pPrevIndexBuffer != job.pIndexBuffer.get())
										{
											const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(job.pIndexBuffer.get());
											pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());

											pPrevIndexBuffer = job.pIndexBuffer.get();
										}

										pCommandList->DrawIndexedInstanced(job.indexCount, 1, job.startIndex, 0, 0);
									}
									else
									{
										pCommandList->IASetIndexBuffer(nullptr);
										pPrevIndexBuffer = nullptr;

										pCommandList->DrawInstanced(job.indexCount, 1, 0, 0);
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

			void ModelRenderer::Impl::RenderSkinnedModel(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, Group emGroup, shader::Pass emPass, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t nRTVHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();
				DescriptorHeap* pSamplerDescriptorHeap = pDeviceInstance->GetSamplerDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				m_umapJobSkinnedMasterBatchs.clear();

				if (m_jobSkinnedCount[emGroup] > 0)
				{
					bool isEnableVelocityMotionBlur = false;
					if (GetOptions().OnMotionBlur == true && GetOptions().motionBlurConfig.IsVelocityMotionBlur() == true)
					{
						isEnableVelocityMotionBlur = true;
					}

					for (size_t i = 0; i < m_jobSkinnedCount[emGroup]; ++i)
					{
						const JobSkinned& job = m_jobSkinneds[emGroup][i];
						if (job.isCulled == true)
							continue;

						UMapJobSkinnedBatch& umapJobSkinnedBatch = m_umapJobSkinnedMasterBatchs[job.data.pMaterial];

						auto iter = umapJobSkinnedBatch.find(job.data.pKey);
						if (iter != umapJobSkinnedBatch.end())
						{
							iter.value().instanceData.emplace_back(job.data.matWorld, job.data.VTFID);
							iter.value().prevInstanceData.emplace_back(job.data.matPrevWorld, job.data.PrevVTFID);
						}
						else
						{
							umapJobSkinnedBatch.emplace(job.data.pKey, JobSkinnedBatch(&job, job.data.matWorld, job.data.matPrevWorld, job.data.VTFID, job.data.PrevVTFID));
						}
					}

					tsl::robin_map<PSOKey, std::vector<const JobSkinnedBatch*>> umapJobSkinnedMaskBatch;
					umapJobSkinnedMaskBatch.rehash(m_umapJobSkinnedMasterBatchs.size());

					for (auto& iter_master : m_umapJobSkinnedMasterBatchs)
					{
						const UMapJobSkinnedBatch& umapJobSkinnedBatch = iter_master.second;

						for (auto& iter : umapJobSkinnedBatch)
						{
							const JobSkinnedBatch& jobBatch = iter.second;

							uint32_t mask = 0;
							auto iter_find = umapMaterialMask.find(jobBatch.pJob->data.pMaterial.get());
							if (iter_find != umapMaterialMask.end())
							{
								mask = iter_find->second;
							}
							else
							{
								mask = shader::GetMaterialMask(jobBatch.pJob->data.pMaterial.get());
								umapMaterialMask.emplace(jobBatch.pJob->data.pMaterial.get(), mask);
							}

							if (jobBatch.instanceData.size() > 1)
							{
								mask |= shader::eUseInstancing;
							}

							if (emGroup == Group::eAlphaBlend)
							{
								mask |= shader::eUseAlphaBlending;
							}

							if (isEnableVelocityMotionBlur == true)
							{
								mask |= shader::eUseMotionBlur;
							}

							mask |= shader::eUseSkinning;

							PSOKey maskKey = GetPSOKey(emGroup, emPass, mask, jobBatch.pJob->data.pMaterial.get());
							umapJobSkinnedMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					const size_t nCommandListCount = pDeviceInstance->GetCommandListCount();
					std::vector<ID3D12GraphicsCommandList2*> vecCommandLists(nCommandListCount);
					pDeviceInstance->GetCommandLists(vecCommandLists.data());

					auto iter = umapJobSkinnedMaskBatch.begin();

					jobsystem::ParallelFor(vecCommandLists.size(), [&](const size_t index)
					{
						const IVertexBuffer* pPrevVertexBuffer = nullptr;
						const IIndexBuffer* pPrevIndexBuffer = nullptr;

						ID3D12GraphicsCommandList2* pCommandList = vecCommandLists[index];
						pDeviceInstance->ResetCommandList(index, nullptr);

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

							const PSOKey* pPSOKey = nullptr;
							std::vector<const JobSkinnedBatch*>* pJobBatchs = nullptr;
							{
								{
									thread::SRWWriteLock writeLock(&m_srwLock_logic);

									if (iter == umapJobSkinnedMaskBatch.end())
										break;

									pPSOKey = &iter->first;
									pJobBatchs = &iter.value();
									++iter;
								}

								pRenderPipeline = CreateRenderPipeline(pDevice, *pPSOKey);

								if (pRenderPipeline == nullptr || pRenderPipeline->psoCache.pPipelineState == nullptr || pRenderPipeline->psoCache.pRootSignature == nullptr)
								{
									const uint32_t mask = shader::eUseSkinning | (pPSOKey->mask & shader::eUseAlphaBlending) | (pPSOKey->mask & shader::eUseInstancing);

									const PSOKey psoDefaultSkinnedKey(mask, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Write_On);
									pRenderPipeline = CreateRenderPipeline(pDevice, psoDefaultSkinnedKey);
									if (pRenderPipeline == nullptr || pRenderPipeline->psoCache.pPipelineState == nullptr || pRenderPipeline->psoCache.pRootSignature == nullptr)
									{
										throw_line("invalid default skinned pipeline state");
										continue;
									}
								}
							}

							pCommandList->SetPipelineState(pRenderPipeline->psoCache.pPipelineState);
							pCommandList->SetGraphicsRootSignature(pRenderPipeline->psoCache.pRootSignature);

							if (pPSOKey->emBlendState != EmBlendState::eOff)
							{
								pCommandList->OMSetBlendFactor(&math::float4::Zero.x);
							}

							if (pRenderPipeline->rootParameterIndex[eRP_StandardDescriptor] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootDescriptorTable(
									pRenderPipeline->rootParameterIndex[eRP_StandardDescriptor],
									pSRVDescriptorHeap->GetStartGPUHandle());
							}

							if (pRenderPipeline->rootParameterIndex[eRP_SamplerStates] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootDescriptorTable(
									pRenderPipeline->rootParameterIndex[eRP_SamplerStates],
									pSamplerDescriptorHeap->GetStartGPUHandle());
							}

							if (pRenderPipeline->rootParameterIndex[eRP_VSConstantsCB] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootConstantBufferView(
									pRenderPipeline->rootParameterIndex[eRP_VSConstantsCB],
									m_vsConstantsBuffer.GPUAddress(frameIndex));
							}

							if (pRenderPipeline->rootParameterIndex[eRP_CommonContentsCB] != eRP_InvalidIndex)
							{
								pCommandList->SetGraphicsRootConstantBufferView(
									pRenderPipeline->rootParameterIndex[eRP_CommonContentsCB],
									m_commonContentsBuffer.GPUAddress(frameIndex));
							}

							if ((pPSOKey->mask & shader::eUseInstancing) == shader::eUseInstancing)
							{
								for (auto& pJobBatch : (*pJobBatchs))
								{
									const RenderJobSkinned& job = pJobBatch->pJob->data;

									if (pRenderPipeline->rootParameterIndex[eRP_ObjectDataCB] != eRP_InvalidIndex)
									{
										const size_t objectBufferIndex = m_objectBufferIndex++;

										shader::ObjectDataBuffer* pBuffer = m_objectDataBuffer.Cast(frameIndex, objectBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_objectDataBuffer.GPUAddress(frameIndex, objectBufferIndex);

										shader::SetObjectData(pBuffer, job.pMaterial.get(), math::Matrix::Identity, math::Matrix::Identity, 0, 0);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_ObjectDataCB], gpuAddress);
									}

									if (pRenderPipeline->rootParameterIndex[eRP_SRVIndicesCB] != eRP_InvalidIndex)
									{
										const size_t srvBufferIndex = m_srvBufferIndex++;

										shader::MaterialSRVIndexConstants* pBuffer = m_materialSRVIndexConstantsBuffer.Cast(frameIndex, srvBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_materialSRVIndexConstantsBuffer.GPUAddress(frameIndex, srvBufferIndex);

										shader::SetMaterial(pBuffer, job.pMaterial.get());

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_SRVIndicesCB], gpuAddress);
									}

									const SkinningInstancingData* pInstanceData = pJobBatch->instanceData.data();
									const SkinningInstancingData* pPrevInstanceData = pJobBatch->prevInstanceData.data();
									const size_t instanceCount = pJobBatch->instanceData.size();

									const size_t loopCount = instanceCount / eMaxInstancingCount + 1;
									for (size_t i = 0; i < loopCount; ++i)
									{
										const size_t enableDrawCount = std::min(eMaxInstancingCount * (i + 1), instanceCount);
										const size_t drawInstanceCount = enableDrawCount - i * eMaxInstancingCount;

										if (drawInstanceCount <= 0)
											break;

										const size_t nSkinningBufferIndex = m_skinningBufferIndex++;

										shader::SkinningInstancingDataBuffer* pSkinnedInstancingData = m_skinningInstancingDataBuffer.Cast(frameIndex, nSkinningBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_skinningInstancingDataBuffer.GPUAddress(frameIndex, nSkinningBufferIndex);

										memory::Copy(pSkinnedInstancingData->data.data(), sizeof(pSkinnedInstancingData->data),
											&pInstanceData[i * eMaxInstancingCount], sizeof(SkinningInstancingData) * drawInstanceCount);

										memory::Copy(pSkinnedInstancingData->prevData.data(), sizeof(pSkinnedInstancingData->prevData),
											&pPrevInstanceData[i * eMaxInstancingCount], sizeof(SkinningInstancingData) * drawInstanceCount);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_SkinningInstancingDataCB], gpuAddress);

										if (pPrevVertexBuffer != job.pVertexBuffer.get())
										{
											const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(job.pVertexBuffer.get());
											pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

											pPrevVertexBuffer = job.pVertexBuffer.get();
										}

										if (job.pIndexBuffer != nullptr)
										{
											if (pPrevIndexBuffer != job.pIndexBuffer.get())
											{
												const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(job.pIndexBuffer.get());
												pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());

												pPrevIndexBuffer = job.pIndexBuffer.get();
											}

											pCommandList->DrawIndexedInstanced(job.indexCount, static_cast<uint32_t>(drawInstanceCount), job.startIndex, 0, 0);
										}
										else
										{
											pCommandList->IASetIndexBuffer(nullptr);
											pPrevIndexBuffer = nullptr;

											pCommandList->DrawInstanced(job.indexCount, static_cast<uint32_t>(drawInstanceCount), 0, 0);
										}
									}
								}
							}
							else
							{
								std::sort(pJobBatchs->begin(), pJobBatchs->end(), [](const JobSkinnedBatch* a, const JobSkinnedBatch* b)
								{
									return a->pJob->data.depth < b->pJob->data.depth;
								});

								for (auto& pJobBatch : (*pJobBatchs))
								{
									const RenderJobSkinned& job = pJobBatch->pJob->data;

									if (pRenderPipeline->rootParameterIndex[eRP_SRVIndicesCB] != eRP_InvalidIndex)
									{
										const size_t srvBufferIndex = m_srvBufferIndex++;

										shader::MaterialSRVIndexConstants* pBuffer = m_materialSRVIndexConstantsBuffer.Cast(frameIndex, srvBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_materialSRVIndexConstantsBuffer.GPUAddress(frameIndex, srvBufferIndex);

										shader::SetMaterial(pBuffer, job.pMaterial.get());

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_SRVIndicesCB], gpuAddress);
									}

									if (pRenderPipeline->rootParameterIndex[eRP_ObjectDataCB] != eRP_InvalidIndex)
									{
										const size_t objectBufferIndex = m_objectBufferIndex++;

										shader::ObjectDataBuffer* pBuffer = m_objectDataBuffer.Cast(frameIndex, objectBufferIndex);
										D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_objectDataBuffer.GPUAddress(frameIndex, objectBufferIndex);

										shader::SetObjectData(pBuffer, job.pMaterial.get(), job.matWorld, job.matPrevWorld, job.VTFID, job.PrevVTFID);

										pCommandList->SetGraphicsRootConstantBufferView(pRenderPipeline->rootParameterIndex[eRP_ObjectDataCB], gpuAddress);
									}

									if (pPrevVertexBuffer != job.pVertexBuffer.get())
									{
										const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(job.pVertexBuffer.get());
										pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());

										pPrevVertexBuffer = job.pVertexBuffer.get();
									}

									if (job.pIndexBuffer != nullptr)
									{
										if (pPrevIndexBuffer != job.pIndexBuffer.get())
										{
											const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(job.pIndexBuffer.get());
											pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());

											pPrevIndexBuffer = job.pIndexBuffer.get();
										}

										pCommandList->DrawIndexedInstanced(job.indexCount, 1, job.startIndex, 0, 0);
									}
									else
									{
										pCommandList->IASetIndexBuffer(nullptr);
										pPrevIndexBuffer = nullptr;

										pCommandList->DrawInstanced(job.indexCount, 1, 0, 0);
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

			ID3D12RootSignature* ModelRenderer::Impl::CreateRootSignature(ID3D12Device* pDevice, uint32_t mask, std::array<uint32_t, eRP_Count>& rootParameterIndex_out)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> rootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = rootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_ALL);
				rootParameterIndex_out[eRP_StandardDescriptor] = static_cast<uint32_t>(rootParameters.size() - 1);

				D3D12_DESCRIPTOR_RANGE samplerRange{};
				samplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				samplerRange.NumDescriptors = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				samplerRange.BaseShaderRegister = 0;
				samplerRange.RegisterSpace = 0;
				samplerRange.OffsetInDescriptorsFromTableStart = 0;

				CD3DX12_ROOT_PARAMETER& samplerDescriptorTable = rootParameters.emplace_back();
				samplerDescriptorTable.InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_PIXEL);
				rootParameterIndex_out[eRP_SamplerStates] = static_cast<uint32_t>(rootParameters.size() - 1);

				CD3DX12_ROOT_PARAMETER& srvIndexParameter = rootParameters.emplace_back();
				srvIndexParameter.InitAsConstantBufferView(shader::eCB_SRVIndex, 0, D3D12_SHADER_VISIBILITY_PIXEL);
				rootParameterIndex_out[eRP_SRVIndicesCB] = static_cast<uint32_t>(rootParameters.size() - 1);

				CD3DX12_ROOT_PARAMETER& matrixParameter = rootParameters.emplace_back();
				matrixParameter.InitAsConstantBufferView(shader::eCB_VSConstants, 0, D3D12_SHADER_VISIBILITY_VERTEX);
				rootParameterIndex_out[eRP_VSConstantsCB] = static_cast<uint32_t>(rootParameters.size() - 1);

				CD3DX12_ROOT_PARAMETER& objectParameter = rootParameters.emplace_back();
				objectParameter.InitAsConstantBufferView(shader::eCB_ObjectData, 0, D3D12_SHADER_VISIBILITY_ALL);
				rootParameterIndex_out[eRP_ObjectDataCB] = static_cast<uint32_t>(rootParameters.size() - 1);

				if ((mask & shader::eUseInstancing) == shader::eUseInstancing)
				{
					CD3DX12_ROOT_PARAMETER& instancingParameter = rootParameters.emplace_back();
					if ((mask & shader::eUseSkinning) == shader::eUseSkinning)
					{
						instancingParameter.InitAsConstantBufferView(shader::eCB_SkinningInstancingData, 0, D3D12_SHADER_VISIBILITY_VERTEX);
						rootParameterIndex_out[eRP_SkinningInstancingDataCB] = static_cast<uint32_t>(rootParameters.size() - 1);
					}
					else
					{
						instancingParameter.InitAsConstantBufferView(shader::eCB_StaticInstancingData, 0, D3D12_SHADER_VISIBILITY_VERTEX);
						rootParameterIndex_out[eRP_StaticInstancingDataCB] = static_cast<uint32_t>(rootParameters.size() - 1);
					}
				}

				std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
				if ((mask & shader::eUseAlphaBlending) == shader::eUseAlphaBlending)
				{
					CD3DX12_ROOT_PARAMETER& commonContentsParameter = rootParameters.emplace_back();
					commonContentsParameter.InitAsConstantBufferView(shader::eCB_CommonContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);
					rootParameterIndex_out[eRP_CommonContentsCB] = static_cast<uint32_t>(rootParameters.size() - 1);

					staticSamplers =
					{
						util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipPointClamp, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
						util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipLinearClamp, 1, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					};
				}

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(rootParameters.size()), rootParameters.data(),
					static_cast<uint32_t>(staticSamplers.size()), staticSamplers.data(),
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void ModelRenderer::Impl::CreatePipelineState(ID3D12Device* pDevice, const PSOKey& psoKey, RenderPipeline* pRenderPipeline)
			{
				if (pRenderPipeline->psoCache.pVSBlob == nullptr || pRenderPipeline->psoCache.pPSBlob == nullptr)
				{
					std::vector<D3D_SHADER_MACRO> vecMacros = shader::GetMacros(psoKey.mask);

					if (pRenderPipeline->psoCache.pVSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(m_pShaderBlob, vecMacros.data(), m_shaderPath.c_str(), "VS", shader::VS_CompileVersion, &pRenderPipeline->psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (pRenderPipeline->psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(m_pShaderBlob, vecMacros.data(), m_shaderPath.c_str(), "PS", shader::PS_CompileVersion, &pRenderPipeline->psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring debugName = string::Format(L"%u_%d_%d_%d", psoKey.mask, psoKey.emRasterizerState, psoKey.emBlendState, psoKey.emDepthStencilState);

				std::array<uint32_t, eRP_Count> rootParameterIndex;
				rootParameterIndex.fill(eRP_InvalidIndex);

				ID3D12RootSignature* pRootSignature = nullptr;
				if (pRenderPipeline->psoCache.pRootSignature != nullptr)
				{
					pRootSignature = pRenderPipeline->psoCache.pRootSignature;
					rootParameterIndex = pRenderPipeline->rootParameterIndex;
				}
				else
				{
					pRootSignature = CreateRootSignature(pDevice, psoKey.mask, rootParameterIndex);
					pRootSignature->SetName(debugName.c_str());
				}

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.VS.BytecodeLength = pRenderPipeline->psoCache.pVSBlob->GetBufferSize();
				psoDesc.VS.pShaderBytecode = pRenderPipeline->psoCache.pVSBlob->GetBufferPointer();

				psoDesc.PS.BytecodeLength = pRenderPipeline->psoCache.pPSBlob->GetBufferSize();
				psoDesc.PS.pShaderBytecode = pRenderPipeline->psoCache.pPSBlob->GetBufferPointer();

				const D3D12_INPUT_ELEMENT_DESC* pInputElements = nullptr;
				size_t nElementCount = 0;
				if ((psoKey.mask & shader::eUseSkinning) == shader::eUseSkinning)
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

				psoDesc.pRootSignature = pRootSignature;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.SampleMask = 0xffffffff;
				psoDesc.RasterizerState = util::GetRasterizerDesc(psoKey.emRasterizerState);
				psoDesc.BlendState = util::GetBlendDesc(psoKey.emBlendState);

				if ((psoKey.mask & shader::eUseAlphaBlending) == shader::eUseAlphaBlending)
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
				else if ((psoKey.mask & shader::eUseMotionBlur) == shader::eUseMotionBlur)
				{
					psoDesc.NumRenderTargets = GBufferTypeCount;
					psoDesc.RTVFormats[GBufferType::eNormals] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eNormals));
					psoDesc.RTVFormats[GBufferType::eColors] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eColors));
					psoDesc.RTVFormats[GBufferType::eDisneyBRDF] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eDisneyBRDF));
					psoDesc.RTVFormats[GBufferType::eVelocity] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eVelocity));
				}
				else
				{
					psoDesc.NumRenderTargets = GBufferTypeCount - 1;
					psoDesc.RTVFormats[GBufferType::eNormals] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eNormals));
					psoDesc.RTVFormats[GBufferType::eColors] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eColors));
					psoDesc.RTVFormats[GBufferType::eDisneyBRDF] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eDisneyBRDF));
				}

				psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(psoKey.emDepthStencilState);

				ID3D12PipelineState* pPipelineState = nullptr;
				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				pPipelineState->SetName(debugName.c_str());

				{
					thread::SRWWriteLock writeLock(&m_srwLock_pipelines);

					if (pRenderPipeline->psoCache.pPipelineState != nullptr)
					{
						util::ReleaseResource(pRenderPipeline->psoCache.pPipelineState);
						pRenderPipeline->psoCache.pPipelineState = nullptr;
					}

					pRenderPipeline->psoCache.pPipelineState = pPipelineState;
					pRenderPipeline->psoCache.pRootSignature = pRootSignature;
					pRenderPipeline->rootParameterIndex = rootParameterIndex;
				}
			}

			ModelRenderer::ModelRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			ModelRenderer::~ModelRenderer()
			{
			}

			void ModelRenderer::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void ModelRenderer::Render(const RenderElement& renderElement, Group emGroup, const math::Matrix& matPrevViewProjection)
			{
				m_pImpl->Render(renderElement, emGroup, matPrevViewProjection);
			}

			void ModelRenderer::Cleanup()
			{
				m_pImpl->Cleanup();
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