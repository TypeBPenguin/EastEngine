#include "stdafx.h"
#include "MotionBlurDX11.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"
#include "DepthStencilDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct MotionBlur_PSConstants
				{
					float blurAmount{ 1.f };
					math::float3 padding;

					math::float2 SourceDimensions;
					math::float2 DestinationDimensions;
				};

				struct DepthMotionBlur_PSConstants
				{
					math::Matrix matInvView;
					math::Matrix matInvProj;
					math::Matrix matLastViewProj;
				};

				enum CBSlot
				{
					eCB_MotionBlur_PSConstants = 0,
					eCB_DepthMotionBlur_PSConstants = 1,
				};

				enum SamplerSlot
				{
					eSampler_Point = 0,
				};

				enum SRVSlot
				{
					eSRV_Color = 0,
					eSRV_Depth = 1,
					eSRV_Velocity = 1,
					eSRV_PrevVelocity = 2,
				};

				enum PSType
				{
					eDepthBuffer4SamplesPS = 0,
					eDepthBuffer8SamplesPS,
					eDepthBuffer12SamplesPS,

					eVelocityBuffer4SamplesPS,
					eVelocityBuffer8SamplesPS,
					eVelocityBuffer12SamplesPS,

					eDualVelocityBuffer4SamplesPS,
					eDualVelocityBuffer8SamplesPS,
					eDualVelocityBuffer12SamplesPS,

					ePS_Count,
				};
				static_assert(eDepthBuffer4SamplesPS == Options::MotionBlurConfig::eDepthBuffer_4Samples, "mismatch MotionBlur And Options");
				static_assert(eDepthBuffer8SamplesPS == Options::MotionBlurConfig::eDepthBuffer_8Samples, "mismatch MotionBlur And Options");
				static_assert(eDepthBuffer12SamplesPS == Options::MotionBlurConfig::eDepthBuffer_12Samples, "mismatch MotionBlur And Options");
				static_assert(eVelocityBuffer4SamplesPS == Options::MotionBlurConfig::eVelocityBuffer_4Samples, "mismatch MotionBlur And Options");
				static_assert(eVelocityBuffer8SamplesPS == Options::MotionBlurConfig::eVelocityBuffer_8Samples, "mismatch MotionBlur And Options");
				static_assert(eVelocityBuffer12SamplesPS == Options::MotionBlurConfig::eVelocityBuffer_12Samples, "mismatch MotionBlur And Options");
				static_assert(eDualVelocityBuffer4SamplesPS == Options::MotionBlurConfig::eDualVelocityBuffer_4Samples, "mismatch MotionBlur And Options");
				static_assert(eDualVelocityBuffer8SamplesPS == Options::MotionBlurConfig::eDualVelocityBuffer_8Samples, "mismatch MotionBlur And Options");
				static_assert(eDualVelocityBuffer12SamplesPS == Options::MotionBlurConfig::eDualVelocityBuffer_12Samples, "mismatch MotionBlur And Options");

				const char* GetMotionBlurPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case eDepthBuffer4SamplesPS:
						return "DepthBuffer4SamplesPS";
					case eDepthBuffer8SamplesPS:
						return "DepthBuffer8SamplesPS";
					case eDepthBuffer12SamplesPS:
						return "DepthBuffer12SamplesPS";
					case eVelocityBuffer4SamplesPS:
						return "VelocityBuffer4SamplesPS";
					case eVelocityBuffer8SamplesPS:
						return "VelocityBuffer8SamplesPS";
					case eVelocityBuffer12SamplesPS:
						return "VelocityBuffer12SamplesPS";
					case eDualVelocityBuffer4SamplesPS:
						return "DualVelocityBuffer4SamplesPS";
					case eDualVelocityBuffer8SamplesPS:
						return "DualVelocityBuffer8SamplesPS";
					case eDualVelocityBuffer12SamplesPS:
						return "DualVelocityBuffer12SamplesPS";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetMotionBlur_PSConstants(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<MotionBlur_PSConstants>* pCB_PSConstants,
					float blurAmount, const math::float2& sourceDimensions, const math::float2& destinationDimensions)
				{
					MotionBlur_PSConstants* pPSConstants = pCB_PSConstants->Map(pDeviceContext);
					{
						pPSConstants->blurAmount = blurAmount;

						pPSConstants->SourceDimensions = sourceDimensions;
						pPSConstants->DestinationDimensions = destinationDimensions;
					}
					pCB_PSConstants->Unmap(pDeviceContext);
				}

				void SetDepthMotionBlur_PSConstants(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<DepthMotionBlur_PSConstants>* pCB_PSConstants,
					const math::Matrix& matInvView, const math::Matrix& matInvProj, const math::Matrix& matLastViewProj)
				{
					DepthMotionBlur_PSConstants* pPSConstants = pCB_PSConstants->Map(pDeviceContext);
					{
						pPSConstants->matInvView = matInvView.Transpose();
						pPSConstants->matInvProj = matInvProj.Transpose();
						pPSConstants->matLastViewProj = matLastViewProj.Transpose();
					}
					pCB_PSConstants->Unmap(pDeviceContext);
				}
			}

			class MotionBlur::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(Camera* pCamera, const math::Matrix& matPrevViewProj, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);
				void Apply(Camera* pCamera, const RenderTarget* pSource, const RenderTarget* pVelocity, RenderTarget* pResult);
				void Apply(Camera* pCamera, const RenderTarget* pSource, const RenderTarget* pVelocity, const RenderTarget* pPrevVelocity, RenderTarget* pResult);

			private:
				void CommonReady(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext, const RenderTarget* pSource, RenderTarget* pResult);
				void CommonDraw(ID3D11DeviceContext* pDeviceContext, shader::PSType emPSType, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShaders{ nullptr };

				ConstantBuffer<shader::MotionBlur_PSConstants> m_psContents;
				ConstantBuffer<shader::DepthMotionBlur_PSConstants> m_psContents_depth;
			};

			MotionBlur::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\MotionBlur\\MotionBlur.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : MotionBlur.hlsl");
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

					const char* psName = shader::GetMotionBlurPSTypeToString(emPSType);
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), psName, shader::PS_CompileVersion, &m_pPixelShaders[i], psName) == false)
					{
						std::string dump = string::Format("failed to create %s", psName);
						throw_line(dump.c_str());
					}
				}

				SafeRelease(pShaderBlob);

				m_psContents.Create(pDevice, "MotionBlur_PSConstants");
				m_psContents_depth.Create(pDevice, "DepthMotionBlur_PSConstants");
			}

			MotionBlur::Impl::~Impl()
			{
				m_psContents.Destroy();
				m_psContents_depth.Destroy();

				SafeRelease(m_pVertexShader);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					SafeRelease(m_pPixelShaders[i]);
				}
			}

			void MotionBlur::Impl::Apply(Camera* pCamera, const math::Matrix& matPrevViewProj, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				if (pSource == nullptr || pDepth == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(DepthBufferMotionBlur);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				const Options::MotionBlurConfig& motionBlurConfig = GetOptions().motionBlurConfig;

				CommonReady(pDeviceInstance, pDeviceContext, pSource, pResult);

				shader::SetDepthMotionBlur_PSConstants(pDeviceContext, &m_psContents_depth,
					pCamera->GetViewMatrix().Invert(), pCamera->GetProjectionMatrix().Invert(), matPrevViewProj);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_DepthMotionBlur_PSConstants, 1, &m_psContents_depth.pBuffer);

				ID3D11ShaderResourceView* pSourceSRV = pDepth->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Depth, 1, &pSourceSRV);

				CommonDraw(pDeviceContext, static_cast<shader::PSType>(motionBlurConfig.emMode), pResult);
			}

			void MotionBlur::Impl::Apply(Camera* pCamera, const RenderTarget* pSource, const RenderTarget* pVelocity, RenderTarget* pResult)
			{
				if (pSource == nullptr || pVelocity == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(VelocityBufferMotionBlur);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				const Options::MotionBlurConfig& motionBlurConfig = GetOptions().motionBlurConfig;

				CommonReady(pDeviceInstance, pDeviceContext, pSource, pResult);

				ID3D11ShaderResourceView* pSourceSRV = pVelocity->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Velocity, 1, &pSourceSRV);

				CommonDraw(pDeviceContext, static_cast<shader::PSType>(motionBlurConfig.emMode), pResult);
			}

			void MotionBlur::Impl::Apply(Camera* pCamera, const RenderTarget* pSource, const RenderTarget* pVelocity, const RenderTarget* pPrevVelocity, RenderTarget* pResult)
			{
				if (pSource == nullptr || pVelocity == nullptr || pPrevVelocity == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(DualVelocityBufferMotionBlur);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				const Options::MotionBlurConfig& motionBlurConfig = GetOptions().motionBlurConfig;

				CommonReady(pDeviceInstance, pDeviceContext, pSource, pResult);

				ID3D11ShaderResourceView* pSourceSRV = pVelocity->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Velocity, 1, &pSourceSRV);

				pSourceSRV = pPrevVelocity->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_PrevVelocity, 1, &pSourceSRV);

				CommonDraw(pDeviceContext, static_cast<shader::PSType>(motionBlurConfig.emMode), pResult);
			}

			void MotionBlur::Impl::CommonReady(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext, const RenderTarget* pSource, RenderTarget* pResult)
			{
				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(BlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(DepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_Point, 1, &pSamplerPointClamp);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);

				ID3D11ShaderResourceView* pSourceSRV = pSource->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Color, 1, &pSourceSRV);

				D3D11_TEXTURE2D_DESC sourceDesc{};
				pSource->GetDesc2D(&sourceDesc);

				const math::float2 sourceDimensions{ static_cast<float>(sourceDesc.Width), static_cast<float>(sourceDesc.Height) };

				D3D11_TEXTURE2D_DESC destinationDesc{};
				pResult->GetDesc2D(&destinationDesc);

				const math::float2 destinationDimensions{ static_cast<float>(destinationDesc.Width), static_cast<float>(destinationDesc.Height) };

				const Options::MotionBlurConfig& motionBlurConfig = GetOptions().motionBlurConfig;

				shader::SetMotionBlur_PSConstants(pDeviceContext, &m_psContents,
					motionBlurConfig.blurAmount, sourceDimensions, destinationDimensions);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_MotionBlur_PSConstants, 1, &m_psContents.pBuffer);
			}

			void MotionBlur::Impl::CommonDraw(ID3D11DeviceContext* pDeviceContext, shader::PSType emPSType, RenderTarget* pResult)
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

			MotionBlur::MotionBlur()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			MotionBlur::~MotionBlur()
			{
			}

			void MotionBlur::Apply(Camera* pCamera, const math::Matrix& matPrevViewProj, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCamera, matPrevViewProj, pSource, pDepth, pResult);
			}

			void MotionBlur::Apply(Camera* pCamera, const RenderTarget* pSource, const RenderTarget* pVelocity, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCamera, pSource, pVelocity, pResult);
			}

			void MotionBlur::Apply(Camera* pCamera, const RenderTarget* pSource, const RenderTarget* pVelocity, const RenderTarget* pPrevVelocity, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCamera, pSource, pVelocity, pPrevVelocity, pResult);
			}
		}
	}
}