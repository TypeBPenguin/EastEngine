#include "stdafx.h"
#include "ParticleRendererDX11.h"

#include "Graphics/Interface/Instancing.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "GBufferDX11.h"
#include "VTFManagerDX11.h"

#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"
#include "TextureDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				enum SamplerSlot
				{
					eSampler = 0,
				};

				enum SRVSlot
				{
					eColor = 0,

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

					eSRV_Depth = 15,

					SRVSlotCount,
				};

				enum Pass
				{
					ePass_Emitter = 0,
					ePass_Decal,
				};

				enum Mask : uint64_t
				{
					eUseDecal = 1 << MaterialMaskCount,

					MaskCount = MaterialMaskCount + 1,
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

						"USE_DECAL",
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

				//void Draw(ID3D11DeviceContext* pDeviceContext, const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer)
				//{
				//	ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
				//	const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
				//	const uint32_t nOffsets[] = { 0, };
				//	pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);
				//
				//	if (pIndexBuffer != nullptr)
				//	{
				//		pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
				//		pDeviceContext->DrawIndexed(pIndexBuffer->GetIndexCount(), 0, 0);
				//	}
				//	else
				//	{
				//		pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
				//		pDeviceContext->Draw(pVertexBuffer->GetVertexCount(), 0);
				//	}
				//}

				//void DrawInstance(ID3D11DeviceContext* pDeviceContext,
				//	ConstantBuffer<InstanceContents>* pCB_InstanceContents,
				//	const math::Matrix& matViewProjection,
				//	const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer,
				//	const VertexInstancingData* pInstanceData, size_t nInstanceCount)
				//{
				//	ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
				//	const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
				//	const uint32_t nOffsets[] = { 0, };
				//	pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);
				//
				//	if (pIndexBuffer != nullptr)
				//	{
				//		pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
				//	}
				//	else
				//	{
				//		pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
				//	}
				//
				//	const size_t loopCount = nInstanceCount / eMaxInstancingCount + 1;
				//	for (size_t i = 0; i < loopCount; ++i)
				//	{
				//		const size_t nEnableDrawCount = std::min(eMaxInstancingCount * (i + 1), nInstanceCount);
				//		const size_t nDrawInstanceCount = nEnableDrawCount - i * eMaxInstancingCount;
				//
				//		if (nDrawInstanceCount <= 0)
				//			break;
				//
				//		InstanceContents* pInstanceContents = pCB_InstanceContents->Map(pDeviceContext);
				//		pInstanceContents->matViewProjection = matViewProjection.Transpose();
				//		memory::Copy(pInstanceContents->data.data(), sizeof(pInstanceContents->data), &pInstanceData[i * eMaxInstancingCount], sizeof(VertexInstancingData) * nDrawInstanceCount);
				//		pCB_InstanceContents->Unmap(pDeviceContext);
				//
				//		if (pIndexBuffer != nullptr)
				//		{
				//			pDeviceContext->DrawIndexedInstanced(pIndexBuffer->GetIndexCount(), static_cast<uint32_t>(nDrawInstanceCount), 0, 0, 0);
				//		}
				//		else
				//		{
				//			pDeviceContext->DrawInstanced(pVertexBuffer->GetVertexCount(), static_cast<uint32_t>(nDrawInstanceCount), 0, 0);
				//		}
				//	}
				//}
			}
		}

		ParticleRendererDX11::ParticleRendererDX11()
		{
		}

		ParticleRendererDX11::~ParticleRendererDX11()
		{
		}
	}
}