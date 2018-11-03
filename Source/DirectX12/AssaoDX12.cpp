#include "stdafx.h"
#include "AssaoDX12.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/AssaoInterface.h"
#include "GraphicsInterface/Camera.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"

#include "RenderTargetDX12.h"
#include "DepthStencilDX12.h"

#undef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				// ** WARNING ** if changing anything here, update the corresponding shader code! ** WARNING **
				struct ASSAOConstants
				{
					math::Vector2 ViewportPixelSize; // .zw == 1.0 / ViewportSize.xy
					math::Vector2 HalfViewportPixelSize; // .zw == 1.0 / ViewportHalfSize.xy

					math::Vector2 DepthUnpackConsts;
					math::Vector2 CameraTanHalfFOV;

					math::Vector2 NDCToViewMul;
					math::Vector2 NDCToViewAdd;

					math::Int2 PerPassFullResCoordOffset;
					math::Vector2 PerPassFullResUVOffset;

					math::Vector2 Viewport2xPixelSize;
					math::Vector2 Viewport2xPixelSize_x_025; // Viewport2xPixelSize* 0.25 (for fusing add+mul into mad)

					float EffectRadius; // world (viewspace) maximum size of the shadow
					float EffectShadowStrength; // global strength of the effect (0 - 5)
					float EffectShadowPow;
					float EffectShadowClamp;

					float EffectFadeOutMul; // effect fade out from distance (ex. 25)
					float EffectFadeOutAdd; // effect fade out to distance (ex. 100)
					float EffectHorizonAngleThreshold; // limit errors on slopes and caused by insufficient geometry tessellation (0.05 to 0.5)
					float EffectSamplingRadiusNearLimitRec; // if viewspace pixel closer than this, don't enlarge shadow sampling radius anymore (makes no sense to grow beyond some distance, not enough samples to cover everything, so just limit the shadow growth; could be SSAOSettingsFadeOutFrom* 0.1 or less)

					float DepthPrecisionOffsetMod;
					float NegRecEffectRadius; // -1.0 / EffectRadius
					float LoadCounterAvgDiv; // 1.0 / ( halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeX* halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeY )
					float AdaptiveSampleCountLimit;

					float InvSharpness;
					int PassIndex;
					math::Vector2 QuarterResPixelSize; // used for importance map only

					math::Vector4 PatternRotScaleMatrices[5];

					float NormalsUnpackMul;
					float NormalsUnpackAdd;
					float DetailAOStrength;
					float Dummy0;

#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
					math::Matrix NormalsWorldToViewspaceMatrix;
#endif
				};

				struct ASSAOSRVContents
				{
					uint32_t nTexDepthSourceIndex{ 0 };
					uint32_t nTexNormalMapSourceIndex{ 0 };

					uint32_t nTexViewspaceDepthSourceIndex{ 0 };
					uint32_t nTexViewspaceDepthSource1Index{ 0 };
					uint32_t nTexViewspaceDepthSource2Index{ 0 };
					uint32_t nTexViewspaceDepthSource3Index{ 0 };

					uint32_t nTexImportanceMapIndex{ 0 };
					uint32_t nTexLoadCounterIndex{ 0 };

					uint32_t nTexBlurInputIndex{ 0 };
					uint32_t nTexFinalSSAOIndex{ 0 };

					uint32_t nUavNormalsOutpuIndex{ 0 };
					uint32_t nUavLoadCounterOutpuIndex{ 0 };
				};

				enum CBSlot
				{
					eCB_ASSAOConstants = 0,
					eCB_ASSAOSRVContents = 1,
				};

				enum UAVSlot
				{
					eUAV_NormalsOutputUAV = 4,
					eUAV_LoadCounterOutputUAV = 4,
				};

				enum PSType
				{
					ePS_PrepareDepths = 0,
					//ePS_PrepareDepthsAndNormals,
					ePS_PrepareDepthsHalf,
					//ePS_PrepareDepthsAndNormalsHalf,
					ePS_PrepareDepthMip1,
					ePS_PrepareDepthMip2,
					ePS_PrepareDepthMip3,
					ePS_GenerateQ0,
					ePS_GenerateQ1,
					ePS_GenerateQ2,
					ePS_GenerateQ3,
					ePS_GenerateQ3Base,
					ePS_SmartBlur,
					ePS_SmartBlurWide,
					ePS_Apply,
					ePS_NonSmartBlur,
					ePS_NonSmartApply,
					ePS_NonSmartHalfApply,
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					ePS_GenerateImportanceMap,
					ePS_PostprocessImportanceMapA,
					ePS_PostprocessImportanceMapB,
#endif

					ePS_Count,
				};

				const char* GetASSAOPSTypeString(PSType emPSType)
				{
					switch (emPSType)
					{
					case ePS_PrepareDepths:
						return "PSPrepareDepths";
					//case ePS_PrepareDepthsAndNormals:
					//	return "PSPrepareDepthsAndNormals";
					case ePS_PrepareDepthsHalf:
						return "PSPrepareDepthsHalf";
					//case ePS_PrepareDepthsAndNormalsHalf:
					//	return "PSPrepareDepthsAndNormalsHalf";
					case ePS_PrepareDepthMip1:
						return "PSPrepareDepthMip1";
					case ePS_PrepareDepthMip2:
						return "PSPrepareDepthMip2";
					case ePS_PrepareDepthMip3:
						return "PSPrepareDepthMip3";
					case ePS_GenerateQ0:
						return "PSGenerateQ0";
					case ePS_GenerateQ1:
						return "PSGenerateQ1";
					case ePS_GenerateQ2:
						return "PSGenerateQ2";
					case ePS_GenerateQ3:
						return "PSGenerateQ3";
					case ePS_GenerateQ3Base:
						return "PSGenerateQ3Base";
					case ePS_SmartBlur:
						return "PSSmartBlur";
					case ePS_SmartBlurWide:
						return "PSSmartBlurWide";
					case ePS_Apply:
						return "PSApply";
					case ePS_NonSmartBlur:
						return "PSNonSmartBlur";
					case ePS_NonSmartApply:
						return "PSNonSmartApply";
					case ePS_NonSmartHalfApply:
						return "PSNonSmartHalfApply";
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					case ePS_GenerateImportanceMap:
						return "PSGenerateImportanceMap";
					case ePS_PostprocessImportanceMapA:
						return "PSPostprocessImportanceMapA";
					case ePS_PostprocessImportanceMapB:
						return "PSPostprocessImportanceMapB";
#endif
					default:
						throw_line("unknown ps type");
						break;
					}
				}
			}

			// Simplify texture creation and potential future porting
			struct D3D12Texture2D
			{
				ID3D12Resource* Resource{ nullptr };
				uint32_t SRVIndex{ 0 };
				uint32_t RTVIndex{ 0 };
				uint32_t UAVIndex{ 0 };
				math::UInt2 Size;

				math::Color ClearColor{ math::Color::Transparent };

				D3D12_RESOURCE_STATES State{ D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE };

				std::vector<D3D12Texture2D> vecSubResource;

				~D3D12Texture2D() { Reset(); }

				void Reset();
				bool ReCreateIfNeeded(ID3D12Device* pDevice, const math::UInt2& size, DXGI_FORMAT format, float& inoutTotalSizeSum, uint16_t mipLevels, uint16_t arraySize, const math::Color& clearColor, bool supportUAVs, const char* debugName);
				bool ReCreateMIPViewIfNeeded(ID3D12Device* pDevice, D3D12Texture2D& original, int mipViewSlice);
				bool ReCreateArrayViewIfNeeded(ID3D12Device* pDevice, D3D12Texture2D& original, int arraySlice);

				static void SetResourceBarrier(ID3D12GraphicsCommandList2* pCommandList, const std::vector<D3D12Texture2D*>& barrierTarget, D3D12_RESOURCE_STATES targetState, uint32_t nSubResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			};

			void D3D12Texture2D::Reset()
			{
				DescriptorHeap* pDescriptorHeapSRV = Device::GetInstance()->GetSRVDescriptorHeap();
				DescriptorHeap* pDescriptorHeapRTV = Device::GetInstance()->GetRTVDescriptorHeap();
				DescriptorHeap* pDescriptorHeapUAV = Device::GetInstance()->GetUAVDescriptorHeap();

				if (SRVIndex > 0)
				{
					pDescriptorHeapSRV->FreePersistent(SRVIndex);
					SRVIndex = 0;
				}

				if (RTVIndex > 0)
				{
					pDescriptorHeapRTV->FreePersistent(RTVIndex);
					RTVIndex = 0;
				}

				if (UAVIndex > 0)
				{
					pDescriptorHeapUAV->FreePersistent(UAVIndex);
					UAVIndex = 0;
				}

				util::ReleaseResource(Resource);
				Resource = nullptr;
				Size = math::UInt2::Zero;
			}

			bool D3D12Texture2D::ReCreateIfNeeded(ID3D12Device* pDevice, const math::UInt2& size, DXGI_FORMAT format, float& inoutTotalSizeSum, uint16_t mipLevels, uint16_t arraySize, const math::Color& clearColor, bool supportUAVs, const char* debugName)
			{
				int approxSize = size.x * size.y * util::GetPixelSizeInBytes(format);
				if (mipLevels != 1)
				{
					approxSize = approxSize * 2; // is this an overestimate?
				}
				inoutTotalSizeSum += approxSize * arraySize;

				if ((size.x == 0) || (size.y == 0) || (format == DXGI_FORMAT_UNKNOWN))
				{
					Reset();
				}
				else
				{
					if (Resource != nullptr)
					{
						D3D12_RESOURCE_DESC desc = Resource->GetDesc();
						if ((desc.Format == format) && (desc.Width == size.x) && (desc.Height == size.y) && (desc.MipLevels == mipLevels) && (desc.DepthOrArraySize == arraySize))
							return false;
					}

					Reset();

					D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
					if (supportUAVs)
					{
						flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
					}

					State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
					ClearColor = clearColor;

					D3D12_CLEAR_VALUE clearValue{};
					clearValue.Format = format;
					clearValue.Color[0] = clearColor.r;
					clearValue.Color[1] = clearColor.g;
					clearValue.Color[2] = clearColor.b;
					clearValue.Color[3] = clearColor.a;

					CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, arraySize, mipLevels, 1, 0, flags);
					CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
					HRESULT hr = pDevice->CreateCommittedResource(&heapProperties,
						D3D12_HEAP_FLAG_NONE,
						&resourceDesc,
						State,
						(arraySize == 1 ? &clearValue : nullptr),
						IID_PPV_ARGS(&Resource));
					if (FAILED(hr))
					{
						throw_line("failed to create ASSAO Resource");
					}

					const std::wstring wstrDebugName = string::MultiToWide(debugName);
					Resource->SetName(wstrDebugName.c_str());

					DescriptorHeap* pDescriptorHeapSRV = Device::GetInstance()->GetSRVDescriptorHeap();
					PersistentDescriptorAlloc srvAlloc = pDescriptorHeapSRV->AllocatePersistent();
					SRVIndex = srvAlloc.nIndex;

					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
					srvDesc.Format = format;
					srvDesc.ViewDimension = (arraySize == 1) ? (D3D12_SRV_DIMENSION_TEXTURE2D) : (D3D12_SRV_DIMENSION_TEXTURE2DARRAY);
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

					if (arraySize == 1)
					{
						srvDesc.Texture2D.MipLevels = 1;
						srvDesc.Texture2D.MostDetailedMip = 0;
						srvDesc.Texture2D.PlaneSlice = 0;
						srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
					}
					else
					{
						srvDesc.Texture2DArray.ArraySize = arraySize;
						srvDesc.Texture2DArray.FirstArraySlice = 0;
						srvDesc.Texture2DArray.MipLevels = 1;
						srvDesc.Texture2DArray.MostDetailedMip = 0;
						srvDesc.Texture2DArray.PlaneSlice = 0;
						srvDesc.Texture2DArray.ResourceMinLODClamp = 0.f;
					}

					for (uint32_t i = 0; i < pDescriptorHeapSRV->GetHeapCount(); ++i)
					{
						pDevice->CreateShaderResourceView(Resource, &srvDesc, srvAlloc.cpuHandles[i]);
					}

					if (arraySize == 1)
					{
						DescriptorHeap* pDescriptorHeapRTV = Device::GetInstance()->GetRTVDescriptorHeap();
						PersistentDescriptorAlloc rtvAlloc = pDescriptorHeapRTV->AllocatePersistent();
						RTVIndex = rtvAlloc.nIndex;

						D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
						rtvDesc.Format = format;
						rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

						pDevice->CreateRenderTargetView(Resource, &rtvDesc, rtvAlloc.cpuHandles[0]);
					}

					if (supportUAVs == true)
					{
						DescriptorHeap* pDescriptorHeapUAV = Device::GetInstance()->GetUAVDescriptorHeap();
						PersistentDescriptorAlloc uavAlloc = pDescriptorHeapUAV->AllocatePersistent();
						UAVIndex = uavAlloc.nIndex;

						D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
						uavDesc.Format = format;
						uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

						pDevice->CreateUnorderedAccessView(Resource, nullptr, &uavDesc, uavAlloc.cpuHandles[0]);
					}

					this->Size = size;
				}

				return true;
			}

			bool D3D12Texture2D::ReCreateMIPViewIfNeeded(ID3D12Device* pDevice, D3D12Texture2D& original, int mipViewSlice)
			{
				if (original.Resource == this->Resource)
					return true;

				Reset();

				this->Resource = original.Resource;
				this->Resource->AddRef();

				const D3D12_RESOURCE_DESC desc = Resource->GetDesc();

				DescriptorHeap* pDescriptorHeapSRV = Device::GetInstance()->GetSRVDescriptorHeap();
				PersistentDescriptorAlloc srvAlloc = pDescriptorHeapSRV->AllocatePersistent();
				SRVIndex = srvAlloc.nIndex;

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
				srvDesc.Format = desc.Format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.PlaneSlice = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

				for (uint32_t i = 0; i < pDescriptorHeapSRV->GetHeapCount(); ++i)
				{
					pDevice->CreateShaderResourceView(Resource, &srvDesc, srvAlloc.cpuHandles[i]);
				}

				DescriptorHeap* pDescriptorHeapRTV = Device::GetInstance()->GetRTVDescriptorHeap();
				PersistentDescriptorAlloc rtvAlloc = pDescriptorHeapRTV->AllocatePersistent();
				RTVIndex = rtvAlloc.nIndex;

				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
				rtvDesc.Format = desc.Format;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = mipViewSlice;

				pDevice->CreateRenderTargetView(Resource, &rtvDesc, rtvAlloc.cpuHandles[0]);

				this->Size = original.Size;

				for (int i = 0; i < mipViewSlice; i++)
				{
					this->Size.x = (this->Size.x + 1) / 2;
					this->Size.y = (this->Size.y + 1) / 2;
				}
				this->Size.x = std::max(this->Size.x, 1u);
				this->Size.y = std::max(this->Size.y, 1u);

				return true;
			}

			bool D3D12Texture2D::ReCreateArrayViewIfNeeded(ID3D12Device* pDevice, D3D12Texture2D& original, int arraySlice)
			{
				if (original.Resource == this->Resource)
					return true;

				Reset();

				this->Resource = original.Resource;
				this->Resource->AddRef();

				const D3D12_RESOURCE_DESC desc = Resource->GetDesc();

				DescriptorHeap* pDescriptorHeapRTV = Device::GetInstance()->GetRTVDescriptorHeap();
				PersistentDescriptorAlloc rtvAlloc = pDescriptorHeapRTV->AllocatePersistent();
				RTVIndex = rtvAlloc.nIndex;

				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
				rtvDesc.Format = desc.Format;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.MipSlice = 0;
				rtvDesc.Texture2DArray.FirstArraySlice = arraySlice;
				rtvDesc.Texture2DArray.ArraySize = 1;

				pDevice->CreateRenderTargetView(Resource, &rtvDesc, rtvAlloc.cpuHandles[0]);

				this->Size = original.Size;

				return true;
			}

			void D3D12Texture2D::SetResourceBarrier(ID3D12GraphicsCommandList2* pCommandList, const std::vector<D3D12Texture2D*>& barrierTarget, D3D12_RESOURCE_STATES targetState, uint32_t nSubResource)
			{
				if (barrierTarget.empty() == true)
					return;

				std::vector<CD3DX12_RESOURCE_BARRIER> transition;
				transition.reserve(barrierTarget.size());

				const size_t nSize = barrierTarget.size();
				for (size_t i = 0; i < nSize; ++i)
				{
					if (nSubResource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
					{
						if (barrierTarget[i]->vecSubResource.empty() == false)
						{
							const size_t nSubResourceSize = barrierTarget[i]->vecSubResource.size();
							for (size_t j = 0; j < nSubResourceSize; ++j)
							{
								if (barrierTarget[i]->vecSubResource[j].State != targetState)
								{
									transition.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(barrierTarget[i]->vecSubResource[j].Resource, barrierTarget[i]->vecSubResource[j].State, targetState, static_cast<uint32_t>(j)));
									barrierTarget[i]->vecSubResource[j].State = targetState;
								}
							}

							barrierTarget[i]->State = targetState;
						}
						else
						{
							if (barrierTarget[i]->State != targetState)
							{
								transition.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(barrierTarget[i]->Resource, barrierTarget[i]->State, targetState));
								barrierTarget[i]->State = targetState;
							}
						}
					}
					else
					{
						if (barrierTarget[i]->vecSubResource.empty() == false)
						{
							if (barrierTarget[i]->vecSubResource[nSubResource].State != targetState)
							{
								transition.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(barrierTarget[i]->vecSubResource[nSubResource].Resource, barrierTarget[i]->vecSubResource[nSubResource].State, targetState, nSubResource));
								barrierTarget[i]->vecSubResource[nSubResource].State = targetState;
							}
						}
					}
				}

				if (transition.empty() == true)
					return;

				pCommandList->ResourceBarrier(static_cast<uint32_t>(transition.size()), transition.data());
			}

			static const int cMaxBlurPassCount = 6;

			static GUID c_IID_ID3D12Texture2D = { 0x6f15aaf2,0xd208,0x4e89,{ 0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c } };

			struct AssaoInputsDX12 : public IAssaoInputs
			{
				ID3D12GraphicsCommandList2* pCommandList{ nullptr };

				// Hardware screen depths
				//  - R32_FLOAT (R32_TYPELESS) or R24_UNORM_X8_TYPELESS (R24G8_TYPELESS) texture formats are supported.
				//  - Multisampling not yet supported.
				//  - Decoded using provided ProjectionMatrix.
				//  - For custom decoding see PSPrepareDepths/PSPrepareDepthsAndNormals where they get converted to linear viewspace storage.
				uint32_t DepthSRV{ 0 };

				// Viewspace normals (optional) 
				//  - If NULL, normals are generated from the depth buffer, otherwise provided normals are used for AO. ASSAO is less
				//    costly when input normals are provided, and has a more defined effect. However, aliasing in normals can result in 
				//    aliasing/flickering in the effect so, in some rare cases, normals generated from the depth buffer can look better.
				//  - _FLOAT or _UNORM texture formats are supported.
				//  - Input normals are expected to be in viewspace, encoded in [0, 1] with "encodedNormal.xyz = (normal.xyz* 0.5 + 0.5)" 
				//    or similar. Decode is done in LoadNormal() function with "normal.xyz = (encodedNormal.xyz* 2.0 - 1.0)", which can be
				//    easily modified for any custom decoding.
				//  - Use SSAO_SMOOTHEN_NORMALS for additional normal smoothing to reduce aliasing/flickering. This, however, also reduces
				//    high detail AO and increases cost.
				uint32_t NormalSRV{ 0 };

				// If not NULL, instead writing into currently bound render target, Draw will use this. Current render target will be restored 
				// to what it was originally after the Draw call.
				uint32_t OverrideOutputRTV{ 0 };

				uint32_t nFrameIndex{ eFrameBufferCount };
				DescriptorHeap* pDescriptorHeapSRV{ nullptr };
				DescriptorHeap* pDescriptorHeapRTV{ nullptr };
				DescriptorHeap* pDescriptorHeapUAV{ nullptr };
			};

			class Assao::Impl : public IAssaoEffect
			{
			public:
				Impl();
				virtual ~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);

			public:
				// ASSAO_Effect implementation
				virtual void PreAllocateVideoMemory(const IAssaoInputs* inputs) override;
				virtual void DeleteAllocatedVideoMemory() override;

				virtual uint32_t GetAllocatedVideoMemory() override;

				virtual void GetVersion(int& major, int& minor) override;

				// Apply the SSAO effect to the currently selected render target using provided config and platform-dependent inputs
				virtual void Draw(const Options::AssaoConfig& config, const IAssaoInputs* inputs) override;

			private:
				bool InitializeDX();
				void CleanupDX();

			private:
				void UpdateTextures(const AssaoInputsDX12* inputs);
				void UpdateConstants(const Options::AssaoConfig& config, const AssaoInputsDX12* inputs, int pass);
				void FullscreenPassDraw(const AssaoInputsDX12* inputs, shader::PSType emPSType);
				void PrepareDepths(const Options::AssaoConfig& config, const AssaoInputsDX12* inputs);
				void GenerateSSAO(const Options::AssaoConfig& config, const AssaoInputsDX12* inputs, bool adaptiveBasePass);

				shader::ASSAOSRVContents* AllocateSRVContents(uint32_t nFrameIndex)
				{
					assert(m_nSrvBufferIndex < eMaxSRVBufferCount);
					shader::ASSAOSRVContents* pSRV = m_srvContentsBuffer.Cast(nFrameIndex, m_nSrvBufferIndex);
					m_srvContentsBufferGPUAddress = m_srvContentsBuffer.GPUAddress(nFrameIndex, m_nSrvBufferIndex);
					m_nSrvBufferIndex++;

					return pSRV;
				}

				void ResetSRVContents(uint32_t nFrameIndex)
				{
					m_nSrvBufferIndex = 0;
					m_srvContentsBufferGPUAddress = m_srvContentsBuffer.GPUAddress(nFrameIndex, 0);
				}

				shader::ASSAOConstants* AllocateConstantBuffer(uint32_t nFrameIndex)
				{
					assert(m_nConstantsBufferIndex < eMaxConstantsBufferCount);
					shader::ASSAOConstants* pConstantBuffer = m_constantsBuffer.Cast(nFrameIndex, m_nConstantsBufferIndex);
					m_constantsBufferGPUAddress = m_constantsBuffer.GPUAddress(nFrameIndex, m_nConstantsBufferIndex);
					m_nConstantsBufferIndex++;

					return pConstantBuffer;
				}

				void ResetConstantBuffer(uint32_t nFrameIndex)
				{
					m_nConstantsBufferIndex = 0;
					m_constantsBufferGPUAddress = m_constantsBuffer.GPUAddress(nFrameIndex, 0);
				}

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,
					
					eRP_ASSAOConstantsCB,
					eRP_ASSAOSRVContentsCB,

					eRP_NormalsUAV,
					eRP_LoadCounterUAV,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice, shader::PSType emPSType);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath, shader::PSType emPSType);
				void CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType);

			private:
				struct BufferFormats
				{
					DXGI_FORMAT DepthBufferViewspaceLinear{ DXGI_FORMAT_R16_FLOAT };	// increase this to DXGI_FORMAT_R32_FLOAT if using very low FOVs (for a scope effect) or similar, or in case you suspect artifacts caused by lack of precision; performance will degrade
					DXGI_FORMAT Normals{ DXGI_FORMAT_R8G8B8A8_UNORM };	//Normals = DXGI_FORMAT_R8G8B8A8_UNORM;
					DXGI_FORMAT AOResult{ DXGI_FORMAT_R8G8_UNORM };
					DXGI_FORMAT ImportanceMap{ DXGI_FORMAT_R8_UNORM };
				};
				BufferFormats m_formats;

				math::UInt2 m_size;
				math::UInt2 m_halfSize;
				math::UInt2 m_quarterSize;
				math::UInt4 m_fullResOutScissorRect;
				math::UInt4 m_halfResOutScissorRect;

				uint32_t m_allocatedVRAM{ 0 };

				enum
				{
					eMaxConstantsBufferCount = 5,
					eMaxSRVBufferCount = 32,
				};

				ConstantBuffer<shader::ASSAOConstants> m_constantsBuffer;
				D3D12_GPU_VIRTUAL_ADDRESS m_constantsBufferGPUAddress{};
				uint32_t m_nConstantsBufferIndex{ 0 };

				ConstantBuffer<shader::ASSAOSRVContents> m_srvContentsBuffer;
				D3D12_GPU_VIRTUAL_ADDRESS m_srvContentsBufferGPUAddress{};
				uint32_t m_nSrvBufferIndex{ 0 };

				ID3D12Device* m_pDevice{ nullptr };

				struct RenderPipeline
				{
					PSOCache psoCache;

					std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> pBundles{ nullptr };
				};
				std::array<RenderPipeline, shader::ePS_Count> m_pipelineStates;

				D3D12Texture2D m_halfDepths[4];

				D3D12Texture2D m_pingPongHalfResultA;
				D3D12Texture2D m_pingPongHalfResultB;
				D3D12Texture2D m_finalResults;
				D3D12Texture2D m_normals;
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				// Only needed for quality level 3 (adaptive quality)
				D3D12Texture2D m_importanceMap;
				D3D12Texture2D m_importanceMapPong;
				
				ID3D12Resource* m_loadCounter{ nullptr };
				uint32_t m_nLoadCounterDescriptorSRVIndex{ std::numeric_limits<uint32_t>::max() };
				uint32_t m_nLoadCounterDescriptorUAVIndex{ std::numeric_limits<uint32_t>::max() };
