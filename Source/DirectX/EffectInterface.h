#pragma once

namespace eastengine
{
	namespace graphics
	{
		class ITexture;
		class IStructuredBuffer;
		class ISamplerState;
		class IRasterizerState;
		class IBlendState;
		class IDepthStencilState;
		class IDeviceContext;

		namespace EmVertexFormat
		{
			enum Type;
		}

		namespace EmEffectHandle
		{
			enum Type
			{
				eScalar = 0,
				eVector,
				eMatrix,
				eString,
				eShaderResource,
				eUnorderedAccessView,
				eRenderTargetView,
				eDepthStencilView,
				eConstanceBuffer,
				eBlend,
				eDepthStencil,
				eRasterizer,
				eSampler,
				eRaw,

				TypeCount,
			};
		};

		struct EffectRawHandle;
		struct EffectScalarHandle;
		struct EffectVectorHandle;
		struct EffectMatrixHandle;
		struct EffectStringHandle;
		struct EffectShaderResourceHandle;
		struct EffectUnorderedAccessViewHandle;
		struct EffectRenderTargetViewHandle;
		struct EffectDepthStencilViewHandle;
		struct EffectConstanceBufferHandle;
		struct EffectBlendHandle;
		struct EffectDepthStencilHandle;
		struct EffectRasterizerHandle;
		struct EffectSamplerHandle;

		struct ShaderMacros
		{
		private:
			struct Macro
			{
				const char* Name = nullptr;
				const char* Definition = nullptr;

				Macro(const char* strName, const char* strDefinition);
			};
			std::vector<Macro> vecMacros;

		public:
			void AddMacro(const char* strName, const char* strDefinition)
			{
				vecMacros.emplace_back(strName, strDefinition);
			}

			void EndSet()
			{
				vecMacros.emplace_back(nullptr, nullptr);
			}

			void PrintMacros(std::string& strBuf) const;

			const void* GetMacros() const;

			size_t GetMacroCount() const { return vecMacros.size(); }
			void GetMacro(size_t nIndex, std::string& strName_out, std::string& strDefinition_out) const
			{
				if (vecMacros[nIndex].Name != nullptr)
				{
					strName_out = vecMacros[nIndex].Name;
				}

				if (vecMacros[nIndex].Definition != nullptr)
				{
					strDefinition_out = vecMacros[nIndex].Definition;
				}
			}
		};

		class IEffectTech
		{
		public:
			IEffectTech() = default;
			virtual ~IEffectTech() = default;

			virtual void PassApply(uint32_t nIndex, IDeviceContext* pd3dDeviceContext, uint32_t applyFlags = 0) = 0;

			virtual void GetInputSignatureDesc(uint8_t** ppIAInputSignature_out, std::size_t& IAInputSignatureSize_out) = 0;
			virtual uint32_t GetPassCount() = 0;

		public:
			virtual const string::StringID& GetTechName() const = 0;
			virtual EmVertexFormat::Type GetLayoutFormat() const = 0;
		};

		class IEffect
		{
		protected:
			IEffect() = default;
			virtual ~IEffect() = default;

		public:
			static IEffect* Compile(const string::StringID& strName, const std::string& strPath, const ShaderMacros* pShaderMacros);
			static IEffect* CompileAsync(const string::StringID& strName, const std::string& strPath, const ShaderMacros* pShaderMacros, std::function<void(IEffect*, bool)> funcCallback);
			static IEffect* Create(const string::StringID& strName, const std::string& strPath);
			static void Destroy(IEffect** ppEffect);

		public:
			virtual bool IsValid() const = 0;
			virtual void ClearState(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech) = 0;

			virtual const string::StringID& GetName() = 0;

		public:
			virtual IEffectTech* CreateTechnique(const string::StringID& strName, EmVertexFormat::Type emLayoutFormat) = 0;
			virtual IEffectTech* GetTechnique(const string::StringID& strName) = 0;

