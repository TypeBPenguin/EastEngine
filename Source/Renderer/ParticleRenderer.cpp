#include "stdafx.h"
#include "ParticleRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/CameraManager.h"

namespace StrID
{
	RegisterStringID(IEffect);

	RegisterStringID(Material);

	RegisterStringID(Emitter);
	RegisterStringID(Decal);

	RegisterStringID(g_texDepth);
	RegisterStringID(g_sampler);
	RegisterStringID(g_InstancesMatWVP);
	RegisterStringID(g_InstancesMatWorld);
	RegisterStringID(g_InstancesMatInvWorld);
	RegisterStringID(g_f3CameraPos);
	RegisterStringID(g_f3CameraTopRight);
	RegisterStringID(g_matView);
	RegisterStringID(g_matProj);
	RegisterStringID(g_matInvView);
	RegisterStringID(g_matInvProj);

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
		namespace EmParticleShader
		{
			enum Mask : uint64_t
			{
				eUseDecal = 0,
				eUseTexAlbedo,
				eUseTexMask,
				eUseTexNormal,
				eUseTexDisplacement,
				eUseTexSpecularColor,
				eUseTexRoughness,
				eUseTexMetallic,
				eUseTexEmissive,
				eUseTexSubsurface,
				eUseTexSpecular,
				eUseTexSpecularTint,
				eUseTexAnisotropic,
				eUseTexSheen,
				eUseTexSheenTint,
				eUseTexClearcoat,
				eUseTexClearcoatGloss,

				MaskCount,
			};

			const char* GetMaskName(uint64_t nMask)
			{
				static std::string s_strMaskName[] =
				{
					"USE_DECAL",
					"USE_TEX_ALBEDO",
					"USE_TEX_MASK",
					"USE_TEX_NORMAL",
					"USE_TEX_DISPLACEMENT",
					"USE_TEX_SPECULARCOLOR",
					"USE_TEX_ROUGHNESS",
					"USE_TEX_METALLIC",
					"USE_TEX_EMISSIVE",
					"USE_TEX_SUBSURFACE",
					"USE_TEX_SPECULAR",
					"USE_TEX_SPECULARTINT",
					"USE_TEX_ANISOTROPIC",
					"USE_TEX_SHEEN",
					"USE_TEX_SHEENTINT",
					"USE_TEX_CLEARCOAT",
					"USE_TEX_CLEARCOATGLOSS",
				};

				return s_strMaskName[nMask].c_str();
			}
		}

		static IEffect* GetEffect(uint64_t nMask)
		{
			String::StringID strName;
			strName.Format("IEffect_%lld", nMask);

			IEffect* pEffect = ShaderManager::GetInstance()->GetEffect(strName);
			if (pEffect != nullptr)
				return pEffect;

			ShaderMacros macros;
			for (int i = 0; i < EmParticleShader::MaskCount; ++i)
			{
				if (GetBitMask64(nMask, i) == true)
				{
					macros.AddMacro(EmParticleShader::GetMaskName(i), "1");
				}
			}
			macros.EndSet();

			static std::string strPath;
			if (strPath.empty() == true)
			{
				strPath.append(File::GetPath(File::EmPath::eFx));
				strPath.append("Particle\\Particle.fx");
			}

			pEffect = IEffect::Compile(strName, strPath.c_str(), &macros);
			if (pEffect == nullptr)
				return nullptr;

			if (GetBitMask64(nMask, EmParticleShader::eUseDecal))
			{
				pEffect->CreateTechnique(StrID::Decal, EmVertexFormat::ePos);
			}
			else
			{
				pEffect->CreateTechnique(StrID::Emitter, EmVertexFormat::ePosTexCol);
			}

			return pEffect;
		}

