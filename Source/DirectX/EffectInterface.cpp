#include "stdafx.h"
#include "EffectInterface.h"

#include "CommonLib/ThreadPool.h"

#include "D3DInterface.h"

#include "ShaderMgr.h"
#include "Effect.h"

#include <D3DCompiler.h>

namespace eastengine
{
	namespace graphics
	{
		ShaderMacros::Macro::Macro(const char* strName, const char* strDefinition)
			: Name(strName)
			, Definition(strDefinition)
		{
		}

		void ShaderMacros::PrintMacros(std::string& strBuf) const
		{
			if (vecMacros.empty() == true)
				return;

			for (auto& iter : vecMacros)
			{
				strBuf += String::Format("%s(%s) |", iter.Name, iter.Definition);
			}
		}

		const void* ShaderMacros::GetMacros() const
		{
			return &vecMacros.front();
		}

		ID3DX11Effect* EffectCompile(const std::string& strPath, const ShaderMacros* pShaderMacros)
		{
			ID3DX11Effect* pD3DEffect = nullptr;

			uint32_t nHLSLFlag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if defined( DEBUG ) || defined( _DEBUG )
			// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
			// Setting this flag improves the shader debugging experience, but still allows 
			// the shaders to be optimized and to run exactly the way they will run in 
			// the release configuration of this program.
			nHLSLFlag |= D3DCOMPILE_DEBUG;
#endif
			const D3D_SHADER_MACRO* pMacros = pShaderMacros == nullptr ? nullptr : reinterpret_cast<const D3D_SHADER_MACRO*>(pShaderMacros->GetMacros());

			ID3DBlob* pError = nullptr;
			if (FAILED(D3DX11CompileEffectFromFile(String::MultiToWide(strPath).c_str(), pMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE, nHLSLFlag, 0, GetDevice()->GetInterface(), &pD3DEffect, &pError)))
			{
				std::string str;
				if (pShaderMacros != nullptr)
				{
					pShaderMacros->PrintMacros(str);
				}

				if (pError != nullptr)
				{
					LOG_ERROR("%s : %s", pError->GetBufferPointer(), str.c_str());
				}
				else
				{
					LOG_ERROR("Unknown Error : %s", pError->GetBufferPointer(), str.c_str());
				}
			}

			SafeRelease(pError);

			return pD3DEffect;
		}

		IEffect* IEffect::Compile(const String::StringID& strName, const std::string& strPath, const ShaderMacros* pShaderMacros)
		{
			IEffect* pEffect = ShaderManager::GetInstance()->GetEffect(strName);
			if (pEffect != nullptr)
				return pEffect;

			ID3DX11Effect* pD3DEffect = EffectCompile(strPath, pShaderMacros);
			if (pD3DEffect != nullptr)
			{
				if (pD3DEffect->IsValid() == false)
				{
					LOG_WARNING("Invalid Effect : %s, %s", strName.c_str(), strPath.c_str());
					SafeRelease(pD3DEffect);
					return nullptr;
				}

				Effect* pNewEffect = new Effect(strName);
				pNewEffect->Init(pD3DEffect);

				ShaderManager::GetInstance()->AddEffect(pNewEffect);

				return pNewEffect;
			}

			return nullptr;
		}

		IEffect* IEffect::CompileAsync(const String::StringID& strName, const std::string& strPath, const ShaderMacros* pShaderMacros, std::function<void(IEffect*, bool)> funcCallback)
		{
			IEffect* pEffect = ShaderManager::GetInstance()->GetEffect(strName);
			if (pEffect != nullptr)
				return pEffect;

			Effect* pNewEffect = new Effect(strName);

			ShaderManager::GetInstance()->AddEffect(pNewEffect);

			struct AsyncMacro
			{
				std::string Name;
				std::string Definition;
			};
			std::vector<AsyncMacro> vecMacros;

			const size_t nMacroCount = pShaderMacros->GetMacroCount();
			vecMacros.reserve(nMacroCount);
			for (size_t i = 0; i < nMacroCount; ++i)
			{
				AsyncMacro& asyncMacro = vecMacros.emplace_back();

				pShaderMacros->GetMacro(i, asyncMacro.Name, asyncMacro.Definition);
			}

			thread::CreateTask([pNewEffect, strName, strPath, vecMacros, funcCallback]()
			{
				ShaderMacros macros;
				const size_t nSize = vecMacros.size();
				for (size_t i = 0; i < nSize; ++i)
				{
					const char* Name = vecMacros[i].Name.empty() == false ? vecMacros[i].Name.c_str() : nullptr;
					const char* Definition = vecMacros[i].Definition.empty() == false ? vecMacros[i].Definition.c_str() : nullptr;
					macros.AddMacro(Name, Definition);
				}

				bool isSuccess = false;
				ID3DX11Effect* pD3DEffect = EffectCompile(strPath, &macros);
				if (pD3DEffect != nullptr)
				{
					if (pD3DEffect->IsValid() == false)
					{
						isSuccess = false;

						LOG_WARNING("Invalid Effect : %s, %s", strName.c_str(), strPath.c_str());
						SafeRelease(pD3DEffect);
					}
					else
					{
						pNewEffect->Init(pD3DEffect);
						isSuccess = true;
					}
				}

				ShaderManager::GetInstance()->CompleteEffectAsyncLoad(pNewEffect, isSuccess, funcCallback);
			});

			return pNewEffect;
		}

		IEffect* IEffect::Create(const String::StringID& strName, const std::string& strPath)
		{
			IEffect* pEffect = ShaderManager::GetInstance()->GetEffect(strName);
			if (pEffect != nullptr)
				return pEffect;

			ID3DX11Effect* pD3DEffect = nullptr;

			ID3DBlob* pBlob = nullptr;
			std::wstring wstrPath(String::MultiToWide(strPath));
			if (FAILED(D3DReadFileToBlob(wstrPath.c_str(), &pBlob)))
			{
				LOG_ERROR("Cant Load Effect File : %s, %s", strName.c_str(), strPath.c_str());
				return nullptr;
			}

			if (FAILED(D3DX11CreateEffectFromMemory(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), 0, GetDevice()->GetInterface(), &pD3DEffect, strName.c_str())))
			{
				LOG_ERROR("Create Fail Effect File : %s, %s", strName.c_str(), strPath.c_str());
				return nullptr;
			}

			if (pD3DEffect != nullptr)
			{
				if (pD3DEffect->IsValid() == false)
				{
					LOG_WARNING("Invalid Effect : %s, %s", strName.c_str(), strPath.c_str());
					SafeRelease(pD3DEffect);
					return nullptr;
				}

				Effect* pNewEffect = new Effect(strName);
				pNewEffect->Init(pD3DEffect);

				ShaderManager::GetInstance()->AddEffect(pNewEffect);

				return pNewEffect;
			}

			return nullptr;
		}

		void IEffect::Destroy(IEffect** ppEffect)
		{
			if (ppEffect == nullptr || *ppEffect == nullptr)
				return;

			ShaderManager::GetInstance()->RemoveEffect(*ppEffect);
		}
	}
}