		public:
			virtual EffectRawHandle* GetRawHandle(const string::StringID& strName) = 0;
			virtual EffectScalarHandle* GetScalarHandle(const string::StringID& strName) = 0;
			virtual EffectVectorHandle* GetVectorHandle(const string::StringID& strName) = 0;
			virtual EffectMatrixHandle* GetMatrixHandle(const string::StringID& strName) = 0;
			virtual EffectStringHandle* GetStringHandle(const string::StringID& strName) = 0;
			virtual EffectShaderResourceHandle* GetShaderResourceHandle(const string::StringID& strName) = 0;
			virtual EffectUnorderedAccessViewHandle* GetUnorderedAccessViewHandle(const string::StringID& strName) = 0;
			virtual EffectRenderTargetViewHandle* GetRenderTargetViewHandle(const string::StringID& strName) = 0;
			virtual EffectDepthStencilViewHandle* GetDepthStencilViewHandle(const string::StringID& strName) = 0;
			virtual EffectConstanceBufferHandle* GetConstanceBufferHandle(const string::StringID& strName) = 0;
			virtual EffectBlendHandle* GetBlendHandle(const string::StringID& strName) = 0;
			virtual EffectDepthStencilHandle* GetDepthStencilHandle(const string::StringID& strName) = 0;
			virtual EffectRasterizerHandle* GetRasterizerHandle(const string::StringID& strName) = 0;
			virtual EffectSamplerHandle* GetSamplerHandle(const string::StringID& strName) = 0;

		public:
			virtual void SetRawValue(const string::StringID& strName, const void* pData, uint32_t nByteOffset, uint32_t nByteCount) = 0;
			virtual void SetBool(const string::StringID& strName, bool value) = 0;
			virtual void SetBoolArray(const string::StringID& strName, bool* pValue, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetInt(const string::StringID& strName, int nValue) = 0;
			virtual void SetIntArray(const string::StringID& strName, const int* pValue, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetFloat(const string::StringID& strName, float fValue) = 0;
			virtual void SetFloatArray(const string::StringID& strName, const float* pValue, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVector(const string::StringID& strName, const math::int2& n2Value) = 0;
			virtual void SetVector(const string::StringID& strName, const math::int3& n3Value) = 0;
			virtual void SetVector(const string::StringID& strName, const math::int4& n4Value) = 0;
			virtual void SetVector(const string::StringID& strName, const math::float2& f2Value) = 0;
			virtual void SetVector(const string::StringID& strName, const math::float3& f3Value) = 0;
			virtual void SetVector(const string::StringID& strName, const math::float4& f4Value) = 0;
			virtual void SetVectorArray(const string::StringID& strName, const math::int2* pn2Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const string::StringID& strName, const math::int3* pn3Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const string::StringID& strName, const math::int4* pn4Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const string::StringID& strName, const math::float2* pf2Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const string::StringID& strName, const math::float3* pf3Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const string::StringID& strName, const math::float4* pf4Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetMatrix(const string::StringID& strName, const math::Matrix& matrix) = 0;
			virtual void SetMatrixArray(const string::StringID& strName, const math::Matrix* pMatrix, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetString(const string::StringID& strName, const char* str, uint32_t nCount) = 0;
			virtual void SetTexture(const string::StringID& strName, const std::shared_ptr<ITexture>& pTexture) = 0;
			virtual void SetTextureArray(const string::StringID& strName, const std::shared_ptr<ITexture>* pTextures, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetStructuredBuffer(const string::StringID& strName, IStructuredBuffer* pDataBuffer) = 0;
			virtual void SetUnorderedAccessView(const string::StringID& strName, IStructuredBuffer* pDataBuffer) = 0;
			virtual void SetUnorderedAccessViewArray(const string::StringID& strName, IStructuredBuffer** ppDataBuffer, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetBlendState(const string::StringID& strName, IBlendState* pBlendState, uint32_t nIndex) = 0;
			virtual void UndoBlendState(const string::StringID& strName, uint32_t nIndex) = 0;
			virtual void SetDepthStencilState(const string::StringID& strName, IDepthStencilState* pDepthStencilState, uint32_t nIndex) = 0;
			virtual void UndoDepthStencilState(const string::StringID& strName, uint32_t nIndex) = 0;
			virtual void SetRasterizerState(const string::StringID& strName, IRasterizerState* pRasterizerState, uint32_t nIndex) = 0;
			virtual void UndoRasterizerState(const string::StringID& strName, uint32_t nIndex) = 0;
			virtual void SetSamplerState(const string::StringID& strName, ISamplerState* pSamplerState, uint32_t nIndex) = 0;
			virtual void UndoSamplerState(const string::StringID& strName, uint32_t nIndex) = 0;
		};

		struct EffectHandle
		{
			virtual ~EffectHandle() = default;
			virtual EmEffectHandle::Type GetType() = 0;
		};

		struct EffectRawHandle : public EffectHandle
		{
			virtual ~EffectRawHandle() = default;
			virtual EmEffectHandle::Type GetType() { return EmEffectHandle::eRaw; }

			virtual void SetRawValue(const void* pData, uint32_t nByteOffset, uint32_t nByteCount) = 0;
		};

		struct EffectScalarHandle : public EffectHandle
		{
			virtual ~EffectScalarHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eScalar; }

			virtual void SetBool(bool value) = 0;
			virtual void SetBoolArray(bool* pValue, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetInt(int nValue) = 0;
			virtual void SetIntArray(const int* pValue, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetFloat(float fValue) = 0;
			virtual void SetFloatArray(const float* pValue, uint32_t nOffset, uint32_t nCount) = 0;
		};

		struct EffectVectorHandle : public EffectHandle
		{
			virtual ~EffectVectorHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eVector; }

			virtual void SetVector(const math::int2& n2Value) = 0;
			virtual void SetVector(const math::int3& n3Value) = 0;
			virtual void SetVector(const math::int4& n4Value) = 0;
			virtual void SetVector(const math::float2& f2Value) = 0;
			virtual void SetVector(const math::float3& f3Value) = 0;
			virtual void SetVector(const math::float4& f4Value) = 0;
			virtual void SetVectorArray(const math::int2* pn2Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const math::int3* pn3Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const math::int4* pn4Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const math::float2* pf2Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const math::float3* pf3Value, uint32_t nOffset, uint32_t nCount) = 0;
			virtual void SetVectorArray(const math::float4* pf4Value, uint32_t nOffset, uint32_t nCount) = 0;
		};

		struct EffectMatrixHandle : public EffectHandle
		{
			virtual ~EffectMatrixHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eMatrix; }

			virtual void SetMatrix(const math::Matrix& matrix) = 0;
			virtual void SetMatrixArray(const math::Matrix* pMatrix, uint32_t nOffset, uint32_t nCount) = 0;
		};

		struct EffectStringHandle : public EffectHandle
		{
			virtual ~EffectStringHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eString; }

			virtual void SetString(const char* str, uint32_t nCount) = 0;
		};

		struct EffectShaderResourceHandle : public EffectHandle
		{
			virtual ~EffectShaderResourceHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eShaderResource; }

			virtual void SetTexture(const std::shared_ptr<ITexture>& pTexture) = 0;
			virtual void SetTextureArray(const std::shared_ptr<ITexture>* pTextures, uint32_t nOffset, uint32_t nCount) = 0;

			virtual void SetStructuredBuffer(IStructuredBuffer* pDataBuffer) = 0;
		};

		struct EffectUnorderedAccessViewHandle : public EffectHandle
		{
			virtual ~EffectUnorderedAccessViewHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eUnorderedAccessView; }

			virtual void SetUnorderedAccessView(IStructuredBuffer* pDataBuffer) = 0;
			virtual void SetUnorderedAccessViewArray(IStructuredBuffer** ppDataBuffer, uint32_t nOffset, uint32_t nCount) = 0;
		};

		struct EffectRenderTargetViewHandle : public EffectHandle
		{
			virtual ~EffectRenderTargetViewHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eRenderTargetView; }
		};

		struct EffectDepthStencilViewHandle : public EffectHandle
		{
			virtual ~EffectDepthStencilViewHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eDepthStencilView; }
		};

