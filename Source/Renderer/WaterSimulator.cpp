#include "stdafx.h"
#include "WaterSimulator.h"

#include "CommonLib/FileUtil.h"

#define HALF_SQRT_2	0.7071068f
#define GRAV_ACCEL	981.0f	// The acceleration of gravity, cm/s^2

#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

namespace StrID
{
	RegisterStringID(EffectWaterSimulator);
	RegisterStringID(WaterSimulator_SpectrumCS);
	RegisterStringID(WaterSimulator_Displacement);
	RegisterStringID(WaterSimulator_Gradient);

	RegisterStringID(g_ActualDim);
	RegisterStringID(g_InWidth);
	RegisterStringID(g_OutWidth);
	RegisterStringID(g_OutHeight);
	RegisterStringID(g_DxAddressOffset);
	RegisterStringID(g_DyAddressOffset);
	RegisterStringID(g_Time);
	RegisterStringID(g_ChoppyScale);
	RegisterStringID(g_GridLen);
	RegisterStringID(g_InputH0);
	RegisterStringID(g_InputOmega);
	RegisterStringID(g_OutputHt);
	RegisterStringID(g_InputDxyz);
	RegisterStringID(g_texDisplacementMap);
	RegisterStringID(LinearSampler);

	RegisterStringID(WaterFFT_CS);
	RegisterStringID(WaterFFT_CS2);

	RegisterStringID(thread_count);
	RegisterStringID(ostride);
	RegisterStringID(istride);
	RegisterStringID(pstride);
	RegisterStringID(phase_base);
	RegisterStringID(g_SrcData);
	RegisterStringID(g_DstData);
}

namespace EastEngine
{
	// Generating gaussian random number with mean 0 and standard deviation 1.
	float Gauss()
	{
		float u1 = Math::Random<float>() / (float)RAND_MAX;
		float u2 = Math::Random<float>() / (float)RAND_MAX;
		if (u1 < 1e-6f)
			u1 = 1e-6f;
		return sqrtf(-2 * logf(u1)) * cosf(2 * Math::PI * u2);
	}

	// Phillips Spectrum
	// K: normalized wave vector, W: wind direction, v: wind velocity, a: amplitude constant
	float Phillips(const Math::Vector2& K, const Math::Vector2& W, float v, float a, float dir_depend)
	{
		// largest possible wave from constant wind of velocity v
		float l = v * v / GRAV_ACCEL;
		// damp out waves with very small length w << l
		float w = l / 1000;

		float Ksqr = K.x * K.x + K.y * K.y;
		float Kcos = K.x * W.x + K.y * W.y;
		float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

		// filter out waves moving opposite to wind
		if (Kcos < 0)
			phillips *= dir_depend;

		// damp out waves with very small length w << l
		return phillips * expf(-Ksqr * w * w);
	}

	namespace Graphics
	{
#define COHERENCY_GRANULARITY 128

		////////////////////////////////////////////////////////////////////////////////
		// Common constants
		////////////////////////////////////////////////////////////////////////////////
#define TWO_PI 6.283185307179586476925286766559

#define FFT_DIMENSIONS 3U
#define FFT_PLAN_SIZE_LIMIT (1U << 27)

#define FFT_FORWARD -1
#define FFT_INVERSE 1

		IRenderTarget* createTextureAndViews(uint32_t width, uint32_t height, DXGI_FORMAT format)
		{
			// Create 2D texture
			RenderTargetDesc2D tex_desc;
			tex_desc.Width = width;
			tex_desc.Height = height;
			tex_desc.MipLevels = 1;
			tex_desc.ArraySize = 1;
			tex_desc.Format = format;
			tex_desc.SampleDesc.Count = 1;
			tex_desc.SampleDesc.Quality = 0;
			tex_desc.Usage = D3D11_USAGE_DEFAULT;
			tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			tex_desc.CPUAccessFlags = 0;
			tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
			tex_desc.Build();

			return IRenderTarget::Create(tex_desc);
		}

		WaterSimulator::WaterSimulator()
			: m_pDisplacementMap(nullptr)
			, m_pGradientMap(nullptr)
			, m_pBuffer_Float2_H0(nullptr)
			, m_pBuffer_Float_Omega(nullptr)
			, m_pBuffer_Float2_Ht(nullptr)
			, m_pBuffer_Float_Dxyz(nullptr)
			, m_pQuadVB(nullptr)
		{
		}

