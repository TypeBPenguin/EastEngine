#include "stdafx.h"
#include "WaterRenderer.h"

#include "WaterSimulator.h"

#include "../CommonLib/FileUtil.h"
#include "../CommonLib/Timer.h"

#include "../DirectX/CameraManager.h"

#define FRESNEL_TEX_SIZE			256
#define PERLIN_TEX_SIZE				64

namespace StrID
{	
	RegisterStringID(EffectWater);
	RegisterStringID(Water);
	RegisterStringID(WaterWireframe);
	RegisterStringID(WaterFresnelMap);

	RegisterStringID(g_SkyColor);
	RegisterStringID(g_WaterbodyColor);
	RegisterStringID(g_Shineness);
	RegisterStringID(g_SunDir);
	RegisterStringID(g_SunColor);
	RegisterStringID(g_BendParam);
	RegisterStringID(g_PerlinSize);
	RegisterStringID(g_PerlinAmplitude);
	RegisterStringID(g_PerlinOctave);
	RegisterStringID(g_PerlinGradient);
	RegisterStringID(g_TexelLength_x2);
	RegisterStringID(g_UVScale);
	RegisterStringID(g_UVOffset);
	RegisterStringID(g_matLocal);
	RegisterStringID(g_matWorldViewProj);
	RegisterStringID(g_UVBase);
	RegisterStringID(g_PerlinMovement);
	RegisterStringID(g_LocalEye);
	RegisterStringID(g_texDisplacement);
	RegisterStringID(g_texPerlin);
	RegisterStringID(g_texGradient);
	RegisterStringID(g_texFresnel);
	RegisterStringID(g_texReflectCube);
	RegisterStringID(g_samplerDisplacement);
	RegisterStringID(g_samplerPerlin);
	RegisterStringID(g_samplerGradient);
	RegisterStringID(g_samplerFresnel);
	RegisterStringID(g_samplerCube);
}

namespace EastEngine
{
	namespace Graphics
	{
		WaterRenderer::WaterRenderer()
			: m_pWaterSimulator(nullptr)
			, m_nMeshDim(128)
			, m_fPatchLength(0.f)
			, m_nDisplaceMapDim(0)
			, m_fUpperGridCoverage(64.0f)
			, m_nFurthestCover(8)
			, m_f3SkyColor(0.38f, 0.45f, 0.56f)
			, m_f3WaterbodyColor(0.07f, 0.15f, 0.2f)
			, m_fSkyBlending(16.0f)
			, m_fPerlinSize(1.0f)
			, m_fPerlinSpeed(0.06f)
			, m_f3PerlinAmplitude(35, 42, 57)
			, m_f3PerlinGradient(1.4f, 1.6f, 2.2f)
			, m_f3PerlinOctave(1.12f, 0.59f, 0.23f)
			, m_f3BendParam(0.1f, -0.4f, 0.2f)
			, m_f3SunDir(0.936016f, -0.343206f, 0.0780013f)
			, m_f3SunColor(1.0f, 1.0f, 0.6f)
			, m_fShineness(400.0f)
			, m_nLods(0)
			, m_pVertexBuffer(nullptr)
			, m_pIndexBuffer(nullptr)
			, m_pFresnelMap(nullptr)
		{
			Memory::Clear(&m_mesh_patterns, sizeof(m_mesh_patterns));
		}

		WaterRenderer::~WaterRenderer()
		{
			SafeReleaseDelete(m_pWaterSimulator);
			SafeDelete(m_pVertexBuffer);
			SafeDelete(m_pIndexBuffer);

			m_pFresnelMap.reset();
			m_pSRV_Perlin.reset();
			m_pSRV_ReflectCube.reset();
		}

