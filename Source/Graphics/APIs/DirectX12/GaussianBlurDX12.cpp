#include "stdafx.h"
#include "GaussianBlurDX12.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"

#include "DescriptorHeapDX12.h"
#include "RenderTargetDX12.h"
#include "DepthStencilDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				struct GaussianBlurContents
				{
					float fSigma{ 0.5f };
					math::float2 f2SourceDimensions;
					float padding{ 0.f };

					uint32_t nTexColorIndex{ 0 };
					uint32_t nTexDepthIndex{ 0 };
					math::float2 padding2;
				};

				enum CBSlot
				{
					eCB_GaussianBlurContents = 0,
				};

				enum PSType
				{
					ePS_GaussianBlurH = 0,
					ePS_GaussianBlurV,
					ePS_GaussianDepthBlurH,
					ePS_GaussianDepthBlurV,

					ePS_Count,
				};

				const char* GetGaussianBlurPSTypeString(PSType emPSType)
				{
					switch (emPSType)
					{
					case ePS_GaussianBlurH:
						return "GaussianBlurH_PS";
					case ePS_GaussianBlurV:
						return "GaussianBlurV_PS";
					case ePS_GaussianDepthBlurH:
						return "GaussianDepthBlurH_PS";
					case ePS_GaussianDepthBlurV:
						return "GaussianDepthBlurV_PS";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetGaussianBlurContents(GaussianBlurContents* pGaussianBlurContents,
					float fSigma, const math::float2& f2SourceDimensions,
					uint32_t nTexColorIndex, uint32_t nTexDepthIndex)
				{
					pGaussianBlurContents->fSigma = fSigma;
					pGaussianBlurContents->f2SourceDimensions = f2SourceDimensions;
					pGaussianBlurContents->padding = 0.f;

					pGaussianBlurContents->nTexColorIndex = nTexColorIndex;
					pGaussianBlurContents->nTexDepthIndex = nTexDepthIndex;
					pGaussianBlurContents->padding2 = {};
				}
			}

			class GaussianBlur::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Apply(const RenderTarget* pSource, RenderTarget* pResult, float fSigma);
				void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float fSigma);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_GaussianBlurContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType);
				void CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType);

				void ApplyBlur(Device* pDeviceInstance, shader::PSType emPSType, float fSigma, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				struct RenderState
				{
					PSOCache psoCache;

					std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> pBundles{ nullptr };
				};
				std::array<RenderState, shader::ePS_Count> m_renderStates;

				ConstantBuffer<shader::GaussianBlurContents> m_gaussianBlurContents;
			};

			GaussianBlur::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\GaussianBlur\\GaussianBlur.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : GaussianBlur.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str(), emPSType);
				}

				m_gaussianBlurContents.Create(pDevice, 1, "GaussianBlurContents");

				SafeRelease(pShaderBlob);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreateBundles(pDevice, emPSType);
				}
			}

			GaussianBlur::Impl::~Impl()
			{
				m_gaussianBlurContents.Destroy();

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

			void GaussianBlur::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, nullptr, nullptr, emPSType);
					CreateBundles(pDevice, emPSType);
				}
			}

			void GaussianBlur::Impl::Apply(const RenderTarget* pSource, RenderTarget* pResult, float fSigma)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				const D3D12_RESOURCE_DESC desc = pSource->GetDesc();
				RenderTarget* pGaussianBlur = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent);

				ApplyBlur(pDeviceInstance, shader::ePS_GaussianBlurH, fSigma, pSource, nullptr, pGaussianBlur);
				ApplyBlur(pDeviceInstance, shader::ePS_GaussianBlurV, fSigma, pGaussianBlur, nullptr, pResult);

				pDeviceInstance->ReleaseRenderTargets(&pGaussianBlur);
			}

			void GaussianBlur::Impl::Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float fSigma)
			{
				if (pSource == nullptr || pDepth == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				const D3D12_RESOURCE_DESC desc = pSource->GetDesc();
				RenderTarget* pGaussianBlur = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent);

				ApplyBlur(pDeviceInstance, shader::ePS_GaussianDepthBlurH, fSigma, pSource, pDepth, pGaussianBlur);
				ApplyBlur(pDeviceInstance, shader::ePS_GaussianDepthBlurV, fSigma, pGaussianBlur, pDepth, pResult);

				pDeviceInstance->ReleaseRenderTargets(&pGaussianBlur);
			}

			ID3D12RootSignature* GaussianBlur::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& gaussianBlurContentsParameter = vecRootParameters.emplace_back();
				gaussianBlurContentsParameter.InitAsConstantBufferView(shader::eCB_GaussianBlurContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(EmSamplerState::eAnisotropicWrap, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void GaussianBlur::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType)
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
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, "VS", shader::VS_CompileVersion, &psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, GetGaussianBlurPSTypeString(emPSType), shader::PS_CompileVersion, &psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide(GetGaussianBlurPSTypeString(emPSType));

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

			void GaussianBlur::Impl::CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType)
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
						pSRVDescriptorHeap->GetHeap(),
					};
					pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					pCommandList->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle());
					pCommandList->SetGraphicsRootConstantBufferView(eRP_GaussianBlurContents, m_gaussianBlurContents.GPUAddress(i));

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

			void GaussianBlur::Impl::ApplyBlur(Device* pDeviceInstance, shader::PSType emPSType, float fSigma, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
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
					const uint32_t nTexDepthIndex = pDepth != nullptr ? pDepth->GetTexture()->GetDescriptorIndex() : 0;

					const D3D12_RESOURCE_DESC desc_source = pSource->GetDesc();

					shader::GaussianBlurContents* pGaussianBlurContents = m_gaussianBlurContents.Cast(frameIndex);
					shader::SetGaussianBlurContents(pGaussianBlurContents,
						fSigma, { static_cast<float>(desc_source.Width), static_cast<float>(desc_source.Height) },
						nTexColorIndex, nTexDepthIndex);
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

				pCommandList->ExecuteBundle(m_renderStates[emPSType].pBundles[frameIndex]);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			GaussianBlur::GaussianBlur()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			GaussianBlur::~GaussianBlur()
			{
			}

			void GaussianBlur::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void GaussianBlur::Apply(const RenderTarget* pSource, RenderTarget* pResult, float fSigma)
			{
				m_pImpl->Apply(pSource, pResult, fSigma);
			}

			void GaussianBlur::Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float fSigma)
			{
				m_pImpl->Apply(pSource, pDepth, pResult, fSigma);
			}
		}
	}
}