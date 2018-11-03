#include "stdafx.h"
#include "DepthOfFieldDX11.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"

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
				struct DepthOfFieldContents
				{
					enum
					{
						NUM_DOF_TAPS = 12,
					};

					float fFocalDistance{ 0.f };
					float fFocalWidth{ 0.f };
					math::Vector2 padding;

					std::array<math::Vector4, NUM_DOF_TAPS> f4FilterTaps;

					math::Matrix matInvProj;
				};

				enum CBSlot
				{
					eCB_DepthOfFieldContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_Point = 0,
					eSampler_Linear = 1,
				};

				enum SRVSlot
				{
					eSRV_Color = 0,
					eSRV_Depth,
				};

				void SetDepthOfFieldContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<DepthOfFieldContents>* pCB_DepthOfFieldContents, 
					const Camera* pCamera,
					float fFocalDistance, float fFocalWidth,
					uint32_t nWidth, uint32_t nHeight)
				{
					DepthOfFieldContents* pDepthOfFieldContents = pCB_DepthOfFieldContents->Map(pDeviceContext);

					pDepthOfFieldContents->fFocalDistance = fFocalDistance;;
					pDepthOfFieldContents->fFocalWidth = fFocalWidth;
					pDepthOfFieldContents->padding = {};

					// Scale tap offsets based on render target size
					const float dx = 0.5f / static_cast<float>(nWidth);
					const float dy = 0.5f / static_cast<float>(nHeight);

					// Generate the texture coordinate offsets for our disc
					pDepthOfFieldContents->f4FilterTaps[0] = { -0.326212f * dx, -0.40581f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[1] = { -0.840144f * dx, -0.07358f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[2] = { -0.840144f * dx, 0.457137f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[3] = { -0.203345f * dx, 0.620716f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[4] = { 0.96234f * dx, -0.194983f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[5] = { 0.473434f * dx, -0.480026f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[6] = { 0.519456f * dx, 0.767022f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[7] = { 0.185461f * dx, -0.893124f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[8] = { 0.507431f * dx, 0.064425f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[9] = { 0.89642f * dx, 0.412458f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[10] = { -0.32194f * dx, -0.932615f * dy, 0.f, 0.f };
					pDepthOfFieldContents->f4FilterTaps[11] = { -0.791559f * dx, -0.59771f * dy, 0.f, 0.f };

					pDepthOfFieldContents->matInvProj = pCamera->GetProjMatrix().Invert().Transpose();

					pCB_DepthOfFieldContents->Unmap(pDeviceContext);
				}
			}

			class DepthOfField::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(Camera* pCamera, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				ID3D11PixelShader* m_pPixelShader{ nullptr };

				ConstantBuffer<shader::DepthOfFieldContents> m_depthOfFieldContents;
			};

			DepthOfField::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\DepthOfField\\DepthOfField.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(string::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : DepthOfField.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "VS", shader::VS_CompileVersion, &m_pVertexShader, "DepthOfField_VS") == false)
				{
					throw_line("failed to create DepthOfField_VS");
				}

				if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "DofDiscPS", shader::PS_CompileVersion, &m_pPixelShader, "DofDiscPS") == false)
				{
					throw_line("failed to create DepthOfFieldH_PS");
				}

				SafeRelease(pShaderBlob);

				m_depthOfFieldContents.Create(pDevice, "DepthOfFieldContents");
			}

			DepthOfField::Impl::~Impl()
			{
				m_depthOfFieldContents.Destroy();

				SafeRelease(m_pVertexShader);
				SafeRelease(m_pPixelShader);
			}

			void DepthOfField::Impl::Apply(Camera* pCamera, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				if (pSource == nullptr || pDepth == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(DepthOfField);

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

				ID3D11SamplerState* pSamplerPoint = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointWrap);
				pDeviceContext->PSSetSamplers(shader::eSampler_Point, 1, &pSamplerPoint);

				ID3D11SamplerState* pSamplerLinear = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipLinearWrap);
				pDeviceContext->PSSetSamplers(shader::eSampler_Linear, 1, &pSamplerLinear);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
				pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

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

				ID3D11ShaderResourceView* pSRV = pSource->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Color, 1, &pSRV);

				pSRV = pDepth->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Depth, 1, &pSRV);

				const Options& options = GetOptions();
				const Options::DepthOfFieldConfig& config = options.depthOfFieldConfig;

				shader::SetDepthOfFieldContents(pDeviceContext, &m_depthOfFieldContents, pCamera, config.FocalDistnace, config.FocalWidth, desc.Width, desc.Height);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_DepthOfFieldContents, 1, &m_depthOfFieldContents.pBuffer);

				pDeviceContext->Draw(4, 0);
			}

			DepthOfField::DepthOfField()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			DepthOfField::~DepthOfField()
			{
			}

			void DepthOfField::Apply(Camera* pCamera, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCamera, pSource, pDepth, pResult);
			}
		}
	}
}