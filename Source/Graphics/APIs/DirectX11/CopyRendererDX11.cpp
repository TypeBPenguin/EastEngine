#include "stdafx.h"
#include "CopyRendererDX11.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"

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
					eSampler_Point = 0,
				};

				enum SRVSlot
				{
					eSRV_Color = 0,
				};

				enum PSType
				{
					ePS_RGBA = 0,
					ePS_RGB,

					ePS_Count,
				};

				const char* GetCopyPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case ePS_RGBA:
						return "PS_RGBA";
					case ePS_RGB:
						return "PS_RGB";
					default:
						throw_line("unknown ps type");
						break;
					}
				}
			}

			class CopyRenderer::Impl
			{
			public:
				Impl();
				virtual ~Impl();

			public:
				void Copy_RGBA(const RenderTarget* pSource, RenderTarget* pResult);
				void Copy_RGB(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				void CommonReady(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext, const RenderTarget* pSource, RenderTarget* pResult);
				void CommonDraw(ID3D11DeviceContext* pDeviceContext, shader::PSType emPSType, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShaders{ nullptr };
			};

			CopyRenderer::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\Copy.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Copy.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "VS", shader::VS_CompileVersion, &m_pVertexShader, "VS") == false)
				{
					throw_line("failed to create VS");
				}

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					shader::PSType emPSType = static_cast<shader::PSType>(i);

					const char* psName = shader::GetCopyPSTypeToString(emPSType);
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), psName, shader::PS_CompileVersion, &m_pPixelShaders[i], psName) == false)
					{
						std::string dump = string::Format("failed to create %s", psName);
						throw_line(dump.c_str());
					}
				}

				SafeRelease(pShaderBlob);
			}

			CopyRenderer::Impl::~Impl()
			{
			}

			void CopyRenderer::Impl::Copy_RGBA(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(Copy_RGBA);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				CommonReady(pDeviceInstance, pDeviceContext, pSource, pResult);
				CommonDraw(pDeviceContext, shader::ePS_RGBA, pResult);
			}

			void CopyRenderer::Impl::Copy_RGB(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(Copy_RGB);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				CommonReady(pDeviceInstance, pDeviceContext, pSource, pResult);
				CommonDraw(pDeviceContext, shader::ePS_RGB, pResult);
			}

			void CopyRenderer::Impl::CommonReady(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext, const RenderTarget* pSource, RenderTarget* pResult)
			{
				pDeviceContext->ClearRenderTargetView(pResult->GetRenderTargetView(), &math::Color::Transparent.r);

				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_Point, 1, &pSamplerPointClamp);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);

				ID3D11ShaderResourceView* pSourceSRV = pSource->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Color, 1, &pSourceSRV);
			}

			void CopyRenderer::Impl::CommonDraw(ID3D11DeviceContext* pDeviceContext, shader::PSType emPSType, RenderTarget* pResult)
			{
				D3D11_TEXTURE2D_DESC desc{};
				pResult->GetDesc2D(&desc);

				D3D11_VIEWPORT viewport{};
				viewport.Width = static_cast<float>(desc.Width);
				viewport.Height = static_cast<float>(desc.Height);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pDeviceContext->RSSetViewports(1, &viewport);

				ID3D11RenderTargetView* pRTV[] = { pResult->GetRenderTargetView() };
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				pDeviceContext->PSSetShader(m_pPixelShaders[emPSType], nullptr, 0);
				pDeviceContext->Draw(4, 0);
			}

			CopyRenderer::CopyRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			CopyRenderer::~CopyRenderer()
			{
			}

			void CopyRenderer::Copy_RGBA(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Copy_RGBA(pSource, pResult);
			}

			void CopyRenderer::Copy_RGB(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Copy_RGB(pSource, pResult);
			}
		}
	}
}