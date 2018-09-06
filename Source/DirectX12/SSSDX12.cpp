#include "stdafx.h"
#include "SSSDX12.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"

#include "RenderTargetDX12.h"
#include "DepthStencilDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				struct SSSContents
				{
					float sssWidth{ 1.f };

					uint32_t nTexColorIndex{ 0 };
					uint32_t nTexDepthIndex{ 0 };
					float padding{ 0 };
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

				void SetSSSContents(SSSContents* pSSSContents, float sssWidth, uint32_t nTexColorIndex, uint32_t nTexDepthIndex)
				{
					pSSSContents->sssWidth = sssWidth;
					pSSSContents->nTexColorIndex = nTexColorIndex;
					pSSSContents->nTexDepthIndex = nTexDepthIndex;

					pSSSContents->padding = {};
				}
			}

			class SSS::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(ID3D12GraphicsCommandList2* pCommandList, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_SSSContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath, shader::PSType emPSType);

			private:
				struct RenderPipeline
				{
					ID3D12PipelineState* pPipelineState{ nullptr };
					ID3D12RootSignature* pRootSignature{ nullptr };

					std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> pBundles;
				};
				std::array<RenderPipeline, shader::ePS_Count> m_pipelineStates;

				ConstantBuffer<shader::SSSContents> m_sssContents;
			};

			SSS::Impl::Impl()
			{
				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\SSS\\SSS.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(String::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : SSS.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);

					CreatePipelineState(pDevice, pShaderBlob, strShaderPath.c_str(), emPSType);
				}

				m_sssContents.Create(pDevice, 1, "SSSContents");

				SafeRelease(pShaderBlob);

				DescriptorHeap* pDescriptorHeapSRV = Device::GetInstance()->GetSRVDescriptorHeap();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);

					for (int j = 0; j < eFrameBufferCount; ++j)
					{
						m_pipelineStates[emPSType].pBundles[j] = Device::GetInstance()->CreateBundle(m_pipelineStates[emPSType].pPipelineState);
						ID3D12GraphicsCommandList2* pBundles = m_pipelineStates[emPSType].pBundles[j];

						pBundles->SetGraphicsRootSignature(m_pipelineStates[emPSType].pRootSignature);

						ID3D12DescriptorHeap* pDescriptorHeaps[] =
						{
							pDescriptorHeapSRV->GetHeap(j),
						};
						pBundles->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

						pBundles->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pDescriptorHeapSRV->GetStartGPUHandle(j));

						pBundles->SetGraphicsRootConstantBufferView(eRP_SSSContents, m_sssContents.GPUAddress(j));

						pBundles->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
						pBundles->IASetVertexBuffers(0, 0, nullptr);
						pBundles->IASetIndexBuffer(nullptr);

						pBundles->DrawInstanced(4, 1, 0, 0);

						HRESULT hr = pBundles->Close();
						if (FAILED(hr))
						{
							throw_line("failed to close bundle");
						}
					}
				}
			}

			SSS::Impl::~Impl()
			{
				m_sssContents.Destroy();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					for (int j = 0; j < eFrameBufferCount; ++j)
					{
						SafeRelease(m_pipelineStates[i].pBundles[j]);
					}

					SafeRelease(m_pipelineStates[i].pPipelineState);
					SafeRelease(m_pipelineStates[i].pRootSignature);
				}
			}

			void SSS::Impl::Apply(ID3D12GraphicsCommandList2* pCommandList, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				int nFrameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
				{
					pResult->GetCPUHandle(),
				};

				{
					const Options& options = GetOptions();
					const Options::SSSConfig& config = options.sssConfig;

					shader::SSSContents* pSSSContents = m_sssContents.Cast(nFrameIndex);
					shader::SetSSSContents(pSSSContents, config.fWidth, pSource->GetTexture()->GetDescriptorIndex(), pDepth->GetTexture()->GetDescriptorIndex());
				}

				pCommandList->RSSetViewports(1, pViewport);
				pCommandList->RSSetScissorRects(1, pScissorRect);
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->SetGraphicsRootSignature(m_pipelineStates[shader::eHorizontal].pRootSignature);
				pCommandList->ExecuteBundle(m_pipelineStates[shader::eHorizontal].pBundles[nFrameIndex]);

				pCommandList->SetGraphicsRootSignature(m_pipelineStates[shader::eVertical].pRootSignature);
				pCommandList->ExecuteBundle(m_pipelineStates[shader::eVertical].pBundles[nFrameIndex]);
			}

			ID3D12RootSignature* SSS::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& sssContentsParameter = vecRootParameters.emplace_back();
				sssContentsParameter.InitAsConstantBufferView(shader::eCB_SSSContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagLinearMipPointClamp, shader::eSampler_LinearPointClamp, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipPointClamp, shader::eSampler_PointClamp, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void SSS::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath, shader::PSType emPSType)
			{
				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX12", "1" },
					{ nullptr, nullptr },
				};

				ID3DBlob* pVertexShaderBlob = nullptr;
				bool isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, "SSSSBlurVS", "vs_5_1", &pVertexShaderBlob);
				if (isSuccess == false)
				{
					throw_line("failed to compile vertex shader");
				}

				ID3DBlob* pPixelShaderBlob = nullptr;
				isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, shader::GetSSSPSTypeToString(emPSType), "ps_5_1", &pPixelShaderBlob);
				if (isSuccess == false)
				{
					throw_line("failed to compile pixel shader");
				}

				D3D12_SHADER_BYTECODE vertexShaderBytecode{};
				vertexShaderBytecode.BytecodeLength = pVertexShaderBlob->GetBufferSize();
				vertexShaderBytecode.pShaderBytecode = pVertexShaderBlob->GetBufferPointer();

				D3D12_SHADER_BYTECODE pixelShaderBytecode{};
				pixelShaderBytecode.BytecodeLength = pPixelShaderBlob->GetBufferSize();
				pixelShaderBytecode.pShaderBytecode = pPixelShaderBlob->GetBufferPointer();

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				const std::wstring wstrDebugName = String::MultiToWide(shader::GetSSSPSTypeToString(emPSType));

				ID3D12RootSignature* pRootSignature = CreateRootSignature(pDevice);
				pRootSignature->SetName(wstrDebugName.c_str());

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.pRootSignature = pRootSignature;
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

				ID3D12PipelineState* pPipelineState = nullptr;
				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				pPipelineState->SetName(wstrDebugName.c_str());

				m_pipelineStates[emPSType].pRootSignature = pRootSignature;
				m_pipelineStates[emPSType].pPipelineState = pPipelineState;

				SafeRelease(pVertexShaderBlob);
				SafeRelease(pPixelShaderBlob);
			}

			SSS::SSS()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			SSS::~SSS()
			{
			}

			void SSS::Apply(ID3D12GraphicsCommandList2* pCommandList, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCommandList, pSource, pDepth, pResult);
			}
		}
	}
}
