#include "stdafx.h"
#include "FxaaDX12.h"

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
				struct FxaaContents
				{
					math::Vector4 f4RcpFrame;

					uint32_t nTexInputIndex{ 0 };
					math::Vector3 padding;
				};

				enum CBSlot
				{
					eCB_FxaaContents = 0,
				};

				void SetFxaaContents(FxaaContents* pFxaaContents, const math::UInt2& n2ScreenSize, uint32_t nTexInputIndex)
				{
					const math::Vector4 vRcpFrame(1.f / static_cast<float>(n2ScreenSize.x), 1.f / static_cast<float>(n2ScreenSize.y), 0.f, 0.f);
					pFxaaContents->f4RcpFrame = vRcpFrame;
					pFxaaContents->nTexInputIndex = nTexInputIndex;
				}
			}

			class Fxaa::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_FxaaContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath);

			private:
				ID3D12PipelineState* m_pPipelineState{ nullptr };
				ID3D12RootSignature* m_pRootSignature{ nullptr };

				std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> m_pBundles;

				ConstantBuffer<shader::FxaaContents> m_fxaaContents;
			};

			Fxaa::Impl::Impl()
			{
				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\FXAA\\FXAA.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(String::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : FXAA.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreatePipelineState(pDevice, pShaderBlob, strShaderPath.c_str());

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					if (util::CreateConstantBuffer(pDevice, m_fxaaContents.AlignedSize(), &m_fxaaContents.pUploadHeaps[i], L"FxaaContents") == false)
					{
						throw_line("failed to create constant buffer, FxaaContents");
					}
				}

				m_fxaaContents.Initialize(m_fxaaContents.AlignedSize());

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
					m_pBundles[i]->SetGraphicsRootConstantBufferView(eRP_FxaaContents, m_fxaaContents.GPUAddress(i));

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

			Fxaa::Impl::~Impl()
			{
				for (auto& pBundle : m_pBundles)
				{
					SafeRelease(pBundle);
				}

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					SafeRelease(m_fxaaContents.pUploadHeaps[i]);
				}

				SafeRelease(m_pPipelineState);
				SafeRelease(m_pRootSignature);
			}

			void Fxaa::Impl::Apply(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				const math::UInt2& n2ScreenSize = pDeviceInstance->GetScreenSize();
				int nFrameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
				{
					pResult->GetCPUHandle(),
				};

				{
					shader::FxaaContents* pFxaaContents = m_fxaaContents.Cast(nFrameIndex);
					shader::SetFxaaContents(pFxaaContents, n2ScreenSize, pSource->GetTexture()->GetDescriptorIndex());
				}

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				pCommandList->RSSetViewports(1, pViewport);
				pCommandList->RSSetScissorRects(1, pScissorRect);
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				pCommandList->SetGraphicsRootSignature(m_pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->ExecuteBundle(m_pBundles[nFrameIndex]);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			ID3D12RootSignature* Fxaa::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<D3D12_ROOT_PARAMETER> vecRootParameters;
				D3D12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				standardDescriptorTable.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				standardDescriptorTable.DescriptorTable.NumDescriptorRanges = eStandardDescriptorRangesCount_SRV;
				standardDescriptorTable.DescriptorTable.pDescriptorRanges = Device::GetInstance()->GetStandardDescriptorRanges();

				D3D12_ROOT_PARAMETER& lightContentsParameter = vecRootParameters.emplace_back();
				lightContentsParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				lightContentsParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				lightContentsParameter.Descriptor.ShaderRegister = shader::eCB_FxaaContents;
				lightContentsParameter.Descriptor.RegisterSpace = 0;

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(EmSamplerState::eAnisotropicWrap, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
				rootSignatureDesc.Init(static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

				ID3DBlob* pError = nullptr;
				ID3DBlob* pSignature = nullptr;
				HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError);
				if (FAILED(hr))
				{
					std::string strError = String::Format("%s : %s", "failed to serialize root signature", pError->GetBufferPointer());
					SafeRelease(pError);
					throw_line(strError.c_str());
				}

				ID3D12RootSignature* pRootSignature{ nullptr };
				hr = pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&pRootSignature));
				if (FAILED(hr))
				{
					throw_line("failed to create root signature");
				}
				SafeRelease(pSignature);

				return pRootSignature;
			}

			void Fxaa::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath)
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
				isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, "PS", "ps_5_1", &pPixelShaderBlob);
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

				m_pRootSignature = CreateRootSignature(pDevice);
				m_pRootSignature->SetName(L"FXAA");

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.pRootSignature = m_pRootSignature;
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

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				m_pPipelineState->SetName(L"FXAA");

				SafeRelease(pVertexShaderBlob);
				SafeRelease(pPixelShaderBlob);
			}

			Fxaa::Fxaa()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			Fxaa::~Fxaa()
			{
			}

			void Fxaa::Apply(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply(pSource, pResult);
			}
		}
	}
}