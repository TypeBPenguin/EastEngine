#include "stdafx.h"
#include "SSSDX11.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"
#include "DepthStencilDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct SSSContents
				{
					float sssWidth{ 1.f };
					
					math::Vector3 padding;
				};

				enum CBSlot
				{
					eCB_SSSContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_LinearPointClamp = 0,
					eSampler_PointClamp = 1,
				};

				enum SRVSlot
				{
					eSRV_Source = 0,
					eSRV_Depth = 1,
				};

				enum PSType
				{
					eHorizontal = 0,
					eVertical,

					ePS_Count,
				};

				const char* GetSSSPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case eHorizontal:
						return "PS_Horizontal";
					case eVertical:
						return "PS_Vertical";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetSSSContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<SSSContents>* pCB_SSSContents, float sssWidth)
				{
					SSSContents* pSSSContents = pCB_SSSContents->Map(pDeviceContext);
					pSSSContents->sssWidth = sssWidth;

					pSSSContents->padding = {};

					pCB_SSSContents->Unmap(pDeviceContext);
				}
			}

			class SSS::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShaders{ nullptr };

				ConstantBuffer<shader::SSSContents> m_sssContents;
			};

			SSS::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\SSS\\SSS.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(String::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : SSS.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "SSSSBlurVS", "vs_5_0", &m_pVertexShader, "SSS_VS") == false)
				{
					throw_line("failed to create SSS_VS");
				}

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					shader::PSType emPSType = static_cast<shader::PSType>(i);

					const char* strPSName = shader::GetSSSPSTypeToString(emPSType);
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), strPSName, "ps_5_0", &m_pPixelShaders[i], strPSName) == false)
					{
						std::string dump = String::Format("failed to create %s", strPSName);
						throw_line(dump.c_str());
					}
				}

				SafeRelease(pShaderBlob);

				m_sssContents.Create(pDevice, "SSSContents");
			}

			SSS::Impl::~Impl()
			{
				m_sssContents.Destroy();

				SafeRelease(m_pVertexShader);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					SafeRelease(m_pPixelShaders[i]);
				}
			}

			void SSS::Impl::Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(SSS);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerLinearPointClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagLinearMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_LinearPointClamp, 1, &pSamplerLinearPointClamp);

				ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_PointClamp, 1, &pSamplerPointClamp);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);

				ID3D11RenderTargetView* pRTV[] = { pResult->GetRenderTargetView() };
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				D3D11_TEXTURE2D_DESC desc{};
				pResult->GetDesc2D(&desc);

				D3D11_VIEWPORT viewport{};
				viewport.Width = static_cast<float>(desc.Width);
				viewport.Height = static_cast<float>(desc.Height);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pDeviceContext->RSSetViewports(1, &viewport);

				ID3D11ShaderResourceView* pSourceSRV = pSource->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Source, 1, &pSourceSRV);

				ID3D11ShaderResourceView* pDepthSRV = pDepth->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Depth, 1, &pDepthSRV);

				const Options& options = GetOptions();
				const Options::SSSConfig& config = options.sssConfig;

				shader::SetSSSContents(pDeviceContext, &m_sssContents, config.fWidth);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_SSSContents, 1, &m_sssContents.pBuffer);

				pDeviceContext->PSSetShader(m_pPixelShaders[shader::eHorizontal], nullptr, 0);
				pDeviceContext->Draw(4, 0);

				pDeviceContext->PSSetShader(m_pPixelShaders[shader::eVertical], nullptr, 0);
				pDeviceContext->Draw(4, 0);
			}

			SSS::SSS()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			SSS::~SSS()
			{
			}

			void SSS::Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				m_pImpl->Apply(pSource, pDepth, pResult);
			}
		}
	}
}