		bool WaterRenderer::Init(const Math::Viewport& viewport)
		{
			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Water\\Water_D.cso");
#else
			strPath.append("Water\\Water.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectWater, strPath.c_str());
			m_pEffect->CreateTechnique(StrID::Water, EmVertexFormat::ePos);
			m_pEffect->CreateTechnique(StrID::WaterWireframe, EmVertexFormat::ePos);

			OceanParameter ocean_param;
			// The size of displacement map. In this sample, it's fixed to 512.
			ocean_param.dmap_dim = 512;
			// The side length (world space) of square patch
			ocean_param.patch_length = 2000.0f;
			// Adjust this parameter to control the simulation speed
			ocean_param.time_scale = 0.8f;
			// A scale to control the amplitude. Not the world space height
			ocean_param.wave_amplitude = 0.35f;
			// 2D wind direction. No need to be normalized
			ocean_param.wind_dir = Math::Vector2(0.8f, 0.6f);
			// The bigger the wind speed, the larger scale of wave crest.
			// But the wave scale can be no larger than patch_length
			ocean_param.wind_speed = 600.0f;
			// Damp out the components opposite to wind direction.
			// The smaller the value, the higher wind dependency
			ocean_param.wind_dependency = 0.07f;
			// Control the scale of horizontal movement. Higher value creates
			// pointy crests.
			ocean_param.choppy_scale = 1.f;

			m_pWaterSimulator = new WaterSimulator;
			if (m_pWaterSimulator->Init(ocean_param) == false)
			{
				SafeReleaseDelete(m_pWaterSimulator);
				return false;
			}

			// Update the simulation for the first time.
			m_pWaterSimulator->UpdateDisplacementMap(0.f);

			// Init D3D11 resources for rendering
			m_fPatchLength = ocean_param.patch_length;
			m_nDisplaceMapDim = ocean_param.dmap_dim;
			m_f2WindDir = ocean_param.wind_dir;

			// D3D buffers
			createSurfaceMesh();
			createFresnelMap();
			loadTextures();

			// Samplers
			SamplerStateDesc sam_desc;
			sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sam_desc.MipLODBias = 0;
			sam_desc.MaxAnisotropy = 1;
			sam_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sam_desc.BorderColor[0] = 1.0f;
			sam_desc.BorderColor[1] = 1.0f;
			sam_desc.BorderColor[2] = 1.0f;
			sam_desc.BorderColor[3] = 1.0f;
			sam_desc.MinLOD = 0;
			sam_desc.MaxLOD = FLT_MAX;
			m_pSamplerHeight = ISamplerState::Create(sam_desc);

			sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			m_pSamplerCube = ISamplerState::Create(sam_desc);

			sam_desc.Filter = D3D11_FILTER_ANISOTROPIC;
			sam_desc.MaxAnisotropy = 8;
			m_pSamplerGradient = ISamplerState::Create(sam_desc);

			sam_desc.MaxLOD = FLT_MAX;
			sam_desc.MaxAnisotropy = 4;
			m_pSamplerPerlin = ISamplerState::Create(sam_desc);

			sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			m_pSamplerFresnel = ISamplerState::Create(sam_desc);

			BlendStateDesc blendDesc;
			blendDesc.AlphaToCoverageEnable = false;
			blendDesc.IndependentBlendEnable = false;
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			m_pBlendState = IBlendState::Create(blendDesc);

			return true;
		}

		void WaterRenderer::Render(uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(Water);

			double dGameTime = Timer::GetInstance()->GetGameTime();

			simulate((float)(dGameTime));

			render((float)(dGameTime));
		}

		void WaterRenderer::Flush()
		{
			m_vecRenderNode.clear();
		}

		void WaterRenderer::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech)
		{
			m_pEffect->SetTexture(StrID::g_texDisplacement, nullptr);
			m_pEffect->SetTexture(StrID::g_texPerlin, nullptr);
			m_pEffect->SetTexture(StrID::g_texGradient, nullptr);
			m_pEffect->SetTexture(StrID::g_texFresnel, nullptr);
			m_pEffect->SetTexture(StrID::g_texReflectCube, nullptr);
			m_pEffect->UndoSamplerState(StrID::g_samplerDisplacement, 0);
			m_pEffect->UndoSamplerState(StrID::g_samplerPerlin, 0);
			m_pEffect->UndoSamplerState(StrID::g_samplerGradient, 0);
			m_pEffect->UndoSamplerState(StrID::g_samplerFresnel, 0);
			m_pEffect->UndoSamplerState(StrID::g_samplerCube, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pEffectTech);
		}

		void WaterRenderer::simulate(float fTime)
		{
			D3D_PROFILING(Simulate);

			m_pWaterSimulator->UpdateDisplacementMap(fTime);
		}

		void WaterRenderer::render(float fTime)
		{
			D3D_PROFILING(Render);
			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Water);
			if (pEffectTech == nullptr)
			{
				PRINT_LOG("Not Exist EffectTech !!");
				return;
			}

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				PRINT_LOG("Not Exist Main Camera !!");
				return;
			}

			// Build rendering list
			float ocean_extent = m_fPatchLength * (1 << m_nFurthestCover);
			QuadNode root_node = { Math::Vector2(-ocean_extent * 0.5f, -ocean_extent * 0.5f), ocean_extent, 0,{ -1,-1,-1,-1 } };

			buildNodeList(root_node, pCamera);

			// Matrices
			//const Matrix& matView = pCamera->GetViewMatrix();
			const Math::Matrix matView = Math::Matrix(1.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 0.f, 1.f) * pCamera->GetViewMatrix();
			const Math::Matrix& matProj = pCamera->GetProjMatrix();

			IDevice* pDevice = GetDevice();
			IDeviceContext* pDeviceContext = GetDeviceContext();

			pDeviceContext->ClearState();

