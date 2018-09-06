#include "stdafx.h"
#include "DepthOfFieldDX12.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"

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
				struct DepthOfFieldContents
				{
					enum
					{
						NUM_DOF_TAPS = 12,
					};

					float fFocalDistance{ 0.f };
					float fFocalWidth{ 0.f };

					uint32_t nTexColorIndex{ 0 };
					uint32_t nTexDepthIndex{ 0 };

					std::array<math::Vector4, NUM_DOF_TAPS> f4FilterTaps;

					math::Matrix matInvProj;
				};

				enum CBSlot
				{
					eCB_DepthOfFieldContents = 0,
				};

				void SetDepthOfFieldContents(DepthOfFieldContents* pDepthOfFieldContents,
					const Camera* pCamera,
					float fFocalDistance, float fFocalWidth,
					uint64_t nWidth, uint64_t nHeight,
					uint32_t nTexColorIndex, uint32_t nTexDepthIndex)
				{
					pDepthOfFieldContents->fFocalDistance = fFocalDistance;;
					pDepthOfFieldContents->fFocalWidth = fFocalWidth;
					pDepthOfFieldContents->nTexColorIndex = nTexColorIndex;
					pDepthOfFieldContents->nTexDepthIndex = nTexDepthIndex;

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
				}
			}

			class DepthOfField::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(ID3D12GraphicsCommandList2* pCommandList, Camera* pCamera, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_DepthOfFieldContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath);

			private:
				ID3D12PipelineState* m_pPipelineState{ nullptr };
				ID3D12RootSignature* m_pRootSignature{ nullptr };

				std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> m_pBundles;

				ConstantBuffer<shader::DepthOfFieldContents> m_depthOfFieldContents;
			};

			DepthOfField::Impl::Impl()
			{
				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\DepthOfField\\DepthOfField.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(String::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : DepthOfField.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreatePipelineState(pDevice, pShaderBlob, strShaderPath.c_str());

				m_depthOfFieldContents.Create(pDevice, 1, "DepthOfFieldContents");

				SafeRelease(pShaderBlob);

				DescriptorHeap* pSRVDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_pBundles[i] = Device::GetInstance()->CreateBundle(m_pPipelineState);

					m_pBundles[i]->SetGraphicsRootSignature(m_pRootSignature);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pSRVDescriptorHeap->GetHeap(i),
					};
					m_pBundles[i]->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					m_pBundles[i]->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle(i));
					m_pBundles[i]->SetGraphicsRootConstantBufferView(eRP_DepthOfFieldContents, m_depthOfFieldContents.GPUAddress(i));

					m_pBundles[i]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					m_pBundles[i]->IASetVertexBuffers(0, 0, nullptr);
					m_pBundles[i]->IASetIndexBuffer(nullptr);

					m_pBundles[i]->DrawInstanced(4, 1, 0, 0);

					HRESULT hr = m_pBundles[i]->Close();
					if (FAILED(hr))
					{
						throw_line("failed to close bundle");
					}
				}
			}

			DepthOfField::Impl::~Impl()
			{
				m_depthOfFieldContents.Destroy();

				for (auto& pBundles : m_pBundles)
				{
					SafeRelease(pBundles);
				}

				SafeRelease(m_pPipelineState);
				SafeRelease(m_pRootSignature);
			}

			void DepthOfField::Impl::Apply(ID3D12GraphicsCommandList2* pCommandList, Camera* pCamera, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				if (pSource == nullptr || pDepth == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				const int nFrameIndex = pDeviceInstance->GetFrameIndex();
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
					const Options& options = GetOptions();
					const Options::DepthOfFieldConfig& config = options.depthOfFieldConfig;

					const uint32_t nTexColorIndex = pSource->GetTexture()->GetDescriptorIndex();
					const uint32_t nTexDepthIndex = pDepth != nullptr ? pDepth->GetTexture()->GetDescriptorIndex() : 0;

					const D3D12_RESOURCE_DESC desc_source = pSource->GetDesc();

					shader::DepthOfFieldContents* pDepthOfFieldContents = m_depthOfFieldContents.Cast(nFrameIndex);
					shader::SetDepthOfFieldContents(pDepthOfFieldContents, pCamera,
						config.FocalDistnace, config.FocalWidth,
						desc_source.Width, desc_source.Height,
						nTexColorIndex, nTexDepthIndex);
				}

				pCommandList->RSSetViewports(1, &viewport);
				pCommandList->RSSetScissorRects(1, &scissorRect);
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				pCommandList->SetGraphicsRootSignature(m_pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->ExecuteBundle(m_pBundles[nFrameIndex]);
			}

			ID3D12RootSignature* DepthOfField::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& depthOfFieldContentsParameter = vecRootParameters.emplace_back();
				depthOfFieldContentsParameter.InitAsConstantBufferView(shader::eCB_DepthOfFieldContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

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

			void DepthOfField::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath)
			{
				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX12", "1" },
					{ nullptr, nullptr },
				};

				ID3DBlob* pVertexShaderBlob = nullptr;
				bool isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, "VS", "vs_5_1", &pVertexShaderBlob);
				if (isSuccess == false)
				{
					throw_line("failed to compile vertex shader");
				}

				ID3DBlob* pPixelShaderBlob = nullptr;
				isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, "DofDiscPS", "ps_5_1", &pPixelShaderBlob);
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

				ID3D12RootSignature* pRootSignature = CreateRootSignature(pDevice);
				pRootSignature->SetName(L"DepthOfField");

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
				pPipelineState->SetName(L"DepthOfField");

				m_pPipelineState = pPipelineState;
				m_pRootSignature = pRootSignature;

				SafeRelease(pVertexShaderBlob);
				SafeRelease(pPixelShaderBlob);
			}

			DepthOfField::DepthOfField()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			DepthOfField::~DepthOfField()
			{
			}

			void DepthOfField::Apply(ID3D12GraphicsCommandList2* pCommandList, Camera* pCamera, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCommandList, pCamera, pSource, pDepth, pResult);
			}
		}
	}
}