#endif

				bool m_requiresClear{ false };
			};

			Assao::Impl::Impl()
			{
				InitializeDX();
			}

			Assao::Impl::~Impl()
			{
				CleanupDX();
			}

			void Assao::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				CreatePipelineState(pDevice, nullptr, nullptr, shader::ePS_Apply);
				CreateBundles(pDevice, shader::ePS_Apply);

				CreatePipelineState(pDevice, nullptr, nullptr, shader::ePS_NonSmartApply);
				CreateBundles(pDevice, shader::ePS_NonSmartApply);

				CreatePipelineState(pDevice, nullptr, nullptr, shader::ePS_NonSmartHalfApply);
				CreateBundles(pDevice, shader::ePS_NonSmartHalfApply);
			}

			void Assao::Impl::PreAllocateVideoMemory(const IAssaoInputs* _inputs)
			{
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// TODO: dynamic_cast if supported in _DEBUG to check for correct type cast below
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				const AssaoInputsDX12* inputs = static_cast<const AssaoInputsDX12*>(_inputs);

				UpdateTextures(inputs);
			}

			void Assao::Impl::DeleteAllocatedVideoMemory()
			{
			}

			uint32_t Assao::Impl::GetAllocatedVideoMemory()
			{
				return m_allocatedVRAM;
			}

			void Assao::Impl::GetVersion(int& major, int& minor)
			{
				major = 1;
				minor = 0;
			}

			void Assao::Impl::Draw(const Options::AssaoConfig& config, const IAssaoInputs* _inputs)
			{
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// TODO: dynamic_cast if supported in _DEBUG to check for correct type cast below
				////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				const AssaoInputsDX12* inputs = static_cast<const AssaoInputsDX12*>(_inputs);

				assert(inputs->pCommandList != nullptr);
				assert(inputs->DepthSRV != 0);
				assert(inputs->NormalSRV != 0);
				assert(inputs->OverrideOutputRTV != 0);
				assert(inputs->nFrameIndex != -1);

				assert(config.QualityLevel >= -1 && config.QualityLevel <= 3);
#ifndef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				if (config.QualityLevel == 3)
				{
					assert(false);
					return;
				}
#endif
				ResetSRVContents(inputs->nFrameIndex);
				ResetConstantBuffer(inputs->nFrameIndex);

				UpdateTextures(inputs);

				UpdateConstants(config, inputs, 0);

				ID3D12GraphicsCommandList2* pCommandList = inputs->pCommandList;

				{
					if (m_requiresClear == true)
					{
						if (m_normals.RTVIndex != 0)
						{
							D3D12Texture2D::SetResourceBarrier(pCommandList,
								{
									&m_halfDepths[0],
									&m_halfDepths[1],
									&m_halfDepths[2],
									&m_halfDepths[3],
									&m_pingPongHalfResultA,
									&m_pingPongHalfResultB,
									&m_finalResults,
									&m_normals,
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
									&m_importanceMap,
									&m_importanceMapPong,
#endif
								}, D3D12_RESOURCE_STATE_RENDER_TARGET);
						}
						else
						{
							D3D12Texture2D::SetResourceBarrier(pCommandList,
								{
									&m_halfDepths[0],
									&m_halfDepths[1],
									&m_halfDepths[2],
									&m_halfDepths[3],
									&m_pingPongHalfResultA,
									&m_pingPongHalfResultB,
									&m_finalResults,
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
									&m_importanceMap,
									&m_importanceMapPong,
#endif
								}, D3D12_RESOURCE_STATE_RENDER_TARGET);
						}

						auto ClearRenderTargetView = [&](const D3D12Texture2D& texture)
						{
							D3D12_CPU_DESCRIPTOR_HANDLE handle = inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(texture.RTVIndex);
							pCommandList->ClearRenderTargetView(handle, reinterpret_cast<const float*>(&texture.ClearColor.r), 0, nullptr);
						};

						ClearRenderTargetView(m_halfDepths[0]);
						ClearRenderTargetView(m_halfDepths[1]);
						ClearRenderTargetView(m_halfDepths[2]);
						ClearRenderTargetView(m_halfDepths[3]);
						ClearRenderTargetView(m_pingPongHalfResultA);
						ClearRenderTargetView(m_pingPongHalfResultB);
						ClearRenderTargetView(m_finalResults.vecSubResource[0]);
						ClearRenderTargetView(m_finalResults.vecSubResource[1]);
						ClearRenderTargetView(m_finalResults.vecSubResource[2]);
						ClearRenderTargetView(m_finalResults.vecSubResource[3]);

						if (m_normals.RTVIndex != 0)
						{
							ClearRenderTargetView(m_normals);
						}
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
						ClearRenderTargetView(m_importanceMap);
						ClearRenderTargetView(m_importanceMapPong);
#endif
						m_requiresClear = false;
					}

					// Generate depths
					PrepareDepths(config, inputs);

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					// for adaptive quality, importance map pass
					if (config.QualityLevel == 3)
					{
						// Generate simple quality SSAO
						GenerateSSAO(config, inputs, true);

						// Generate importance map
						{
							const CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_quarterSize.x), static_cast<float>(m_quarterSize.y));
							const CD3DX12_RECT rect = CD3DX12_RECT(0, 0, m_quarterSize.x, m_quarterSize.y);
							pCommandList->RSSetViewports(1, &viewport);
							pCommandList->RSSetScissorRects(1, &rect);

							{
								const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
								{
									inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_importanceMap.RTVIndex),
								};

								// drawing into importanceMap
								pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

								// select 4 deinterleaved AO textures (texture array)
								shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
								pSRV->nTexFinalSSAOIndex = m_finalResults.SRVIndex;

								FullscreenPassDraw(inputs, shader::ePS_GenerateImportanceMap);
							}

							// postprocess A
							{
								const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
								{
									inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_importanceMapPong.RTVIndex),
								};

								// drawing into importanceMap
								pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

								shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
								pSRV->nTexImportanceMapIndex = m_importanceMap.SRVIndex;

								FullscreenPassDraw(inputs, shader::ePS_PostprocessImportanceMapA);
							}

							// postprocess B
							{
								const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
								{
									inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_importanceMap.RTVIndex),
								};

								const uint32_t fourZeroes[4] = { 0, 0, 0, 0 };
								D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = inputs->pDescriptorHeapUAV->GetGPUHandleFromIndex(m_nLoadCounterDescriptorUAVIndex);
								D3D12_CPU_DESCRIPTOR_HANDLE cpuStartHandle = inputs->pDescriptorHeapUAV->GetStartCPUHandle(inputs->nFrameIndex);
								pCommandList->ClearUnorderedAccessViewUint(gpuHandle, cpuStartHandle, m_loadCounter, fourZeroes, 0, nullptr);

								pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

								// select previous pass input importance map
								shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
								pSRV->nTexImportanceMapIndex = m_importanceMap.SRVIndex;

								FullscreenPassDraw(inputs, shader::ePS_PostprocessImportanceMapB);
							}
						}
					}