			IRenderTarget* pRenderTarget = pDevice->GetRenderTarget(pDevice->GetLastUseRenderTarget()->GetDesc2D());
			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, pDevice->GetMainDepthStencil());

			// IA setup
			pDeviceContext->SetVertexBuffers(m_pVertexBuffer, m_pVertexBuffer->GetFormatSize(), 0);
			pDeviceContext->SetIndexBuffer(m_pIndexBuffer, 0);

			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOn);
			pDeviceContext->SetRasterizerState(EmRasterizerState::eNone);
			pDeviceContext->SetBlendState(EmBlendState::eLinear);

			// Textures
			m_pEffect->SetTexture(StrID::g_texDisplacement, m_pWaterSimulator->GetTextureDisplacementMap());
			m_pEffect->SetTexture(StrID::g_texPerlin, m_pSRV_Perlin);

			m_pEffect->SetTexture(StrID::g_texGradient, m_pWaterSimulator->GetTextureGradientMap());
			m_pEffect->SetTexture(StrID::g_texFresnel, m_pFresnelMap);

			m_pEffect->SetTexture(StrID::g_texReflectCube, m_pSRV_ReflectCube);

			// Samplers
			m_pEffect->SetSamplerState(StrID::g_samplerDisplacement, m_pSamplerHeight, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerPerlin, m_pSamplerPerlin, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerFresnel, m_pSamplerFresnel, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerGradient, m_pSamplerGradient, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerCube, m_pSamplerCube, 0);

			m_pEffect->SetFloat(StrID::g_TexelLength_x2, m_fPatchLength / m_nDisplaceMapDim * 2);

			m_pEffect->SetVector(StrID::g_SkyColor, m_f3SkyColor);
			m_pEffect->SetVector(StrID::g_WaterbodyColor, m_f3WaterbodyColor);

			m_pEffect->SetFloat(StrID::g_UVScale, 1.f / m_fPatchLength);
			m_pEffect->SetFloat(StrID::g_UVOffset, 0.5f / m_nDisplaceMapDim);

			m_pEffect->SetFloat(StrID::g_PerlinSize, m_fPerlinSize);
			m_pEffect->SetVector(StrID::g_PerlinAmplitude, m_f3PerlinAmplitude);
			m_pEffect->SetVector(StrID::g_PerlinGradient, m_f3PerlinGradient);
			m_pEffect->SetVector(StrID::g_PerlinOctave, m_f3PerlinOctave);

			m_pEffect->SetVector(StrID::g_BendParam, m_f3BendParam);

			m_pEffect->SetVector(StrID::g_SunColor, m_f3SunColor);
			m_pEffect->SetVector(StrID::g_SunDir, m_f3SunDir);
			m_pEffect->SetFloat(StrID::g_Shineness, m_fShineness);

			// We assume the center of the ocean surface at (0, 0, 0).
			uint32_t nSize = m_vecRenderNode.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				QuadNode& node = m_vecRenderNode[i];

				if (isLeaf(node) == false)
					continue;

				// Check adjacent patches and select mesh pattern
				QuadRenderParam& render_param = selectMeshPattern(node);

				// Find the right LOD to render
				int level_size = m_nMeshDim;
				for (int lod = 0; lod < node.lod; lod++)
					level_size >>= 1;

				Math::Matrix matScale = Math::Matrix::CreateScale(node.length / level_size, node.length / level_size, 0.f);
				m_pEffect->SetMatrix(StrID::g_matLocal, matScale);

				// WVP matrix
				Math::Matrix matWorld = Math::Matrix::CreateTranslation(node.bottom_left.x, node.bottom_left.y, 0.f);
				Math::Matrix matWVP = matWorld * matView * matProj;
				m_pEffect->SetMatrix(StrID::g_matWorldViewProj, matWVP);

				// Texcoord for perlin noise
				Math::Vector2 uv_base = Math::Vector2(node.bottom_left / m_fPatchLength) * m_fPerlinSize;
				m_pEffect->SetVector(StrID::g_UVBase, uv_base);

				// Constant g_PerlinSpeed need to be adjusted mannually
				Math::Vector2 perlin_move = -m_f2WindDir * fTime * m_fPerlinSpeed;
				m_pEffect->SetVector(StrID::g_PerlinMovement, perlin_move);

				// Eye point
				Math::Matrix matInvWV = matWorld;
				m_pEffect->SetVector(StrID::g_LocalEye, matWorld.Invert().Translation());

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					// Perform draw call
					if (render_param.num_inner_faces > 0)
					{
						// Inner mesh of the patch
						pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
						pDeviceContext->DrawIndexed(render_param.num_inner_faces + 2, render_param.inner_start_index, 0);
					}

					if (render_param.num_boundary_faces > 0)
					{
						// Boundary mesh of the patch
						pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						pDeviceContext->DrawIndexed(render_param.num_boundary_faces * 3, render_param.boundary_start_index, 0);
					}
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);

			pDevice->ReleaseRenderTargets(&pRenderTarget);
		}

		int WaterRenderer::buildNodeList(QuadNode& quad_node, Camera* pCamera)
		{
			// Check against view frustum
			if (!checkNodeVisibility(quad_node, pCamera))
				return -1;

			// Estimate the min grid coverage
			uint32_t num_vps = 1;
			D3D11_VIEWPORT vp;
			GetDeviceContext()->GetInterface()->RSGetViewports(&num_vps, &vp);
			float min_coverage = estimateGridCoverage(quad_node, pCamera, vp.Width * vp.Height);

			// Recursively attatch sub-nodes.
			bool visible = true;
			if (min_coverage > m_fUpperGridCoverage && quad_node.length > m_fPatchLength)
			{
				// Recursive rendering for sub-quads.
				QuadNode sub_node_0 = { quad_node.bottom_left, quad_node.length / 2, 0,{ -1, -1, -1, -1 } };
				quad_node.sub_node[0] = buildNodeList(sub_node_0, pCamera);

				QuadNode sub_node_1 = { quad_node.bottom_left + Math::Vector2(quad_node.length / 2, 0), quad_node.length / 2, 0,{ -1, -1, -1, -1 } };
				quad_node.sub_node[1] = buildNodeList(sub_node_1, pCamera);

				QuadNode sub_node_2 = { quad_node.bottom_left + Math::Vector2(quad_node.length / 2, quad_node.length / 2), quad_node.length / 2, 0,{ -1, -1, -1, -1 } };
				quad_node.sub_node[2] = buildNodeList(sub_node_2, pCamera);

				QuadNode sub_node_3 = { quad_node.bottom_left + Math::Vector2(0, quad_node.length / 2), quad_node.length / 2, 0,{ -1, -1, -1, -1 } };
				quad_node.sub_node[3] = buildNodeList(sub_node_3, pCamera);

				visible = !isLeaf(quad_node);
			}

			if (visible == false)
				return -1;

			// Estimate mesh LOD
			int lod = 0;
			for (lod = 0; lod < m_nLods - 1; ++lod)
			{
				if (min_coverage > m_fUpperGridCoverage)
					break;

				min_coverage *= 4;
			}

			// We don't use 1x1 and 2x2 patch. So the highest level is g_Lods - 2.
			quad_node.lod = std::min(lod, m_nLods - 2);

			// Insert into the list
			int position = (int)m_vecRenderNode.size();
			m_vecRenderNode.push_back(quad_node);

			return position;
		}

		bool WaterRenderer::checkNodeVisibility(const QuadNode& quad_node, Camera* pCamera)
		{
			// Plane equation setup
			//const Matrix& matView = pCamera->GetViewMatrix();
			const Math::Matrix matView = Math::Matrix(1.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 0.f, 1.f) * pCamera->GetViewMatrix();
			const Math::Matrix& matProj = pCamera->GetProjMatrix();

			// Left plane
			float fov_x = std::atan(1.0f / matProj.m[0][0]);
			Math::Vector4 plane_left(cos(fov_x), 0.f, sin(fov_x), 0.f);
			// Right plane
			Math::Vector4 plane_right(-cos(fov_x), 0.f, sin(fov_x), 0.f);

			// Bottom plane
			float fov_y = std::atan(1.0f / matProj.m[1][1]);
			Math::Vector4 plane_bottom(0.f, cos(fov_y), sin(fov_y), 0.f);
			// Top plane
			Math::Vector4 plane_top(0.f, -cos(fov_y), sin(fov_y), 0.f);

			// Test quad corners against view frustum in view space
			Math::Vector4 corner_verts[4];
			corner_verts[0] = Math::Vector4(quad_node.bottom_left.x, quad_node.bottom_left.y, 0.f, 1.f);
			corner_verts[1] = corner_verts[0] + Math::Vector4(quad_node.length, 0.f, 0.f, 0.f);
			corner_verts[2] = corner_verts[0] + Math::Vector4(quad_node.length, quad_node.length, 0.f, 0.f);
			corner_verts[3] = corner_verts[0] + Math::Vector4(0, quad_node.length, 0.f, 0.f);

			Math::Vector4::Transform(corner_verts[0], matView, corner_verts[0]);
			Math::Vector4::Transform(corner_verts[1], matView, corner_verts[1]);
			Math::Vector4::Transform(corner_verts[2], matView, corner_verts[2]);
			Math::Vector4::Transform(corner_verts[3], matView, corner_verts[3]);

			// Test against eye plane
			if (corner_verts[0].z < 0.f && corner_verts[1].z < 0.f && corner_verts[2].z < 0.f && corner_verts[3].z < 0.f)
				return false;

			// Test against left plane
			float dist_0 = corner_verts[0].Dot(plane_left);
			float dist_1 = corner_verts[1].Dot(plane_left);
			float dist_2 = corner_verts[2].Dot(plane_left);
			float dist_3 = corner_verts[3].Dot(plane_left);
			if (dist_0 < 0.f && dist_1 < 0.f && dist_2 < 0.f && dist_3 < 0.f)
				return false;

			// Test against right plane
			dist_0 = corner_verts[0].Dot(plane_right);
			dist_1 = corner_verts[1].Dot(plane_right);
			dist_2 = corner_verts[2].Dot(plane_right);
			dist_3 = corner_verts[3].Dot(plane_right);
			if (dist_0 < 0.f && dist_1 < 0.f && dist_2 < 0.f && dist_3 < 0.f)
				return false;

			// Test against bottom plane
			dist_0 = corner_verts[0].Dot(plane_bottom);
			dist_1 = corner_verts[1].Dot(plane_bottom);
			dist_2 = corner_verts[2].Dot(plane_bottom);
			dist_3 = corner_verts[3].Dot(plane_bottom);
			if (dist_0 < 0.f && dist_1 < 0.f && dist_2 < 0.f && dist_3 < 0.f)
				return false;

			// Test against top plane
			dist_0 = corner_verts[0].Dot(plane_top);
			dist_1 = corner_verts[1].Dot(plane_top);
			dist_2 = corner_verts[2].Dot(plane_top);
			dist_3 = corner_verts[3].Dot(plane_top);
			if (dist_0 < 0.f && dist_1 < 0.f && dist_2 < 0.f && dist_3 < 0.f)
				return false;

			return true;
		}

		float WaterRenderer::estimateGridCoverage(const QuadNode& quad_node, Camera* pCamera, float screen_area)
		{
			// Estimate projected area

			// Test 16 points on the quad and find out the biggest one.
			const static float sample_pos[16][2] =
			{
				{ 0, 0 },
				{ 0, 1 },
				{ 1, 0 },
				{ 1, 1 },
				{ 0.5f, 0.333f },
				{ 0.25f, 0.667f },
				{ 0.75f, 0.111f },
				{ 0.125f, 0.444f },
				{ 0.625f, 0.778f },
				{ 0.375f, 0.222f },
				{ 0.875f, 0.556f },
				{ 0.0625f, 0.889f },
				{ 0.5625f, 0.037f },
				{ 0.3125f, 0.37f },
				{ 0.8125f, 0.704f },
				{ 0.1875f, 0.148f },
			};

			const Math::Matrix& matProj = pCamera->GetProjMatrix();
			const Math::Vector3& eye_point = pCamera->GetPosition();
			//eye_point = Vector3(eye_point.x, eye_point.z, eye_point.y);
			float grid_len_world = quad_node.length / m_nMeshDim;

			float max_area_proj = 0;
			for (int i = 0; i < 16; ++i)
			{
				Math::Vector3 test_point(quad_node.bottom_left.x + quad_node.length * sample_pos[i][0], quad_node.bottom_left.y + quad_node.length * sample_pos[i][1], 0);
				Math::Vector3 eye_vec = test_point - eye_point;
				float dist = eye_vec.Length();

				float area_world = grid_len_world * grid_len_world;// * abs(eye_point.z) / sqrt(nearest_sqr_dist);
				float area_proj = area_world * matProj.m[0][0] * matProj.m[1][1] / (dist * dist);

				if (max_area_proj < area_proj)
					max_area_proj = area_proj;
			}

			float pixel_coverage = max_area_proj * screen_area * 0.25f;

			return pixel_coverage;
		}

		int WaterRenderer::searchLeaf(const std::vector<QuadNode>& node_list, const Math::Vector2& point)
		{
			int index = -1;

			int size = (int)node_list.size();
			QuadNode node = node_list[size - 1];

			while (isLeaf(node) == false)
			{
				bool found = false;

				for (int i = 0; i < 4; i++)
				{
					index = node.sub_node[i];
					if (index == -1)
						continue;

					QuadNode sub_node = node_list[index];
					if (point.x >= sub_node.bottom_left.x && point.x <= sub_node.bottom_left.x + sub_node.length &&
						point.y >= sub_node.bottom_left.y && point.y <= sub_node.bottom_left.y + sub_node.length)
					{
						node = sub_node;
						found = true;
						break;
					}
				}

				if (!found)
					return -1;
			}

			return index;
		}

		QuadRenderParam& WaterRenderer::selectMeshPattern(const QuadNode& quad_node)
		{
			// Check 4 adjacent quad.
			Math::Vector2 point_left = quad_node.bottom_left + Math::Vector2(-m_fPatchLength * 0.5f, quad_node.length * 0.5f);
			int left_adj_index = searchLeaf(m_vecRenderNode, point_left);

			Math::Vector2 point_right = quad_node.bottom_left + Math::Vector2(quad_node.length + m_fPatchLength * 0.5f, quad_node.length * 0.5f);
			int right_adj_index = searchLeaf(m_vecRenderNode, point_right);

			Math::Vector2 point_bottom = quad_node.bottom_left + Math::Vector2(quad_node.length * 0.5f, -m_fPatchLength * 0.5f);
			int bottom_adj_index = searchLeaf(m_vecRenderNode, point_bottom);

			Math::Vector2 point_top = quad_node.bottom_left + Math::Vector2(quad_node.length * 0.5f, quad_node.length + m_fPatchLength * 0.5f);
			int top_adj_index = searchLeaf(m_vecRenderNode, point_top);

			int left_type = 0;
			if (left_adj_index != -1 && m_vecRenderNode[left_adj_index].length > quad_node.length * 0.999f)
			{
				QuadNode adj_node = m_vecRenderNode[left_adj_index];
				float scale = adj_node.length / quad_node.length * (m_nMeshDim >> quad_node.lod) / (m_nMeshDim >> adj_node.lod);
				if (scale > 3.999f)
					left_type = 2;
				else if (scale > 1.999f)
					left_type = 1;
			}

			int right_type = 0;
			if (right_adj_index != -1 && m_vecRenderNode[right_adj_index].length > quad_node.length * 0.999f)
			{
				QuadNode adj_node = m_vecRenderNode[right_adj_index];
				float scale = adj_node.length / quad_node.length * (m_nMeshDim >> quad_node.lod) / (m_nMeshDim >> adj_node.lod);
				if (scale > 3.999f)
					right_type = 2;
				else if (scale > 1.999f)
					right_type = 1;
			}

			int bottom_type = 0;
			if (bottom_adj_index != -1 && m_vecRenderNode[bottom_adj_index].length > quad_node.length * 0.999f)
			{
				QuadNode adj_node = m_vecRenderNode[bottom_adj_index];
				float scale = adj_node.length / quad_node.length * (m_nMeshDim >> quad_node.lod) / (m_nMeshDim >> adj_node.lod);
				if (scale > 3.999f)
					bottom_type = 2;
				else if (scale > 1.999f)
					bottom_type = 1;
			}

			int top_type = 0;
			if (top_adj_index != -1 && m_vecRenderNode[top_adj_index].length > quad_node.length * 0.999f)
			{
				QuadNode adj_node = m_vecRenderNode[top_adj_index];
				float scale = adj_node.length / quad_node.length * (m_nMeshDim >> quad_node.lod) / (m_nMeshDim >> adj_node.lod);
				if (scale > 3.999f)
					top_type = 2;
				else if (scale > 1.999f)
					top_type = 1;
			}

			// Check lookup table, [L][R][B][T]
			return m_mesh_patterns[quad_node.lod][left_type][right_type][bottom_type][top_type];
		}

		void WaterRenderer::createSurfaceMesh()
		{
			// --------------------------------- Vertex Buffer -------------------------------
			int num_verts = (m_nMeshDim + 1) * (m_nMeshDim + 1);
			std::vector<VertexPos> vertexCollector;
			vertexCollector.resize(num_verts);

			for (int i = 0; i <= m_nMeshDim; ++i)
			{
				for (int j = 0; j <= m_nMeshDim; ++j)
				{
					vertexCollector[i * (m_nMeshDim + 1) + j].pos.x = (float)j;
					vertexCollector[i * (m_nMeshDim + 1) + j].pos.y = (float)i;
				}
			}

			m_pVertexBuffer = IVertexBuffer::Create(VertexPos::Format(), vertexCollector.size(), &vertexCollector.front(), D3D11_USAGE_IMMUTABLE);
			if (m_pVertexBuffer == nullptr)
			{
				SafeDelete(m_pVertexBuffer);
			}

			// --------------------------------- Index Buffer -------------------------------
			// The index numbers for all mesh LODs (up to 256x256)
			const int index_size_lookup[] = { 0, 0, 4284, 18828, 69444, 254412, 956916, 3689820, 14464836 };

			m_nLods = 0;
			for (int i = m_nMeshDim; i > 1; i >>= 1)
			{
				++m_nLods;
			}

			// Generate patch meshes. Each patch contains two parts: the inner mesh which is a regular
			// grids in a triangle strip. The boundary mesh is constructed w.r.t. the edge degrees to
			// meet water-tight requirement.
			std::vector<uint32_t> indexCollector;
			indexCollector.resize(index_size_lookup[m_nLods]);

			int offset = 0;
			int level_size = m_nMeshDim;

			// Enumerate patterns
			for (int level = 0; level <= m_nLods - 2; level++)
			{
				int left_degree = level_size;

				for (int left_type = 0; left_type < 3; left_type++)
				{
					int right_degree = level_size;

					for (int right_type = 0; right_type < 3; right_type++)
					{
						int bottom_degree = level_size;

						for (int bottom_type = 0; bottom_type < 3; bottom_type++)
						{
							int top_degree = level_size;

							for (int top_type = 0; top_type < 3; top_type++)
							{
								QuadRenderParam* pattern = &m_mesh_patterns[level][left_type][right_type][bottom_type][top_type];

								// Inner mesh (triangle strip)
								Math::Rect inner_rect;
								inner_rect.left = (left_degree == level_size) ? 0 : 1;
								inner_rect.right = (right_degree == level_size) ? level_size : level_size - 1;
								inner_rect.bottom = (bottom_degree == level_size) ? 0 : 1;
								inner_rect.top = (top_degree == level_size) ? level_size : level_size - 1;

								int num_new_indices = generateInnerMesh(inner_rect, &indexCollector[offset]);

								pattern->inner_start_index = offset;
								pattern->num_inner_verts = (level_size + 1) * (level_size + 1);
								pattern->num_inner_faces = num_new_indices - 2;
								offset += num_new_indices;

								// Boundary mesh (triangle list)
								int l_degree = (left_degree == level_size) ? 0 : left_degree;
								int r_degree = (right_degree == level_size) ? 0 : right_degree;
								int b_degree = (bottom_degree == level_size) ? 0 : bottom_degree;
								int t_degree = (top_degree == level_size) ? 0 : top_degree;

								Math::Rect outer_rect = { 0, level_size, level_size, 0 };
								num_new_indices = generateBoundaryMesh(l_degree, r_degree, b_degree, t_degree, outer_rect, &indexCollector[offset]);

								pattern->boundary_start_index = offset;
								pattern->num_boundary_verts = (level_size + 1) * (level_size + 1);
								pattern->num_boundary_faces = num_new_indices / 3;
								offset += num_new_indices;

								top_degree /= 2;
							}
							bottom_degree /= 2;
						}
						right_degree /= 2;
					}
					left_degree /= 2;
				}
				level_size /= 2;
			}

			assert(offset == index_size_lookup[m_nLods]);

			m_pIndexBuffer = IIndexBuffer::Create(indexCollector.size(), &indexCollector.front(), D3D11_USAGE_IMMUTABLE);
		}

		void WaterRenderer::createFresnelMap()
		{
			DWORD* buffer = new DWORD[FRESNEL_TEX_SIZE];
			for (int i = 0; i < FRESNEL_TEX_SIZE; i++)
			{
				float cos_a = i / (FLOAT)FRESNEL_TEX_SIZE;
				// Using water's refraction index 1.33
				
				Math::Vector3 f3Fresnel(Math::Vector3::FresnelTerm(Math::Vector3(cos_a, 0.f, 0.f), Math::Vector3(1.33f, 0.f, 0.f)) * 255);
				DWORD fresnel = (DWORD)f3Fresnel.x;


				DWORD sky_blend = (DWORD)(powf(1 / (1 + cos_a), m_fSkyBlending) * 255);

				buffer[i] = (sky_blend << 8) | fresnel;
			}

			D3D11_TEXTURE1D_DESC tex_desc;
			tex_desc.Width = FRESNEL_TEX_SIZE;
			tex_desc.MipLevels = 1;
			tex_desc.ArraySize = 1;
			tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
			tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			tex_desc.CPUAccessFlags = 0;
			tex_desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA init_data;
			init_data.pSysMem = buffer;
			init_data.SysMemPitch = 0;
			init_data.SysMemSlicePitch = 0;

			m_pFresnelMap = ITexture::Create(StrID::WaterFresnelMap, tex_desc, &init_data);

			SafeDeleteArray(buffer);
		}

		void WaterRenderer::loadTextures()
		{
			std::string strPerlinPath = File::GetPath(File::eTexture);
			strPerlinPath.append("perlin_noise.dds");
			m_pSRV_Perlin = ITexture::Create("perlin_noise.dds", strPerlinPath.c_str(), false);

			std::string strReflectCubePath = File::GetPath(File::eTexture);
			strReflectCubePath.append("reflect_cube.dds");
			m_pSRV_ReflectCube = ITexture::Create("reflect_cube.dds", strReflectCubePath.c_str(), false);
		}

