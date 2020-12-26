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
#include "LightResourceManagerDX12.h"
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
					math::Matrix worldMatrix;
					math::Matrix prevWorldMatrix;

					math::Color f4AlbedoColor;
					math::Color f4EmissiveColor;

					math::float4 paddingRoughMetEmi;
					math::float4 surSpecTintAniso;
					math::float4 sheenTintClearcoatGloss;

					float stippleTransparencyFactor{ 0.f };
					uint32_t VTFID{ 0 };
					uint32_t PrevVTFID{ 0 };

					float padding{ 0 };
				};

				struct VSConstantsBuffer
				{
					math::Matrix viewMatrix;
					math::Matrix viewProjectionMatrix;
					math::Matrix prevViewPrjectionMatrix;

					uint32_t nTexVTFIndex{ 0 };
					uint32_t nTexPrevVTFIndex{ 0 };
					math::float2 padding;
				};

				struct CommonContents
				{
					math::float3 cameraPosition;
					float farClip{ 0.f };

					uint32_t nTexDiffuseHDRIndex{ 0 };
					uint32_t nTexSpecularHDRIndex{ 0 };
					uint32_t nTexSpecularBRDFIndex{ 0 };
					float padding;
				};

				struct DirectionalLightBuffer
				{
					uint32_t directionalLightCount{ 0 };
					math::float3 padding;

					std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
				};

				struct PointLightBuffer
				{
					uint32_t pointLightCount{ 0 };
					math::float3 padding;

					std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
				};

				struct SpotLightBuffer
				{
					uint32_t spotLightCount{ 0 };
					math::float3 padding;

					std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};
				};

				struct ShadowBuffer
				{
					uint32_t cascadeShadowCount{ 0 };
					math::float3 padding;

					math::uint4 cascadeShadowIndex[ILight::eMaxDirectionalLightCount]{};
					std::array<CascadedShadowData, ILight::eMaxDirectionalLightCount> cascadedShadow{};
				};

				enum CBSlot
				{
					eCB_SkinningInstancingData = 0,
					eCB_StaticInstancingData,
					eCB_ObjectData,
					eCB_VSConstants,

					eCB_SRVIndex,

					eCB_CommonContents = 5,
					eCB_DirectionalLightBuffer = 6,
					eCB_PointLightBuffer = 7,
					eCB_SpotLightBuffer = 8,
					eCB_ShadowBuffer = 9,
				};

				enum Mask : uint32_t
				{
					eUseInstancing = 1 << MaterialMaskCount,
					eUseSkinning = 1 << (MaterialMaskCount + 1),
					eUseAlphaBlending = 1 << (MaterialMaskCount + 2),
					eUseMotionBlur = 1 << (MaterialMaskCount + 3),
					eUseWriteDepth = 1 << (shader::MaterialMaskCount + 4),

					MaskCount = MaterialMaskCount + 5,
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

				PSOKey GetPSOKey(ModelRenderer::Group emGroup, uint32_t mask, const IMaterial* pMaterial)
				{
					RasterizerState::Type rasterizerState = RasterizerState::eSolidCCW;
					BlendState::Type blendState = BlendState::eOff;
					DepthStencilState::Type depthStencilState = DepthStencilState::eRead_Write_On;

					if (pMaterial != nullptr)
					{
						blendState = pMaterial->GetBlendState();
						rasterizerState = pMaterial->GetRasterizerState();
						depthStencilState = pMaterial->GetDepthStencilState();
					}

					return { mask, rasterizerState, blendState, depthStencilState };
				}

				void SetObjectData(ObjectDataBuffer* pObjectDataBuffer,
					const IMaterial* pMaterial, const math::Matrix& worldMatrix, const math::Matrix& prevWorldMatrix, uint32_t VTFID, uint32_t PrevVTFID)
				{
					pObjectDataBuffer->worldMatrix = worldMatrix.Transpose();
					pObjectDataBuffer->prevWorldMatrix = prevWorldMatrix.Transpose();

					if (pMaterial != nullptr)
					{
						pObjectDataBuffer->f4AlbedoColor = pMaterial->GetAlbedoColor();
						pObjectDataBuffer->f4EmissiveColor = pMaterial->GetEmissiveColor();

						pObjectDataBuffer->paddingRoughMetEmi = pMaterial->GetPaddingRoughMetEmi();
						pObjectDataBuffer->surSpecTintAniso = pMaterial->GetSurSpecTintAniso();
						pObjectDataBuffer->sheenTintClearcoatGloss = pMaterial->GetSheenTintClearcoatGloss();

						pObjectDataBuffer->stippleTransparencyFactor = pMaterial->GetStippleTransparencyFactor();
						pObjectDataBuffer->VTFID = VTFID;
						pObjectDataBuffer->PrevVTFID = PrevVTFID;

						pObjectDataBuffer->padding = 0.f;
					}
					else
					{
						pObjectDataBuffer->f4AlbedoColor = math::Color::White;
						pObjectDataBuffer->f4EmissiveColor = math::Color::Black;

						pObjectDataBuffer->paddingRoughMetEmi = math::float4::Zero;
						pObjectDataBuffer->surSpecTintAniso = math::float4::Zero;
						pObjectDataBuffer->sheenTintClearcoatGloss = math::float4::Zero;

						pObjectDataBuffer->stippleTransparencyFactor = 0.f;
						pObjectDataBuffer->VTFID = VTFID;
						pObjectDataBuffer->PrevVTFID = PrevVTFID;

						pObjectDataBuffer->padding = 0.f;
					}
				}
			}

			class ModelRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Render(const RenderElement& renderElement, Group emGroup, const math::Matrix& prevViewPrjectionMatrixection);
				void AllCleanup();
				void Cleanup();

			public:
				void PushJob(const RenderJobStatic& job);
				void PushJob(const RenderJobSkinned& job);

			private:
				void RenderStaticModel(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, const math::Viewport& viewport, const math::Rect& scissorRect, Group emGroup, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t rtvHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask);
				void RenderSkinnedModel(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, const math::Viewport& viewport, const math::Rect& scissorRect, Group emGroup, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t rtvHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask);

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
					eRP_DirectionalLightCB,
					eRP_PointLIghtCB,
					eRP_SpotLightCB,
					eRP_ShadowCB,

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
				ConstantBuffer<shader::DirectionalLightBuffer> m_directionalLightBuffer;
				ConstantBuffer<shader::PointLightBuffer> m_pointLightBuffer;
				ConstantBuffer<shader::SpotLightBuffer> m_spotLightBuffer;
				ConstantBuffer<shader::ShadowBuffer> m_shadowBuffer;

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

					JobStaticBatch(const JobStatic* pJob, const math::Matrix& worldMatrix, const math::Matrix& prevWorldMatrix)
						: pJob(pJob)
					{
						instanceData.emplace_back(worldMatrix);
						prevInstanceData.emplace_back(prevWorldMatrix);
					}
				};

				std::array<std::array<JobStatic, shader::eMaxJobCount>, GroupCount> m_jobStatics[2];
				std::array<size_t, GroupCount> m_jobStaticCount[2]{};

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

					JobSkinnedBatch(const JobSkinned* pJob, const math::Matrix& worldMatrix, const math::Matrix& prevWorldMatrix, uint32_t VTFID, uint32_t PrevVTFID)
						: pJob(pJob)
					{
						instanceData.emplace_back(worldMatrix, VTFID);
						prevInstanceData.emplace_back(prevWorldMatrix, PrevVTFID);
					}
				};

				std::array<std::array<JobSkinned, shader::eMaxJobCount>, GroupCount> m_jobSkinneds[2];
				std::array<size_t, GroupCount> m_jobSkinnedCount[2]{};

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

				CreateRenderPipeline(pDevice, { 0, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseInstancing, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseAlphaBlending, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseInstancing | shader::eUseAlphaBlending, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On });

				CreateRenderPipeline(pDevice, { shader::eUseSkinning, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseSkinning | shader::eUseInstancing, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseSkinning | shader::eUseAlphaBlending, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On });
				CreateRenderPipeline(pDevice, { shader::eUseSkinning | shader::eUseInstancing | shader::eUseAlphaBlending, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On });

				m_skinningInstancingDataBuffer.Create(pDevice, shader::eMaxInstancingJobCount, "SkinningInstancingDataBuffer");
				m_staticInstancingDataBuffer.Create(pDevice, shader::eMaxInstancingJobCount, "StaticInstancingDataBuffer");
				m_objectDataBuffer.Create(pDevice, shader::eMaxJobCount, "ObjectDataBuffer");
				m_materialSRVIndexConstantsBuffer.Create(pDevice, shader::eMaxJobCount, "MaterialSRVIndexConstantsBuffer");

				constexpr size_t BufferCount = 1ui64 + (ILight::eMaxDirectionalLightCount * CascadedShadowsConfig::eMaxCascades);
				m_vsConstantsBuffer.Create(pDevice, BufferCount, "VSConstantsBuffer");
				m_commonContentsBuffer.Create(pDevice, 1, "CommonContentsBuffer");
				m_directionalLightBuffer.Create(pDevice, 1, "DirectionalLightBuffer");
				m_pointLightBuffer.Create(pDevice, 1, "PointLightBuffer");
				m_spotLightBuffer.Create(pDevice, 1, "SpotLightBuffer");
				m_shadowBuffer.Create(pDevice, 1, "ShadowBuffer");

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
				m_directionalLightBuffer.Destroy();
				m_pointLightBuffer.Destroy();
				m_spotLightBuffer.Destroy();
				m_shadowBuffer.Destroy();

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

			void ModelRenderer::Impl::Render(const RenderElement& renderElement, Group emGroup, const math::Matrix& prevViewPrjectionMatrixection)
			{
				if (m_jobStaticCount[RenderThread()][emGroup] == 0 && m_jobSkinnedCount[RenderThread()][emGroup] == 0)
					return;

				Device* pDeviceInstance = Device::GetInstance();
				LightResourceManager* pLightResourceManager = pDeviceInstance->GetLightResourceManager();
				if (emGroup == Group::eShadow)
				{
					if (pLightResourceManager->GetShadowCount(ILight::Type::eDirectional) == 0 &&
						pLightResourceManager->GetShadowCount(ILight::Type::ePoint) == 0 &&
						pLightResourceManager->GetShadowCount(ILight::Type::eSpot) == 0)
						return;
				}

				Camera* pCamera = renderElement.pCamera;

				TRACER_EVENT(__FUNCTIONW__);
				const OcclusionCulling* pOcclusionCulling = OcclusionCulling::GetInstance();
				if (emGroup == Group::eDeferred)
				{
					TRACER_EVENT(L"Culling");

					const collision::Frustum& frustum = pCamera->GetFrustum();
					jobsystem::ParallelFor(m_jobStaticCount[RenderThread()][emGroup], [&](size_t i)
					{
						JobStatic& job = m_jobStatics[RenderThread()][emGroup][i];
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

					jobsystem::ParallelFor(m_jobSkinnedCount[RenderThread()][emGroup], [&](size_t i)
					{
						JobSkinned& job = m_jobSkinneds[RenderThread()][emGroup][i];
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

				ID3D12Device* pDevice = pDeviceInstance->GetInterface();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();

				if (emGroup == Group::eAlphaBlend)
				{
					const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();

					{
						shader::CommonContents* pCommonContents = m_commonContentsBuffer.Cast(frameIndex);
						{
							pCommonContents->cameraPosition = pCamera->GetPosition();

							Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR().get());
							pCommonContents->nTexDiffuseHDRIndex = pDiffuseHDR->GetDescriptorIndex();

							Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR().get());
							pCommonContents->nTexSpecularHDRIndex = pSpecularHDR->GetDescriptorIndex();

							Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF().get());
							pCommonContents->nTexSpecularBRDFIndex = pSpecularBRDF->GetDescriptorIndex();
						}

						shader::DirectionalLightBuffer* pDirectionalLightBuffer = m_directionalLightBuffer.Cast(frameIndex);
						{
							const DirectionalLightData* pDirectionalLightData = nullptr;
							pLightResourceManager->GetDirectionalLightRenderData(&pDirectionalLightData, &pDirectionalLightBuffer->directionalLightCount);
							memory::Copy(pDirectionalLightBuffer->lightDirectional, pDirectionalLightData, sizeof(DirectionalLightData) * pDirectionalLightBuffer->directionalLightCount);
						}

						shader::PointLightBuffer* pPointLightBuffer = m_pointLightBuffer.Cast(frameIndex);
						{
							const PointLightData* pPointLightData = nullptr;
							pLightResourceManager->GetPointLightRenderData(&pPointLightData, &pPointLightBuffer->pointLightCount);
							memory::Copy(pPointLightBuffer->lightPoint, pPointLightData, sizeof(PointLightData) * pPointLightBuffer->pointLightCount);
						}

						shader::SpotLightBuffer* pSpotLightBuffer = m_spotLightBuffer.Cast(frameIndex);
						{
							const SpotLightData* pSpotLightData = nullptr;
							pLightResourceManager->GetSpotLightRenderData(&pSpotLightData, &pSpotLightBuffer->spotLightCount);
							memory::Copy(pSpotLightBuffer->lightSpot, pSpotLightData, sizeof(SpotLightData) * pSpotLightBuffer->spotLightCount);
						}

						shader::ShadowBuffer* pShadowBuffer = m_shadowBuffer.Cast(frameIndex);
						{
							*pShadowBuffer = {};

							const size_t lightCount = pLightResourceManager->GetLightCount(ILight::Type::eDirectional);
							for (size_t i = 0; i < lightCount; ++i)
							{
								DirectionalLightPtr pDirectionalLight = std::static_pointer_cast<IDirectionalLight>(pLightResourceManager->GetLight(ILight::Type::eDirectional, i));
								if (pDirectionalLight != nullptr)
								{
									DepthStencil* pDepthStencil = static_cast<DepthStencil*>(pDirectionalLight->GetDepthMapResource());
									if (pDepthStencil != nullptr)
									{
										const CascadedShadows& cascadedShadows = pDirectionalLight->GetCascadedShadows();

										const uint32_t index = pShadowBuffer->cascadeShadowCount;
										pShadowBuffer->cascadedShadow[index] = cascadedShadows.GetRenderData();
										pShadowBuffer->cascadeShadowIndex[index].x = pDepthStencil->GetTexture()->GetDescriptorIndex();

										++pShadowBuffer->cascadeShadowCount;
									}
								}
							}

							if (pShadowBuffer->cascadeShadowCount > 0)
							{
								ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
								pDeviceInstance->ResetCommandList(0, nullptr);

								for (size_t i = 0; i < lightCount; ++i)
								{
									DirectionalLightPtr pDirectionalLight = std::static_pointer_cast<IDirectionalLight>(pLightResourceManager->GetLight(ILight::Type::eDirectional, i));
									if (pDirectionalLight != nullptr)
									{
										DepthStencil* pDepthStencil = static_cast<DepthStencil*>(pDirectionalLight->GetDepthMapResource());
										if (pDepthStencil != nullptr)
										{
											util::ChangeResourceState(pCommandList, pDepthStencil, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
										}
									}
								}

								pCommandList->Close();
								pDeviceInstance->ExecuteCommandList(pCommandList);
							}
						}
					}
				}

				if (emGroup == Group::eDeferred || emGroup == Group::eAlphaBlend)
				{
					{
						VTFManager* pVTFManager = pDeviceInstance->GetVTFManager();
						Texture* pVTFTexture = pVTFManager->GetTexture();
						Texture* pPrevVTFTexture = pVTFManager->GetPrevTexture();

						m_vsConstantsBuffer.SetBufferIndex(0);

						shader::VSConstantsBuffer* pVSConstantsBuffer = m_vsConstantsBuffer.Cast(frameIndex);
						pVSConstantsBuffer->viewMatrix = pCamera->GetViewMatrix().Transpose();

						pVSConstantsBuffer->viewProjectionMatrix = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();
						pVSConstantsBuffer->viewProjectionMatrix = pVSConstantsBuffer->viewProjectionMatrix.Transpose();

						pVSConstantsBuffer->prevViewPrjectionMatrix = prevViewPrjectionMatrixection.Transpose();

						pVSConstantsBuffer->nTexVTFIndex = pVTFTexture->GetDescriptorIndex();
						pVSConstantsBuffer->nTexPrevVTFIndex = pPrevVTFTexture->GetDescriptorIndex();
					}

					tsl::robin_map<const IMaterial*, uint32_t> umapMaterialMask;
					umapMaterialMask.rehash(m_umapJobStaticMasterBatchs.size() + m_umapJobSkinnedMasterBatchs.size());

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

					const math::Viewport& viewport = pDeviceInstance->GetViewport();
					const math::Rect& scissorRect = pDeviceInstance->GetScissorRect();

					RenderStaticModel(pDeviceInstance, pDevice, pCamera, viewport, scissorRect, emGroup, renderElement.rtvHandles, renderElement.rtvCount, renderElement.GetDSVHandle(), umapMaterialMask);
					RenderSkinnedModel(pDeviceInstance, pDevice, pCamera, viewport, scissorRect, emGroup, renderElement.rtvHandles, renderElement.rtvCount, renderElement.GetDSVHandle(), umapMaterialMask);

					{
						ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
						pDeviceInstance->ResetCommandList(0, nullptr);

						util::ChangeResourceState(pCommandList, renderElement.pDepthStencil, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

						pCommandList->Close();
						pDeviceInstance->ExecuteCommandList(pCommandList);
					}
				}
				else if (emGroup == Group::eShadow)
				{
					tsl::robin_map<const IMaterial*, uint32_t> umapMaterialMask;
					umapMaterialMask.rehash(m_umapJobStaticMasterBatchs.size() + m_umapJobSkinnedMasterBatchs.size());

					uint32_t bufferIndex = 0;

					for (uint32_t i = 0; i < ILight::eCount; ++i)
					{
						const ILight::Type type = static_cast<ILight::Type>(i);

						const size_t lightCount = pLightResourceManager->GetLightCount(type);
						for (uint32_t j = 0; j < lightCount; ++j)
						{
							const LightPtr pLight = pLightResourceManager->GetLight(type, j);
							switch (type)
							{
							case ILight::Type::eDirectional:
							{
								IDirectionalLight* pDirectionalLight = static_cast<IDirectionalLight*>(pLight.get());
								const CascadedShadows& cascadedShadows = pDirectionalLight->GetCascadedShadows();
								const CascadedShadowsConfig& cascadedShadowsConfig = cascadedShadows.GetConfig();

								DepthStencil* pCascadedDepthStencil = pLightResourceManager->GetDepthStencil(pDeviceInstance, pDirectionalLight);
								if (pCascadedDepthStencil != nullptr)
								{
									if (pCascadedDepthStencil->GetResourceState() != D3D12_RESOURCE_STATE_DEPTH_WRITE)
									{
										ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
										pDeviceInstance->ResetCommandList(0, nullptr);

										util::ChangeResourceState(pCommandList, pCascadedDepthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);

										pCommandList->Close();
										pDeviceInstance->ExecuteCommandList(pCommandList);
									}

									const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = pCascadedDepthStencil->GetCPUHandle();

									for (uint32_t cascadeLevel = 0; cascadeLevel < cascadedShadowsConfig.numCascades; ++cascadeLevel)
									{
										const math::Matrix& viewMatrix = cascadedShadows.GetViewMatrix(cascadeLevel);
										math::Matrix projectionMatrix = cascadedShadows.GetProjectionMatrix(cascadeLevel);
										projectionMatrix._33 /= pCamera->GetProjection().farClip;
										projectionMatrix._43 /= pCamera->GetProjection().farClip;

										{
											VTFManager* pVTFManager = pDeviceInstance->GetVTFManager();
											Texture* pVTFTexture = pVTFManager->GetTexture();
											Texture* pPrevVTFTexture = pVTFManager->GetPrevTexture();

											m_vsConstantsBuffer.SetBufferIndex((ILight::eMaxDirectionalLightCount * bufferIndex) + cascadeLevel + 1);

											shader::VSConstantsBuffer* pVSConstantsBuffer = m_vsConstantsBuffer.Cast(frameIndex);
											pVSConstantsBuffer->viewMatrix = viewMatrix.Transpose();

											pVSConstantsBuffer->viewProjectionMatrix = viewMatrix * projectionMatrix;
											pVSConstantsBuffer->viewProjectionMatrix = pVSConstantsBuffer->viewProjectionMatrix.Transpose();

											pVSConstantsBuffer->prevViewPrjectionMatrix = prevViewPrjectionMatrixection.Transpose();

											pVSConstantsBuffer->nTexVTFIndex = pVTFTexture->GetDescriptorIndex();
											pVSConstantsBuffer->nTexPrevVTFIndex = pPrevVTFTexture->GetDescriptorIndex();
										}

										const math::Viewport& viewport = cascadedShadows.GetViewport(cascadeLevel);
										math::Rect scissorRect;
										scissorRect.left = 0;
										scissorRect.top = 0;
										scissorRect.right = static_cast<long>(viewport.width);
										scissorRect.bottom = static_cast<long>(viewport.height);

										RenderStaticModel(pDeviceInstance, pDevice, pCamera, viewport, scissorRect, emGroup, 0, 0, &dsvHandle, umapMaterialMask);
										RenderSkinnedModel(pDeviceInstance, pDevice, pCamera, viewport, scissorRect, emGroup, 0, 0, &dsvHandle, umapMaterialMask);
									}

									++bufferIndex;
								}
							}
							break;
							case ILight::Type::ePoint:
								break;
							case ILight::Type::eSpot:
							{
							}
							break;
							default:
								continue;
							}
						}
					}
				}
			}

			void ModelRenderer::Impl::AllCleanup()
			{
				for (int i = 0; i < GroupCount; ++i)
				{
					m_jobStatics[UpdateThread()][i].fill({});
					m_jobStatics[RenderThread()][i].fill({});

					m_jobSkinneds[UpdateThread()][i].fill({});
					m_jobSkinneds[RenderThread()][i].fill({});
				}
			}

			void ModelRenderer::Impl::Cleanup()
			{
				m_jobStaticCount[RenderThread()].fill(0);
				m_jobSkinnedCount[RenderThread()].fill(0);

				m_skinningBufferIndex = 0;
				m_staticBufferIndex = 0;
				m_objectBufferIndex = 0;
				m_srvBufferIndex = 0;
			}

			void ModelRenderer::Impl::PushJob(const RenderJobStatic& job)
			{
				const MaterialPtr& pMaterial = job.pMaterial;
				if (pMaterial == nullptr || pMaterial->GetBlendState() == BlendState::eOff)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_jobStatic);
					{
						const size_t index = m_jobStaticCount[UpdateThread()][eDeferred];
						m_jobStatics[UpdateThread()][eDeferred][index].Set(job);
						++m_jobStaticCount[UpdateThread()][eDeferred];
					}

					{
						const size_t index = m_jobStaticCount[UpdateThread()][eShadow];
						m_jobStatics[UpdateThread()][eShadow][index].Set(job);
						++m_jobStaticCount[UpdateThread()][eShadow];
					}
				}
				else
				{
					thread::SRWWriteLock writeLock(&m_srwLock_jobStatic);
					{
						const size_t index = m_jobStaticCount[UpdateThread()][eAlphaBlend];
						m_jobStatics[UpdateThread()][eAlphaBlend][index].Set(job);
						++m_jobStaticCount[UpdateThread()][eAlphaBlend];
					}
				}
			}

			void ModelRenderer::Impl::PushJob(const RenderJobSkinned& job)
			{
				const MaterialPtr& pMaterial = job.pMaterial;
				if (pMaterial == nullptr || pMaterial->GetBlendState() == BlendState::eOff)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_jobSkinned);
					{
						const size_t index = m_jobSkinnedCount[UpdateThread()][eDeferred];
						m_jobSkinneds[UpdateThread()][eDeferred][index].Set(job);
						++m_jobSkinnedCount[UpdateThread()][eDeferred];
					}

					{
						const size_t index = m_jobSkinnedCount[UpdateThread()][eShadow];
						m_jobSkinneds[UpdateThread()][eShadow][index].Set(job);
						++m_jobSkinnedCount[UpdateThread()][eShadow];
					}
				}
				else
				{
					thread::SRWWriteLock writeLock(&m_srwLock_jobSkinned);
					{
						const size_t index = m_jobSkinnedCount[UpdateThread()][eAlphaBlend];
						m_jobSkinneds[UpdateThread()][eAlphaBlend][index].Set(job);
						++m_jobSkinnedCount[UpdateThread()][eAlphaBlend];
					}
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
					LOG_MESSAGE(L"CreatePipelineState[%u_%d_%d_%d] : ElapsedTime[%lf]", psoKey.mask, psoKey.rasterizerState, psoKey.blendState, psoKey.depthStencilState, stopwatch.Elapsed());
					//});
				}

				return pRenderPipeline;
			}

			void ModelRenderer::Impl::RenderStaticModel(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, const math::Viewport& viewport, const math::Rect& scissorRect, Group emGroup, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t rtvHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();
				DescriptorHeap* pSamplerDescriptorHeap = pDeviceInstance->GetSamplerDescriptorHeap();

				m_umapJobStaticMasterBatchs.clear();

				if (m_jobStaticCount[RenderThread()][emGroup] > 0)
				{
					bool isEnableVelocityMotionBlur = false;
					if (RenderOptions().OnMotionBlur == true && RenderOptions().motionBlurConfig.IsVelocityMotionBlur() == true)
					{
						isEnableVelocityMotionBlur = true;
					}

					for (size_t i = 0; i < m_jobStaticCount[RenderThread()][emGroup]; ++i)
					{
						const JobStatic& job = m_jobStatics[RenderThread()][emGroup][i];
						if (job.isCulled == true)
							continue;

						UMapJobStaticBatch& umapJobStaticBatch = m_umapJobStaticMasterBatchs[job.data.pMaterial];

						auto iter = umapJobStaticBatch.find(job.data.pKey);
						if (iter != umapJobStaticBatch.end())
						{
							iter.value().instanceData.emplace_back(job.data.worldMatrix.Transpose());
							iter.value().prevInstanceData.emplace_back(job.data.worldMatrix.Transpose());
						}
						else
						{
							umapJobStaticBatch.emplace(job.data.pKey, JobStaticBatch(&job, job.data.worldMatrix.Transpose(), job.data.prevWorldMatrix.Transpose()));
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
								if (emGroup != Group::eShadow)
								{
									mask = shader::GetMaterialMask(jobBatch.pJob->data.pMaterial.get());
								}
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
							else if (emGroup == Group::eShadow)
							{
								mask |= shader::eUseWriteDepth;
							}

							if (isEnableVelocityMotionBlur == true)
							{
								mask |= shader::eUseMotionBlur;
							}

							PSOKey maskKey = shader::GetPSOKey(emGroup, mask, jobBatch.pJob->data.pMaterial.get());
							umapJobStaticMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
					pDeviceInstance->ResetCommandList(0, nullptr);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pSRVDescriptorHeap->GetHeap(),
						pSamplerDescriptorHeap->GetHeap(),
					};
					pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					pCommandList->RSSetViewports(1, util::Convert(viewport));
					pCommandList->RSSetScissorRects(1, &scissorRect);
					pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

					pCommandList->OMSetRenderTargets(static_cast<uint32_t>(rtvHandleCount), pRTVHandles, FALSE, pDSVHandle);

					for (auto iter = umapJobStaticMaskBatch.begin(); iter != umapJobStaticMaskBatch.end(); ++iter)
					{
						const IVertexBuffer* pPrevVertexBuffer = nullptr;
						const IIndexBuffer* pPrevIndexBuffer = nullptr;

						const RenderPipeline* pRenderPipeline = nullptr;

						const PSOKey& psoKey = iter->first;
						std::vector<const JobStaticBatch*>& jobBatchs = iter.value();
						{
							pRenderPipeline = CreateRenderPipeline(pDevice, psoKey);

							if (pRenderPipeline == nullptr || pRenderPipeline->psoCache.pPipelineState == nullptr || pRenderPipeline->psoCache.pRootSignature == nullptr)
							{
								const uint32_t mask = (psoKey.mask & shader::eUseAlphaBlending) | (psoKey.mask & shader::eUseInstancing);

								const PSOKey psoDefaultStaticKey(mask, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On);
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

						if (psoKey.blendState != BlendState::eOff)
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

						if (pRenderPipeline->rootParameterIndex[eRP_DirectionalLightCB] != eRP_InvalidIndex)
						{
							pCommandList->SetGraphicsRootConstantBufferView(
								pRenderPipeline->rootParameterIndex[eRP_DirectionalLightCB],
								m_directionalLightBuffer.GPUAddress(frameIndex));
						}

						if (pRenderPipeline->rootParameterIndex[eRP_PointLIghtCB] != eRP_InvalidIndex)
						{
							pCommandList->SetGraphicsRootConstantBufferView(
								pRenderPipeline->rootParameterIndex[eRP_PointLIghtCB],
								m_pointLightBuffer.GPUAddress(frameIndex));
						}

						if (pRenderPipeline->rootParameterIndex[eRP_SpotLightCB] != eRP_InvalidIndex)
						{
							pCommandList->SetGraphicsRootConstantBufferView(
								pRenderPipeline->rootParameterIndex[eRP_SpotLightCB],
								m_spotLightBuffer.GPUAddress(frameIndex));
						}

						if (pRenderPipeline->rootParameterIndex[eRP_ShadowCB] != eRP_InvalidIndex)
						{
							pCommandList->SetGraphicsRootConstantBufferView(
								pRenderPipeline->rootParameterIndex[eRP_ShadowCB],
								m_shadowBuffer.GPUAddress(frameIndex));
						}

						if ((psoKey.mask & shader::eUseInstancing) == shader::eUseInstancing)
						{
							for (auto& pJobBatch : jobBatchs)
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
							std::sort(jobBatchs.begin(), jobBatchs.end(), [](const JobStaticBatch* a, const JobStaticBatch* b)
								{
									return a->pJob->data.depth < b->pJob->data.depth;
								});

							for (auto& pJobBatch : jobBatchs)
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

									shader::SetObjectData(pBuffer, job.pMaterial.get(), job.worldMatrix, job.prevWorldMatrix, 0, 0);

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
					pDeviceInstance->ExecuteCommandList(pCommandList);
				}

				m_umapJobStaticMasterBatchs.clear();
			}

			void ModelRenderer::Impl::RenderSkinnedModel(Device* pDeviceInstance, ID3D12Device* pDevice, Camera* pCamera, const math::Viewport& viewport, const math::Rect& scissorRect, Group emGroup, const D3D12_CPU_DESCRIPTOR_HANDLE* pRTVHandles, size_t rtvHandleCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSVHandle, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();
				DescriptorHeap* pSamplerDescriptorHeap = pDeviceInstance->GetSamplerDescriptorHeap();

				m_umapJobSkinnedMasterBatchs.clear();

				if (m_jobSkinnedCount[RenderThread()][emGroup] > 0)
				{
					bool isEnableVelocityMotionBlur = false;
					if (RenderOptions().OnMotionBlur == true && RenderOptions().motionBlurConfig.IsVelocityMotionBlur() == true)
					{
						isEnableVelocityMotionBlur = true;
					}

					for (size_t i = 0; i < m_jobSkinnedCount[RenderThread()][emGroup]; ++i)
					{
						const JobSkinned& job = m_jobSkinneds[RenderThread()][emGroup][i];
						if (job.isCulled == true)
							continue;

						UMapJobSkinnedBatch& umapJobSkinnedBatch = m_umapJobSkinnedMasterBatchs[job.data.pMaterial];

						auto iter = umapJobSkinnedBatch.find(job.data.pKey);
						if (iter != umapJobSkinnedBatch.end())
						{
							iter.value().instanceData.emplace_back(job.data.worldMatrix, job.data.VTFID);
							iter.value().prevInstanceData.emplace_back(job.data.prevWorldMatrix, job.data.PrevVTFID);
						}
						else
						{
							umapJobSkinnedBatch.emplace(job.data.pKey, JobSkinnedBatch(&job, job.data.worldMatrix, job.data.prevWorldMatrix, job.data.VTFID, job.data.PrevVTFID));
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
								if (emGroup != Group::eShadow)
								{
									mask = shader::GetMaterialMask(jobBatch.pJob->data.pMaterial.get());
								}
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
							else if (emGroup == Group::eShadow)
							{
								mask |= shader::eUseWriteDepth;
							}

							if (isEnableVelocityMotionBlur == true)
							{
								mask |= shader::eUseMotionBlur;
							}

							mask |= shader::eUseSkinning;

							PSOKey maskKey = shader::GetPSOKey(emGroup, mask, jobBatch.pJob->data.pMaterial.get());
							umapJobSkinnedMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(1);
					pDeviceInstance->ResetCommandList(1, nullptr);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pSRVDescriptorHeap->GetHeap(),
						pSamplerDescriptorHeap->GetHeap(),
					};
					pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					pCommandList->RSSetViewports(1, util::Convert(viewport));
					pCommandList->RSSetScissorRects(1, &scissorRect);
					pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

					pCommandList->OMSetRenderTargets(static_cast<uint32_t>(rtvHandleCount), pRTVHandles, FALSE, pDSVHandle);

					for (auto iter = umapJobSkinnedMaskBatch.begin(); iter != umapJobSkinnedMaskBatch.end(); ++iter)
					{
						const IVertexBuffer* pPrevVertexBuffer = nullptr;
						const IIndexBuffer* pPrevIndexBuffer = nullptr;

						const RenderPipeline* pRenderPipeline = nullptr;

						const PSOKey& psoKey = iter->first;
						std::vector<const JobSkinnedBatch*>& jobBatchs = iter.value();
						{
							pRenderPipeline = CreateRenderPipeline(pDevice, psoKey);

							if (pRenderPipeline == nullptr || pRenderPipeline->psoCache.pPipelineState == nullptr || pRenderPipeline->psoCache.pRootSignature == nullptr)
							{
								const uint32_t mask = shader::eUseSkinning | (psoKey.mask & shader::eUseAlphaBlending) | (psoKey.mask & shader::eUseInstancing);

								const PSOKey psoDefaultSkinnedKey(mask, RasterizerState::eSolidCCW, BlendState::eOff, DepthStencilState::eRead_Write_On);
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

						if (psoKey.blendState != BlendState::eOff)
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

						if (pRenderPipeline->rootParameterIndex[eRP_DirectionalLightCB] != eRP_InvalidIndex)
						{
							pCommandList->SetGraphicsRootConstantBufferView(
								pRenderPipeline->rootParameterIndex[eRP_DirectionalLightCB],
								m_directionalLightBuffer.GPUAddress(frameIndex));
						}

						if (pRenderPipeline->rootParameterIndex[eRP_PointLIghtCB] != eRP_InvalidIndex)
						{
							pCommandList->SetGraphicsRootConstantBufferView(
								pRenderPipeline->rootParameterIndex[eRP_PointLIghtCB],
								m_pointLightBuffer.GPUAddress(frameIndex));
						}

						if (pRenderPipeline->rootParameterIndex[eRP_SpotLightCB] != eRP_InvalidIndex)
						{
							pCommandList->SetGraphicsRootConstantBufferView(
								pRenderPipeline->rootParameterIndex[eRP_SpotLightCB],
								m_spotLightBuffer.GPUAddress(frameIndex));
						}

						if (pRenderPipeline->rootParameterIndex[eRP_ShadowCB] != eRP_InvalidIndex)
						{
							pCommandList->SetGraphicsRootConstantBufferView(
								pRenderPipeline->rootParameterIndex[eRP_ShadowCB],
								m_shadowBuffer.GPUAddress(frameIndex));
						}

						if ((psoKey.mask & shader::eUseInstancing) == shader::eUseInstancing)
						{
							for (auto& pJobBatch : jobBatchs)
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
							std::sort(jobBatchs.begin(), jobBatchs.end(), [](const JobSkinnedBatch* a, const JobSkinnedBatch* b)
								{
									return a->pJob->data.depth < b->pJob->data.depth;
								});

							for (auto& pJobBatch : jobBatchs)
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

									shader::SetObjectData(pBuffer, job.pMaterial.get(), job.worldMatrix, job.prevWorldMatrix, job.VTFID, job.PrevVTFID);

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
					pDeviceInstance->ExecuteCommandList(pCommandList);
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

					CD3DX12_ROOT_PARAMETER& directionalLightBufferParameter = rootParameters.emplace_back();
					directionalLightBufferParameter.InitAsConstantBufferView(shader::eCB_DirectionalLightBuffer, 0, D3D12_SHADER_VISIBILITY_PIXEL);
					rootParameterIndex_out[eRP_DirectionalLightCB] = static_cast<uint32_t>(rootParameters.size() - 1);

					CD3DX12_ROOT_PARAMETER& pointLightBufferParameter = rootParameters.emplace_back();
					pointLightBufferParameter.InitAsConstantBufferView(shader::eCB_PointLightBuffer, 0, D3D12_SHADER_VISIBILITY_PIXEL);
					rootParameterIndex_out[eRP_PointLIghtCB] = static_cast<uint32_t>(rootParameters.size() - 1);

					CD3DX12_ROOT_PARAMETER& spotLightBufferParameter = rootParameters.emplace_back();
					spotLightBufferParameter.InitAsConstantBufferView(shader::eCB_SpotLightBuffer, 0, D3D12_SHADER_VISIBILITY_PIXEL);
					rootParameterIndex_out[eRP_SpotLightCB] = static_cast<uint32_t>(rootParameters.size() - 1);

					CD3DX12_ROOT_PARAMETER& shadowBufferParameter = rootParameters.emplace_back();
					shadowBufferParameter.InitAsConstantBufferView(shader::eCB_ShadowBuffer, 0, D3D12_SHADER_VISIBILITY_PIXEL);
					rootParameterIndex_out[eRP_ShadowCB] = static_cast<uint32_t>(rootParameters.size() - 1);

					D3D12_STATIC_SAMPLER_DESC shadowSamplerDesc{};
					shadowSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
					shadowSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
					shadowSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
					shadowSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
					shadowSamplerDesc.MipLODBias = 0.f;
					shadowSamplerDesc.MaxAnisotropy = 0;
					shadowSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
					shadowSamplerDesc.MinLOD = 0.f;
					shadowSamplerDesc.MaxLOD = 0.f;
					shadowSamplerDesc.ShaderRegister = 2;
					shadowSamplerDesc.RegisterSpace = 100;
					shadowSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
					shadowSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

					staticSamplers =
					{
						util::GetStaticSamplerDesc(SamplerState::eMinMagMipPointClamp, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
						util::GetStaticSamplerDesc(SamplerState::eMinMagMipLinearClamp, 1, 100, D3D12_SHADER_VISIBILITY_PIXEL),
						shadowSamplerDesc,
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

					if (pRenderPipeline->psoCache.pPSBlob == nullptr && (psoKey.mask & shader::eUseWriteDepth) == 0)
					{
						const bool isSuccess = util::CompileShader(m_pShaderBlob, vecMacros.data(), m_shaderPath.c_str(), "PS", shader::PS_CompileVersion, &pRenderPipeline->psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring debugName = string::Format(L"%u_%d_%d_%d", psoKey.mask, psoKey.rasterizerState, psoKey.blendState, psoKey.depthStencilState);

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

				if ((psoKey.mask & shader::eUseWriteDepth) == 0)
				{
					psoDesc.PS.BytecodeLength = pRenderPipeline->psoCache.pPSBlob->GetBufferSize();
					psoDesc.PS.pShaderBytecode = pRenderPipeline->psoCache.pPSBlob->GetBufferPointer();
				}

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
				psoDesc.RasterizerState = util::GetRasterizerDesc(psoKey.rasterizerState);
				psoDesc.BlendState = util::GetBlendDesc(psoKey.blendState);

				if ((psoKey.mask & shader::eUseAlphaBlending) == shader::eUseAlphaBlending)
				{
					psoDesc.NumRenderTargets = 1;

					if (RenderOptions().OnHDR == true)
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
				else if ((psoKey.mask & shader::eUseWriteDepth) == shader::eUseWriteDepth)
				{
					psoDesc.NumRenderTargets = 0;
				}
				else
				{
					psoDesc.NumRenderTargets = GBufferTypeCount - 1;
					psoDesc.RTVFormats[GBufferType::eNormals] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eNormals));
					psoDesc.RTVFormats[GBufferType::eColors] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eColors));
					psoDesc.RTVFormats[GBufferType::eDisneyBRDF] = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eDisneyBRDF));
				}

				psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(psoKey.depthStencilState);

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

			void ModelRenderer::Render(const RenderElement& renderElement, Group emGroup, const math::Matrix& prevViewPrjectionMatrixection)
			{
				m_pImpl->Render(renderElement, emGroup, prevViewPrjectionMatrixection);
			}

			void ModelRenderer::AllCleanup()
			{
				m_pImpl->AllCleanup();
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