		WaterSimulator::~WaterSimulator()
		{
			Release();
		}

		bool WaterSimulator::Init(OceanParameter& params)
		{
			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Water\\WaterSimulator_D.cso");
#else
			strPath.append("Water\\WaterSimulator.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectWaterSimulator, strPath.c_str());
			m_pEffect->CreateTechnique(StrID::WaterSimulator_SpectrumCS, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::WaterSimulator_Displacement, EmVertexFormat::ePos);
			m_pEffect->CreateTechnique(StrID::WaterSimulator_Gradient, EmVertexFormat::ePos);

			strPath = File::GetPath(File::EmPath::eFx);

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Water\\FFT_D.cso");
#else
			strPath.append("Water\\FFT.cso");
#endif

			m_pEffect_FFT = IEffect::Create(StrID::EffectWaterSimulator, strPath.c_str());
			m_pEffect_FFT->CreateTechnique(StrID::WaterFFT_CS, EmVertexFormat::eUnknown);
			m_pEffect_FFT->CreateTechnique(StrID::WaterFFT_CS2, EmVertexFormat::eUnknown);

			// Quad vertex buffer
			D3D11_BUFFER_DESC vb_desc;
			vb_desc.ByteWidth = 4 * sizeof(Math::Vector4);
			vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
			vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vb_desc.CPUAccessFlags = 0;
			vb_desc.MiscFlags = 0;

			float quad_verts[] =
			{
				-1.f, -1.f, 0.f, 1.f,
				-1.f,  1.f, 0.f, 1.f,
				1.f, -1.f, 0.f, 1.f,
				1.f,  1.f, 0.f, 1.f,
			};

			D3D11_SUBRESOURCE_DATA init_data;
			init_data.pSysMem = &quad_verts[0];
			init_data.SysMemPitch = 0;
			init_data.SysMemSlicePitch = 0;

			if (FAILED(GetDevice()->CreateBuffer(&vb_desc, &init_data, &m_pQuadVB)))
			{
				Release();
				return false;
			}

			// Height map H(0)
			int height_map_size = (params.dmap_dim + 4) * (params.dmap_dim + 1);
			Math::Vector2* h0_data = new Math::Vector2[height_map_size * sizeof(Math::Vector2)];
			float* omega_data = new float[height_map_size * sizeof(float)];
			InitHeightMap(params, h0_data, omega_data);

			m_param = params;
			int hmap_dim = params.dmap_dim;
			int input_full_size = (hmap_dim + 4) * (hmap_dim + 1);
			// This value should be (hmap_dim / 2 + 1) * hmap_dim, but we use full sized buffer here for simplicity.
			int input_half_size = hmap_dim * hmap_dim;
			int output_size = hmap_dim * hmap_dim;

			// For filling the buffer with zeroes.
			char* zero_data = new char[3 * output_size * sizeof(float) * 2];
			memset(zero_data, 0, 3 * output_size * sizeof(float) * 2);

			// RW buffer allocations
			// H0
			UINT float2_stride = 2 * sizeof(float);
			m_pBuffer_Float2_H0 = IStructuredBuffer::Create(h0_data, input_full_size * float2_stride, float2_stride);

			// Notice: The following 3 buffers should be half sized buffer because of conjugate symmetric input. But
			// we use full sized buffers due to the CS4.0 restriction.

			// Put H(t), Dx(t) and Dy(t) into one buffer because CS4.0 allows only 1 UAV at a time
			m_pBuffer_Float2_Ht = IStructuredBuffer::Create(zero_data, 3 * input_half_size * float2_stride, float2_stride);

			// omega
			m_pBuffer_Float_Omega = IStructuredBuffer::Create(omega_data, input_full_size * sizeof(float), sizeof(float));

			// Notice: The following 3 should be real number data. But here we use the complex numbers and C2C FFT
			// due to the CS4.0 restriction.
			// Put Dz, Dx and Dy into one buffer because CS4.0 allows only 1 UAV at a time
			m_pBuffer_Float_Dxyz = IStructuredBuffer::Create(zero_data, 3 * output_size * float2_stride, float2_stride);

			SafeDeleteArray(zero_data);
			SafeDeleteArray(h0_data);
			SafeDeleteArray(omega_data);

			// D3D11 Textures
			m_pDisplacementMap = createTextureAndViews(hmap_dim, hmap_dim, DXGI_FORMAT_R32G32B32A32_FLOAT);
			m_pGradientMap = createTextureAndViews(hmap_dim, hmap_dim, DXGI_FORMAT_R16G16B16A16_FLOAT);

			// FFT
			if (fft512x512_create_plan(&m_fft_plan, 3) == false)
			{
				Release();
				return false;
			}

			// Samplers
			SamplerStateDesc samplerDesc;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.f;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			samplerDesc.BorderColor[0] = 1.f;
			samplerDesc.BorderColor[1] = 1.f;
			samplerDesc.BorderColor[2] = 1.f;
			samplerDesc.BorderColor[3] = 1.f;
			samplerDesc.MinLOD = -FLT_MAX;
			samplerDesc.MaxLOD = FLT_MAX;

			m_pSampler = ISamplerState::Create(samplerDesc);
			if (m_pSampler == nullptr)
			{
				Release();
				return false;
			}

#ifdef CS_DEBUG_BUFFER
			D3D11_BUFFER_DESC buf_desc;
			buf_desc.ByteWidth = 3 * input_half_size * float2_stride;
			buf_desc.Usage = D3D11_USAGE_STAGING;
			buf_desc.BindFlags = 0;
			buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			buf_desc.StructureByteStride = float2_stride;

			m_pd3dDevice->CreateBuffer(&buf_desc, nullptr, &m_pDebugBuffer);
			assert(m_pDebugBuffer);
#endif

			return true;
		}

		void WaterSimulator::Release()
		{
			SafeDelete(m_pDisplacementMap);
			SafeDelete(m_pGradientMap);

			SafeDelete(m_pBuffer_Float2_H0);
			SafeDelete(m_pBuffer_Float_Omega);
			SafeDelete(m_pBuffer_Float2_Ht);
			SafeDelete(m_pBuffer_Float_Dxyz);

			SafeRelease(m_pQuadVB);

			fft512x512_destroy_plan(&m_fft_plan);
		}

		void WaterSimulator::UpdateDisplacementMap(float fTime)
		{
			// ---------------------------- H(0) -> H(t), D(x, t), D(y, t) --------------------------------
			// Compute shader
			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::WaterSimulator_SpectrumCS);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !! : %s", StrID::WaterSimulator_SpectrumCS.c_str());
				return;
			}

