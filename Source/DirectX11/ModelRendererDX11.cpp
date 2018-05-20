#include "stdafx.h"
#include "ModelRendererDX11.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Lock.h"
#include "CommonLib/PhantomType.h"

#include "GraphicsInterface/Instancing.h"
#include "GraphicsInterface/Camera.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "GBufferDX11.h"

#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"
#include "TextureDX11.h"

#include <bitset>

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

					SkinningInstancingDataBuffer()
					{
						const size_t c = sizeof(SkinningInstancingData);
						const size_t a = sizeof(TransformInstancingData);
						const size_t b = sizeof(MotionInstancingData);
					}
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

				struct MatrixBuffer
				{
					math::Matrix matViewProj;
				};

				enum CBSlot
				{
					eCB_SkinningInstancingData = 0,
					eCB_StaticInstancingData,
					eCB_ObjectData,
					eCB_Matrix,
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

					eSRV_VTF,

					SRVSlotCount,
				};
				static_assert(SRVSlotCount <= D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT, "input register count over");

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

					MaskCount = 17,
				};

				const char* GetMaskName(uint32_t nMaskBit)
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

					return s_strMaskName[nMaskBit].c_str();
				}

				std::vector<D3D_SHADER_MACRO> GetMacros(const MaskKey& maskKey)
				{
					std::vector<D3D_SHADER_MACRO> vecMacros;
					vecMacros.push_back({ "DX11", "1" });
					for (uint32_t i = 0; i < shader::MaskCount; ++i)
					{
						if ((maskKey.value & (1 << i)) != 0)
						{
							vecMacros.push_back({GetMaskName(i), "1"});
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

				void SetMaterial(ID3D11DeviceContext* pDeviceContext, const IMaterial* pMaterial)
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
				}

				void SetObjectData(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<ObjectDataBuffer>* pCB_ObjectDataBuffer,
					const IMaterial* pMaterial, const math::Matrix& matWorld, uint32_t nVTFID = 0)
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
						pObjectDataBuffer->nVTFID = 0;

						pObjectDataBuffer->f2Padding = {};

						ID3D11RasterizerState* pRasterizerState = Device::GetInstance()->GetRasterizerState(pMaterial->GetRasterizerState());
						pDeviceContext->RSSetState(pRasterizerState);

						ID3D11BlendState* pBlendState = Device::GetInstance()->GetBlendState(pMaterial->GetBlendState());
						pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

						ID3D11SamplerState* pSamplerStates[] =
						{
							Device::GetInstance()->GetSamplerState(EmSamplerState::eMinMagLinearMipPointWrap),
						};
						pDeviceContext->PSSetSamplers(0, _countof(pSamplerStates), pSamplerStates);

						ID3D11DepthStencilState* pDepthStencilState = Device::GetInstance()->GetDepthStencilState(pMaterial->GetDepthStencilState());
						pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);
					}
					else
					{
						pObjectDataBuffer->f4AlbedoColor = math::Color::White;
						pObjectDataBuffer->f4EmissiveColor = math::Color::Black;

						pObjectDataBuffer->f4PaddingRoughMetEmi = math::Vector4::Zero;
						pObjectDataBuffer->f4SurSpecTintAniso = math::Vector4::Zero;
						pObjectDataBuffer->f4SheenTintClearcoatGloss = math::Vector4::Zero;

						pObjectDataBuffer->fStippleTransparencyFactor = 0.f;
						pObjectDataBuffer->nVTFID = 0;

						pObjectDataBuffer->f2Padding = {};

						ID3D11RasterizerState* pRasterizerState = Device::GetInstance()->GetRasterizerState(EmRasterizerState::eSolidCCW);
						pDeviceContext->RSSetState(pRasterizerState);

						ID3D11BlendState* pBlendState = Device::GetInstance()->GetBlendState(EmBlendState::eOff);
						pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

						ID3D11SamplerState* pSamplerStates[] =
						{
							Device::GetInstance()->GetSamplerState(EmSamplerState::eMinMagLinearMipPointWrap),
						};
						pDeviceContext->PSSetSamplers(0, _countof(pSamplerStates), pSamplerStates);

						ID3D11DepthStencilState* pDepthStencilState = Device::GetInstance()->GetDepthStencilState(EmDepthStencilState::eRead_Write_On);
						pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);
					}
					pCB_ObjectDataBuffer->Unmap(pDeviceContext);
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

					const size_t nLoopCount = nInstanceCount / eMaxInstancingCount + 1;
					for (size_t i = 0; i < nLoopCount; ++i)
					{
						const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), nInstanceCount);
						const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

						if (nDrawInstanceCount <= 0)
							break;

						StaticInstancingDataBuffer* pStaticInstancingData = pCB_StaticInstancingDataBuffer->Map(pDeviceContext);
						Memory::Copy(pStaticInstancingData->data.data(), sizeof(pStaticInstancingData->data), &pInstanceData[i * eMaxInstancingCount], sizeof(math::Matrix) * nDrawInstanceCount);
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

					const size_t nLoopCount = nInstanceCount / eMaxInstancingCount + 1;
					for (size_t i = 0; i < nLoopCount; ++i)
					{
						const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), nInstanceCount);
						const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;

						if (nDrawInstanceCount <= 0)
							break;

						SkinningInstancingDataBuffer* pSkinningInstancingData = pCB_SkinningInstancingDataBuffer->Map(pDeviceContext);
						Memory::Copy(pSkinningInstancingData->data.data(), sizeof(pSkinningInstancingData->data), &pInstanceData[i * eMaxInstancingCount], sizeof(math::Matrix) * nDrawInstanceCount);
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
				void Flush();

			public:
				void PushJob(const RenderJobStatic& job);
				void PushJob(const RenderJobSkinned& job);

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
				thread::Lock m_lock;
				std::string m_strShaderPath;
				ID3DBlob* m_pShaderBlob{ nullptr };

				ConstantBuffer<shader::SkinningInstancingDataBuffer> m_skinningInstancingDataBuffer;
				ConstantBuffer<shader::StaticInstancingDataBuffer> m_staticInstancingDataBuffer;
				ConstantBuffer<shader::ObjectDataBuffer> m_objectDataBuffer;
				ConstantBuffer<shader::MatrixBuffer> m_matrixBuffer;

				std::unordered_map<shader::MaskKey, RenderState> m_umapRenderStates;

				struct JobStatic
				{
					RenderJobStatic data;
					bool isCulling = false;

					void Set(const RenderJobStatic& source)
					{
						data = source;
						isCulling = false;
					}
				};

				struct JobStaticBatch
				{
					const JobStatic* pJob = nullptr;
					std::vector<math::Matrix> vecInstanceData;

					JobStaticBatch(const JobStatic* pJob, const math::Matrix& matWorld)
						: pJob(pJob)
					{
						vecInstanceData.emplace_back(matWorld);
					}
				};

				std::array<std::vector<JobStatic>, GroupCount> m_vecJobStatics;
				std::array<size_t, GroupCount> m_nJobStaticCount;

				using UMapJobStaticBatch = std::unordered_map<const void*, JobStaticBatch>;
				using UMapJobStaticMaterialBatch = std::unordered_map<const IMaterial*, UMapJobStaticBatch>;
				UMapJobStaticMaterialBatch m_umapJobStaticMasterBatchs;

				struct JobSkinned
				{
					RenderJobSkinned data;
					bool isCulling = false;

					void Set(const RenderJobSkinned& source)
					{
						data = source;
						isCulling = false;
					}
				};

				struct JobSkinnedBatch
				{
					const JobSkinned* pJob = nullptr;
					std::vector<SkinningInstancingData> vecInstanceData;

					JobSkinnedBatch(const JobSkinned* pJob, const math::Matrix& matWorld, uint32_t nVTFID)
						: pJob(pJob)
					{
						vecInstanceData.emplace_back(matWorld, nVTFID);
					}
				};

				std::array<std::vector<JobSkinned>, GroupCount> m_vecJobSkinneds;
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

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				CreateVertexShader(pDevice, 0, "VS");
				CreateVertexShader(pDevice, shader::eUseSkinning, "VS");

				CreatePixelShader(pDevice, 0, "PS");
				
				if (util::CreateConstantBuffer(pDevice, m_skinningInstancingDataBuffer.Size(), &m_skinningInstancingDataBuffer.pBuffer) == false)
				{
					throw_line("failed to create SkinningInstancingDataBuffer");
				}

				if (util::CreateConstantBuffer(pDevice, m_staticInstancingDataBuffer.Size(), &m_staticInstancingDataBuffer.pBuffer) == false)
				{
					throw_line("failed to create StaticInstancingDataBuffer");
				}

				if (util::CreateConstantBuffer(pDevice, m_objectDataBuffer.Size(), &m_objectDataBuffer.pBuffer) == false)
				{
					throw_line("failed to create ObjectDataBuffer");
				}

				if (util::CreateConstantBuffer(pDevice, m_matrixBuffer.Size(), &m_matrixBuffer.pBuffer) == false)
				{
					throw_line("failed to create MatrixBuffer");
				}

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
				SafeRelease(m_skinningInstancingDataBuffer.pBuffer);
				SafeRelease(m_staticInstancingDataBuffer.pBuffer);
				SafeRelease(m_objectDataBuffer.pBuffer);
				SafeRelease(m_matrixBuffer.pBuffer);

				std::for_each(m_umapRenderStates.begin(), m_umapRenderStates.end(), [](std::pair<const shader::MaskKey, RenderState>& iter)
				{
					RenderState& renderState = iter.second;
					SafeRelease(renderState.pVertexShader);
					SafeRelease(renderState.pPixelShader);
					SafeRelease(renderState.pInputLayout);
				});
				m_umapRenderStates.clear();

				SafeRelease(m_pShaderBlob);
			}

			void ModelRenderer::Impl::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera, Group emGroup)
			{
				const Collision::Frustum& frustum = pCamera->GetFrustum();

				pDeviceContext->ClearState();

				const D3D11_VIEWPORT* pViewport = Device::GetInstance()->GetViewport();
				pDeviceContext->RSSetViewports(1, pViewport);

				const GBuffer* pGBuffer = Device::GetInstance()->GetGBuffer();
				ID3D11RenderTargetView* pRTV[] =
				{
					pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetRenderTargetView(),
					pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetRenderTargetView(),
					pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetRenderTargetView(),
				};
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, pGBuffer->GetDepthStencil()->GetDepthStencilView());

				{
					shader::MatrixBuffer* pMatrixBuffer = m_matrixBuffer.Map(pDeviceContext);
					pMatrixBuffer->matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();
					pMatrixBuffer->matViewProj = pMatrixBuffer->matViewProj.Transpose();
					m_matrixBuffer.Unmap(pDeviceContext);
				}

				pDeviceContext->VSSetConstantBuffers(shader::eCB_Matrix, 1, &m_matrixBuffer.pBuffer);

				pDeviceContext->VSSetConstantBuffers(shader::eCB_ObjectData, 1, &m_objectDataBuffer.pBuffer);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_ObjectData, 1, &m_objectDataBuffer.pBuffer);

				std::unordered_map<const IMaterial*, uint32_t> umapMaterialMask;
				umapMaterialMask.rehash(m_umapJobStaticMasterBatchs.size() + m_umapJobSkinnedMasterBatchs.size());

				// Job Static
				m_umapJobStaticMasterBatchs.clear();
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

					std::unordered_map<shader::MaskKey, std::vector<const JobStaticBatch*>> umapJobStaticMaskBatch;
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

							shader::MaskKey maskKey(nMask);
							umapJobStaticMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					for (auto& iter : umapJobStaticMaskBatch)
					{
						std::vector<const JobStaticBatch*>& vecJobBatch = iter.second;

						const RenderState* pRenderState = GetRenderState(pDevice, iter.first);
						if (pRenderState == nullptr)
							continue;

						const IMaterial* pMaterial = vecJobBatch[0]->pJob->data.pMaterial;

						if ((iter.first.value & shader::eUseInstancing) == shader::eUseInstancing)
						{
							pDeviceContext->VSSetShader(pRenderState->pVertexShader, nullptr, 0);
							pDeviceContext->PSSetShader(pRenderState->pPixelShader, nullptr, 0);

							pDeviceContext->VSSetConstantBuffers(shader::eCB_StaticInstancingData, 1, &m_staticInstancingDataBuffer.pBuffer);

							pDeviceContext->IASetInputLayout(pRenderState->pInputLayout);
							pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

							shader::SetMaterial(pDeviceContext, pMaterial);
							shader::SetObjectData(pDeviceContext, &m_objectDataBuffer, pMaterial, math::Matrix::Identity);

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobStatic& job = pJobBatch->pJob->data;

								shader::DrawInstance(pDeviceContext,
									&m_staticInstancingDataBuffer,
									static_cast<const VertexBuffer*>(job.pVertexBuffer),
									static_cast<const IndexBuffer*>(job.pIndexBuffer),
									job.nStartIndex, job.nIndexCount,
									pJobBatch->vecInstanceData.data(), pJobBatch->vecInstanceData.size());
							}
						}
						else
						{
							pDeviceContext->VSSetShader(pRenderState->pVertexShader, nullptr, 0);
							pDeviceContext->PSSetShader(pRenderState->pPixelShader, nullptr, 0);

							pDeviceContext->IASetInputLayout(pRenderState->pInputLayout);
							pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

							shader::SetMaterial(pDeviceContext, pMaterial);

							std::sort(vecJobBatch.begin(), vecJobBatch.end(), [](const JobStaticBatch* a, const JobStaticBatch* b)
							{
								return a->pJob->data.fDepth < b->pJob->data.fDepth;
							});

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobStatic& job = pJobBatch->pJob->data;

								shader::SetObjectData(pDeviceContext, &m_objectDataBuffer, job.pMaterial, job.matWorld);

								shader::Draw(pDeviceContext,
									static_cast<const VertexBuffer*>(job.pVertexBuffer),
									static_cast<const IndexBuffer*>(job.pIndexBuffer),
									job.nStartIndex, job.nIndexCount);
							}
						}
					}
				}
				m_umapJobStaticMasterBatchs.clear();

				// Job Skinned
				m_umapJobSkinnedMasterBatchs.clear();
				{
					for (size_t i = 0; i < m_nJobSkinnedCount[emGroup]; ++i)
					{
						const JobSkinned& job = m_vecJobSkinneds[emGroup][i];

						if (job.isCulling == true)
							continue;

						UMapJobSkinnedBatch& umapJobSkinnedBatch = m_umapJobSkinnedMasterBatchs[job.data.pMaterial];

						auto iter = umapJobSkinnedBatch.find(job.data.pKey);
						if (iter != umapJobSkinnedBatch.end())
						{
							iter->second.vecInstanceData.emplace_back(job.data.matWorld.Transpose(), job.data.nVTFID);
						}
						else
						{
							umapJobSkinnedBatch.emplace(job.data.pKey, JobSkinnedBatch(&job, job.data.matWorld.Transpose(), job.data.nVTFID));
						}
					}

					std::unordered_map<shader::MaskKey, std::vector<const JobSkinnedBatch*>> umapJobSkinnedMaskBatch;
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
								umapMaterialMask.emplace(jobBatch.pJob->data.pMaterial, maskKey.value);
								nMask = maskKey.value;
							}

							if (jobBatch.vecInstanceData.size() > 1)
							{
								nMask |= shader::eUseInstancing;
							}

							nMask |= shader::eUseSkinning;

							shader::MaskKey maskKey(nMask);
							umapJobSkinnedMaskBatch[maskKey].emplace_back(&jobBatch);
						}
					}

					for (auto& iter : umapJobSkinnedMaskBatch)
					{
						std::vector<const JobSkinnedBatch*>& vecJobBatch = iter.second;

						const RenderState* pRenderState = GetRenderState(pDevice, iter.first);
						if (pRenderState == nullptr)
							continue;

						const IMaterial* pMaterial = vecJobBatch[0]->pJob->data.pMaterial;

						if ((iter.first.value & shader::eUseInstancing) == shader::eUseInstancing)
						{
							pDeviceContext->VSSetShader(pRenderState->pVertexShader, nullptr, 0);
							pDeviceContext->PSSetShader(pRenderState->pPixelShader, nullptr, 0);

							pDeviceContext->VSSetConstantBuffers(shader::eCB_SkinningInstancingData, 1, &m_staticInstancingDataBuffer.pBuffer);

							pDeviceContext->IASetInputLayout(pRenderState->pInputLayout);
							pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

							shader::SetMaterial(pDeviceContext, pMaterial);
							shader::SetObjectData(pDeviceContext, &m_objectDataBuffer, pMaterial, math::Matrix::Identity);

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobSkinned& job = pJobBatch->pJob->data;

								shader::DrawInstance(pDeviceContext,
									&m_skinningInstancingDataBuffer,
									static_cast<const VertexBuffer*>(job.pVertexBuffer),
									static_cast<const IndexBuffer*>(job.pIndexBuffer),
									job.nStartIndex, job.nIndexCount,
									pJobBatch->vecInstanceData.data(), pJobBatch->vecInstanceData.size());
							}
						}
						else
						{
							pDeviceContext->VSSetShader(pRenderState->pVertexShader, nullptr, 0);
							pDeviceContext->PSSetShader(pRenderState->pPixelShader, nullptr, 0);

							pDeviceContext->IASetInputLayout(pRenderState->pInputLayout);
							pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

							shader::SetMaterial(pDeviceContext, pMaterial);

							std::sort(vecJobBatch.begin(), vecJobBatch.end(), [](const JobSkinnedBatch* a, const JobSkinnedBatch* b)
							{
								return a->pJob->data.fDepth < b->pJob->data.fDepth;
							});

							for (auto& pJobBatch : vecJobBatch)
							{
								const RenderJobSkinned& job = pJobBatch->pJob->data;

								shader::SetObjectData(pDeviceContext, &m_objectDataBuffer, pMaterial, job.matWorld, job.nVTFID);

								shader::Draw(pDeviceContext,
									static_cast<const VertexBuffer*>(job.pVertexBuffer),
									static_cast<const IndexBuffer*>(job.pIndexBuffer),
									job.nStartIndex, job.nIndexCount);
							}
						}
					}
				}
				m_umapJobSkinnedMasterBatchs.clear();
			}

			void ModelRenderer::Impl::Flush()
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

				thread::AutoLock autoLock(&m_lock);

				size_t nIndex = m_nJobStaticCount[emGroup];
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

				thread::AutoLock autoLock(&m_lock);

				size_t nIndex = m_nJobStaticCount[emGroup];
				if (nIndex >= m_vecJobSkinneds[emGroup].size())
				{
					m_vecJobSkinneds[emGroup].resize(m_vecJobSkinneds[emGroup].size() * 2);
				}

				m_vecJobSkinneds[emGroup][nIndex].Set(job);
				++m_nJobStaticCount[emGroup];
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
				std::vector<D3D_SHADER_MACRO> vecMacros = shader::GetMacros(maskKey);

				ID3DBlob* pShaderBlob = nullptr;
				bool isSuccess = util::CompileShader(m_pShaderBlob, vecMacros.data(), m_strShaderPath.c_str(), pFunctionName, "vs_5_0", &pShaderBlob);
				if (isSuccess == false)
					return;
				
				ID3D11VertexShader* pVertexShader = nullptr;
				HRESULT hr = pDevice->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &pVertexShader);
				if (FAILED(hr))
				{
					LOG_ERROR("failed to create vertex shader : %u", maskKey.value);
				}
				else
				{
					m_umapRenderStates[maskKey].pVertexShader = pVertexShader;
				}

				const D3D11_INPUT_ELEMENT_DESC* pInputElements = nullptr;
				size_t nElementCount = 0;

				if ((maskKey.value & shader::eUseSkinning) == shader::eUseSkinning)
				{
					util::GetInputElementDesc(VertexPosTexNorWeiIdx::Format(), &pInputElements, nElementCount);
				}
				else
				{
					util::GetInputElementDesc(VertexPosTexNor::Format(), &pInputElements, nElementCount);
				}

				if (pInputElements != nullptr || nElementCount > 0)
				{
					ID3D11InputLayout* pInputLayout = nullptr;
					hr = pDevice->CreateInputLayout(pInputElements, static_cast<uint32_t>(nElementCount), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), &pInputLayout);
					if (FAILED(hr))
					{
						LOG_ERROR("failed to create input layout : %u", maskKey.value);
					}
					else
					{
						m_umapRenderStates[maskKey].pInputLayout = pInputLayout;
					}
				}

				SafeRelease(pShaderBlob);
			}

			void ModelRenderer::Impl::CreatePixelShader(ID3D11Device* pDevice, const shader::MaskKey& maskKey, const char* pFunctionName)
			{
				std::vector<D3D_SHADER_MACRO> vecMacros = shader::GetMacros(maskKey);

				ID3DBlob* pShaderBlob = nullptr;
				bool isSuccess = util::CompileShader(m_pShaderBlob, vecMacros.data(), m_strShaderPath.c_str(), pFunctionName, "ps_5_0", &pShaderBlob);
				if (isSuccess == false)
					return;

				ID3D11PixelShader* pPixelShader = nullptr;
				HRESULT hr = pDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &pPixelShader);
				if (FAILED(hr))
				{
					LOG_ERROR("failed to create pixel shader : %u", maskKey.value);
				}
				else
				{
					m_umapRenderStates[maskKey].pPixelShader = pPixelShader;
				}

				SafeRelease(pShaderBlob);
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

			void ModelRenderer::Flush()
			{
				m_pImpl->Flush();
			}

			void ModelRenderer::PushJob(const RenderJobStatic& subset)
			{
				m_pImpl->PushJob(subset);
			}

			void ModelRenderer::PushJob(const RenderJobSkinned& subset)
			{
				m_pImpl->PushJob(subset);
			}
		}
	}
}