		struct EffectConstanceBufferHandle : public EffectHandle
		{
			virtual ~EffectConstanceBufferHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eConstanceBuffer; }
		};

		struct EffectBlendHandle : public EffectHandle
		{
			virtual ~EffectBlendHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eBlend; }

			virtual void SetBlendState(IBlendState* pBlendState, uint32_t nIndex) = 0;
			virtual void UndoBlendState(uint32_t nIndex) = 0;
		};

		struct EffectDepthStencilHandle : public EffectHandle
		{
			virtual ~EffectDepthStencilHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eDepthStencil; }

			virtual void SetDepthStencilState(IDepthStencilState* pDepthStencilState, uint32_t nIndex) = 0;
			virtual void UndoDepthStencilState(uint32_t nIndex) = 0;
		};

		struct EffectRasterizerHandle : public EffectHandle
		{
			virtual ~EffectRasterizerHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eRasterizer; }

			virtual void SetRasterizerState(IRasterizerState* pRasterizerState, uint32_t nIndex) = 0;
			virtual void UndoRasterizerState(uint32_t nIndex) = 0;
		};

		struct EffectSamplerHandle : public EffectHandle
		{
			virtual ~EffectSamplerHandle() = default;
			virtual EmEffectHandle::Type GetType() override { return EmEffectHandle::eSampler; }

			virtual void SetSamplerState(ISamplerState* pSamplerState, uint32_t nIndex) = 0;
			virtual void UndoSamplerState(uint32_t nIndex) = 0;
		};
	}
}