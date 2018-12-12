#include "stdafx.h"
#include "DownScaleDX12.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"

#include "DescriptorHeapDX12.h"
#include "RenderTargetDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				struct DownScaleContents
				{
					math::float2 f2SourceDimensions;
					uint32_t nTexColorIndex{ 0 };
					float padding{ 0.f };
				};

				enum CBSlot
				{
					eCB_DownScaleContents = 0,
				};

				enum PSType
				{
					ePS_Downscale4 = 0,
					ePS_Downscale4Luminance,
					ePS_DownscaleHW,

					ePS_Count,
				};

				const char* GetDownScalePSTypeString(PSType emPSType)
				{
					switch (emPSType)
					{
					case ePS_Downscale4:
						return "DownscalePS";
					case ePS_Downscale4Luminance:
						return "DownscaleLuminancePS";
					case ePS_DownscaleHW:
						return "HWScalePS";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetDownScaleContents(DownScaleContents* pDownScaleContents, const math::float2& f2SourceDimensions, uint32_t nTexColorIndex)
				{
					pDownScaleContents->f2SourceDimensions = f2SourceDimensions;
					pDownScaleContents->nTexColorIndex = nTexColorIndex;
					pDownScaleContents->padding = {};
				}
			}

			class DownScale::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);

				void Apply4SW(const RenderTarget* pSource, RenderTarget* pResult, bool isLuminance = false);
				void Apply16SW(const RenderTarget* pSource, RenderTarget* pResult);

				void ApplyHW(const RenderTarget* pSource, RenderTarget* pResult);
				void Apply16HW(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_DownScaleContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath, shader::PSType emPSType);
				void CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType);

				void ApplyDownScale(Device* pDeviceInstance, shader::PSType emPSType, const RenderTarget* pSource, RenderTarget* pResult);

			private:
				struct RenderState
				{
					PSOCache psoCache;
					std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> pBundles{ nullptr };
				};
				std::array<RenderState, shader::ePS_Count> m_renderStates;

				ConstantBuffer<shader::DownScaleContents> m_downScaleContents;
			};

			DownScale::Impl::Impl()
			{
				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\DownScale\\DownScale.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(string::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : DownScale.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, pShaderBlob, strShaderPath.c_str(), emPSType);
				}

				m_downScaleContents.Create(pDevice, 1, "DownScaleContents");

				SafeRelease(pShaderBlob);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreateBundles(pDevice, emPSType);
				}
			}

			DownScale::Impl::~Impl()
			{
				m_downScaleContents.Destroy();

				for (auto& renderState : m_renderStates)
				{
					for (int j = 0; j < eFrameBufferCount; ++j)
					{
						util::ReleaseResource(renderState.pBundles[j]);
						renderState.pBundles[j] = nullptr;
					}

					renderState.psoCache.Destroy();
				}
			}

			void DownScale::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, nullptr, nullptr, emPSType);
					CreateBundles(pDevice, emPSType);
				}
			}

			void DownScale::Impl::Apply4SW(const RenderTarget* pSource, RenderTarget* pResult, bool isLuminance)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				shader::PSType emPSType = shader::ePS_Downscale4;
				if (isLuminance == true)
				{
					emPSType = shader::ePS_Downscale4Luminance;
				}
				ApplyDownScale(pDeviceInstance, emPSType, pSource, pResult);
			}

			void DownScale::Impl::Apply16SW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				D3D12_RESOURCE_DESC desc = pSource->GetDesc();

				desc.Width /= 4;
				desc.Height /= 4;

				RenderTarget* pDownscale = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent, false);

				ApplyDownScale(pDeviceInstance, shader::ePS_Downscale4, pSource, pDownscale);
				ApplyDownScale(pDeviceInstance, shader::ePS_Downscale4, pDownscale, pResult);

				pDeviceInstance->ReleaseRenderTargets(&pDownscale);
			}

			void DownScale::Impl::ApplyHW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				ApplyDownScale(pDeviceInstance, shader::ePS_DownscaleHW, pSource, pResult);
			}

			void DownScale::Impl::Apply16HW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				D3D12_RESOURCE_DESC desc = pSource->GetDesc();

				// 2
				desc.Width /= 2;
				desc.Height /= 2;

				RenderTarget* pDownscale1 = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent, false);
				ApplyDownScale(pDeviceInstance, shader::ePS_DownscaleHW, pSource, pDownscale1);

				// 4
				desc.Width /= 2;
				desc.Height /= 2;

				RenderTarget* pDownscale2 = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent, false);
				ApplyDownScale(pDeviceInstance, shader::ePS_DownscaleHW, pDownscale1, pDownscale2);
				pDeviceInstance->ReleaseRenderTargets(&pDownscale1);

				// 8
				desc.Width /= 2;
				desc.Height /= 2;

				RenderTarget* pDownscale3 = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent, false);
				ApplyDownScale(pDeviceInstance, shader::ePS_DownscaleHW, pDownscale2, pDownscale3);
				pDeviceInstance->ReleaseRenderTargets(&pDownscale2);

				// 16
				ApplyDownScale(pDeviceInstance, shader::ePS_DownscaleHW, pDownscale3, pResult);
				pDeviceInstance->ReleaseRenderTargets(&pDownscale3);
			}

			ID3D12RootSignature* DownScale::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& downScaleContentsParameter = vecRootParameters.emplace_back();
				downScaleContentsParameter.InitAsConstantBufferView(shader::eCB_DownScaleContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipPointWrap, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipLinearWrap, 1, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void DownScale::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath, shader::PSType emPSType)
			{
				PSOCache& psoCache = m_renderStates[emPSType].psoCache;

				if (pShaderBlob != nullptr)
				{
					const D3D_SHADER_MACRO macros[] =
					{
						{ "DX12", "1" },
						{ nullptr, nullptr },
					};

					if (psoCache.pVSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, "VS", shader::VS_CompileVersion, &psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, GetDownScalePSTypeString(emPSType), shader::PS_CompileVersion, &psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide(GetDownScalePSTypeString(emPSType));

				if (psoCache.pRootSignature == nullptr)
				{
					psoCache.pRootSignature = CreateRootSignature(pDevice);
					psoCache.pRootSignature->SetName(wstrDebugName.c_str());
				}

				D3D12_SHADER_BYTECODE vertexShaderBytecode{};
				vertexShaderBytecode.BytecodeLength = psoCache.pVSBlob->GetBufferSize();
				vertexShaderBytecode.pShaderBytecode = psoCache.pVSBlob->GetBufferPointer();

				D3D12_SHADER_BYTECODE pixelShaderBytecode{};
				pixelShaderBytecode.BytecodeLength = psoCache.pPSBlob->GetBufferSize();
				pixelShaderBytecode.pShaderBytecode = psoCache.pPSBlob->GetBufferPointer();

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.pRootSignature = psoCache.pRootSignature;
				psoDesc.VS = vertexShaderBytecode;
				psoDesc.PS = pixelShaderBytecode;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.SampleDesc = sampleDesc;
				psoDesc.SampleMask = 0xffffffff;
				psoDesc.RasterizerState = util::GetRasterizerDesc(EmRasterizerState::eSolidCullNone);
				psoDesc.BlendState = util::GetBlendDesc(EmBlendState::eOff);
				psoDesc.NumRenderTargets = 1;

				if (GetOptions().OnHDR == true)
				{
					psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}
				else
				{
					D3D12_RESOURCE_DESC desc = Device::GetInstance()->GetSwapChainRenderTarget(0)->GetDesc();
					psoDesc.RTVFormats[0] = desc.Format;
				}

				psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(EmDepthStencilState::eRead_Write_Off);

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoCache.pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				psoCache.pPipelineState->SetName(wstrDebugName.c_str());
			}

			void DownScale::Impl::CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType)
			{
				DescriptorHeap* pSRVDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					if (m_renderStates[emPSType].pBundles[i] != nullptr)
					{
						util::ReleaseResource(m_renderStates[emPSType].pBundles[i]);
						m_renderStates[emPSType].pBundles[i] = nullptr;
					}

					m_renderStates[emPSType].pBundles[i] = Device::GetInstance()->CreateBundle(m_renderStates[emPSType].psoCache.pPipelineState);

					ID3D12GraphicsCommandList2* pCommandList = m_renderStates[emPSType].pBundles[i];

					pCommandList->SetGraphicsRootSignature(m_renderStates[emPSType].psoCache.pRootSignature);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pSRVDescriptorHeap->GetHeap(i),
					};
					pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					pCommandList->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle(i));
					pCommandList->SetGraphicsRootConstantBufferView(eRP_DownScaleContents, m_downScaleContents.GPUAddress(i));

					pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					pCommandList->IASetVertexBuffers(0, 0, nullptr);
					pCommandList->IASetIndexBuffer(nullptr);

					pCommandList->DrawInstanced(4, 1, 0, 0);

					HRESULT hr = pCommandList->Close();
					if (FAILED(hr))
					{
						throw_line("failed to close bundle");
					}
				}
			}

			void DownScale::Impl::ApplyDownScale(Device* pDeviceInstance, shader::PSType emPSType, const RenderTarget* pSource, RenderTarget* pResult)
			{
				const uint32_t nFrameIndex = pDeviceInstance->GetFrameIndex();
				const DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				const D3D12_RESOURCE_DESC desc = pResult->GetDesc();

				D3D12_VIEWPORT viewport{};
				viewport.TopLeftX = 0.f;
				viewport.TopLeftY = 0.f;
				viewport.Width = static_cast<float>(desc.Width);
				viewport.Height = static_cast<float>(desc.Height);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;

				D3D12_RECT scissorRect{};
				scissorRect.left = 0;
				scissorRect.top = 0;
				scissorRect.right = static_cast<long>(desc.Width);
				scissorRect.bottom = static_cast<long>(desc.Height);

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
				{
					pResult->GetCPUHandle(),
				};

				{
					const uint32_t nTexColorIndex = pSource->GetTexture()->GetDescriptorIndex();

					const D3D12_RESOURCE_DESC desc_source = pSource->GetDesc();

					shader::DownScaleContents* pDownScaleContents = m_downScaleContents.Cast(nFrameIndex);
					shader::SetDownScaleContents(pDownScaleContents, { static_cast<float>(desc_source.Width), static_cast<float>(desc_source.Height) }, nTexColorIndex);
				}

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				pCommandList->RSSetViewports(1, &viewport);
				pCommandList->RSSetScissorRects(1, &scissorRect);
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				pCommandList->SetGraphicsRootSignature(m_renderStates[emPSType].psoCache.pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->ExecuteBundle(m_renderStates[emPSType].pBundles[nFrameIndex]);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			DownScale::DownScale()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			DownScale::~DownScale()
			{
			}

			void DownScale::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void DownScale::Apply4SW(const RenderTarget* pSource, RenderTarget* pResult, bool isLuminance)
			{
				m_pImpl->Apply4SW(pSource, pResult, isLuminance);
			}

			void DownScale::Apply16SW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply16SW(pSource, pResult);
			}

			void DownScale::ApplyHW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->ApplyHW(pSource, pResult);
			}

			void DownScale::Apply16HW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply16HW(pSource, pResult);
			}
		}
	}
}