			IDeviceContext* pDeviceContext = GetDeviceContext();

			// Buffers
			m_pEffect->SetStructuredBuffer(StrID::g_InputH0, m_pBuffer_Float2_H0);
			m_pEffect->SetStructuredBuffer(StrID::g_InputOmega, m_pBuffer_Float_Omega);

			m_pEffect->SetUnorderedAccessView(StrID::g_OutputHt, m_pBuffer_Float2_Ht);

			// Consts
			m_pEffect->SetFloat(StrID::g_Time, fTime * m_param.time_scale);
			m_pEffect->SetFloat(StrID::g_ChoppyScale, m_param.choppy_scale);
			m_pEffect->SetFloat(StrID::g_GridLen, m_param.dmap_dim / m_param.patch_length);

			int actual_dim = m_param.dmap_dim;
			int input_width = actual_dim + 4;
			// We use full sized data here. The value "output_width" should be actual_dim/2+1 though.
			int output_width = actual_dim;
			int output_height = actual_dim;
			int dtx_offset = actual_dim * actual_dim;
			int dty_offset = actual_dim * actual_dim * 2;

			m_pEffect->SetInt(StrID::g_ActualDim, actual_dim);
			m_pEffect->SetInt(StrID::g_InWidth, input_width);
			m_pEffect->SetInt(StrID::g_OutWidth, output_width);
			m_pEffect->SetInt(StrID::g_OutHeight, output_height);
			m_pEffect->SetInt(StrID::g_DxAddressOffset, dtx_offset);
			m_pEffect->SetInt(StrID::g_DyAddressOffset, dty_offset);