		static void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffect* pEffect, IEffectTech* pEffectTech)
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

			pEffect->UndoBlendState(StrID::g_samplerState, 0);

			pEffect->SetTexture(StrID::g_texDepth, nullptr);
			pEffect->UndoBlendState(StrID::g_sampler, 0);

			pEffect->ClearState(pd3dDeviceContext, pEffectTech);
		}

		struct ClassifyParticle
		{
			struct ParticleVertex
			{
				uint32_t nVertexCount = 0;
				uint32_t nCopyCount = 0;
				std::vector<std::pair<VertexPosTexCol*, uint32_t>> vecVerticesPtr;
			};

			std::unordered_map<IBlendState*, ParticleVertex> mapVertex;
		};

		struct ClassifyDecal
		{
			uint32_t nCopyCount = 0;

			std::vector<Math::Matrix> vecInstMatWVP;
			std::vector<Math::Matrix> vecInstMatWorld;
			std::vector<Math::Matrix> vecInstMatInvWorldView;
			std::vector<Math::Color> vecInstColor;
		};

		ParticleRenderer::ParticleRenderer()
			: m_pEffect(nullptr)
		{
		}

		ParticleRenderer::~ParticleRenderer()
		{
			IEffect::Destroy(&m_pEffect);

			SafeDelete(m_pEmitterVB);
			SafeDelete(m_pEmitterIB);

			SafeDelete(m_pDecalVB);
			SafeDelete(m_pDecalIB);
		}

		bool ParticleRenderer::Init(const Math::Viewport& viewport)
		{
			// Emitter
			{
				m_pEmitterVB = IVertexBuffer::Create(VertexPosTexCol::Format(), eEmitterCapacity * 4, nullptr, D3D11_USAGE_DYNAMIC);

				std::vector<uint32_t> vecIndices;
				vecIndices.reserve(eEmitterCapacity * 6);
				uint32_t nSize = eEmitterCapacity * 6;
				for (uint32_t i = 0; i < nSize; i += 4)
				{
					vecIndices.push_back(i);
					vecIndices.push_back(i + 1);
					vecIndices.push_back(i + 2);
					vecIndices.push_back(i);
					vecIndices.push_back(i + 2);
					vecIndices.push_back(i + 3);
				}

				m_pEmitterIB = IIndexBuffer::Create(vecIndices.size(), &vecIndices.front(), D3D11_USAGE_IMMUTABLE);

				if (m_pEmitterVB == nullptr || m_pEmitterIB == nullptr)
					return false;
			}

			// Decal
			{
				std::vector<VertexPos> vecVertices;
				vecVertices.reserve(24);

				std::vector<uint32_t> vecIndices;
				vecIndices.reserve(36);

				// A box has six faces, each one pointing in a different direction.
				const int FaceCount = 6;

				const Math::Vector3 faceNormals[FaceCount] =
				{
					{ 0, 0, 1 },
					{ 0, 0, -1 },
					{ 1, 0, 0 },
					{ -1, 0, 0 },
					{ 0, 1, 0 },
					{ 0, -1, 0 },
				};

				Math::Vector3 tsize = Math::Vector3(0.5f, 0.5f, 0.5f);

				// Create each face in turn.
				for (int i = 0; i < FaceCount; ++i)
				{
					Math::Vector3 normal = faceNormals[i];

					// Get two vectors perpendicular both to the face normal and to each other.
					Math::Vector3 basis = (i >= 4) ? Math::Vector3(0.f, 0.f, 1.f) : Math::Vector3(0.f, 1.f, 0.f);

					Math::Vector3 side1 = normal.Cross(basis);
					Math::Vector3 side2 = normal.Cross(side1);

					// Six indices (two triangles) per face.
					size_t vbase = vecVertices.size();
					vecIndices.push_back(vbase + 0);
					vecIndices.push_back(vbase + 1);
					vecIndices.push_back(vbase + 2);

					vecIndices.push_back(vbase + 0);
					vecIndices.push_back(vbase + 2);
					vecIndices.push_back(vbase + 3);

					// Four vertices per face.
					vecVertices.push_back(VertexPos((normal - side1 - side2) * tsize));
					vecVertices.push_back(VertexPos((normal - side1 + side2) * tsize));
					vecVertices.push_back(VertexPos((normal + side1 + side2) * tsize));
					vecVertices.push_back(VertexPos((normal + side1 - side2) * tsize));
				}

				for (auto it = vecIndices.begin(); it != vecIndices.end(); it += 3)
				{
					std::swap(*it, *(it + 2));
				}

				m_pDecalVB = IVertexBuffer::Create(VertexPos::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_IMMUTABLE);
				m_pDecalIB = IIndexBuffer::Create(vecIndices.size(), &vecIndices.front(), D3D11_USAGE_IMMUTABLE);

				if (m_pDecalVB == nullptr || m_pDecalIB == nullptr)
					return false;
			}

			return true;
		}

		void ParticleRenderer::Render(uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(EffectRenderer);

			if ((nRenderGroupFlag & EmParticleGroup::eDecal) != 0)
			{
				renderDecal();
			}

			if ((nRenderGroupFlag & EmParticleGroup::eEmitter) != 0)
			{
				renderEmitter();
			}
		}

		void ParticleRenderer::Flush()
		{
			m_listDecal.clear();
			m_nEmitterVertexCount = 0;
		}

		void ParticleRenderer::renderEmitter()
		{
			D3D_PROFILING(Particle);

			if (m_queueEmitter.empty())
				return;

			std::map<std::shared_ptr<ITexture>, ClassifyParticle> mapClassifyParticle;

			{
				D3D_PROFILING(Classify);

				std::list<RenderSubsetParticleEmitter>	listParticle;
				while (m_queueEmitter.empty() == false)
				{
					auto& renderSubset = m_queueEmitter.top();

					mapClassifyParticle[renderSubset.pTexture].mapVertex[renderSubset.pBlendState].nVertexCount += renderSubset.nVertexCount;
					mapClassifyParticle[renderSubset.pTexture].mapVertex[renderSubset.pBlendState].nCopyCount++;

					listParticle.emplace_back(renderSubset);

					m_queueEmitter.pop();
				}

				for (auto& iter : mapClassifyParticle)
				{
					ClassifyParticle& particle = iter.second;

					for (auto iter_vertex = particle.mapVertex.begin(); iter_vertex != particle.mapVertex.end(); ++iter_vertex)
					{
						ClassifyParticle::ParticleVertex& vertex = iter_vertex->second;

						vertex.vecVerticesPtr.reserve(vertex.nCopyCount);
					}
				}

				for (auto& iter : listParticle)
				{
					RenderSubsetParticleEmitter& renderSubset = iter;

					ClassifyParticle& quadParticle = mapClassifyParticle[renderSubset.pTexture];

					ClassifyParticle::ParticleVertex& vertex = quadParticle.mapVertex[renderSubset.pBlendState];

					vertex.vecVerticesPtr.emplace_back(std::make_pair(renderSubset.pVertices, renderSubset.nVertexCount));
				}

				VertexPosTexCol* pVertices = nullptr;
				if (m_pEmitterVB->Map(0, D3D11_MAP_WRITE_NO_OVERWRITE, reinterpret_cast<void**>(&pVertices)) == false)
					return;

				for (auto& iter : mapClassifyParticle)
				{
					ClassifyParticle& quadParticle = iter.second;

					auto& mapVertex = quadParticle.mapVertex;
					for (auto& iter_blend : mapVertex)
					{
						ClassifyParticle::ParticleVertex& quadVertex = iter_blend.second;
						for (auto& value : quadVertex.vecVerticesPtr)
						{
							Memory::Copy(pVertices, sizeof(VertexPosTexCol) * value.second, value.first, sizeof(VertexPosTexCol) * value.second);
							pVertices += value.second;
						}
					}
				}

				m_pEmitterVB->Unmap(0);
			}

			{
				D3D_PROFILING(Render);

				IEffect* pEffect = GetEffect(0);
				if (pEffect == nullptr)
					return;

				IEffectTech* pEffectTech = pEffect->GetTechnique(StrID::Emitter);
				if (pEffectTech == nullptr)
				{
					LOG_ERROR("Not Exist EffectTech !!");
					return;
				}

				IDevice* pDevice = GetDevice();
				IDeviceContext* pDeviceContext = GetDeviceContext();
				if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
					return;

				pDeviceContext->ClearState();
				pDeviceContext->SetDefaultViewport();

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

				pDeviceContext->SetRenderTargets(&pRenderTarget, 1, nullptr);
				pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);

				pDeviceContext->SetVertexBuffers(m_pEmitterVB, m_pEmitterVB->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(m_pEmitterIB, 0);

				uint32_t nStartIndex = 0;
				for (auto iter = mapClassifyParticle.begin(); iter != mapClassifyParticle.end(); ++iter)
				{
					ClassifyParticle& quadParticle = iter->second;

					auto& mapVertex = quadParticle.mapVertex;
					for (auto iter_blend = mapVertex.begin(); iter_blend != mapVertex.end(); ++iter_blend)
					{
						pDeviceContext->SetBlendState(iter_blend->first);

						ClassifyParticle::ParticleVertex& quadVertex = iter_blend->second;

						pEffect->SetTexture(StrID::g_texAlbedo, iter->first);

						uint32_t nPassCount = pEffectTech->GetPassCount();
						for (uint32_t p = 0; p < nPassCount; ++p)
						{
							pEffectTech->PassApply(p, pDeviceContext);

							uint32_t nIndexCount = (uint32_t)(quadVertex.nVertexCount * 1.5f);
							pDeviceContext->DrawIndexed(nIndexCount, 0, nStartIndex);

							nStartIndex += nIndexCount;
						}
					}
				}

				ClearEffect(pDeviceContext, pEffect, pEffectTech);

				pDevice->ReleaseRenderTargets(&pRenderTarget);
			}
		}

		void ParticleRenderer::renderDecal()
		{
			D3D_PROFILING(Decal);

			if (m_listDecal.empty())
				return;

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
				return;

			std::map<IMaterial*, ClassifyDecal> mapClassifyDecal;

			{
				D3D_PROFILING(Classify);

				for (auto& iter : m_listDecal)
				{
					mapClassifyDecal[iter.pMaterial].nCopyCount++;
				}

				for (auto& iter : mapClassifyDecal)
				{
					iter.second.vecInstMatWVP.reserve(iter.second.nCopyCount);
					iter.second.vecInstMatWorld.reserve(iter.second.nCopyCount);
					iter.second.vecInstMatInvWorldView.reserve(iter.second.nCopyCount);
					iter.second.vecInstColor.reserve(iter.second.nCopyCount);
				}

				for (auto& iter : m_listDecal)
				{
					mapClassifyDecal[iter.pMaterial].vecInstMatWVP.emplace_back(iter.matWVP);
					mapClassifyDecal[iter.pMaterial].vecInstMatWorld.emplace_back(iter.matWorld);

					Math::Matrix matInvWorldView = iter.matWorld * pCamera->GetViewMatrix();
					//Math::Matrix matInvWorldView = iter.matWorld;
					matInvWorldView = matInvWorldView.Invert();

					mapClassifyDecal[iter.pMaterial].vecInstMatInvWorldView.emplace_back(matInvWorldView);
				}
			}

			{
				D3D_PROFILING(Render);

				IDevice* pDevice = GetDevice();
				IDeviceContext* pDeviceContext = GetDeviceContext();

				pDeviceContext->ClearState();
				pDeviceContext->SetDefaultViewport();

				IRenderTarget* pRenderTarget[] =
				{
					pDevice->GetGBuffers()->GetGBuffer(EmGBuffer::eNormals),
					pDevice->GetGBuffers()->GetGBuffer(EmGBuffer::eColors),
					pDevice->GetGBuffers()->GetGBuffer(EmGBuffer::eDisneyBRDF),
				};
				pDeviceContext->SetRenderTargets(pRenderTarget, _countof(pRenderTarget), nullptr);
				pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				pDeviceContext->SetVertexBuffers(m_pDecalVB, m_pDecalVB->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(m_pDecalIB, 0);

				Math::Vector4 f4CornersTopRight(Math::Vector4::One);
				f4CornersTopRight = Math::Vector4::Transform(f4CornersTopRight, pCamera->GetProjMatrix().Invert());
				f4CornersTopRight /= f4CornersTopRight.w;

				for (auto& iter : mapClassifyDecal)
				{
					int64_t nMask = 0;
					SetBitMask64(nMask, EmParticleShader::eUseDecal);

					IMaterial* pMaterial = iter.first;
					if (pMaterial != nullptr)
					{
						auto IsValidTexture = [&](EmMaterial::Type emType) -> bool
						{
							const std::shared_ptr<ITexture>& pTexture = pMaterial->GetTexture(emType);
							if (pTexture == nullptr)
								return false;

							return pTexture->GetLoadState() == EmLoadState::eComplete;
						};

						SetBitMask64(nMask, IsValidTexture(EmMaterial::eAlbedo) == true ? EmParticleShader::eUseTexAlbedo : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eMask) == true ? EmParticleShader::eUseTexMask : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eNormal) == true ? EmParticleShader::eUseTexNormal : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eDisplacement) == true ? EmParticleShader::eUseTexDisplacement : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eSpecularColor) == true ? EmParticleShader::eUseTexSpecularColor : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eRoughness) == true ? EmParticleShader::eUseTexRoughness : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eMetallic) == true ? EmParticleShader::eUseTexMetallic : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eEmissive) == true ? EmParticleShader::eUseTexEmissive : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eSubsurface) == true ? EmParticleShader::eUseTexSubsurface : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eSpecular) == true ? EmParticleShader::eUseTexSpecular : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eSpecularTint) == true ? EmParticleShader::eUseTexSpecularTint : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eAnisotropic) == true ? EmParticleShader::eUseTexAnisotropic : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eSheen) == true ? EmParticleShader::eUseTexSheen : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eSheenTint) == true ? EmParticleShader::eUseTexSheenTint : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eClearcoat) == true ? EmParticleShader::eUseTexClearcoat : -1);
						SetBitMask64(nMask, IsValidTexture(EmMaterial::eClearcoatGloss) == true ? EmParticleShader::eUseTexClearcoatGloss : -1);
					}

					IEffect* pEffect = GetEffect(nMask);
					if (pEffect == nullptr)
					{
						LOG_ERROR("Not Exist Effect !!");
						return;
					}

					IEffectTech* pEffectTech = pEffect->GetTechnique(StrID::Decal);
					if (pEffectTech == nullptr)
					{
						LOG_ERROR("Not Exist EffectTech !!");
						return;
					}

					if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
						return;

					pEffect->SetTexture(StrID::g_texDepth, pDevice->GetMainDepthStencil()->GetTexture());

					pEffect->SetSamplerState(StrID::g_sampler, pDevice->GetSamplerState(EmSamplerState::eMinMagMipLinearWrap), 0);

					pEffect->SetVector(StrID::g_f3CameraTopRight, Math::Vector3(f4CornersTopRight.x, -f4CornersTopRight.y, f4CornersTopRight.z));
					pEffect->SetVector(StrID::g_f3CameraPos, pCamera->GetPosition());

					pEffect->SetMatrix(StrID::g_matView, pCamera->GetViewMatrix());
					pEffect->SetMatrix(StrID::g_matProj, pCamera->GetProjMatrix());
					pEffect->SetMatrix(StrID::g_matInvView, pCamera->GetViewMatrix().Invert());
					pEffect->SetMatrix(StrID::g_matInvProj, pCamera->GetProjMatrix().Invert());

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
						pEffect->SetTexture(StrID::g_texSurface, pMaterial->GetTexture(EmMaterial::eSubsurface));
						pEffect->SetTexture(StrID::g_texSpecular, pMaterial->GetTexture(EmMaterial::eSpecular));
						pEffect->SetTexture(StrID::g_texSpecularTint, pMaterial->GetTexture(EmMaterial::eSpecularTint));
						pEffect->SetTexture(StrID::g_texAnisotropic, pMaterial->GetTexture(EmMaterial::eAnisotropic));
						pEffect->SetTexture(StrID::g_texSheen, pMaterial->GetTexture(EmMaterial::eSheen));
						pEffect->SetTexture(StrID::g_texSheenTint, pMaterial->GetTexture(EmMaterial::eSheenTint));
						pEffect->SetTexture(StrID::g_texClearcoat, pMaterial->GetTexture(EmMaterial::eClearcoat));
						pEffect->SetTexture(StrID::g_texClearcoatGloss, pMaterial->GetTexture(EmMaterial::eClearcoatGloss));

						ISamplerState* pSamplerState = pDevice->GetSamplerState(pMaterial->GetSamplerState());
						pEffect->SetSamplerState(StrID::g_samplerState, pSamplerState, 0);

						pDeviceContext->SetBlendState(pMaterial->GetBlendState());
						pDeviceContext->SetDepthStencilState(pMaterial->GetDepthStencilState());
						pDeviceContext->SetRasterizerState(pMaterial->GetRasterizerState());
					}
					else
					{
						pEffect->SetVector(StrID::g_f4AlbedoColor, reinterpret_cast<const Math::Vector4&>(Math::Color::White));
						pEffect->SetVector(StrID::g_f4EmissiveColor, reinterpret_cast<const Math::Vector4&>(Math::Color::Black));

						pEffect->SetVector(StrID::g_f4DisRoughMetEmi, reinterpret_cast<const Math::Vector4&>(Math::Color::Transparent));
						pEffect->SetVector(StrID::g_f4SurSpecTintAniso, reinterpret_cast<const Math::Vector4&>(Math::Color::Transparent));
						pEffect->SetVector(StrID::g_f4SheenTintClearcoatGloss, reinterpret_cast<const Math::Vector4&>(Math::Color::Transparent));

						ClearEffect(pDeviceContext, pEffect, pEffectTech);

						pDeviceContext->SetBlendState(pDevice->GetBlendState(EmBlendState::eOff));
						pDeviceContext->SetDepthStencilState(pDevice->GetDepthStencilState(EmDepthStencilState::eRead_Write_On));
						pDeviceContext->SetRasterizerState(pDevice->GetRasterizerState(EmRasterizerState::eSolidCCW));
					}

					uint32_t nInstanceSize = iter.second.nCopyCount;
					uint32_t nLoopCount = iter.second.nCopyCount / 32 + 1;
					for (uint32_t j = 0; j < nLoopCount; ++j)
					{
						int nMax = std::min(32 * (j + 1), nInstanceSize);
						int nNum = nMax - j * 32;

						if (nNum <= 0)
							break;

						pEffect->SetMatrixArray(StrID::g_InstancesMatWVP, &iter.second.vecInstMatWVP[j * 32], 0, nNum);
						pEffect->SetMatrixArray(StrID::g_InstancesMatWorld, &iter.second.vecInstMatWorld[j * 32], 0, nNum);
						pEffect->SetMatrixArray(StrID::g_InstancesMatInvWorld, &iter.second.vecInstMatInvWorldView[j * 32], 0, nNum);

						uint32_t nPassCount = pEffectTech->GetPassCount();
						for (uint32_t p = 0; p < nPassCount; ++p)
						{
							pEffectTech->PassApply(p, pDeviceContext);

							pDeviceContext->DrawIndexedInstanced(m_pDecalIB->GetIndexNum(), nNum, 0, 0, 0);
						}
					}

					ClearEffect(pDeviceContext, pEffect, pEffectTech);
				}

				pDevice->ReleaseRenderTargets(pRenderTarget, _countof(pRenderTarget));
			}
		}
	}
}