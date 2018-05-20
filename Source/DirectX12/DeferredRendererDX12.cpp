#include "stdafx.h"
#include "DeferredRendererDX12.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"
#include "GraphicsInterface/LightManager.h"

#include "UtilDX12.h"
#include "DefineDX12.h"
#include "DeviceDX12.h"
#include "GBufferDX12.h"

#include "DescriptorHeapDX12.h"
#include "RenderTargetDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			namespace deferredshader
			{
				struct LightContents
				{
					uint32_t nDirectionalLightCount{ 0 };
					uint32_t nPointLightCount{ 0 };
					uint32_t nSpotLightCount{ 0 };
					uint32_t padding{ 0 };

					std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
					std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
					std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};
				};

				struct CommonContents
				{
					math::Matrix matInvView;
					math::Matrix matInvProj;

					math::Vector3 f3CameraPos;
					int nEnableShadowCount{ 0 };

					uint32_t nTexDepthIndex{ 0 };
					uint32_t nTexNormalIndex{ 0 };
					uint32_t nTexAlbedoSpecularIndex{ 0 };
					uint32_t nTexDisneyBRDFIndex{ 0 };

					uint32_t nTexDiffuseHDRIndex{ 0 };
					uint32_t nTexSpecularHDRIndex{ 0 };
					uint32_t nTexSpecularBRDFIndex{ 0 };
					uint32_t nTexShadowMapIndex{ 0 };
				};

				enum CBSlot
				{
					eCB_LightContents = 0,
					eCB_CommonContents,
				};
			}

			class DeferredRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(Camera* pCamera);
				void Flush();

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_LightContentsCB,
					eRP_CommonContentsCB,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice);

			private:
				std::string m_strShaderPath;
				ID3DBlob* m_pShaderBlob{ nullptr };

				ID3D12PipelineState* m_pPipelineState{ nullptr };
				ID3D12RootSignature* m_pRootSignature{ nullptr };

				ConstantBuffer<deferredshader::CommonContents> m_commonContentsBuffer;
				ConstantBuffer<deferredshader::LightContents> m_lightContentsBuffer;
			};

			DeferredRenderer::Impl::Impl()
			{
				m_strShaderPath = file::GetPath(file::eFx);
				m_strShaderPath.append("Model\\Deferred.hlsl");

				if (FAILED(D3DReadFileToBlob(String::MultiToWide(m_strShaderPath).c_str(), &m_pShaderBlob)))
				{
					throw_line("failed to read shader file : Model.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreatePipelineState(pDevice);

				auto SetGPUAddress = [](ID3D12Resource* pResource, uint8_t** ppViewGPUAddress, D3D12_GPU_VIRTUAL_ADDRESS* pGPUAddress, size_t nSize)
				{
					CD3DX12_RANGE readRange(0, 0);
					HRESULT hr = pResource->Map(0, &readRange, reinterpret_cast<void**>(ppViewGPUAddress));
					if (FAILED(hr))
					{
						throw_line("failed to map, constant buffer upload heap");
					}

					Memory::Clear(*ppViewGPUAddress, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

					*pGPUAddress = pResource->GetGPUVirtualAddress();
				};

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					if (util::CreateConstantBuffer(pDevice, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, &m_commonContentsBuffer.pUploadHeaps[i], L"CommonContents") == false)
					{
						throw_line("failed to create constant buffer, CommonContents");
					}
					SetGPUAddress(m_commonContentsBuffer.pUploadHeaps[i], &m_commonContentsBuffer.pViewGPUAddress[i], &m_commonContentsBuffer.gpuAddress[i], D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

					if (util::CreateConstantBuffer(pDevice, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, &m_lightContentsBuffer.pUploadHeaps[i], L"LightContents") == false)
					{
						throw_line("failed to create constant buffer, LightContents");
					}
					SetGPUAddress(m_lightContentsBuffer.pUploadHeaps[i], &m_lightContentsBuffer.pViewGPUAddress[i], &m_lightContentsBuffer.gpuAddress[i], D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
				}

				SafeRelease(m_pShaderBlob);
			}

			DeferredRenderer::Impl::~Impl()
			{
				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					SafeRelease(m_commonContentsBuffer.pUploadHeaps[i]);
					SafeRelease(m_lightContentsBuffer.pUploadHeaps[i]);
				}

				SafeRelease(m_pPipelineState);
				SafeRelease(m_pRootSignature);
			}

			void DeferredRenderer::Impl::Render(Camera* pCamera)
			{
				Device* pDeviceInstance = Device::GetInstance();
				LightManager* pLightManager = LightManager::GetInstance();

				int nFrameIndex = pDeviceInstance->GetFrameIndex();
				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer(nFrameIndex);
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();
				DescriptorHeap* pSamplerDescriptorHeap = pDeviceInstance->GetSamplerDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				RenderTarget* pSwapChainRenderTarget = pDeviceInstance->GetSwapChainRenderTarget(nFrameIndex);
				D3D12_RESOURCE_DESC desc = pSwapChainRenderTarget->GetDesc();

				RenderTarget* pRenderTarget = pDeviceInstance->GetRenderTarget(&desc, math::Color::Black, true);
				if (pRenderTarget == nullptr)
				{
					throw_line("failed to get render target");
				}

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] = 
				{
					pRenderTarget->GetCPUHandle(),
				};

				{
					deferredshader::CommonContents* pCommonContents = m_commonContentsBuffer.Cast(nFrameIndex, 0);
					pCommonContents->matInvView = pCamera->GetViewMatrix().Invert().Transpose();
					pCommonContents->matInvProj = pCamera->GetProjMatrix().Invert().Transpose();

					pCommonContents->f3CameraPos = pCamera->GetPosition();
					pCommonContents->nEnableShadowCount = 0;

					pCommonContents->nTexDepthIndex = pGBuffer->GetDepthStencil()->GetTexture()->GetDescriptorIndex();
					pCommonContents->nTexNormalIndex = pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetTexture()->GetDescriptorIndex();
					pCommonContents->nTexAlbedoSpecularIndex = pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetTexture()->GetDescriptorIndex();
					pCommonContents->nTexDisneyBRDFIndex = pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetTexture()->GetDescriptorIndex();

					const ImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();
					Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR());
					pCommonContents->nTexDiffuseHDRIndex = pDiffuseHDR->GetDescriptorIndex();

					Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR());
					pCommonContents->nTexSpecularHDRIndex = pSpecularHDR->GetDescriptorIndex();

					Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF());
					pCommonContents->nTexSpecularBRDFIndex = pSpecularBRDF->GetDescriptorIndex();
				}

				{
					deferredshader::LightContents* pLightContents = m_lightContentsBuffer.Cast(nFrameIndex, 0);
					const DirectionalLightData* pDirectionalLightData = nullptr;
					pLightManager->GetDirectionalLightData(&pDirectionalLightData, &pLightContents->nDirectionalLightCount);
					Memory::Copy(pLightContents->lightDirectional.data(), sizeof(pLightContents->lightDirectional), pDirectionalLightData, sizeof(DirectionalLightData) * pLightContents->nDirectionalLightCount);

					const PointLightData* pPointLightData = nullptr;
					pLightManager->GetPointLightData(&pPointLightData, &pLightContents->nPointLightCount);
					Memory::Copy(pLightContents->lightPoint.data(), sizeof(pLightContents->lightPoint), pPointLightData, sizeof(PointLightData) * pLightContents->nPointLightCount);

					const SpotLightData* pSpotLightData = nullptr;
					pLightManager->GetSpotLightData(&pSpotLightData, &pLightContents->nSpotLightCount);
					Memory::Copy(pLightContents->lightSpot.data(), sizeof(pLightContents->lightSpot), pSpotLightData, sizeof(SpotLightData) * pLightContents->nSpotLightCount);
				}

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->PopCommandList(m_pPipelineState);

				CD3DX12_RESOURCE_BARRIER transition[] =
				{
					CD3DX12_RESOURCE_BARRIER::Transition(pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
					CD3DX12_RESOURCE_BARRIER::Transition(pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
					CD3DX12_RESOURCE_BARRIER::Transition(pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
					CD3DX12_RESOURCE_BARRIER::Transition(pGBuffer->GetDepthStencil()->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
				};
				pCommandList->ResourceBarrier(_countof(transition), transition);

				pCommandList->SetGraphicsRootSignature(m_pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
					pSamplerDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->RSSetViewports(1, pViewport);
				pCommandList->RSSetScissorRects(1, pScissorRect);
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				pCommandList->ClearRenderTargetView(pRenderTarget->GetCPUHandle(), &math::Color::Black.r, 0, nullptr);

				pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				pCommandList->IASetIndexBuffer(nullptr);
				pCommandList->IASetVertexBuffers(0, 0, nullptr);

				pCommandList->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle(nFrameIndex));
				pCommandList->SetGraphicsRootConstantBufferView(eRP_LightContentsCB, m_lightContentsBuffer.gpuAddress[nFrameIndex]);
				pCommandList->SetGraphicsRootConstantBufferView(eRP_CommonContentsCB, m_commonContentsBuffer.gpuAddress[nFrameIndex]);

				pCommandList->DrawInstanced(4, 1, 0, 0);

				pDeviceInstance->PushCommandList(pCommandList);

				pDeviceInstance->ReleaseRenderTarget(&pRenderTarget);
			}

			void DeferredRenderer::Impl::Flush()
			{
			}

			ID3D12RootSignature* DeferredRenderer::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<D3D12_ROOT_PARAMETER> vecRootParameters;
				D3D12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				standardDescriptorTable.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				standardDescriptorTable.DescriptorTable.NumDescriptorRanges = eStandardDescriptorRangesCount;
				standardDescriptorTable.DescriptorTable.pDescriptorRanges = Device::GetInstance()->GetStandardDescriptorRanges();

				D3D12_ROOT_PARAMETER& lightContentsParameter = vecRootParameters.emplace_back();
				lightContentsParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				lightContentsParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				lightContentsParameter.Descriptor.ShaderRegister = deferredshader::eCB_LightContents;
				lightContentsParameter.Descriptor.RegisterSpace = 100;

				D3D12_ROOT_PARAMETER& commonContentsParameter = vecRootParameters.emplace_back();
				commonContentsParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				commonContentsParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				commonContentsParameter.Descriptor.ShaderRegister = deferredshader::eCB_CommonContents;
				commonContentsParameter.Descriptor.RegisterSpace = 100;

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipPointClamp, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipLinearClamp, 1, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
				rootSignatureDesc.Init(static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
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

			void DeferredRenderer::Impl::CreatePipelineState(ID3D12Device* pDevice)
			{
				const D3D_SHADER_MACRO macros[] = 
				{
					{ "DX12", "1" },
					{ nullptr, nullptr },
				};

				ID3DBlob* pVertexShaderBlob = nullptr;
				bool isSuccess = util::CompileShader(m_pShaderBlob, macros, m_strShaderPath.c_str(), "VS", "vs_5_1", &pVertexShaderBlob);
				if (isSuccess == false)
				{
					throw_line("failed to compile vertex shader");
				}

				ID3DBlob* pPixelShaderBlob = nullptr;
				isSuccess = util::CompileShader(m_pShaderBlob, macros, m_strShaderPath.c_str(), "PS", "ps_5_1", &pPixelShaderBlob);
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
				m_pRootSignature->SetName(L"DeferredRenderer");

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
				psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(EmDepthStencilState::eRead_Write_Off);

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				m_pPipelineState->SetName(L"DeferredRenderer");

				SafeRelease(pVertexShaderBlob);
				SafeRelease(pPixelShaderBlob);
			}

			DeferredRenderer::DeferredRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			DeferredRenderer::~DeferredRenderer()
			{
			}

			void DeferredRenderer::Render(Camera* pCamera)
			{
				m_pImpl->Render(pCamera);
			}

			void DeferredRenderer::Flush()
			{
				m_pImpl->Flush();
			}
		}
	}
}