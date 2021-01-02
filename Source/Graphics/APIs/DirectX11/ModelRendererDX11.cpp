#include "stdafx.h"
#include "ModelRendererDX11.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Lock.h"
#include "CommonLib/Timer.h"

#include "Graphics/Interface/Instancing.h"
#include "Graphics/Interface/Camera.h"
#include "Graphics/Interface/OcclusionCulling.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "GBufferDX11.h"
#include "VTFManagerDX11.h"
#include "LightResourceManagerDX11.h"

#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"
#include "TextureDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace modelShader
			{
				static constexpr char VSFuncName[] = { "VS" };
				static constexpr char PSFuncName[] = { "PS" };

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

				struct VSConstants
				{
					math::Matrix viewMatrix;
					math::Matrix viewProjectionMatrix;
					math::Matrix prevViewPrjectionMatrix;
				};

				struct CommonContents
				{
					math::float3 cameraPosition;
					float farClip{ 0.f };

					uint32_t directionalLightCount{ 0 };
					uint32_t pointLightCount{ 0 };
					uint32_t spotLightCount{ 0 };
					uint32_t cascadeShadowCount{ 0 };

					std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
					std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
					std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};

					std::array<CascadedShadowData, ILight::eMaxDirectionalLightCount> cascadedShadow{};
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
					eSampler_ShadowPCF = 3,
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
					eSRV_PrevVTF = 16,

					eSRV_DiffuseHDR = 17,
					eSRV_SpecularHDR = 18,
					eSRV_SpecularBRDF = 19,
					eSRV_ShadowMap = 20,

					SRVSlotCount,
				};

				enum Mask : uint32_t
				{
					eUseInstancing = 1 << shader::MaterialMaskCount,
					eUseSkinning = 1 << (shader::MaterialMaskCount + 1),
					eUseAlphaBlending = 1 << (shader::MaterialMaskCount + 2),
					eUseMotionBlur = 1 << (shader::MaterialMaskCount + 3),
					eUseWriteDepth = 1 << (shader::MaterialMaskCount + 4),

					MaskCount = shader::MaterialMaskCount + 5,
				};

				const char* GetMaskName(uint32_t maskBit)
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
					return s_strMaskName[maskBit].c_str();
				}

				std::vector<D3D_SHADER_MACRO> GetMacros(const shader::MaskKey& maskKey)
				{
					std::vector<D3D_SHADER_MACRO> vecMacros;
					vecMacros.push_back({ "DX11", "1" });
					for (uint32_t i = 0; i < modelShader::MaskCount; ++i)
					{
						if ((maskKey & (1 << i)) != 0)
						{
							vecMacros.push_back({ GetMaskName(i), "1" });
						}
					}
					vecMacros.push_back({ nullptr, nullptr });

					return vecMacros;
				}

				void SetObjectData(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<ObjectDataBuffer>* pCB_ObjectDataBuffer,
					const IMaterial* pMaterial,
					const math::Matrix& worldMatrix, const math::Matrix& prevWorldMatrix,
					uint32_t VTFID, uint32_t PrevVTFID)
				{
					ObjectDataBuffer* pObjectDataBuffer = pCB_ObjectDataBuffer->Map(pDeviceContext);
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
					pCB_ObjectDataBuffer->Unmap(pDeviceContext);
				}

				void SetImageBasedLight(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext, const IImageBasedLight* pImageBasedLight)
				{
					Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR().get());
					ID3D11ShaderResourceView* pSRV = pDiffuseHDR->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(eSRV_DiffuseHDR, 1, &pSRV);

					Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR().get());
					pSRV = pSpecularHDR->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(eSRV_SpecularHDR, 1, &pSRV);

					Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF().get());
					pSRV = pSpecularBRDF->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(eSRV_SpecularBRDF, 1, &pSRV);

					ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipPointClamp);
					pDeviceContext->PSSetSamplers(eSampler_PointClamp, 1, &pSamplerPointClamp);

					ID3D11SamplerState* pSamplerClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipLinearClamp);
					pDeviceContext->PSSetSamplers(eSampler_Clamp, 1, &pSamplerClamp);
				}

				void SetCommonContents_ForAlpha(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<CommonContents>* pCB_CommonContents,
					const LightResourceManager* pLightResourceManager, const math::float3& cameraPosition)
				{
					CommonContents* pCommonContents = pCB_CommonContents->Map(pDeviceContext);

					pCommonContents->cameraPosition = cameraPosition;

					const DirectionalLightData* pDirectionalLightData = nullptr;
					pLightResourceManager->GetDirectionalLightRenderData(&pDirectionalLightData, &pCommonContents->directionalLightCount);
					memory::Copy(pCommonContents->lightDirectional.data(), sizeof(pCommonContents->lightDirectional), pDirectionalLightData, sizeof(DirectionalLightData) * pCommonContents->directionalLightCount);

					const PointLightData* pPointLightData = nullptr;
					pLightResourceManager->GetPointLightRenderData(&pPointLightData, &pCommonContents->pointLightCount);
					memory::Copy(pCommonContents->lightPoint.data(), sizeof(pCommonContents->lightPoint), pPointLightData, sizeof(PointLightData) * pCommonContents->pointLightCount);

					const SpotLightData* pSpotLightData = nullptr;
					pLightResourceManager->GetSpotLightRenderData(&pSpotLightData, &pCommonContents->spotLightCount);
					memory::Copy(pCommonContents->lightSpot.data(), sizeof(pCommonContents->lightSpot), pSpotLightData, sizeof(SpotLightData) * pCommonContents->spotLightCount);

					const size_t lightCount = pLightResourceManager->GetLightCount(ILight::Type::eDirectional);
					for (size_t i = 0; i < pCommonContents->directionalLightCount; ++i)
					{
						DirectionalLightPtr pDirectionalLight = std::static_pointer_cast<IDirectionalLight>(pLightResourceManager->GetLight(ILight::Type::eDirectional, i));
						if (pDirectionalLight != nullptr)
						{
							DepthStencil* pDepthStencil = static_cast<DepthStencil*>(pDirectionalLight->GetDepthMapResource());
							if (pDepthStencil != nullptr)
							{
								const CascadedShadows& cascadedShadows = pDirectionalLight->GetRenderCascadedShadows();
								pCommonContents->cascadedShadow[i] = cascadedShadows.GetRenderData();

								++pCommonContents->cascadeShadowCount;
							}
						}
					}

					pCB_CommonContents->Unmap(pDeviceContext);
				}

				void Draw(ID3D11DeviceContext* pDeviceContext,
					const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer,
					uint32_t startIndex, uint32_t indexCount)
				{
					ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
					const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
					const uint32_t nOffsets[] = { 0, };
					pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);

					if (pIndexBuffer != nullptr)
					{
						pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
						pDeviceContext->DrawIndexed(indexCount, startIndex, 0);
					}
					else
					{
						pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
						pDeviceContext->Draw(indexCount, startIndex);
					}
				}

				void DrawInstance(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<StaticInstancingDataBuffer>* pCB_StaticInstancingDataBuffer,
					const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer,
					uint32_t startIndex, uint32_t indexCount,
					const math::Matrix* pInstanceData, size_t instanceCount,
					const math::Matrix* pPrevInstanceData, size_t prevInstanceCount)
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

					const size_t loopCount = instanceCount / eMaxInstancingCount + 1;
					for (size_t i = 0; i < loopCount; ++i)
					{
						const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), instanceCount);
						const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

						if (nDrawInstanceCount <= 0)
							break;

						StaticInstancingDataBuffer* pStaticInstancingData = pCB_StaticInstancingDataBuffer->Map(pDeviceContext);
						memory::Copy(pStaticInstancingData->data.data(), sizeof(pStaticInstancingData->data), &pInstanceData[i * eMaxInstancingCount], sizeof(math::Matrix) * nDrawInstanceCount);
						memory::Copy(pStaticInstancingData->prevData.data(), sizeof(pStaticInstancingData->prevData), &pPrevInstanceData[i * eMaxInstancingCount], sizeof(math::Matrix) * nDrawInstanceCount);
						pCB_StaticInstancingDataBuffer->Unmap(pDeviceContext);

						if (pIndexBuffer != nullptr)
						{
							pDeviceContext->DrawIndexedInstanced(indexCount, static_cast<uint32_t>(nDrawInstanceCount), startIndex, 0, 0);
						}
						else
						{
							pDeviceContext->DrawInstanced(indexCount, static_cast<uint32_t>(nDrawInstanceCount), startIndex, 0);
						}
					}
				}

				void DrawInstance(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<SkinningInstancingDataBuffer>* pCB_SkinningInstancingDataBuffer,
					const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer,
					uint32_t startIndex, uint32_t indexCount,
					const SkinningInstancingData* pInstanceData, size_t instanceCount,
					const SkinningInstancingData* pPrevInstanceData, size_t prevInstanceCount)
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

					const size_t loopCount = instanceCount / eMaxInstancingCount + 1;
					for (size_t i = 0; i < loopCount; ++i)
					{
						const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), instanceCount);
						const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

						if (nDrawInstanceCount <= 0)
							break;

						SkinningInstancingDataBuffer* pSkinningInstancingData = pCB_SkinningInstancingDataBuffer->Map(pDeviceContext);
						memory::Copy(pSkinningInstancingData->data.data(), sizeof(pSkinningInstancingData->data), &pInstanceData[i * eMaxInstancingCount], sizeof(SkinningInstancingData) * nDrawInstanceCount);
						memory::Copy(pSkinningInstancingData->prevData.data(), sizeof(pSkinningInstancingData->prevData), &pPrevInstanceData[i * eMaxInstancingCount], sizeof(SkinningInstancingData) * nDrawInstanceCount);
						pCB_SkinningInstancingDataBuffer->Unmap(pDeviceContext);

						if (pIndexBuffer != nullptr)
						{
							pDeviceContext->DrawIndexedInstanced(indexCount, static_cast<uint32_t>(nDrawInstanceCount), startIndex, 0, 0);
						}
						else
						{
							pDeviceContext->DrawInstanced(indexCount, static_cast<uint32_t>(nDrawInstanceCount), startIndex, 0);
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
				void Render(const RenderElement& element, Group emGroup, const math::Matrix& prevViewPrjectionMatrixection);
				void AllCleanup();
				void Cleanup();

			public:
				void PushJob(const RenderJobStatic& job);
				void PushJob(const RenderJobSkinned& job);

			private:
				void RenderStaticModel(Device* pDeviceInstance, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Group emGroup, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask);
				void RenderSkinnedModel(Device* pDeviceInstance, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Group emGroup, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask);

			private:
				struct RenderState
				{
					bool isValid{ false };
					ID3D11VertexShader* pVertexShader{ nullptr };
					ID3D11PixelShader* pPixelShader{ nullptr };
					ID3D11InputLayout* pInputLayout{ nullptr };
				};
				const RenderState* GetRenderState(ID3D11Device* pDevice, const shader::MaskKey& mask);
				void CreateRenderStateAsync(const shader::MaskKey& maskKey);

				void CreateRenderState(ID3D11Device* pDevice, const shader::MaskKey& maskKey);
				void CreateVertexShader(ID3D11Device* pDevice, const shader::MaskKey& mask, const char* functionName, ID3D11VertexShader** ppVertexShader_out, ID3D11InputLayout** ppInputLayout_out) const;
				void CreatePixelShader(ID3D11Device* pDevice, const shader::MaskKey& mask, const char* functionName, ID3D11PixelShader** ppPixelShader_out) const;

			private:
				bool m_isStop{ false };
				std::mutex m_mutex_createShaderAsync;
				std::condition_variable m_condition_createShaderAsync;

				std::thread m_thread_createShaderAsync;
				std::queue<shader::MaskKey> m_requestCreateShaderAsyncMaskKeys;

				struct CompleteCreateRenderState
				{
					shader::MaskKey maskKey{ 0 };
					RenderState renderState;
				};
				Concurrency::concurrent_queue<CompleteCreateRenderState> m_completeCreateShaderAsyncMaskKeys;

				thread::SRWLock m_srwLock_static;
				thread::SRWLock m_srwLock_skinned;

				std::wstring m_shaderPath;
				ID3DBlob* m_pShaderBlob{ nullptr };

				ID3D11SamplerState* m_pSamplerShadowPCF{ nullptr };

				ConstantBuffer<modelShader::SkinningInstancingDataBuffer> m_skinningInstancingDataBuffer;
				ConstantBuffer<modelShader::StaticInstancingDataBuffer> m_staticInstancingDataBuffer;
				ConstantBuffer<modelShader::ObjectDataBuffer> m_objectDataBuffer;
				ConstantBuffer<modelShader::VSConstants> m_vsConstants;

				// for alphablend, common.hlsl
				ConstantBuffer<modelShader::CommonContents> m_commonContentsBuffer;

				tsl::robin_map<shader::MaskKey, RenderState> m_umapRenderStates;

				struct JobStatic
				{
					RenderJobStatic data;
					bool isCulled{ false };

					JobStatic() = default;
					JobStatic(const RenderJobStatic& data)
						: data(data)
					{
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
				std::array<std::vector<JobStatic>, GroupCount> m_jobStatics[2];

				using UMapJobStaticBatch = tsl::robin_map<const void*, JobStaticBatch>;
				using UMapJobStaticMaterialBatch = tsl::robin_map<MaterialPtr, UMapJobStaticBatch>;
				UMapJobStaticMaterialBatch m_umapJobStaticMasterBatchs;

				struct JobSkinned
				{
					RenderJobSkinned data;
					bool isCulled{ false };

					JobSkinned() = default;
					JobSkinned(const RenderJobSkinned& data)
						: data(data)
					{
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
				std::array<std::vector<JobSkinned>, GroupCount> m_jobSkinneds[2];

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

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				CreateRenderState(pDevice, 0);
				CreateRenderState(pDevice, modelShader::eUseInstancing);
				CreateRenderState(pDevice, modelShader::eUseSkinning);
				CreateRenderState(pDevice, modelShader::eUseSkinning | modelShader::eUseInstancing);

				m_skinningInstancingDataBuffer.Create(pDevice, "SkinningInstancingDataBuffer");
				m_staticInstancingDataBuffer.Create(pDevice, "StaticInstancingDataBuffer");
				m_objectDataBuffer.Create(pDevice, "ObjectDataBuffer");
				m_vsConstants.Create(pDevice, "VSConstants");
				m_commonContentsBuffer.Create(pDevice, "CommonContents");

				for (int i = 0; i < GroupCount; ++i)
				{
					m_jobStatics[UpdateThread()][i].reserve(512);
					m_jobStatics[RenderThread()][i].reserve(512);
					m_jobSkinneds[UpdateThread()][i].reserve(128);
					m_jobSkinneds[RenderThread()][i].reserve(128);
				}

				m_umapJobStaticMasterBatchs.rehash(512);
				m_umapJobSkinnedMasterBatchs.rehash(128);

				D3D11_SAMPLER_DESC samplerDesc;
				samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
				samplerDesc.MipLODBias = 0.f;
				samplerDesc.MaxAnisotropy = 0;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
				samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 0.f;
				samplerDesc.MinLOD = 0.f;
				samplerDesc.MaxLOD = 0.f;

				HRESULT hr = pDevice->CreateSamplerState(&samplerDesc, &m_pSamplerShadowPCF);
				if (FAILED(hr))
				{
					throw_line("failed to create sampler state");
				}

				m_thread_createShaderAsync = std::thread([&]()
					{
						while (true)
						{
							shader::MaskKey maskKey{ 0 };
							{
								std::unique_lock<std::mutex> lock(m_mutex_createShaderAsync);
								m_condition_createShaderAsync.wait(lock, [&]()
									{
										return m_isStop == true || m_requestCreateShaderAsyncMaskKeys.empty() == false;
									});

								if (m_isStop == true)
									return;

								maskKey = m_requestCreateShaderAsyncMaskKeys.front();
								m_requestCreateShaderAsyncMaskKeys.pop();
							}

							Stopwatch sw;
							sw.Start();

							CompleteCreateRenderState completeCreateRenderState;
							completeCreateRenderState.maskKey = maskKey;
							RenderState& renderState = completeCreateRenderState.renderState;

							ID3D11Device* pDevice = Device::GetInstance()->GetInterface();
							CreateVertexShader(pDevice, maskKey, modelShader::VSFuncName, &renderState.pVertexShader, &renderState.pInputLayout);
							if ((maskKey & modelShader::eUseWriteDepth) != modelShader::eUseWriteDepth)
							{
								CreatePixelShader(pDevice, maskKey, modelShader::PSFuncName, &renderState.pPixelShader);
							}
							m_completeCreateShaderAsyncMaskKeys.push(completeCreateRenderState);

							sw.Stop();

							LOG_MESSAGE(L"Created Shader Async : MaskKey[%u], Duration(ms)[%lld]", maskKey.Value(), sw.MilliSec());
						}
					});
			}

			ModelRenderer::Impl::~Impl()
			{
				{
					std::lock_guard<std::mutex> lock(m_mutex_createShaderAsync);
					m_isStop = true;
				}
				m_condition_createShaderAsync.notify_all();
				m_thread_createShaderAsync.join();

				AllCleanup();

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

			void ModelRenderer::Impl::Render(const RenderElement& element, Group emGroup, const math::Matrix& prevViewPrjectionMatrixection)
			{
				TRACER_EVENT(__FUNCTIONW__);
				DX_PROFILING(ModelRenderer);

				if (m_jobStatics[RenderThread()][emGroup].empty() == true && m_jobSkinneds[RenderThread()][emGroup].empty() == true)
					return;

				Camera* pCamera = element.pCamera;

				Device* pDeviceInstance = Device::GetInstance();
				LightResourceManager* pLightResourceManager = pDeviceInstance->GetLightResourceManager();
				if (emGroup == Group::eShadow)
				{
					if (pLightResourceManager->GetShadowCount(ILight::Type::eDirectional) == 0 &&
						pLightResourceManager->GetShadowCount(ILight::Type::ePoint) == 0 &&
						pLightResourceManager->GetShadowCount(ILight::Type::eSpot) == 0)
						return;
				}

				const OcclusionCulling* pOcclusionCulling = OcclusionCulling::GetInstance();
				if (emGroup == Group::eDeferred)
				{
					TRACER_EVENT(L"Culling");
					DX_PROFILING(Culling);

					const collision::Frustum& frustum = pCamera->GetFrustum();
					jobsystem::ParallelFor(m_jobStatics[RenderThread()][emGroup].size(), [&](size_t i)
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

					jobsystem::ParallelFor(m_jobSkinneds[RenderThread()][emGroup].size(), [&](size_t i)
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

				ID3D11Device* pDevice = element.pDevice;
				ID3D11DeviceContext* pDeviceContext = element.pDeviceContext;
				pDeviceContext->ClearState();

				if (emGroup == Group::eAlphaBlend)
				{
					const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();
					modelShader::SetImageBasedLight(pDeviceInstance, pDeviceContext, pImageBasedLight);

					modelShader::SetCommonContents_ForAlpha(pDeviceContext, &m_commonContentsBuffer, pLightResourceManager, pCamera->GetPosition());
					pDeviceContext->PSSetConstantBuffers(modelShader::eCB_CommonContents, 1, &m_commonContentsBuffer.pBuffer);

					std::vector<ID3D11ShaderResourceView*> cascadeShadowMaps;
					const size_t directionalLightCount = pLightResourceManager->GetLightCount(ILight::Type::eDirectional);
					for (size_t i = 0; i < directionalLightCount; ++i)
					{
						DirectionalLightPtr pDirectionalLight = std::static_pointer_cast<IDirectionalLight>(pLightResourceManager->GetLight(ILight::Type::eDirectional, i));
						if (pDirectionalLight != nullptr)
						{
							DepthStencil* pDepthStencil = static_cast<DepthStencil*>(pDirectionalLight->GetDepthMapResource());
							if (pDepthStencil != nullptr)
							{
								cascadeShadowMaps.emplace_back(pDepthStencil->GetShaderResourceView());
							}
						}
					}
					pDeviceContext->PSSetShaderResources(modelShader::eSRV_ShadowMap, static_cast<uint32_t>(cascadeShadowMaps.size()), cascadeShadowMaps.data());

					pDeviceContext->PSSetSamplers(modelShader::eSampler_ShadowPCF, 1, &m_pSamplerShadowPCF);

					ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipPointClamp);
					pDeviceContext->PSSetSamplers(modelShader::eSampler_PointClamp, 1, &pSamplerPointClamp);

					ID3D11SamplerState* pSamplerClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipLinearClamp);
					pDeviceContext->PSSetSamplers(modelShader::eSampler_Clamp, 1, &pSamplerClamp);
				}

				pDeviceContext->VSSetConstantBuffers(modelShader::eCB_ObjectData, 1, &m_objectDataBuffer.pBuffer);
				pDeviceContext->PSSetConstantBuffers(modelShader::eCB_ObjectData, 1, &m_objectDataBuffer.pBuffer);
				pDeviceContext->VSSetConstantBuffers(modelShader::eCB_VSConstants, 1, &m_vsConstants.pBuffer);
				
				if (emGroup == Group::eDeferred || emGroup == Group::eAlphaBlend)
				{
					const math::Viewport& viewport = pDeviceInstance->GetViewport();
					pDeviceContext->RSSetViewports(1, util::Convert(viewport));

					pDeviceContext->OMSetRenderTargets(element.rtvCount, element.pRTVs, element.pDSV);

					modelShader::VSConstants* pVSConstants = m_vsConstants.Map(pDeviceContext);
					{
						pVSConstants->viewMatrix = pCamera->GetViewMatrix().Transpose();

						pVSConstants->viewProjectionMatrix = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();
						pVSConstants->viewProjectionMatrix = pVSConstants->viewProjectionMatrix.Transpose();

						pVSConstants->prevViewPrjectionMatrix = prevViewPrjectionMatrixection.Transpose();
					}
					m_vsConstants.Unmap(pDeviceContext);

					tsl::robin_map<const IMaterial*, uint32_t> umapMaterialMask;
					umapMaterialMask.rehash(m_umapJobStaticMasterBatchs.size() + m_umapJobSkinnedMasterBatchs.size());

					RenderStaticModel(pDeviceInstance, pDevice, pDeviceContext, emGroup, umapMaterialMask);
					RenderSkinnedModel(pDeviceInstance, pDevice, pDeviceContext, emGroup, umapMaterialMask);
				}
				else if (emGroup == Group::eShadow)
				{
					tsl::robin_map<const IMaterial*, uint32_t> umapMaterialMask;
					umapMaterialMask.rehash(m_umapJobStaticMasterBatchs.size() + m_umapJobSkinnedMasterBatchs.size());

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
								const CascadedShadows& cascadedShadows = pDirectionalLight->GetRenderCascadedShadows();
								const CascadedShadowsConfig& cascadedShadowsConfig = cascadedShadows.GetConfig();

								DepthStencil* pCascadedDepthStencil = pLightResourceManager->GetDepthStencil(pDeviceInstance, pDeviceContext, pDirectionalLight);
								if (pCascadedDepthStencil != nullptr)
								{
									pDeviceContext->OMSetRenderTargets(0, nullptr, pCascadedDepthStencil->GetDepthStencilView());

									ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCCW);
									pDeviceContext->RSSetState(pRasterizerState);

									for (uint32_t cascadeLevel = 0; cascadeLevel < cascadedShadowsConfig.numCascades; ++cascadeLevel)
									{
										pDeviceContext->RSSetViewports(1, util::Convert(cascadedShadows.GetViewport(cascadeLevel)));

										const math::Matrix& viewMatrix = cascadedShadows.GetViewMatrix(cascadeLevel);
										math::Matrix projectionMatrix = cascadedShadows.GetProjectionMatrix(cascadeLevel);
										projectionMatrix._33 /= pCamera->GetProjection().farClip;
										projectionMatrix._43 /= pCamera->GetProjection().farClip;

										modelShader::VSConstants* pVSConstants = m_vsConstants.Map(pDeviceContext);
										{
											pVSConstants->viewMatrix = viewMatrix.Transpose();

											pVSConstants->viewProjectionMatrix = viewMatrix * projectionMatrix;
											pVSConstants->viewProjectionMatrix = pVSConstants->viewProjectionMatrix.Transpose();

											pVSConstants->prevViewPrjectionMatrix = prevViewPrjectionMatrixection.Transpose();
										}
										m_vsConstants.Unmap(pDeviceContext);

										RenderStaticModel(pDeviceInstance, pDevice, pDeviceContext, emGroup, umapMaterialMask);
										RenderSkinnedModel(pDeviceInstance, pDevice, pDeviceContext, emGroup, umapMaterialMask);
									}
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
					m_jobStatics[UpdateThread()][i].clear();
					m_jobStatics[RenderThread()][i].clear();
					m_jobSkinneds[UpdateThread()][i].clear();
					m_jobSkinneds[RenderThread()][i].clear();
				}
			}

			void ModelRenderer::Impl::Cleanup()
			{
				for (int i = 0; i < GroupCount; ++i)
				{
					m_jobStatics[RenderThread()][i].clear();
					m_jobSkinneds[RenderThread()][i].clear();
				}

				m_umapJobStaticMasterBatchs.clear();
				m_umapJobSkinnedMasterBatchs.clear();

				while (m_completeCreateShaderAsyncMaskKeys.empty() == false)
				{
					CompleteCreateRenderState completeCreateRenderState;
					if (m_completeCreateShaderAsyncMaskKeys.try_pop(completeCreateRenderState) == true)
					{
						const bool isValid = completeCreateRenderState.renderState.pInputLayout != nullptr && completeCreateRenderState.renderState.pVertexShader != nullptr;
						if (isValid == true)
						{
							RenderState& renderState = m_umapRenderStates[completeCreateRenderState.maskKey];
							renderState.isValid = true;
							renderState.pInputLayout = completeCreateRenderState.renderState.pInputLayout;
							renderState.pVertexShader = completeCreateRenderState.renderState.pVertexShader;
							renderState.pPixelShader = completeCreateRenderState.renderState.pPixelShader;
						}
					}
				}
			}

			void ModelRenderer::Impl::PushJob(const RenderJobStatic& job)
			{
				const MaterialPtr& pMaterial = job.pMaterial;
				if (pMaterial == nullptr || pMaterial->GetBlendState() == BlendState::eOff)
				{
					const thread::SRWWriteLock writeLock(&m_srwLock_static);
					m_jobStatics[UpdateThread()][eDeferred].emplace_back(job);
					m_jobStatics[UpdateThread()][eShadow].emplace_back(job);
				}
				else
				{
					const thread::SRWWriteLock writeLock(&m_srwLock_static);
					m_jobStatics[UpdateThread()][eAlphaBlend].emplace_back(job);
				}
			}

			void ModelRenderer::Impl::PushJob(const RenderJobSkinned& job)
			{
				const MaterialPtr& pMaterial = job.pMaterial;
				if (pMaterial == nullptr || pMaterial->GetBlendState() == BlendState::eOff)
				{
					thread::SRWWriteLock writeLock(&m_srwLock_skinned);
					m_jobSkinneds[UpdateThread()][eDeferred].emplace_back(job);
					m_jobSkinneds[UpdateThread()][eShadow].emplace_back(job);
				}
				else
				{
					thread::SRWWriteLock writeLock(&m_srwLock_skinned);
					m_jobSkinneds[UpdateThread()][eAlphaBlend].emplace_back(job);
				}
			}

			void ModelRenderer::Impl::RenderStaticModel(Device* pDeviceInstance, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Group emGroup, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				DX_PROFILING(RenderStaticModel);

				m_umapJobStaticMasterBatchs.clear();
				{
					bool isEnableVelocityMotionBlur = false;
					if (emGroup != Group::eShadow && RenderOptions().OnMotionBlur == true && RenderOptions().motionBlurConfig.IsVelocityMotionBlur() == true)
					{
						isEnableVelocityMotionBlur = true;
					}

					for (size_t i = 0; i < m_jobStatics[RenderThread()][emGroup].size(); ++i)
					{
						JobStatic& job = m_jobStatics[RenderThread()][emGroup][i];
						if (job.isCulled == true)
							continue;

						UMapJobStaticBatch& umapJobStaticBatch = m_umapJobStaticMasterBatchs[job.data.pMaterial];

						auto iter = umapJobStaticBatch.find(job.data.pKey);
						if (iter != umapJobStaticBatch.end())
						{
							iter.value().instanceData.emplace_back(job.data.worldMatrix.Transpose());
							iter.value().prevInstanceData.emplace_back(job.data.prevWorldMatrix.Transpose());
						}
						else
						{
							umapJobStaticBatch.emplace(job.data.pKey, JobStaticBatch(&job, job.data.worldMatrix.Transpose(), job.data.prevWorldMatrix.Transpose()));
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
								mask |= modelShader::eUseInstancing;
							}

							if (emGroup == Group::eAlphaBlend)
							{
								mask |= modelShader::eUseAlphaBlending;
							}
							else if (emGroup == Group::eShadow)
							{
								mask |= modelShader::eUseWriteDepth;
							}

							if (isEnableVelocityMotionBlur == true)
							{
								mask |= modelShader::eUseMotionBlur;
							}

							shader::MaskKey maskKey(mask);
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
						if (emGroup != Group::eShadow)
						{
							pDeviceContext->PSSetShader(pRenderState->pPixelShader, nullptr, 0);
						}

						pDeviceContext->IASetInputLayout(pRenderState->pInputLayout);
						pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

						if ((iter->first & modelShader::eUseInstancing) == modelShader::eUseInstancing)
						{
							pDeviceContext->VSSetConstantBuffers(modelShader::eCB_StaticInstancingData, 1, &m_staticInstancingDataBuffer.pBuffer);

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobStatic& job = pJobBatch->pJob->data;

								modelShader::SetObjectData(pDeviceContext, &m_objectDataBuffer, job.pMaterial.get(), math::Matrix::Identity, math::Matrix::Identity, 0, 0);

								if (emGroup == eDeferred || emGroup == eShadow)
								{
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, false);

									modelShader::DrawInstance(pDeviceContext,
										&m_staticInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount,
										pJobBatch->instanceData.data(), pJobBatch->instanceData.size(),
										pJobBatch->prevInstanceData.data(), pJobBatch->prevInstanceData.size());
								}
								else if (emGroup == eAlphaBlend)
								{
									// Pre
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, true);

									modelShader::DrawInstance(pDeviceContext,
										&m_staticInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount,
										pJobBatch->instanceData.data(), pJobBatch->instanceData.size(),
										pJobBatch->prevInstanceData.data(), pJobBatch->prevInstanceData.size());

									// Post
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, false);

									modelShader::DrawInstance(pDeviceContext,
										&m_staticInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount,
										pJobBatch->instanceData.data(), pJobBatch->instanceData.size(),
										pJobBatch->prevInstanceData.data(), pJobBatch->prevInstanceData.size());
								}
							}
						}
						else
						{
							std::sort(vecJobBatch.begin(), vecJobBatch.end(), [](const JobStaticBatch* a, const JobStaticBatch* b)
							{
								return a->pJob->data.depth < b->pJob->data.depth;
							});

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobStatic& job = pJobBatch->pJob->data;

								modelShader::SetObjectData(pDeviceContext, &m_objectDataBuffer, job.pMaterial.get(), job.worldMatrix, job.prevWorldMatrix, 0, 0);

								if (emGroup == eDeferred || emGroup == eShadow)
								{
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, false);

									modelShader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount);
								}
								else if (emGroup == eAlphaBlend)
								{
									// Pre
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, true);

									modelShader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount);

									// Post
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, false);

									modelShader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount);
								}
							}
						}
					}
				}
				m_umapJobStaticMasterBatchs.clear();
			}

			void ModelRenderer::Impl::RenderSkinnedModel(Device* pDeviceInstance, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Group emGroup, tsl::robin_map<const IMaterial*, uint32_t>& umapMaterialMask)
			{
				DX_PROFILING(RenderSkinnedModel);

				VTFManager* pVTFManager = pDeviceInstance->GetVTFManager();
				Texture* pVTFTexture = pVTFManager->GetTexture();
				Texture* pPrevVTFTexture = pVTFManager->GetPrevTexture();

				m_umapJobSkinnedMasterBatchs.clear();
				if (pVTFTexture != nullptr)
				{
					bool isEnableVelocityMotionBlur = false;
					if (RenderOptions().OnMotionBlur == true && RenderOptions().motionBlurConfig.IsVelocityMotionBlur() == true)
					{
						isEnableVelocityMotionBlur = true;
					}

					for (size_t i = 0; i < m_jobSkinneds[RenderThread()][emGroup].size(); ++i)
					{
						JobSkinned& job = m_jobSkinneds[RenderThread()][emGroup][i];
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

					tsl::robin_map<shader::MaskKey, std::vector<const JobSkinnedBatch*>> umapJobSkinnedMaskBatch;
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
								mask |= modelShader::eUseInstancing;
							}

							if (emGroup == Group::eAlphaBlend)
							{
								mask |= modelShader::eUseAlphaBlending;
							}
							else if (emGroup == Group::eShadow)
							{
								mask |= modelShader::eUseWriteDepth;
							}

							if (isEnableVelocityMotionBlur == true)
							{
								mask |= modelShader::eUseMotionBlur;
							}

							mask |= modelShader::eUseSkinning;

							shader::MaskKey maskKey(mask);
							umapJobSkinnedMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					{
						ID3D11ShaderResourceView* pSRVs[] =
						{
							pVTFTexture->GetShaderResourceView(),
						};
						pDeviceContext->VSSetShaderResources(modelShader::eSRV_VTF, _countof(pSRVs), pSRVs);
					}

					if (isEnableVelocityMotionBlur == true)
					{
						ID3D11ShaderResourceView* pSRVs[] =
						{
							pPrevVTFTexture->GetShaderResourceView(),
						};
						pDeviceContext->VSSetShaderResources(modelShader::eSRV_PrevVTF, _countof(pSRVs), pSRVs);
					}

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

						if ((iter->first & modelShader::eUseInstancing) == modelShader::eUseInstancing)
						{
							pDeviceContext->VSSetConstantBuffers(modelShader::eCB_SkinningInstancingData, 1, &m_skinningInstancingDataBuffer.pBuffer);

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobSkinned& job = pJobBatch->pJob->data;

								modelShader::SetObjectData(pDeviceContext, &m_objectDataBuffer, job.pMaterial.get(), math::Matrix::Identity, math::Matrix::Identity, 0, 0);

								if (emGroup == eDeferred || emGroup == eShadow)
								{
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, false);

									modelShader::DrawInstance(pDeviceContext,
										&m_skinningInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount,
										pJobBatch->instanceData.data(), pJobBatch->instanceData.size(),
										pJobBatch->prevInstanceData.data(), pJobBatch->prevInstanceData.size());
								}
								else if (emGroup == eAlphaBlend)
								{
									// Pre
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, true);

									modelShader::DrawInstance(pDeviceContext,
										&m_skinningInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount,
										pJobBatch->instanceData.data(), pJobBatch->instanceData.size(),
										pJobBatch->prevInstanceData.data(), pJobBatch->prevInstanceData.size());

									// Post
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, false);

									modelShader::DrawInstance(pDeviceContext,
										&m_skinningInstancingDataBuffer,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount,
										pJobBatch->instanceData.data(), pJobBatch->instanceData.size(),
										pJobBatch->prevInstanceData.data(), pJobBatch->prevInstanceData.size());
								}
							}
						}
						else
						{
							std::sort(vecJobBatch.begin(), vecJobBatch.end(), [](const JobSkinnedBatch* a, const JobSkinnedBatch* b)
							{
								return a->pJob->data.depth < b->pJob->data.depth;
							});

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobSkinned& job = pJobBatch->pJob->data;

								modelShader::SetObjectData(pDeviceContext, &m_objectDataBuffer, job.pMaterial.get(), job.worldMatrix, job.prevWorldMatrix, job.VTFID, job.PrevVTFID);

								if (emGroup == eDeferred || emGroup == eShadow)
								{
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, false);

									modelShader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount);
								}
								else if (emGroup == eAlphaBlend)
								{
									// Pre
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, true);

									modelShader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount);

									// Post
									shader::SetMaterial(pDeviceContext, job.pMaterial.get(), modelShader::eSampler_Material, false);

									modelShader::Draw(pDeviceContext,
										static_cast<const VertexBuffer*>(job.pVertexBuffer.get()),
										static_cast<const IndexBuffer*>(job.pIndexBuffer.get()),
										job.startIndex, job.indexCount);
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

				if (pRenderState == nullptr)
				{
					CreateRenderStateAsync(maskKey);
				}

				const bool isValidRenderState = pRenderState != nullptr && pRenderState->isValid == true;
				if (isValidRenderState == false)
				{
					pRenderState = nullptr;

					const shader::MaskKey defaultMask(maskKey & modelShader::eUseSkinning | maskKey & modelShader::eUseInstancing);
					iter = m_umapRenderStates.find(defaultMask);
					if (iter != m_umapRenderStates.end())
					{
						pRenderState = &iter->second;
					}
				}

				if (pRenderState == nullptr || pRenderState->isValid == false)
					return nullptr;

				return pRenderState;
			}

			void ModelRenderer::Impl::CreateRenderStateAsync(const shader::MaskKey& maskKey)
			{
				auto& renderState = m_umapRenderStates[maskKey];
				if (renderState.isValid == true)
					return;

				m_requestCreateShaderAsyncMaskKeys.emplace(maskKey);
				m_condition_createShaderAsync.notify_all();
			}

			void ModelRenderer::Impl::CreateRenderState(ID3D11Device* pDevice, const shader::MaskKey& maskKey)
			{
				auto& renderState = m_umapRenderStates[maskKey];
				if (renderState.isValid == true)
					return;

				CreateVertexShader(pDevice, maskKey, modelShader::VSFuncName, &renderState.pVertexShader, &renderState.pInputLayout);
				if ((maskKey & modelShader::eUseWriteDepth) != modelShader::eUseWriteDepth)
				{
					CreatePixelShader(pDevice, maskKey, modelShader::PSFuncName, &renderState.pPixelShader);
					renderState.isValid = renderState.pInputLayout != nullptr && renderState.pVertexShader != nullptr && renderState.pPixelShader != nullptr;
				}
				else
				{
					renderState.isValid = renderState.pInputLayout != nullptr && renderState.pVertexShader != nullptr;
				}
			}

			void ModelRenderer::Impl::CreateVertexShader(ID3D11Device* pDevice, const shader::MaskKey& maskKey, const char* functionName, ID3D11VertexShader** ppVertexShader_out, ID3D11InputLayout** ppInputLayout_out) const
			{
				const std::vector<D3D_SHADER_MACRO> vecMacros = modelShader::GetMacros(maskKey);

				const D3D11_INPUT_ELEMENT_DESC* pInputElements = nullptr;
				size_t nElementCount = 0;

				if ((maskKey & modelShader::eUseSkinning) == modelShader::eUseSkinning)
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

				const std::string debugName = string::Format("ModelVertexShader_%u", maskKey);

				if (util::CreateVertexShader(pDevice, m_pShaderBlob, vecMacros.data(), m_shaderPath.c_str(), functionName, shader::VS_CompileVersion, ppVertexShader_out, pInputElements, nElementCount, ppInputLayout_out, debugName.c_str()) == false)
				{
					LOG_ERROR(L"failed to create vertex shader : %u", maskKey);
					return;
				}
			}

			void ModelRenderer::Impl::CreatePixelShader(ID3D11Device* pDevice, const shader::MaskKey& maskKey, const char* functionName, ID3D11PixelShader** ppPixelShader_out) const
			{
				const std::vector<D3D_SHADER_MACRO> vecMacros = modelShader::GetMacros(maskKey);

				const std::string debugName = string::Format("ModelPixelShader : %u", maskKey);

				if (util::CreatePixelShader(pDevice, m_pShaderBlob, vecMacros.data(), m_shaderPath.c_str(), functionName, shader::PS_CompileVersion, ppPixelShader_out, debugName.c_str()) == false)
				{
					LOG_ERROR(L"failed to create pixel shader : %u", maskKey);
					return;
				}
			}

			ModelRenderer::ModelRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			ModelRenderer::~ModelRenderer()
			{
			}

			void ModelRenderer::Render(const RenderElement& element, Group emGroup, const math::Matrix& prevViewPrjectionMatrixection)
			{
				m_pImpl->Render(element, emGroup, prevViewPrjectionMatrixection);
			}

			void ModelRenderer::AllCleanup()
			{
				m_pImpl->AllCleanup();
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