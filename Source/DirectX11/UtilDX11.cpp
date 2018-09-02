#include "stdafx.h"
#include "UtilDX11.h"

#include "DeviceDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			namespace util
			{
				DXProfiler::DXProfiler(const wchar_t* str)
				{
					Device::GetInstance()->GetUserDefinedAnnotation()->BeginEvent(str);
				}

				DXProfiler::~DXProfiler()
				{
					Device::GetInstance()->GetUserDefinedAnnotation()->EndEvent();
				}

				void GetInputElementDesc(EmVertexFormat::Type emType, const D3D11_INPUT_ELEMENT_DESC** ppInputElements_out, size_t* pElementCount_out)
				{
					switch (emType)
					{
					case EmVertexFormat::ePos:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePos4:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosCol:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTex:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexCol:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexNor:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexNorCol:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexNorTanBin:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexNorWeiIdx:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					default:
						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = nullptr;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = 0;
						}
						break;
					}
				}

				bool CompileShader(ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pDefines, const char* strSourceName, const char* strFunctionName, const char* strProfile, ID3DBlob** ppCompliedShaderBlob_out)
				{
					uint32_t CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if defined( DEBUG ) || defined( _DEBUG )
					CompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
					CompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

					ID3DBlob* pErrorBlob = nullptr;
					bool isSuccess = SUCCEEDED(D3DCompile(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), strSourceName, pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, strFunctionName, strProfile, CompileFlags, 0, ppCompliedShaderBlob_out, &pErrorBlob));
					if (isSuccess == false)
					{
						LOG_ERROR("failed to compile shader : %s", static_cast<const char*>(pErrorBlob->GetBufferPointer()));
					}
					SafeRelease(pErrorBlob);

					return isSuccess;
				}

				bool CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const char* strShaderPath, const char* strFunctionName, const char* strProfile, ID3D11VertexShader** ppVertexShader, const char* strDebugName)
				{
					ID3DBlob* pVertexShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, pMacros, strShaderPath, strFunctionName, strProfile, &pVertexShaderBlob) == false)
						return false;

					HRESULT hr = pDevice->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), nullptr, ppVertexShader);
					
					SafeRelease(pVertexShaderBlob);

					if (FAILED(hr))
						return false;

					util::SetDebugName(*ppVertexShader, strDebugName);

					return true;
				}

				bool CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const char* strShaderPath, const char* strFunctionName, const char* strProfile, ID3D11VertexShader** ppVertexShader, const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, size_t NumElements, ID3D11InputLayout** ppInputLayout, const char* strDebugName)
				{
					ID3DBlob* pVertexShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, pMacros, strShaderPath, strFunctionName, strProfile, &pVertexShaderBlob) == false)
						return false;

					HRESULT hr = pDevice->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), nullptr, ppVertexShader);
					if (FAILED(hr))
					{
						SafeRelease(pVertexShaderBlob);
						return false;
					}

					hr = pDevice->CreateInputLayout(pInputElementDescs, static_cast<uint32_t>(NumElements), pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), ppInputLayout);
					if (FAILED(hr))
					{
						SafeRelease((*ppVertexShader));
						SafeRelease(pVertexShaderBlob);
						return false;
					}

					util::SetDebugName(*ppVertexShader, strDebugName);
					util::SetDebugName(*ppInputLayout, strDebugName);

					SafeRelease(pVertexShaderBlob);

					return true;
				}

				bool CreatePixelShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const char* strShaderPath, const char* strFunctionName, const char* strProfile, ID3D11PixelShader** ppPixelShader, const char* strDebugName)
				{
					ID3DBlob* pPixelShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, pMacros, strShaderPath, strFunctionName, strProfile, &pPixelShaderBlob) == false)
						return false;

					HRESULT hr = pDevice->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize(), nullptr, ppPixelShader);

					SafeRelease(pPixelShaderBlob);

					if (FAILED(hr))
						return false;

					util::SetDebugName(*ppPixelShader, strDebugName);

					return true;
				}

				bool CreateConstantBuffer(ID3D11Device* pDevice, size_t nSize, ID3D11Buffer** pBuffer_out, const char* strDebugName)
				{
					D3D11_BUFFER_DESC desc{};
					desc.ByteWidth = static_cast<uint32_t>(nSize);
					desc.Usage = D3D11_USAGE_DYNAMIC;
					desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
					desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
					desc.MiscFlags = 0;
					desc.StructureByteStride = 0;

					HRESULT hr = pDevice->CreateBuffer(&desc, nullptr, pBuffer_out);
					if (FAILED(hr))
						return false;

					SetDebugName(*pBuffer_out, strDebugName);

					return true;
				}

				int GetPixelSizeInBytes(DXGI_FORMAT val)
				{
					switch (val)
					{
					case DXGI_FORMAT_UNKNOWN: return 0;
					case DXGI_FORMAT_R32G32B32A32_TYPELESS: return 4 * 4;
					case DXGI_FORMAT_R32G32B32A32_FLOAT: return 4 * 4;
					case DXGI_FORMAT_R32G32B32A32_UINT: return 4 * 4;
					case DXGI_FORMAT_R32G32B32A32_SINT: return 4 * 4;
					case DXGI_FORMAT_R32G32B32_TYPELESS: return 3 * 4;
					case DXGI_FORMAT_R32G32B32_FLOAT: return 3 * 4;
					case DXGI_FORMAT_R32G32B32_UINT: return 3 * 4;
					case DXGI_FORMAT_R32G32B32_SINT: return 3 * 4;
					case DXGI_FORMAT_R16G16B16A16_TYPELESS: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_FLOAT: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_UNORM: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_UINT: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_SNORM: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_SINT: return 4 * 2;
					case DXGI_FORMAT_R32G32_TYPELESS: return 2 * 4;
					case DXGI_FORMAT_R32G32_FLOAT: return 2 * 4;
					case DXGI_FORMAT_R32G32_UINT: return 2 * 4;
					case DXGI_FORMAT_R32G32_SINT: return 2 * 4;
					case DXGI_FORMAT_R32G8X24_TYPELESS: return 4 + 1;
					case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return 4 + 1;
					case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return 4 + 1;
					case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return 4 + 1;
					case DXGI_FORMAT_R10G10B10A2_TYPELESS: return 4;
					case DXGI_FORMAT_R10G10B10A2_UNORM: return 4;
					case DXGI_FORMAT_R10G10B10A2_UINT: return 4;
					case DXGI_FORMAT_R11G11B10_FLOAT: return 4;
					case DXGI_FORMAT_R8G8B8A8_TYPELESS: return 4;
					case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
					case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return 4;
					case DXGI_FORMAT_R8G8B8A8_UINT: return 4;
					case DXGI_FORMAT_R8G8B8A8_SNORM: return 4;
					case DXGI_FORMAT_R8G8B8A8_SINT: return 4;
					case DXGI_FORMAT_R16G16_TYPELESS: return 4;
					case DXGI_FORMAT_R16G16_FLOAT: return 4;
					case DXGI_FORMAT_R16G16_UNORM: return 4;
					case DXGI_FORMAT_R16G16_UINT: return 4;
					case DXGI_FORMAT_R16G16_SNORM: return 4;
					case DXGI_FORMAT_R16G16_SINT: return 4;
					case DXGI_FORMAT_R32_TYPELESS: return 4;
					case DXGI_FORMAT_D32_FLOAT: return 4;
					case DXGI_FORMAT_R32_FLOAT: return 4;
					case DXGI_FORMAT_R32_UINT: return 4;
					case DXGI_FORMAT_R32_SINT: return 4;
					case DXGI_FORMAT_R24G8_TYPELESS: return 4;
					case DXGI_FORMAT_D24_UNORM_S8_UINT: return 4;
					case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return 4;
					case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return 4;
					case DXGI_FORMAT_R8G8_TYPELESS: return 2;
					case DXGI_FORMAT_R8G8_UNORM: return 2;
					case DXGI_FORMAT_R8G8_UINT: return 2;
					case DXGI_FORMAT_R8G8_SNORM: return 2;
					case DXGI_FORMAT_R8G8_SINT: return 2;
					case DXGI_FORMAT_R16_TYPELESS: return 2;
					case DXGI_FORMAT_R16_FLOAT: return 2;
					case DXGI_FORMAT_D16_UNORM: return 2;
					case DXGI_FORMAT_R16_UNORM: return 2;
					case DXGI_FORMAT_R16_UINT: return 2;
					case DXGI_FORMAT_R16_SNORM: return 2;
					case DXGI_FORMAT_R16_SINT: return 2;
					case DXGI_FORMAT_R8_TYPELESS: return 1;
					case DXGI_FORMAT_R8_UNORM: return 1;
					case DXGI_FORMAT_R8_UINT: return 1;
					case DXGI_FORMAT_R8_SNORM: return 1;
					case DXGI_FORMAT_R8_SINT: return 1;
					case DXGI_FORMAT_A8_UNORM: return 1;
					case DXGI_FORMAT_R1_UNORM: return 1;
					case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return 4;
					case DXGI_FORMAT_R8G8_B8G8_UNORM: return 4;
					case DXGI_FORMAT_G8R8_G8B8_UNORM: return 4;
					case DXGI_FORMAT_BC1_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC1_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC1_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC2_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC2_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC2_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC3_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC3_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC3_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC4_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC4_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC4_SNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC5_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC5_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC5_SNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_B5G6R5_UNORM: return 2;
					case DXGI_FORMAT_B5G5R5A1_UNORM: return 2;
					case DXGI_FORMAT_B8G8R8A8_UNORM: return 4;
					case DXGI_FORMAT_B8G8R8X8_UNORM: return 4;
					case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return 4;
					case DXGI_FORMAT_B8G8R8A8_TYPELESS: return 4;
					case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return 4;
					case DXGI_FORMAT_B8G8R8X8_TYPELESS: return 4;
					case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return 4;
					case DXGI_FORMAT_BC6H_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC6H_UF16: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC6H_SF16: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC7_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC7_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC7_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_AYUV: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_Y410: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_Y416: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_NV12: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_P010: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_P016: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_YUY2: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_Y210: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_Y216: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_NV11: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_AI44: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_IA44: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_P8: return 1;
					case DXGI_FORMAT_A8P8: return 2;
					case DXGI_FORMAT_B4G4R4A4_UNORM: return 2;
					default: break;
					}
					assert(false);
					return 0;
				}
			}
		}
	}
}