#include "stdafx.h"
#include "SSRDX11.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Timer.h"

#include "Graphics/Interface/Camera.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "GBufferDX11.h"
#include "RenderTargetDX11.h"
#include "DepthStencilDX11.h"
#include "TextureDX11.h"

namespace sid
{
	RegisterStringID(est_SSR_Noise);
}

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct SSRContents
				{
					math::Matrix matInvView;
					math::Matrix matInvProj;

					math::Matrix matView;
					math::Matrix matProj;

					math::float2 resolution;
					int sampleCount{ 16 };
					float padding{ 0.f };
				};

				enum CBSlot
				{
					eCB_SSRContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_LinearClamp = 0,
					eSampler_PointClamp = 1,
				};

				enum SRVSlot
				{
					eSRV_Color = 0,
					eSRV_Normal = 1,
					eSRV_DisneyBRDF = 2,
					eSRV_Depth = 3,
					eSRV_Noise = 4,

					eSRV_SSR = 1,
				};

				enum PSType
				{
					ePS_SSR = 0,
					ePS_SSR_Color,

					ePS_Count,
				};

				const char* GetSSRPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case ePS_SSR:
						return "PS_SSR";
					case ePS_SSR_Color:
						return "PS_SSR_Color";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetSSRContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<SSRContents>* pCB_SSRContents,
					const math::Matrix& matInvView, const math::Matrix& matInvProj,
					const math::Matrix& matView, const math::Matrix& matProj,
					const math::float2& resolution, int sampleCount)
				{
					SSRContents* pSSRContents = pCB_SSRContents->Map(pDeviceContext);

					pSSRContents->matInvView = matInvView.Transpose();
					pSSRContents->matInvProj = matInvProj.Transpose();
					pSSRContents->matView = matView.Transpose();
					pSSRContents->matProj = matProj.Transpose();
					pSSRContents->resolution = resolution;
					pSSRContents->sampleCount = sampleCount;
					pSSRContents->padding = 0.f;

					pCB_SSRContents->Unmap(pDeviceContext);
				}
			}

			class SSR::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void SetRenderState(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext);
				void Apply(Camera* pCamera, const RenderTarget* pSource, const GBuffer* pGBuffer, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShaders{ nullptr };

				ConstantBuffer<shader::SSRContents> m_SSRContents;

				std::unique_ptr<Texture> m_pNoiseTexture;
			};

			SSR::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\SSR\\SSR.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : SSR.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "VS", shader::VS_CompileVersion, &m_pVertexShader, "VS_SSR") == false)
				{
					throw_line("failed to create SSR_VS");
				}

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					shader::PSType emPSType = static_cast<shader::PSType>(i);

					const char* psName = shader::GetSSRPSTypeToString(emPSType);
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), psName, shader::PS_CompileVersion, &m_pPixelShaders[i], psName) == false)
					{
						std::string dump = string::Format("failed to create %s", psName);
						throw_line(dump.c_str());
					}
				}

				SafeRelease(pShaderBlob);

				m_SSRContents.Create(pDevice, "SSRContents");

				std::vector<math::float4> noises(64 * 64);
				for (int i = 0; i < 64 * 64; ++i)
				{
					noises[i].x = math::RandomReal(0.0f, 1.f);
					noises[i].y = math::RandomReal(0.0f, 1.f);
					noises[i].z = math::RandomReal(0.0f, 1.f);
					noises[i].w = math::RandomReal(0.0f, 1.f);
					noises[i].Normalize();
				}

				D3D11_TEXTURE2D_DESC desc{};
				desc.MipLevels = 1;
				desc.Usage = D3D11_USAGE_IMMUTABLE;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = 0;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.ArraySize = 1;
				desc.Width = 64;
				desc.Height = 64;
				desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

				D3D11_SUBRESOURCE_DATA initialData;
				initialData.pSysMem = noises.data();
				initialData.SysMemPitch = 64 * sizeof(math::float4);
				initialData.SysMemSlicePitch = 1;

				ITexture::Key key(sid::est_SSR_Noise);
				m_pNoiseTexture = std::make_unique<Texture>(key);
				m_pNoiseTexture->Initialize(&desc, &initialData);
			}

			SSR::Impl::~Impl()
			{
				m_SSRContents.Destroy();

				SafeRelease(m_pVertexShader);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					SafeRelease(m_pPixelShaders[i]);
				}
			}

			void SSR::Impl::SetRenderState(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext)
			{
				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(BlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(DepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerLinearClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipLinearClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_LinearClamp, 1, &pSamplerLinearClamp);

				ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_PointClamp, 1, &pSamplerPointClamp);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
			}

			void SSR::Impl::Apply(Camera* pCamera, const RenderTarget* pSource, const GBuffer* pGBuffer, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(SSR);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				SetRenderState(pDeviceInstance, pDeviceContext);

				D3D11_TEXTURE2D_DESC desc{};
				pResult->GetDesc2D(&desc);

				RenderTarget* pSSR = pDeviceInstance->GetRenderTarget(&desc);
				pDeviceContext->ClearRenderTargetView(pSSR->GetRenderTargetView(), math::Color::Transparent);

				ID3D11RenderTargetView* pRTV[] = { pSSR->GetRenderTargetView() };
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				D3D11_VIEWPORT viewport{};
				viewport.Width = static_cast<float>(desc.Width);
				viewport.Height = static_cast<float>(desc.Height);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pDeviceContext->RSSetViewports(1, &viewport);

				ID3D11ShaderResourceView* pSourceSRV = pSource->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Color, 1, &pSourceSRV);

				ID3D11ShaderResourceView* pNormalSRV = pGBuffer->GetRenderTarget(GBufferType::eNormals)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Normal, 1, &pNormalSRV);

				ID3D11ShaderResourceView* pDisneyBRDFSRV = pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_DisneyBRDF, 1, &pDisneyBRDFSRV);

				ID3D11ShaderResourceView* pDepthSRV = pDepth->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Depth, 1, &pDepthSRV);

				ID3D11ShaderResourceView* pNoiseTextureSRV = m_pNoiseTexture->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Noise, 1, &pNoiseTextureSRV);

				const graphics::Options::SSRConfig& ssrConfig = graphics::GetOptions().ssrConfig;

				shader::SetSSRContents(pDeviceContext, &m_SSRContents,
					pCamera->GetViewMatrix().Invert(), pCamera->GetProjectionMatrix().Invert(),
					pCamera->GetViewMatrix(), pCamera->GetProjectionMatrix(),
					math::float2(viewport.Width, viewport.Height),
					ssrConfig.sampleCount);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_SSRContents, 1, &m_SSRContents.pBuffer);

				pDeviceContext->PSSetShader(m_pPixelShaders[shader::ePS_SSR], nullptr, 0);
				pDeviceContext->Draw(4, 0);

				ID3D11ShaderResourceView* pNullSRV = { nullptr };
				pDeviceContext->PSSetShaderResources(shader::eSRV_Color, 1, &pNullSRV);
				pDeviceContext->PSSetShaderResources(shader::eSRV_Normal, 1, &pNullSRV);
				pDeviceContext->PSSetShaderResources(shader::eSRV_DisneyBRDF, 1, &pNullSRV);
				pDeviceContext->PSSetShaderResources(shader::eSRV_Depth, 1, &pNullSRV);
				pDeviceContext->PSSetShaderResources(shader::eSRV_Noise, 1, &pNullSRV);

				RenderTarget* pSSR_blur = pDeviceInstance->GetRenderTarget(&desc);
				pDeviceContext->ClearRenderTargetView(pSSR_blur->GetRenderTargetView(), math::Color::Transparent);

				postprocess::gaussianblur::Apply(pSSR, pSSR_blur, ssrConfig.blurSigma);

				pDeviceInstance->ReleaseRenderTargets(&pSSR);

				//////////
				SetRenderState(pDeviceInstance, pDeviceContext);

				pRTV[0] = pResult->GetRenderTargetView();
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				viewport.Width = static_cast<float>(desc.Width);
				viewport.Height = static_cast<float>(desc.Height);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pDeviceContext->RSSetViewports(1, &viewport);

				pDeviceContext->PSSetShaderResources(shader::eSRV_Color, 1, &pSourceSRV);

				ID3D11ShaderResourceView* pSSR_BlurSRV = pSSR_blur->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_SSR, 1, &pSSR_BlurSRV);

				pDeviceContext->PSSetShader(m_pPixelShaders[shader::ePS_SSR_Color], nullptr, 0);
				pDeviceContext->Draw(4, 0);

				pDeviceInstance->ReleaseRenderTargets(&pSSR_blur);
			}

			SSR::SSR()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			SSR::~SSR()
			{
			}

			void SSR::Apply(Camera* pCamera, const RenderTarget* pSource, const GBuffer* pGBuffer, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCamera, pSource, pGBuffer, pDepth, pResult);
			}
		}
	}
}