#define MESH_INDEX_2D(x, y)	(((y) + vert_rect.bottom) * (m_nMeshDim + 1) + (x) + vert_rect.left)
		int WaterRenderer::generateBoundaryMesh(int left_degree, int right_degree, int bottom_degree, int top_degree,
			const Math::Rect& vert_rect, uint32_t* output)
		{
			// Triangle list for bottom boundary
			int i, j;
			int counter = 0;
			int width = vert_rect.right - vert_rect.left;

			if (bottom_degree > 0)
			{
				int b_step = width / bottom_degree;

				for (i = 0; i < width; i += b_step)
				{
					output[counter++] = MESH_INDEX_2D(i, 0);
					output[counter++] = MESH_INDEX_2D(i + b_step / 2, 1);
					output[counter++] = MESH_INDEX_2D(i + b_step, 0);

					for (j = 0; j < b_step / 2; j++)
					{
						if (i == 0 && j == 0 && left_degree > 0)
							continue;

						output[counter++] = MESH_INDEX_2D(i, 0);
						output[counter++] = MESH_INDEX_2D(i + j, 1);
						output[counter++] = MESH_INDEX_2D(i + j + 1, 1);
					}

					for (j = b_step / 2; j < b_step; j++)
					{
						if (i == width - b_step && j == b_step - 1 && right_degree > 0)
							continue;

						output[counter++] = MESH_INDEX_2D(i + b_step, 0);
						output[counter++] = MESH_INDEX_2D(i + j, 1);
						output[counter++] = MESH_INDEX_2D(i + j + 1, 1);
					}
				}
			}

			// Right boundary
			int height = vert_rect.top - vert_rect.bottom;

			if (right_degree > 0)
			{
				int r_step = height / right_degree;

				for (i = 0; i < height; i += r_step)
				{
					output[counter++] = MESH_INDEX_2D(width, i);
					output[counter++] = MESH_INDEX_2D(width - 1, i + r_step / 2);
					output[counter++] = MESH_INDEX_2D(width, i + r_step);

					for (j = 0; j < r_step / 2; j++)
					{
						if (i == 0 && j == 0 && bottom_degree > 0)
							continue;

						output[counter++] = MESH_INDEX_2D(width, i);
						output[counter++] = MESH_INDEX_2D(width - 1, i + j);
						output[counter++] = MESH_INDEX_2D(width - 1, i + j + 1);
					}

					for (j = r_step / 2; j < r_step; j++)
					{
						if (i == height - r_step && j == r_step - 1 && top_degree > 0)
							continue;

						output[counter++] = MESH_INDEX_2D(width, i + r_step);
						output[counter++] = MESH_INDEX_2D(width - 1, i + j);
						output[counter++] = MESH_INDEX_2D(width - 1, i + j + 1);
					}
				}
			}

			// Top boundary
			if (top_degree > 0)
			{
				int t_step = width / top_degree;

				for (i = 0; i < width; i += t_step)
				{
					output[counter++] = MESH_INDEX_2D(i, height);
					output[counter++] = MESH_INDEX_2D(i + t_step / 2, height - 1);
					output[counter++] = MESH_INDEX_2D(i + t_step, height);

					for (j = 0; j < t_step / 2; j++)
					{
						if (i == 0 && j == 0 && left_degree > 0)
							continue;

						output[counter++] = MESH_INDEX_2D(i, height);
						output[counter++] = MESH_INDEX_2D(i + j, height - 1);
						output[counter++] = MESH_INDEX_2D(i + j + 1, height - 1);
					}

					for (j = t_step / 2; j < t_step; j++)
					{
						if (i == width - t_step && j == t_step - 1 && right_degree > 0)
							continue;

						output[counter++] = MESH_INDEX_2D(i + t_step, height);
						output[counter++] = MESH_INDEX_2D(i + j, height - 1);
						output[counter++] = MESH_INDEX_2D(i + j + 1, height - 1);
					}
				}
			}

			// Left boundary
			if (left_degree > 0)
			{
				int l_step = height / left_degree;

				for (i = 0; i < height; i += l_step)
				{
					output[counter++] = MESH_INDEX_2D(0, i);
					output[counter++] = MESH_INDEX_2D(1, i + l_step / 2);
					output[counter++] = MESH_INDEX_2D(0, i + l_step);

					for (j = 0; j < l_step / 2; j++)
					{
						if (i == 0 && j == 0 && bottom_degree > 0)
							continue;

						output[counter++] = MESH_INDEX_2D(0, i);
						output[counter++] = MESH_INDEX_2D(1, i + j);
						output[counter++] = MESH_INDEX_2D(1, i + j + 1);
					}

					for (j = l_step / 2; j < l_step; j++)
					{
						if (i == height - l_step && j == l_step - 1 && top_degree > 0)
							continue;

						output[counter++] = MESH_INDEX_2D(0, i + l_step);
						output[counter++] = MESH_INDEX_2D(1, i + j);
						output[counter++] = MESH_INDEX_2D(1, i + j + 1);
					}
				}
			}

			return counter;
		}

		int WaterRenderer::generateInnerMesh(const Math::Rect& vert_rect, uint32_t* output)
		{
			int counter = 0;
			int width = vert_rect.right - vert_rect.left;
			int height = vert_rect.top - vert_rect.bottom;

			bool reverse = false;
			for (int i = 0; i < height; ++i)
			{
				if (reverse == false)
				{
					output[counter++] = MESH_INDEX_2D(0, i);
					output[counter++] = MESH_INDEX_2D(0, i + 1);
					for (int j = 0; j < width; ++j)
					{
						output[counter++] = MESH_INDEX_2D(j + 1, i);
						output[counter++] = MESH_INDEX_2D(j + 1, i + 1);
					}
				}
				else
				{
					output[counter++] = MESH_INDEX_2D(width, i);
					output[counter++] = MESH_INDEX_2D(width, i + 1);
					for (int j = width - 1; j >= 0; --j)
					{
						output[counter++] = MESH_INDEX_2D(j, i);
						output[counter++] = MESH_INDEX_2D(j, i + 1);
					}
				}

				reverse = !reverse;
			}

			return counter;
		}
	}
}