			// Run the CS
			uint32_t group_count_x = (m_param.dmap_dim + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
			uint32_t group_count_y = (m_param.dmap_dim + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;

			uint32_t nPassCount = pEffectTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pEffectTech->PassApply(p, pDeviceContext);

				// Execute
				pDeviceContext->Dispatch(group_count_x, group_count_y, 1);
			}

			ClearEffect(pDeviceContext, pEffectTech);

			// ------------------------------------ Perform FFT -------------------------------------------
			fft_512x512_c2c(&m_fft_plan, m_pBuffer_Float_Dxyz, m_pBuffer_Float_Dxyz, m_pBuffer_Float2_Ht);

			// --------------------------------- Wrap Dx, Dy and Dz ---------------------------------------
			// Push RT
			ID3D11RenderTargetView* old_target = nullptr;
			ID3D11DepthStencilView* old_depth = nullptr;
			pDeviceContext->GetInterface()->OMGetRenderTargets(1, &old_target, &old_depth);

			D3D11_VIEWPORT old_viewport;
			uint32_t num_viewport = 1;
			pDeviceContext->GetInterface()->RSGetViewports(&num_viewport, &old_viewport);

			D3D11_VIEWPORT new_vp = { 0, 0, (float)m_param.dmap_dim, (float)m_param.dmap_dim, 0.0f, 1.0f };
			pDeviceContext->GetInterface()->RSSetViewports(1, &new_vp);

			// Set RT
			pDeviceContext->GetInterface()->OMSetRenderTargets(1, m_pDisplacementMap->GetRenderTargetViewPtr(), nullptr);

			// VS & PS
			pEffectTech = m_pEffect->GetTechnique(StrID::WaterSimulator_Displacement);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			// Constants
			m_pEffect->SetFloat(StrID::g_Time, fTime * m_param.time_scale);
			m_pEffect->SetFloat(StrID::g_ChoppyScale, m_param.choppy_scale);
			m_pEffect->SetFloat(StrID::g_GridLen, m_param.dmap_dim / m_param.patch_length);

			m_pEffect->SetInt(StrID::g_ActualDim, actual_dim);
			m_pEffect->SetInt(StrID::g_InWidth, input_width);
			m_pEffect->SetInt(StrID::g_OutWidth, output_width);
			m_pEffect->SetInt(StrID::g_OutHeight, output_height);
			m_pEffect->SetInt(StrID::g_DxAddressOffset, dtx_offset);
			m_pEffect->SetInt(StrID::g_DyAddressOffset, dty_offset);

			// Buffer resources
			m_pEffect->SetStructuredBuffer(StrID::g_InputDxyz, m_pBuffer_Float_Dxyz);

			// IA setup
			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			UINT strides[1] = { sizeof(Math::Vector4) };
			UINT offsets[1] = { 0 };
			pDeviceContext->GetInterface()->IASetVertexBuffers(0, 1, &m_pQuadVB, &strides[0], &offsets[0]);

			pDeviceContext->GetInterface()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			nPassCount = pEffectTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pEffectTech->PassApply(p, pDeviceContext);

				// Perform draw call
				pDeviceContext->Draw(4, 0);
			}

			ClearEffect(pDeviceContext, pEffectTech);

			// ----------------------------------- Generate Normal ----------------------------------------
			// Set RT
			pDeviceContext->GetInterface()->OMSetRenderTargets(1, m_pGradientMap->GetRenderTargetViewPtr(), nullptr);

			// VS & PS
			pEffectTech = m_pEffect->GetTechnique(StrID::WaterSimulator_Gradient);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			// Texture resource and sampler
			m_pEffect->SetTexture(StrID::g_texDisplacementMap, m_pGradientMap->GetTexture());
			m_pEffect->SetSamplerState(StrID::LinearSampler, m_pSampler, 0);

			nPassCount = pEffectTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pEffectTech->PassApply(p, pDeviceContext);

				// Perform draw call
				pDeviceContext->Draw(4, 0);
			}

			ClearEffect(pDeviceContext, pEffectTech);

			// Pop RT
			pDeviceContext->GetInterface()->RSSetViewports(1, &old_viewport);
			pDeviceContext->GetInterface()->OMSetRenderTargets(1, &old_target, old_depth);
			SafeRelease(old_target);
			SafeRelease(old_depth);

			pDeviceContext->GenerateMips(m_pGradientMap->GetTexture()->GetShaderResourceView());

