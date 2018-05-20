#include "stdafx.h"
#include "UtilDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			namespace util
			{
				void GetInputElementDesc(EmVertexFormat::Type emType, const D3D11_INPUT_ELEMENT_DESC** ppInputElements_out, size_t& nElementCount_out)
				{
					switch (emType)
					{
					case EmVertexFormat::ePos:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};
						*ppInputElements_out = inputElements;
						nElementCount_out = _countof(inputElements);
					}
					break;
					case EmVertexFormat::ePos4:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};
						*ppInputElements_out = inputElements;
						nElementCount_out = _countof(inputElements);
					}
					break;
					case EmVertexFormat::ePosCol:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
						};
						*ppInputElements_out = inputElements;
						nElementCount_out = _countof(inputElements);
					}
					break;
					case EmVertexFormat::ePosTex:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};
						*ppInputElements_out = inputElements;
						nElementCount_out = _countof(inputElements);
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
						*ppInputElements_out = inputElements;
						nElementCount_out = _countof(inputElements);
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
						*ppInputElements_out = inputElements;
						nElementCount_out = _countof(inputElements);
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
						*ppInputElements_out = inputElements;
						nElementCount_out = _countof(inputElements);
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
						*ppInputElements_out = inputElements;
						nElementCount_out = _countof(inputElements);
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
						*ppInputElements_out = inputElements;
						nElementCount_out = _countof(inputElements);
					}
					break;
					default:
						*ppInputElements_out = nullptr;
						nElementCount_out = 0;
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

				bool CreateConstantBuffer(ID3D11Device* pDevice, size_t nSize, ID3D11Buffer** pBuffer_out)
				{
					D3D11_BUFFER_DESC desc{};
					desc.ByteWidth = static_cast<uint32_t>(nSize);
					desc.Usage = D3D11_USAGE_DYNAMIC;
					desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
					desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
					desc.MiscFlags = 0;
					desc.StructureByteStride = 0;

					return SUCCEEDED(pDevice->CreateBuffer(&desc, nullptr, pBuffer_out));
				}
			}
		}
	}
}