#endif
					// Generate SSAO
					GenerateSSAO(config, inputs, false);

					if (inputs->OverrideOutputRTV != 0)
					{
						// drawing into OverrideOutputRTV
						const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
						{
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(inputs->OverrideOutputRTV),
						};

						pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);
					}

					// Apply
					{
						const CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_size.x), static_cast<float>(m_size.y));
						const CD3DX12_RECT rect = CD3DX12_RECT(m_fullResOutScissorRect.x, m_fullResOutScissorRect.y, m_fullResOutScissorRect.z, m_fullResOutScissorRect.w);
						pCommandList->RSSetViewports(1, &viewport);
						pCommandList->RSSetScissorRects(1, &rect);

						D3D12Texture2D::SetResourceBarrier(pCommandList,
							{
								&m_finalResults,
							}, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

						if (config.QualityLevel < 0)
						{
							shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
							pSRV->nTexFinalSSAOIndex = m_finalResults.SRVIndex;

							FullscreenPassDraw(inputs, shader::ePS_NonSmartHalfApply);
						}
						else if (config.QualityLevel == 0)
						{
							shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
							pSRV->nTexFinalSSAOIndex = m_finalResults.SRVIndex;

							FullscreenPassDraw(inputs, shader::ePS_NonSmartApply);
						}
						else
						{
							shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
							pSRV->nTexFinalSSAOIndex = m_finalResults.SRVIndex;

							FullscreenPassDraw(inputs, shader::ePS_Apply);
						}
					}
				}
			}

			bool Assao::Impl::InitializeDX()
			{
				m_pDevice = Device::GetInstance()->GetInterface();
				m_pDevice->AddRef();

				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\ASSAO\\ASSAO.hlsl");

				// shader load
				ID3DBlob* pShaderBlob = nullptr;
				if (FAILED(D3DReadFileToBlob(string::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Model.hlsl");
				}

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(m_pDevice, pShaderBlob, strShaderPath.c_str(), emPSType);
				}

				// constant buffer
				{
					m_constantsBuffer.Create(m_pDevice, eMaxConstantsBufferCount, "ASSAOConstants");
					m_srvContentsBuffer.Create(m_pDevice, eMaxSRVBufferCount, "ASSAOSRVConstants");
				}

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				// load counter stuff, only needed for adaptive quality (quality level 3)
				{
					CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT_R32_UINT, 1, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
					CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
					HRESULT hr = m_pDevice->CreateCommittedResource(&heapProperties,
						D3D12_HEAP_FLAG_NONE,
						&desc, 
						D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
						nullptr,
						IID_PPV_ARGS(&m_loadCounter));
					if (FAILED(hr))
					{
						throw_line("failed to create LoadCounter Resource");
					}

					{
						DescriptorHeap* pDescriptorHeapSRV = Device::GetInstance()->GetSRVDescriptorHeap();
						PersistentDescriptorAlloc srvAlloc = pDescriptorHeapSRV->AllocatePersistent();
						m_nLoadCounterDescriptorSRVIndex = srvAlloc.nIndex;

						D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
						srvDesc.Format = DXGI_FORMAT_R32_UINT;
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
						srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
						srvDesc.Texture1D.MipLevels = 1;
						srvDesc.Texture1D.MostDetailedMip = 0;
						srvDesc.Texture1D.ResourceMinLODClamp = 0.f;

						for (uint32_t i = 0; i < pDescriptorHeapSRV->GetHeapCount(); ++i)
						{
							m_pDevice->CreateShaderResourceView(m_loadCounter, &srvDesc, srvAlloc.cpuHandles[i]);
						}
					}

					{
						DescriptorHeap* pDescriptorHeapUAV = Device::GetInstance()->GetUAVDescriptorHeap();
						PersistentDescriptorAlloc uavAlloc = pDescriptorHeapUAV->AllocatePersistent();
						m_nLoadCounterDescriptorUAVIndex = uavAlloc.nIndex;

						D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
						uavDesc.Format = DXGI_FORMAT_R32_UINT;
						uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;

						m_pDevice->CreateUnorderedAccessView(m_loadCounter, nullptr, &uavDesc, uavAlloc.cpuHandles[0]);
					}
				}
#endif

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreateBundles(m_pDevice, emPSType);
				}

				return true;
			}

			void Assao::Impl::CleanupDX()
			{
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				DescriptorHeap* pDescriptorHeapSRV = Device::GetInstance()->GetSRVDescriptorHeap();
				DescriptorHeap* pDescriptorHeapUAV = Device::GetInstance()->GetUAVDescriptorHeap();

				if (m_nLoadCounterDescriptorSRVIndex != 0)
				{
					pDescriptorHeapSRV->FreePersistent(m_nLoadCounterDescriptorSRVIndex);
					m_nLoadCounterDescriptorSRVIndex = 0;
				}

				if (m_nLoadCounterDescriptorUAVIndex != 0)
				{
					pDescriptorHeapUAV->FreePersistent(m_nLoadCounterDescriptorUAVIndex);
					m_nLoadCounterDescriptorUAVIndex = 0;
				}
				util::ReleaseResource(m_loadCounter);
				m_loadCounter = nullptr;
#endif
				m_constantsBuffer.Destroy();
				m_srvContentsBuffer.Destroy();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					for (int j = 0; j < eFrameBufferCount; ++j)
					{
						util::ReleaseResource(m_pipelineStates[i].pBundles[j]);
						m_pipelineStates[i].pBundles[j] = nullptr;
					}

					m_pipelineStates[i].psoCache.Destroy();
				}

				SafeRelease(m_pDevice);
			}

			template<typename OutType>
			static OutType* QueryResourceInterface(ID3D12Resource* d3dResource, REFIID riid)
			{
				OutType* ret = nullptr;
				if (SUCCEEDED(d3dResource->QueryInterface(riid, (void**)&ret)))
				{
					return ret;
				}
				else
				{
					return nullptr;
				}
			}

			void Assao::Impl::UpdateTextures(const AssaoInputsDX12* inputs)
			{
				bool needsUpdate = false;

				// We've got input normals? No need to keep ours then.
				if (inputs->NormalSRV != 0)
				{
					if (m_normals.SRVIndex != 0)
					{
						needsUpdate = true;
						m_normals.Reset();
					}
				}
				else
				{
					if (m_normals.SRVIndex == 0)
					{
						needsUpdate = true;
					}
				}

				const uint32_t width = inputs->ViewportWidth;
				const uint32_t height = inputs->ViewportHeight;

				needsUpdate |= (m_size.x != width) || (m_size.y != height);

				m_size.x = width;
				m_size.y = height;
				m_halfSize.x = (width + 1) / 2;
				m_halfSize.y = (height + 1) / 2;
				m_quarterSize.x = (m_halfSize.x + 1) / 2;
				m_quarterSize.y = (m_halfSize.y + 1) / 2;

				math::UInt4 prevScissorRect = m_fullResOutScissorRect;

				if ((inputs->ScissorRight == 0) || (inputs->ScissorBottom == 0))
				{
					m_fullResOutScissorRect = math::UInt4(0, 0, width, height);
				}
				else
				{
					m_fullResOutScissorRect = math::UInt4(std::max(0u, inputs->ScissorLeft), std::max(0u, inputs->ScissorTop), std::min(width, inputs->ScissorRight), std::min(height, inputs->ScissorBottom));
				}

				needsUpdate |= prevScissorRect != m_fullResOutScissorRect;
				if (needsUpdate == false)
					return;

				m_halfResOutScissorRect = math::UInt4(m_fullResOutScissorRect.x / 2, m_fullResOutScissorRect.y / 2, (m_fullResOutScissorRect.z + 1) / 2, (m_fullResOutScissorRect.w + 1) / 2);
				const int blurEnlarge = cMaxBlurPassCount + std::max(0, cMaxBlurPassCount - 2); // +1 for max normal blurs, +2 for wide blurs
				m_halfResOutScissorRect = math::UInt4(std::max(0u, m_halfResOutScissorRect.x - blurEnlarge), std::max(0u, m_halfResOutScissorRect.y - blurEnlarge), std::min(m_halfSize.x, m_halfResOutScissorRect.z + blurEnlarge), std::min(m_halfSize.y, m_halfResOutScissorRect.w + blurEnlarge));

				float totalSizeInMB = 0.f;

				for (int i = 0; i < 4; i++)
				{
					const std::string strDebugName = string::Format("HalfDepth_%d", i);
					if (m_halfDepths[i].ReCreateIfNeeded(m_pDevice, m_halfSize, m_formats.DepthBufferViewspaceLinear, totalSizeInMB, SSAO_DEPTH_MIP_LEVELS, 1, math::Color::Transparent, false, strDebugName.c_str()))
					{
						m_halfDepths[i].vecSubResource.clear();
						m_halfDepths[i].vecSubResource.resize(SSAO_DEPTH_MIP_LEVELS);

						for (int j = 0; j < SSAO_DEPTH_MIP_LEVELS; j++)
						{
							m_halfDepths[i].vecSubResource[j].Reset();
						}

						for (int j = 0; j < SSAO_DEPTH_MIP_LEVELS; j++)
						{
							m_halfDepths[i].vecSubResource[j].ReCreateMIPViewIfNeeded(m_pDevice, m_halfDepths[i], j);
						}
					}
				}

				m_pingPongHalfResultA.ReCreateIfNeeded(m_pDevice, m_halfSize, m_formats.AOResult, totalSizeInMB, 1, 1, math::Color::White, false, "PongHalfResultA");
				m_pingPongHalfResultB.ReCreateIfNeeded(m_pDevice, m_halfSize, m_formats.AOResult, totalSizeInMB, 1, 1, math::Color::Transparent, false, "PongHalfResultB");
				m_finalResults.ReCreateIfNeeded(m_pDevice, m_halfSize, m_formats.AOResult, totalSizeInMB, 1, 4, math::Color::Transparent, false, "FinalResult");
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				m_importanceMap.ReCreateIfNeeded(m_pDevice, m_quarterSize, m_formats.ImportanceMap, totalSizeInMB, 1, 1, math::Color::Transparent, false, "ImportanceMap");
				m_importanceMapPong.ReCreateIfNeeded(m_pDevice, m_quarterSize, m_formats.ImportanceMap, totalSizeInMB, 1, 1, math::Color::Transparent, false, "ImportanceMapPong");
#endif
				m_finalResults.vecSubResource.clear();
				m_finalResults.vecSubResource.resize(4);
				for (int i = 0; i < 4; i++)
				{
					m_finalResults.vecSubResource[i].ReCreateArrayViewIfNeeded(m_pDevice, m_finalResults, i);
				}

				if (inputs->NormalSRV == 0)
				{
					m_normals.ReCreateIfNeeded(m_pDevice, m_size, m_formats.Normals, totalSizeInMB, 1, 1, math::Color::Transparent, true, "Normals");
				}

				totalSizeInMB /= 1024 * 1024;
				// m_debugInfo = vaStringTools::Format( "SSAO (approx. %.2fMB memory used) ", totalSizeInMB );

				// trigger a full buffers clear first time; only really required when using scissor rects
				m_requiresClear = true;
			}

			void Assao::Impl::UpdateConstants(const Options::AssaoConfig& config, const AssaoInputsDX12* inputs, int pass)
			{
				const uint32_t nFrameIndex = inputs->nFrameIndex;

				const bool generateNormals = inputs->NormalSRV == 0;

				// update constants
				shader::ASSAOConstants& consts = *AllocateConstantBuffer(nFrameIndex);
				{
					const math::Matrix& proj = inputs->ProjectionMatrix;

					consts.ViewportPixelSize = math::Vector2(1.0f / (float)m_size.x, 1.0f / (float)m_size.y);
					consts.HalfViewportPixelSize = math::Vector2(1.0f / (float)m_halfSize.x, 1.0f / (float)m_halfSize.y);

					consts.Viewport2xPixelSize = math::Vector2(consts.ViewportPixelSize.x* 2.0f, consts.ViewportPixelSize.y* 2.0f);
					consts.Viewport2xPixelSize_x_025 = math::Vector2(consts.Viewport2xPixelSize.x* 0.25f, consts.Viewport2xPixelSize.y* 0.25f);

					float depthLinearizeMul = (inputs->MatricesRowMajorOrder) ? (-proj.m[3][2]) : (-proj.m[2][3]); // float depthLinearizeMul = ( clipFar* clipNear ) / ( clipFar - clipNear );
					float depthLinearizeAdd = (inputs->MatricesRowMajorOrder) ? (proj.m[2][2]) : (proj.m[2][2]); // float depthLinearizeAdd = clipFar / ( clipFar - clipNear );
																												 // correct the handedness issue. need to make sure this below is correct, but I think it is.
					if (depthLinearizeMul* depthLinearizeAdd < 0)
						depthLinearizeAdd = -depthLinearizeAdd;
					consts.DepthUnpackConsts = math::Vector2(depthLinearizeMul, depthLinearizeAdd);

					float tanHalfFOVY = 1.0f / proj.m[1][1]; // = tanf( drawContext.Camera.GetYFOV( )* 0.5f );
					float tanHalfFOVX = 1.0F / proj.m[0][0]; // = tanHalfFOVY* drawContext.Camera.GetAspect( );
					consts.CameraTanHalfFOV = math::Vector2(tanHalfFOVX, tanHalfFOVY);

					consts.NDCToViewMul = math::Vector2(consts.CameraTanHalfFOV.x* 2.0f, consts.CameraTanHalfFOV.y* -2.0f);
					consts.NDCToViewAdd = math::Vector2(consts.CameraTanHalfFOV.x* -1.0f, consts.CameraTanHalfFOV.y* 1.0f);

					consts.EffectRadius = std::clamp(config.Radius, 0.0f, 100000.0f);
					consts.EffectShadowStrength = std::clamp(config.ShadowMultiplier* 4.3f, 0.0f, 10.0f);
					consts.EffectShadowPow = std::clamp(config.ShadowPower, 0.0f, 10.0f);
					consts.EffectShadowClamp = std::clamp(config.ShadowClamp, 0.0f, 1.0f);
					consts.EffectFadeOutMul = -1.0f / (config.FadeOutTo - config.FadeOutFrom);
					consts.EffectFadeOutAdd = config.FadeOutFrom / (config.FadeOutTo - config.FadeOutFrom) + 1.0f;
					consts.EffectHorizonAngleThreshold = std::clamp(config.HorizonAngleThreshold, 0.0f, 1.0f);

					// 1.2 seems to be around the best trade off - 1.0 means on-screen radius will stop/slow growing when the camera is at 1.0 distance, so, depending on FOV, basically filling up most of the screen
					// This setting is viewspace-dependent and not screen size dependent intentionally, so that when you change FOV the effect stays (relatively) similar.
					float effectSamplingRadiusNearLimit = (config.Radius* 1.2f);

					// if the depth precision is switched to 32bit float, this can be set to something closer to 1 (0.9999 is fine)
					consts.DepthPrecisionOffsetMod = 0.9992f;

					// consts.RadiusDistanceScalingFunctionPow = 1.0f - Clamp( config.RadiusDistanceScalingFunction, 0.0f, 1.0f );

					// used to get average load per pixel; 9.0 is there to compensate for only doing every 9th InterlockedAdd in PSPostprocessImportanceMapB for performance reasons
					consts.LoadCounterAvgDiv = 9.0f / (float)(m_quarterSize.x* m_quarterSize.y* 255.0);

					// Special config for lowest quality level - just nerf the effect a tiny bit
					if (config.QualityLevel <= 0)
					{
						//consts.EffectShadowStrength *= 0.9f;
						effectSamplingRadiusNearLimit *= 1.50f;

						if (config.QualityLevel < 0)
						{
							consts.EffectRadius *= 0.8f;
						}
					}
					effectSamplingRadiusNearLimit /= tanHalfFOVY; // to keep the effect same regardless of FOV

					consts.EffectSamplingRadiusNearLimitRec = 1.0f / effectSamplingRadiusNearLimit;

					consts.AdaptiveSampleCountLimit = config.AdaptiveQualityLimit;

					consts.NegRecEffectRadius = -1.0f / consts.EffectRadius;

					consts.PerPassFullResCoordOffset = math::Int2(pass % 2, pass / 2);
					consts.PerPassFullResUVOffset = math::Vector2(((pass % 2) - 0.0f) / m_size.x, ((pass / 2) - 0.0f) / m_size.y);

					consts.InvSharpness = std::clamp(1.0f - config.Sharpness, 0.0f, 1.0f);
					consts.PassIndex = pass;
					consts.QuarterResPixelSize = math::Vector2(1.0f / (float)m_quarterSize.x, 1.0f / (float)m_quarterSize.y);

					float additionalAngleOffset = config.TemporalSupersamplingAngleOffset; // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
					float additionalRadiusScale = config.TemporalSupersamplingRadiusOffset; // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
					const int subPassCount = 5;
					for (int subPass = 0; subPass < subPassCount; subPass++)
					{
						int a = pass;
						int b = subPass;

						int spmap[5]{ 0, 1, 4, 3, 2 };
						b = spmap[subPass];

						float ca, sa;
						float angle0 = ((float)a + (float)b / (float)subPassCount)* (3.1415926535897932384626433832795f)* 0.5f;
						angle0 += additionalAngleOffset;

						ca = ::cosf(angle0);
						sa = ::sinf(angle0);

						float scale = 1.0f + (a - 1.5f + (b - (subPassCount - 1.0f)* 0.5f) / (float)subPassCount)* 0.07f;
						scale *= additionalRadiusScale;

						consts.PatternRotScaleMatrices[subPass] = math::Vector4(scale* ca, scale* -sa, -scale * sa, -scale * ca);
					}

					if (generateNormals == false)
					{
						consts.NormalsUnpackMul = inputs->NormalsUnpackMul;
						consts.NormalsUnpackAdd = inputs->NormalsUnpackAdd;
					}
					else
					{
						consts.NormalsUnpackMul = 2.0f;
						consts.NormalsUnpackAdd = -1.0f;
					}
					consts.DetailAOStrength = config.DetailShadowStrength;
					consts.Dummy0 = 0.0f;

#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
					if (generateNormals == false)
					{
						consts.NormalsWorldToViewspaceMatrix = inputs->NormalsWorldToViewspaceMatrix;
						if (!inputs->MatricesRowMajorOrder)
						{
							consts.NormalsWorldToViewspaceMatrix.Transpose();
						}
					}
					else
					{
						consts.NormalsWorldToViewspaceMatrix = math::Matrix::Identity;
					}
#endif
				}
			}

			void Assao::Impl::FullscreenPassDraw(const AssaoInputsDX12* inputs, shader::PSType emPSType)
			{
				inputs->pCommandList->SetGraphicsRootSignature(m_pipelineStates[emPSType].psoCache.pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					inputs->pDescriptorHeapSRV->GetHeap(inputs->nFrameIndex),
				};
				inputs->pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				inputs->pCommandList->SetGraphicsRootConstantBufferView(eRP_ASSAOConstantsCB, m_constantsBufferGPUAddress);
				inputs->pCommandList->SetGraphicsRootConstantBufferView(eRP_ASSAOSRVContentsCB, m_srvContentsBufferGPUAddress);

				inputs->pCommandList->ExecuteBundle(m_pipelineStates[emPSType].pBundles[inputs->nFrameIndex]);
			}

			void Assao::Impl::PrepareDepths(const Options::AssaoConfig& config, const AssaoInputsDX12* inputs)
			{
				const bool generateNormals = inputs->NormalSRV == 0;

				ID3D12GraphicsCommandList2* pCommandList = inputs->pCommandList;

				{
					const CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.f, 0.f, static_cast<float>(m_halfSize.x), static_cast<float>(m_halfSize.y));
					const CD3DX12_RECT rect = CD3DX12_RECT(0, 0, m_halfSize.x, m_halfSize.y);
					pCommandList->RSSetViewports(1, &viewport);
					pCommandList->RSSetScissorRects(1, &rect); // no scissor for this
				}

				if (generateNormals == false)
				{
					if (config.QualityLevel < 0)
					{
						D3D12Texture2D::SetResourceBarrier(pCommandList,
							{
								&m_halfDepths[0],
								&m_halfDepths[3],
							}, D3D12_RESOURCE_STATE_RENDER_TARGET);

						const D3D12_CPU_DESCRIPTOR_HANDLE twoDepths[] =
						{
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[0].RTVIndex),
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[3].RTVIndex),
						};

						pCommandList->OMSetRenderTargets(_countof(twoDepths), twoDepths, FALSE, nullptr);

						shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
						pSRV->nTexDepthSourceIndex = inputs->DepthSRV;

						FullscreenPassDraw(inputs, shader::ePS_PrepareDepthsHalf);
					}
					else
					{
						D3D12Texture2D::SetResourceBarrier(pCommandList,
							{
								&m_halfDepths[0],
								&m_halfDepths[1],
								&m_halfDepths[2],
								&m_halfDepths[3],
							}, D3D12_RESOURCE_STATE_RENDER_TARGET);

						const D3D12_CPU_DESCRIPTOR_HANDLE fourDepths[] =
						{
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[0].RTVIndex),
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[1].RTVIndex),
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[2].RTVIndex),
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[3].RTVIndex),
						};

						pCommandList->OMSetRenderTargets(_countof(fourDepths), fourDepths, FALSE, nullptr);

						shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
						pSRV->nTexDepthSourceIndex = inputs->DepthSRV;

						FullscreenPassDraw(inputs, shader::ePS_PrepareDepths);
					}
				}
				else
				{
					assert(false);

					//if (config.QualityLevel < 0)
					//{
					//	shader::ASSAOSRVContents* pSRV = m_srvContentsBuffer.Cast(inputs->nFrameIndex, shader::ePS_PrepareDepthsAndNormalsHalf);
					//	pSRV->nTexDepthSourceIndex = inputs->DepthSRV;
					//
					//	pCommandList->OMSetRenderTargets(_countof(twoDepths), twoDepths, FALSE, nullptr);
					//
					//	FullscreenPassDraw(inputs, shader::ePS_PrepareDepthsAndNormalsHalf);
					//}
					//else
					//{
					//	shader::ASSAOSRVContents* pSRV = m_srvContentsBuffer.Cast(inputs->nFrameIndex, shader::ePS_PrepareDepthsAndNormals);
					//	pSRV->nTexDepthSourceIndex = inputs->DepthSRV;
					//
					//	pCommandList->OMSetRenderTargets(_countof(fourDepths), fourDepths, FALSE, nullptr);
					//
					//	FullscreenPassDraw(inputs, shader::ePS_PrepareDepthsAndNormals);
					//}
				}

				// only do mipmaps for higher quality levels (not beneficial on quality level 1, and detrimental on quality level 0)
				if (config.QualityLevel > 1)
				{
					for (int i = 1; i < SSAO_DEPTH_MIP_LEVELS; i++)
					{
						D3D12Texture2D::SetResourceBarrier(pCommandList,
							{
								&m_halfDepths[0],
								&m_halfDepths[1],
								&m_halfDepths[2],
								&m_halfDepths[3],
							}, D3D12_RESOURCE_STATE_RENDER_TARGET, i);

						const D3D12_CPU_DESCRIPTOR_HANDLE fourDepthMips[] = 
						{
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[0].vecSubResource[i].RTVIndex),
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[1].vecSubResource[i].RTVIndex),
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[2].vecSubResource[i].RTVIndex),
							inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_halfDepths[3].vecSubResource[i].RTVIndex),
						};

						pCommandList->OMSetRenderTargets(_countof(fourDepthMips), fourDepthMips, FALSE, nullptr);

						const CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_halfDepths[0].vecSubResource[i].Size.x), static_cast<float>(m_halfDepths[0].vecSubResource[i].Size.y));
						pCommandList->RSSetViewports(1, &viewport);

						shader::PSType emPSType = static_cast<shader::PSType>(shader::ePS_PrepareDepthMip1 + (i - 1));
						{
							D3D12Texture2D::SetResourceBarrier(pCommandList,
								{
									&m_halfDepths[0],
									&m_halfDepths[1],
									&m_halfDepths[2],
									&m_halfDepths[3],
								}, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, i - 1);

							shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
							pSRV->nTexViewspaceDepthSourceIndex = m_halfDepths[0].vecSubResource[i - 1].SRVIndex;
							pSRV->nTexViewspaceDepthSource1Index = m_halfDepths[1].vecSubResource[i - 1].SRVIndex;
							pSRV->nTexViewspaceDepthSource2Index = m_halfDepths[2].vecSubResource[i - 1].SRVIndex;
							pSRV->nTexViewspaceDepthSource3Index = m_halfDepths[3].vecSubResource[i - 1].SRVIndex;
						}

						FullscreenPassDraw(inputs, emPSType);
					}
				}
			}

			void Assao::Impl::GenerateSSAO(const Options::AssaoConfig& config, const AssaoInputsDX12* inputs, bool adaptiveBasePass)
			{
				const int normalmapSRV = (inputs->NormalSRV == 0) ? (m_normals.SRVIndex) : (inputs->NormalSRV);

				ID3D12GraphicsCommandList2* pCommandList = inputs->pCommandList;

				{
					const CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)m_halfSize.x, (float)m_halfSize.y);
					const CD3DX12_RECT rect = CD3DX12_RECT(m_halfResOutScissorRect.x, m_halfResOutScissorRect.y, m_halfResOutScissorRect.z, m_halfResOutScissorRect.w);
					pCommandList->RSSetViewports(1, &viewport);
					pCommandList->RSSetScissorRects(1, &rect);
				}

				if (adaptiveBasePass == true)
				{
					assert(config.QualityLevel == 3);
				}

				const int passCount = 4;

				for (int pass = 0; pass < passCount; pass++)
				{
					if ((config.QualityLevel < 0) && ((pass == 1) || (pass == 2)))
						continue;

					int blurPasses = config.BlurPassCount;
					blurPasses = std::min(blurPasses, cMaxBlurPassCount);

#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					if (config.QualityLevel == 3)
					{
						// if adaptive, at least one blur pass needed as the first pass needs to read the final texture results - kind of awkward
						if (adaptiveBasePass)
						{
							blurPasses = 0;
						}
						else
						{
							blurPasses = std::max(1, blurPasses);
						}
					}
					else
#endif
					{
						if (config.QualityLevel <= 0)
						{
							// just one blur pass allowed for minimum quality 
							blurPasses = std::min(1, config.BlurPassCount);
						}
					}

					UpdateConstants(config, inputs, pass);

					D3D12Texture2D* pPingRT = &m_pingPongHalfResultA;
					D3D12Texture2D* pPongRT = &m_pingPongHalfResultB;

					// Generate
					{
						D3D12_CPU_DESCRIPTOR_HANDLE rts[1]{};

						// no blur?
						if (blurPasses == 0)
						{
							D3D12Texture2D::SetResourceBarrier(pCommandList,
								{
									&m_finalResults,
								}, D3D12_RESOURCE_STATE_RENDER_TARGET, pass);

							rts[0] = inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_finalResults.vecSubResource[pass].RTVIndex);
						}
						else
						{
							D3D12Texture2D::SetResourceBarrier(pCommandList,
								{
									pPingRT,
								}, D3D12_RESOURCE_STATE_RENDER_TARGET);

							rts[0] = inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(pPingRT->RTVIndex);
						}

						pCommandList->OMSetRenderTargets(_countof(rts), rts, FALSE, nullptr);

						shader::PSType emPSType = static_cast<shader::PSType>(shader::ePS_GenerateQ0 + std::max(0, (!adaptiveBasePass) ? (config.QualityLevel) : (4)));
						{
							D3D12Texture2D::SetResourceBarrier(pCommandList,
								{
									&m_halfDepths[pass],
								}, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

							shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
							pSRV->nTexViewspaceDepthSourceIndex = m_halfDepths[pass].SRVIndex;
							pSRV->nTexNormalMapSourceIndex = normalmapSRV;

							// m_loadCounterSRV used only for quality level 3
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
							if (!adaptiveBasePass && (config.QualityLevel == 3))
							{
								pSRV->nTexLoadCounterIndex = m_nLoadCounterDescriptorSRVIndex;
								pSRV->nTexImportanceMapIndex = m_importanceMap.SRVIndex;
								pSRV->nTexFinalSSAOIndex = m_finalResults.SRVIndex;
							}
#endif
						}

						FullscreenPassDraw(inputs, emPSType);
					}

					// Blur
					if (blurPasses > 0)
					{
						int wideBlursRemaining = std::max(0, blurPasses - 2);

						for (int i = 0; i < blurPasses; i++)
						{
							D3D12_CPU_DESCRIPTOR_HANDLE rts[1]{};

							// last pass?
							if (i == (blurPasses - 1))
							{
								D3D12Texture2D::SetResourceBarrier(pCommandList,
									{
										&m_finalResults,
									}, D3D12_RESOURCE_STATE_RENDER_TARGET, pass);

								rts[0] = inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(m_finalResults.vecSubResource[pass].RTVIndex);
							}
							else
							{
								D3D12Texture2D::SetResourceBarrier(pCommandList,
									{
										pPongRT,
									}, D3D12_RESOURCE_STATE_RENDER_TARGET);

								rts[0] = inputs->pDescriptorHeapRTV->GetCPUHandleFromIndex(pPongRT->RTVIndex);
							}

							pCommandList->OMSetRenderTargets(_countof(rts), rts, FALSE, nullptr);

							D3D12Texture2D::SetResourceBarrier(pCommandList,
								{
									pPingRT,
								}, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

							if (config.QualityLevel > 0)
							{
								if (wideBlursRemaining > 0)
								{
									shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
									pSRV->nTexBlurInputIndex = pPingRT->SRVIndex;

									FullscreenPassDraw(inputs, shader::ePS_SmartBlurWide);
									wideBlursRemaining--;
								}
								else
								{
									shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
									pSRV->nTexBlurInputIndex = pPingRT->SRVIndex;

									FullscreenPassDraw(inputs, shader::ePS_SmartBlur);
								}
							}
							else
							{
								shader::ASSAOSRVContents* pSRV = AllocateSRVContents(inputs->nFrameIndex);
								pSRV->nTexBlurInputIndex = pPingRT->SRVIndex;

								FullscreenPassDraw(inputs, shader::ePS_NonSmartBlur);
							}

							std::swap(pPingRT, pPongRT);
						}
					}
				}
			}
			
			ID3D12RootSignature* Assao::Impl::CreateRootSignature(ID3D12Device* pDevice, shader::PSType emPSType)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& assaoConstantParameter = vecRootParameters.emplace_back();
				assaoConstantParameter.InitAsConstantBufferView(shader::eCB_ASSAOConstants, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& srvContentsParameter = vecRootParameters.emplace_back();
				srvContentsParameter.InitAsConstantBufferView(shader::eCB_ASSAOSRVContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				//if (emPSType == shader::ePS_PrepareDepthsAndNormalsHalf ||
				//	emPSType == shader::ePS_PrepareDepthsAndNormals)
				//{
				//	D3D12_ROOT_PARAMETER& normalsUAVParameter = vecRootParameters.emplace_back();
				//	normalsUAVParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
				//	normalsUAVParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				//	normalsUAVParameter.Descriptor.ShaderRegister = shader::eUAV_NormalsOutputUAV;
				//	normalsUAVParameter.Descriptor.RegisterSpace = 0;
				//}
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				else if (emPSType == shader::ePS_PostprocessImportanceMapB)
				{
					D3D12_ROOT_PARAMETER& loadCounterUAVParameter = vecRootParameters.emplace_back();
					loadCounterUAVParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
					loadCounterUAVParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
					loadCounterUAVParameter.Descriptor.ShaderRegister = shader::eUAV_LoadCounterOutputUAV;
					loadCounterUAVParameter.Descriptor.RegisterSpace = 0;
				}
#endif

				CD3DX12_STATIC_SAMPLER_DESC staticSamplerDesc[4]{};
				staticSamplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				staticSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				staticSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				staticSamplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				staticSamplerDesc[0].MipLODBias = 0;
				staticSamplerDesc[0].MaxAnisotropy = 1;
				staticSamplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				staticSamplerDesc[0].MinLOD = -FLT_MAX;
				staticSamplerDesc[0].MaxLOD = FLT_MAX;
				staticSamplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
				staticSamplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				staticSamplerDesc[0].RegisterSpace = 100;

				staticSamplerDesc[1] = staticSamplerDesc[2] = staticSamplerDesc[3] = staticSamplerDesc[0];

				staticSamplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				staticSamplerDesc[0].AddressU = staticSamplerDesc[0].AddressV = staticSamplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				staticSamplerDesc[0].ShaderRegister = 0;

				staticSamplerDesc[1].AddressU = staticSamplerDesc[1].AddressV = staticSamplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
				staticSamplerDesc[1].ShaderRegister = 1;

				staticSamplerDesc[2].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				staticSamplerDesc[2].AddressU = staticSamplerDesc[2].AddressV = staticSamplerDesc[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				staticSamplerDesc[2].ShaderRegister = 2;

				staticSamplerDesc[3].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				staticSamplerDesc[3].AddressU = staticSamplerDesc[3].AddressV = staticSamplerDesc[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				staticSamplerDesc[3].ShaderRegister = 3;

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void Assao::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath, shader::PSType emPSType)
			{
				PSOCache& psoCache = m_pipelineStates[emPSType].psoCache;

				if (pShaderBlob != nullptr)
				{
					const D3D_SHADER_MACRO macros[] =
					{
						{ "DX12", "1" },
						{ "SSAO_MAX_TAPS" , SSA_STRINGIZIZER(SSAO_MAX_TAPS) },
						{ "SSAO_ADAPTIVE_TAP_BASE_COUNT" , SSA_STRINGIZIZER(SSAO_ADAPTIVE_TAP_BASE_COUNT) },
						{ "SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT" , SSA_STRINGIZIZER(SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT) },
						{ "SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION", SSA_STRINGIZIZER(SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION) },
						{ nullptr, nullptr }
					};

					if (psoCache.pVSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, "VSMain", shader::VS_CompileVersion, &psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, shader::GetASSAOPSTypeString(emPSType), shader::PS_CompileVersion, &psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide(shader::GetASSAOPSTypeString(emPSType));
				if (psoCache.pRootSignature == nullptr)
				{
					psoCache.pRootSignature = CreateRootSignature(pDevice, emPSType);
					psoCache.pRootSignature->SetName(wstrDebugName.c_str());
				}

				if (psoCache.pPipelineState != nullptr)
				{
					util::ReleaseResource(psoCache.pPipelineState);
					psoCache.pPipelineState = nullptr;
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

				switch (emPSType)
				{
				// Opaque
				case shader::ePS_PrepareDepths:
				//case shader::ePS_PrepareDepthsAndNormals:
				case shader::ePS_PrepareDepthsHalf:
				//case shader::ePS_PrepareDepthsAndNormalsHalf:
				case shader::ePS_PrepareDepthMip1:
				case shader::ePS_PrepareDepthMip2:
				case shader::ePS_PrepareDepthMip3:
				case shader::ePS_GenerateQ0:
				case shader::ePS_GenerateQ1:
				case shader::ePS_GenerateQ2:
				case shader::ePS_GenerateQ3:
				case shader::ePS_GenerateQ3Base:
				case shader::ePS_SmartBlur:
				case shader::ePS_SmartBlurWide:
				case shader::ePS_NonSmartBlur:
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				case shader::ePS_GenerateImportanceMap:
				case shader::ePS_PostprocessImportanceMapA:
				case shader::ePS_PostprocessImportanceMapB:
#endif
					psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
					psoDesc.BlendState.RenderTarget[0].BlendEnable = false;
					break;
				// Multiply
				case shader::ePS_NonSmartHalfApply:
				case shader::ePS_NonSmartApply:
				case shader::ePS_Apply:
					psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
					psoDesc.BlendState.IndependentBlendEnable = true;
					psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
					psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
					psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
					psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
					psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_SRC_ALPHA;
					break;
				default:
					throw_line("unknown psType");
					break;
				}

				switch (emPSType)
				{
				case shader::ePS_PrepareDepths:
				//case shader::ePS_PrepareDepthsAndNormals:
				{
					psoDesc.RTVFormats[0] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.RTVFormats[1] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.RTVFormats[2] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.RTVFormats[3] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.NumRenderTargets = 4;
				}
				break;
				case shader::ePS_PrepareDepthsHalf:
				//case shader::ePS_PrepareDepthsAndNormalsHalf:
				{
					psoDesc.RTVFormats[0] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.RTVFormats[1] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.NumRenderTargets = 2;
				}
				break;
				case shader::ePS_PrepareDepthMip1:
				case shader::ePS_PrepareDepthMip2:
				case shader::ePS_PrepareDepthMip3:
				{
					psoDesc.RTVFormats[0] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.RTVFormats[1] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.RTVFormats[2] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.RTVFormats[3] = m_formats.DepthBufferViewspaceLinear;
					psoDesc.NumRenderTargets = 4;
				}
				break;
				case shader::ePS_GenerateQ0:
				case shader::ePS_GenerateQ1:
				case shader::ePS_GenerateQ2:
				case shader::ePS_GenerateQ3:
				case shader::ePS_GenerateQ3Base:
				case shader::ePS_SmartBlur:
				case shader::ePS_SmartBlurWide:
				case shader::ePS_NonSmartBlur:
				{
					psoDesc.RTVFormats[0] = m_formats.AOResult;
					psoDesc.NumRenderTargets = 1;
				}
				break;
				case shader::ePS_Apply:
				case shader::ePS_NonSmartApply:
				case shader::ePS_NonSmartHalfApply:
				{
					if (GetOptions().OnHDR == true)
					{
						psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
					}
					else
					{
						D3D12_RESOURCE_DESC desc = Device::GetInstance()->GetSwapChainRenderTarget(0)->GetDesc();
						psoDesc.RTVFormats[0] = desc.Format;
					}
					psoDesc.NumRenderTargets = 1;
				}
				break;
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
				case shader::ePS_GenerateImportanceMap:
				case shader::ePS_PostprocessImportanceMapA:
				case shader::ePS_PostprocessImportanceMapB:
				{
					psoDesc.RTVFormats[0] = m_formats.ImportanceMap;
					psoDesc.NumRenderTargets = 1;
				}
				break;
#endif
				default:
					throw_line("unknown psType");
					break;
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

			void Assao::Impl::CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType)
			{
				DescriptorHeap* pDescriptorHeapSRV = Device::GetInstance()->GetSRVDescriptorHeap();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					if (m_pipelineStates[emPSType].pBundles[i] != nullptr)
					{
						util::ReleaseResource(m_pipelineStates[emPSType].pBundles[i]);
						m_pipelineStates[emPSType].pBundles[i] = nullptr;
					}

					m_pipelineStates[emPSType].pBundles[i] = Device::GetInstance()->CreateBundle(m_pipelineStates[emPSType].psoCache.pPipelineState);
					ID3D12GraphicsCommandList2* pBundles = m_pipelineStates[emPSType].pBundles[i];

					pBundles->SetGraphicsRootSignature(m_pipelineStates[emPSType].psoCache.pRootSignature);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pDescriptorHeapSRV->GetHeap(i),
					};
					pBundles->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					pBundles->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pDescriptorHeapSRV->GetStartGPUHandle(i));

					//if (emPSType == shader::ePS_PrepareDepthsAndNormalsHalf ||
					//	emPSType == shader::ePS_PrepareDepthsAndNormals)
					//{
					//	pBundles->SetGraphicsRootUnorderedAccessView(eRP_NormalsUAV, m_normals.Resource->GetGPUVirtualAddress());
					//}
#ifdef INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
					else if (emPSType == shader::ePS_PostprocessImportanceMapB)
					{
						pBundles->SetGraphicsRootUnorderedAccessView(eRP_LoadCounterUAV, m_loadCounter->GetGPUVirtualAddress());
					}
#endif

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

			Assao::Assao()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			Assao::~Assao()
			{
			}

			void Assao::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void Assao::Apply(const Camera* pCamera, const RenderTarget* pNormalMap, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				TRACER_EVENT("ASSAO::Apply");

				Device* pDeviceInstance = Device::GetInstance();
				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				const D3D12_VIEWPORT* pViewport = Device::GetInstance()->GetViewport();

				AssaoInputsDX12 inputs;
				inputs.ScissorLeft = 0;
				inputs.ScissorTop = 0;
				inputs.ScissorRight = static_cast<int>(pViewport->Width);
				inputs.ScissorBottom = static_cast<int>(pViewport->Height);
				inputs.ProjectionMatrix = pCamera->GetProjMatrix();
				inputs.ViewportWidth = static_cast<int>(pViewport->Width);
				inputs.ViewportHeight = static_cast<int>(pViewport->Height);
				inputs.MatricesRowMajorOrder = true;
#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
				inputs.NormalsWorldToViewspaceMatrix = pCamera->GetViewMatrix().Transpose();
#endif
				inputs.pCommandList = pCommandList;
				inputs.DepthSRV = pDepth->GetTexture()->GetDescriptorIndex();
				inputs.NormalSRV = pNormalMap->GetTexture()->GetDescriptorIndex();
				inputs.OverrideOutputRTV = pResult->GetDescriptorIndex();
				inputs.nFrameIndex = pDeviceInstance->GetFrameIndex();
				inputs.pDescriptorHeapSRV = pDeviceInstance->GetSRVDescriptorHeap();
				inputs.pDescriptorHeapRTV = pDeviceInstance->GetRTVDescriptorHeap();
				inputs.pDescriptorHeapUAV = pDeviceInstance->GetUAVDescriptorHeap();

				const Options::AssaoConfig& config = GetOptions().assaoConfig;
				m_pImpl->Draw(config, &inputs);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}
		}
	}
}