			// Define CS_DEBUG_BUFFER to enable writing a buffer into a file.
#ifdef CS_DEBUG_BUFFER
			{
				m_pDeviceContext->CopyResource(m_pDebugBuffer, m_pBuffer_Float_Dxyz);
				D3D11_MAPPED_SUBRESOURCE mapped_res;
				m_pDeviceContext->Map(m_pDebugBuffer, 0, D3D11_MAP_READ, 0, &mapped_res);

				// set a break point below, and drag MappedResource.pData into in your Watch window
				// and cast it as (float*)

				// Write to disk
				D3DXVECTOR2* v = (D3DXVECTOR2*)mapped_res.pData;

				FILE* fp = fopen(".\\tmp\\Ht_raw.dat", "wb");
				fwrite(v, 512 * 512 * sizeof(float) * 2 * 3, 1, fp);
				fclose(fp);

				m_pDeviceContext->Unmap(m_pDebugBuffer, 0);
			}
#endif
		}

		const std::shared_ptr<ITexture>& WaterSimulator::GetTextureDisplacementMap()
		{
			return m_pDisplacementMap->GetTexture();
		}

		const std::shared_ptr<ITexture>& WaterSimulator::GetTextureGradientMap()
		{
			return m_pGradientMap->GetTexture();
		}

		void WaterSimulator::InitHeightMap(OceanParameter& params, Math::Vector2* out_h0, float* out_omega)
		{
			Math::Vector2 K, Kn;

			Math::Vector2 wind_dir;
			params.wind_dir.Normalize(wind_dir);

			float a = params.wave_amplitude * 1e-7f;	// It is too small. We must scale it for editing.
			float v = params.wind_speed;
			float dir_depend = params.wind_dependency;

			int height_map_dim = params.dmap_dim;
			float patch_length = params.patch_length;

			// initialize random generator.
			for (int i = 0; i <= height_map_dim; ++i)
			{
				// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
				K.y = (-height_map_dim / 2.0f + i) * (2 * Math::PI / patch_length);

				for (int j = 0; j <= height_map_dim; ++j)
				{
					K.x = (-height_map_dim / 2.0f + j) * (2 * Math::PI / patch_length);

					float phil = (K.x == 0 && K.y == 0) ? 0 : sqrtf(Phillips(K, wind_dir, v, a, dir_depend));

					out_h0[i * (height_map_dim + 4) + j].x = float(phil * Gauss() * HALF_SQRT_2);
					out_h0[i * (height_map_dim + 4) + j].y = float(phil * Gauss() * HALF_SQRT_2);

					// The angular frequency is following the dispersion relation:
					//            out_omega^2 = g*k
					// The equation of Gerstner wave:
					//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
					//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
					// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
					// motion with the center (x0, y0, z0), radius A, and the circular plane is parallel to
					// vector K.
					out_omega[i * (height_map_dim + 4) + j] = sqrtf(GRAV_ACCEL * sqrtf(K.x * K.x + K.y * K.y));
				}
			}
		}

		void WaterSimulator::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech)
		{
			m_pEffect->SetStructuredBuffer(StrID::g_InputH0, nullptr);
			m_pEffect->SetStructuredBuffer(StrID::g_InputOmega, nullptr);
			m_pEffect->SetUnorderedAccessView(StrID::g_OutputHt, nullptr);
			m_pEffect->SetStructuredBuffer(StrID::g_InputDxyz, nullptr);
			m_pEffect->SetTexture(StrID::g_texDisplacementMap, nullptr);
			m_pEffect->UndoSamplerState(StrID::LinearSampler, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pEffectTech);
		}

		void WaterSimulator::ClearEffect_FFT(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech)
		{
			m_pEffect_FFT->SetStructuredBuffer(StrID::g_SrcData, nullptr);
			m_pEffect_FFT->SetUnorderedAccessView(StrID::g_DstData, nullptr);

			m_pEffect_FFT->ClearState(pd3dDeviceContext, pEffectTech);
		}

		void WaterSimulator::radix008A(IStructuredBuffer* pUAV_Dst,
			IStructuredBuffer* pSRV_Src,
			CB_Structure& structure,
			uint32_t thread_count,
			uint32_t istride)
		{
			D3D_PROFILING(WaterFFT);
			IEffectTech* pEffectTech = m_pEffect_FFT->GetTechnique(istride > 1 ? StrID::WaterFFT_CS : StrID::WaterFFT_CS2);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			IDeviceContext* pDeviceContext = GetDeviceContext();

			// Setup execution configuration
			uint32_t grid = thread_count / COHERENCY_GRANULARITY;

			m_pEffect_FFT->SetInt(StrID::thread_count, structure.thread_count);
			m_pEffect_FFT->SetInt(StrID::ostride, structure.ostride);
			m_pEffect_FFT->SetInt(StrID::istride, structure.istride);
			m_pEffect_FFT->SetInt(StrID::pstride, structure.pstride);
			m_pEffect_FFT->SetFloat(StrID::phase_base, structure.phase_base);

			// Buffers
			m_pEffect_FFT->SetStructuredBuffer(StrID::g_SrcData, pSRV_Src);
			m_pEffect_FFT->SetUnorderedAccessView(StrID::g_DstData, pUAV_Dst);

			uint32_t nPassCount = pEffectTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pEffectTech->PassApply(p, pDeviceContext);

				// Execute
				pDeviceContext->Dispatch(grid, 1, 1);
			}

			ClearEffect_FFT(pDeviceContext, pEffectTech);
		}

		void WaterSimulator::fft_512x512_c2c(CSFFT512x512_Plan* fft_plan,
			IStructuredBuffer* pUAV_Dst,
			IStructuredBuffer* pSRV_Dst,
			IStructuredBuffer* pSRV_Src)
		{
			const uint32_t thread_count = fft_plan->slices * (512 * 512) / 8;

			uint32_t istride = 512 * 512 / 8;
			radix008A(fft_plan->pBuffer_Tmp, pSRV_Src, fft_plan->radix008A_CB[0], thread_count, istride);

			istride /= 8;
			radix008A(pUAV_Dst, fft_plan->pBuffer_Tmp, fft_plan->radix008A_CB[1], thread_count, istride);

			istride /= 8;
			radix008A(fft_plan->pBuffer_Tmp, pSRV_Dst, fft_plan->radix008A_CB[2], thread_count, istride);

			istride /= 8;
			radix008A(pUAV_Dst, fft_plan->pBuffer_Tmp, fft_plan->radix008A_CB[3], thread_count, istride);

			istride /= 8;
			radix008A(fft_plan->pBuffer_Tmp, pSRV_Dst, fft_plan->radix008A_CB[4], thread_count, istride);

			istride /= 8;
			radix008A(pUAV_Dst, fft_plan->pBuffer_Tmp, fft_plan->radix008A_CB[5], thread_count, istride);
		}

		void create_cbuffers_512x512(CSFFT512x512_Plan* plan, uint32_t slices)
		{
			// Create 6 cbuffers for 512x512 transform.

			// Buffer 0
			const uint32_t thread_count = slices * (512 * 512) / 8;
			uint32_t ostride = 512 * 512 / 8;
			uint32_t istride = ostride;
			double phase_base = -TWO_PI / (512.0 * 512.0);

			plan->radix008A_CB[0].Set(thread_count, ostride, istride, 512, (float)(phase_base));

			// Buffer 1
			istride /= 8;
			phase_base *= 8.0;

			plan->radix008A_CB[1].Set(thread_count, ostride, istride, 512, (float)(phase_base));

			// Buffer 2
			istride /= 8;
			phase_base *= 8.0;

			plan->radix008A_CB[2].Set(thread_count, ostride, istride, 512, (float)(phase_base));

			// Buffer 3
			istride /= 8;
			phase_base *= 8.0;
			ostride /= 512;

			plan->radix008A_CB[3].Set(thread_count, ostride, istride, 1, (float)(phase_base));

			// Buffer 4
			istride /= 8;
			phase_base *= 8.0;

			plan->radix008A_CB[4].Set(thread_count, ostride, istride, 1, (float)(phase_base));

			// Buffer 5
			istride /= 8;
			phase_base *= 8.0;

			plan->radix008A_CB[5].Set(thread_count, ostride, istride, 1, (float)(phase_base));
		}

		bool WaterSimulator::fft512x512_create_plan(CSFFT512x512_Plan* plan, uint32_t slices)
		{
			plan->slices = slices;

			// Constants
			// Create 6 cbuffers for 512x512 transform
			create_cbuffers_512x512(plan, slices);

			plan->pBuffer_Tmp = IStructuredBuffer::Create(nullptr, sizeof(float) * 2 * (512 * slices) * 512, sizeof(float) * 2);
			if (plan->pBuffer_Tmp == nullptr)
			{
				fft512x512_destroy_plan(plan);
				return false;
			}

			return true;
		}

		void WaterSimulator::fft512x512_destroy_plan(CSFFT512x512_Plan* plan)
		{
			SafeDelete(plan->pBuffer_Tmp);
		}
	}
}