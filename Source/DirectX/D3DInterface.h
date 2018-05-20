#pragma once

#include "Resource.h"
#include "d3dDefine.h"
#include "Vertex.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11Texture1D;
struct ID3D11Texture2D;
struct ID3D11Texture3D;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11SamplerState;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
struct ID3DUserDefinedAnnotation;

struct D3D11_BOX;
struct D3D11_SUBRESOURCE_DATA;
struct D3D11_MAPPED_SUBRESOURCE;

namespace eastengine
{
	namespace graphics
	{
		namespace EmVertexFormat
		{
			enum Type;
		}

		struct VertexPos;
		struct TextureDesc1D;
		struct TextureDesc2D;
		struct TextureDesc3D;

		class IDevice;
		class IDeviceContext;
		class IDeferredContext;
		class IGBuffers;
		class IImageBasedLight;
		class ITexture;
		class IVertexBuffer;
		class IIndexBuffer;
		class IRenderTarget;
		class IDepthStencil;
		class ISamplerState;
		class IBlendState;
		class IRasterizerState;
		class IDepthStencilState;

		IDevice* GetDevice();
		IDeviceContext* GetImmediateContext();
		IDeviceContext* GetDeferredContext(ThreadType emThreadType);
		IGBuffers* GetGBuffers();
		IImageBasedLight* GetImageBasedLight();

		int GetThreadID(ThreadType emThreadType);

		class IDevice
		{
		protected:
			IDevice() = default;
			virtual ~IDevice() = default;

		public:
			virtual ID3D11Device* GetInterface() = 0;

		public:
			virtual HRESULT CreateInputLayout(EmVertexFormat::Type emInputLayout, const uint8_t* pIAInputSignature, std::size_t IAInputSignatureSize, ID3D11InputLayout** ppInputLayout = nullptr) = 0;
			virtual HRESULT CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) = 0;
			virtual HRESULT CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView) = 0;
			virtual HRESULT CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D) = 0;
			virtual HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) = 0;
			virtual HRESULT CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D) = 0;
			virtual HRESULT CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView) = 0;
			virtual HRESULT CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView) = 0;
			virtual HRESULT CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppShaderResourceView) = 0;
			virtual HRESULT CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState) = 0;
			virtual HRESULT CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState **ppBlendState) = 0;
			virtual HRESULT CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState) = 0;
			virtual HRESULT CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) = 0;

		public:
			virtual IDeviceContext* GetImmediateContext() = 0;

			virtual int GetThreadID(ThreadType emThreadType) const = 0;
			virtual IDeviceContext* GetDeferredContext(int nThreadID) = 0;

			virtual IGBuffers* GetGBuffers() = 0;
			virtual IImageBasedLight* GetImageBasedLight() = 0;
			virtual IRenderTarget* GetMainRenderTarget() = 0;
			virtual IDepthStencil* GetMainDepthStencil() = 0;
			virtual IRenderTarget* GetLastUseRenderTarget() = 0;

			// required call function ReleaseRenderTargets
			virtual IRenderTarget* GetRenderTarget(const RenderTargetDesc2D& renderTargetDesc, bool isIncludeLastUseRenderTarget = true) = 0;
			virtual IRenderTarget* GetRenderTarget(const RenderTargetKey& renderTargetKey, bool isIncludeLastUseRenderTarget = true) = 0;
			virtual void ReleaseRenderTargets(IRenderTarget** ppRenderTarget, uint32_t nSize = 1, bool isSetLastRenderTarget = true) = 0;

			virtual ID3D11InputLayout* GetInputLayout(EmVertexFormat::Type emVertexFormat) = 0;

			virtual const SamplerStateDesc& GetSamplerStateDesc(EmSamplerState::Type emSamplerState) = 0;
			virtual ISamplerState* GetSamplerState(const SamplerStateKey& key) = 0;
			virtual ISamplerState* GetSamplerState(const SamplerStateDesc& samplerStateDesc) = 0;
			virtual ISamplerState* GetSamplerState(EmSamplerState::Type emSamplerState) = 0;

			virtual const BlendStateDesc& GetBlendStateDesc(EmBlendState::Type emBlendState) = 0;
			virtual IBlendState* GetBlendState(const BlendStateKey& key) = 0;
			virtual IBlendState* GetBlendState(const BlendStateDesc& blendStateDesc) = 0;
			virtual IBlendState* GetBlendState(EmBlendState::Type emBlendState) = 0;

			virtual const RasterizerStateDesc& GetRasterizerStateDesc(EmRasterizerState::Type emRasterizerState) = 0;
			virtual IRasterizerState* GetRasterizerState(const RasterizerStateKey& key) = 0;
			virtual IRasterizerState* GetRasterizerState(const RasterizerStateDesc& rasterizerStateDesc) = 0;
			virtual IRasterizerState* GetRasterizerState(EmRasterizerState::Type emRasterizerState) = 0;

			virtual const DepthStencilStateDesc& GetDepthStencilStateDesc(EmDepthStencilState::Type emDepthStencilState) = 0;
			virtual IDepthStencilState* GetDepthStencilState(const DepthStencilStateKey& key) = 0;
			virtual IDepthStencilState* GetDepthStencilState(const DepthStencilStateDesc& depthStencilStateDesc) = 0;
			virtual IDepthStencilState* GetDepthStencilState(EmDepthStencilState::Type emDepthStencilState) = 0;

			virtual const math::Viewport& GetViewport() = 0;

		public:
			virtual void SetDebugName(ID3D11DeviceChild* pResource, const std::string& strName) = 0;

		public:
			virtual HWND GetHWND() = 0;
			virtual const math::UInt2& GetScreenSize() const = 0;
			virtual bool IsFullScreen() const = 0;
			virtual bool IsVSync() const = 0;
			virtual void SetVSync(bool isVSync) = 0;

			virtual void GetVideoCardInfo(std::string& strCardName, int& nMemory) const = 0;
		};

		class IDeviceContext
		{
		protected:
			IDeviceContext() = default;
			virtual ~IDeviceContext() = default;

		public:
			virtual ID3D11DeviceContext* GetInterface() = 0;

		public:
			virtual void ClearState() = 0;

			virtual void ClearRenderTargetView(IRenderTarget* pRenderTarget, const math::Color& color) = 0;
			virtual void ClearDepthStencilView(IDepthStencil* pDepthStencil, uint32_t clearFlag = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL) = 0;
			virtual void ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView* pUnorderedAccessView, const math::UInt4& n4Uint) = 0;

		public:
			virtual void DrawAuto() = 0;
			virtual void Draw(uint32_t nVertexCount, uint32_t nStartVertexLocation) = 0;
			virtual void DrawInstanced(uint32_t nVertexCountPerInstance, uint32_t nInstanceCount, uint32_t nStartVertexLocation, uint32_t nStartInstanceLocation) = 0;
			virtual void DrawIndexed(uint32_t nIndexCount, uint32_t nStartIndexLocation, uint32_t nBaseVertexLocation) = 0;
			virtual void DrawIndexedInstanced(uint32_t nIndexCountPerInstance, uint32_t nInstanceCount, uint32_t nStartIndexLocation, int nBaseVertexLocation, uint32_t nStartInstanceLocation) = 0;
			virtual void Dispatch(uint32_t nThreadGroupCountX, uint32_t nThreadGroupCountY, uint32_t nThreadGroupCountZ) = 0;

		public:
			virtual bool SetInputLayout(EmVertexFormat::Type emVertexFormat) = 0;
			virtual bool SetInputLayout(ID3D11InputLayout* pInputLayout) = 0;

			virtual void SetBlendState(EmBlendState::Type emBlendState, const math::Vector4& f4BlendFactor = math::Vector4::Zero, uint32_t nSimpleMask = 0xffffffff) = 0;
			virtual void SetBlendState(const IBlendState* pBlendState, const math::Vector4& f4BlendFactor = math::Vector4::Zero, uint32_t nSimpleMask = 0xffffffff) = 0;

			virtual void SetDepthStencilState(EmDepthStencilState::Type emDepthStencil, uint32_t nStencilRef = 1) = 0;
			virtual void SetDepthStencilState(const IDepthStencilState* pDepthStencilState, uint32_t nStencilRef = 1) = 0;

			virtual void SetRasterizerState(EmRasterizerState::Type emRasterizerState) = 0;
			virtual void SetRasterizerState(const IRasterizerState* pRasterizerState) = 0;

			virtual void SetVertexBuffers(const IVertexBuffer* pVertexBuffer, uint32_t stride, uint32_t offset, uint32_t nStartSlot = 0, uint32_t numBuffers = 1) = 0;
			virtual void SetIndexBuffer(const IIndexBuffer* pIndexBuffer, uint32_t offset) = 0;

			virtual void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology) = 0;
			virtual void SetScissorRects(math::Rect* pRects, uint32_t nNumRects) = 0;

			virtual void SetRenderTargets(const RenderTargetDesc2D& renderTargetInfo, const IDepthStencil* pDepthStencil = nullptr) = 0;
			virtual void SetRenderTargets(IRenderTarget** ppRenderTarget, uint32_t nSize, const IDepthStencil* pDepthStencil = nullptr) = 0;
			virtual void SetRenderTargetsAndUnorderedAccessViews(IRenderTarget** ppRenderTarget, uint32_t nSize, IDepthStencil* pDepthStencil, uint32_t nUAVStartSlot, uint32_t nUAVCount, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const uint32_t* pUAVInitialCounts) = 0;

			virtual void SetViewport(const math::Viewport& viewport) = 0;
			virtual void SetViewport(const math::Viewport* pViewports, uint32_t nCount) = 0;
			virtual void SetDefaultViewport() = 0;

		public:
			virtual void CopySubresourceRegion(ID3D11Resource* pDstResource, uint32_t DstSubresource, uint32_t DstX, uint32_t DstY, uint32_t DstZ, ID3D11Resource* pSrcResource, uint32_t SrcSubresource, const D3D11_BOX* pSrcBox) = 0;

			virtual void UpdateSubresource(ID3D11Resource* pDstResource, uint32_t DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, uint32_t SrcRowPitch, uint32_t SrcDepthPitch) = 0;

			virtual HRESULT Map(ID3D11Resource* pResource, uint32_t Subresource, D3D11_MAP MapType, uint32_t MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource) = 0;
			virtual void Unmap(ID3D11Resource* pResource, uint32_t Subresource) = 0;

			virtual void GenerateMips(ID3D11ShaderResourceView* pShaderResourceView) = 0;

		public:
			virtual ID3DUserDefinedAnnotation* GetUserDefineAnnotation() = 0;
		};
		
		class D3DProfiler
		{
		public:
			D3DProfiler(IDeviceContext* pDeviceContext, const wchar_t* str);
			~D3DProfiler();

		private:
			IDeviceContext* m_pDeviceContext;
		};
		D3DProfiler D3DProfiling(IDeviceContext* pDeviceContext, const wchar_t* strBeginEvent);
#define D3D_PROFILING(pDeviceContext, name)	auto profiler_##name = eastengine::graphics::D3DProfiling(pDeviceContext, L#name)

		class IDeferredContext
		{
		protected:
			IDeferredContext() = default;
			virtual ~IDeferredContext() = default;

		public:
			virtual bool FinishCommandList() = 0;
			virtual void ExecuteCommandList(IDeviceContext* pImmediateContext) = 0;
		};

		class IGBuffers
		{
		protected:
			IGBuffers() = default;
			virtual ~IGBuffers() = default;

		public:
			virtual IRenderTarget* GetGBuffer(EmGBuffer::Type emType) = 0;
			virtual IRenderTarget** GetGBuffers() = 0;
		};

		class IImageBasedLight
		{
		protected:
			IImageBasedLight() = default;
			virtual ~IImageBasedLight() = default;

		public:
			virtual const std::shared_ptr<ITexture>& GetEnvHDR() const = 0;
			virtual void SetEnvHDR(const std::shared_ptr<ITexture>& pEnvHDR) = 0;

			virtual const std::shared_ptr<ITexture>& GetDiffuseHDR() const = 0;
			virtual void SetDiffuseHDR(const std::shared_ptr<ITexture>& pDiffuseHDR) = 0;

			virtual const std::shared_ptr<ITexture>& GetSpecularHDR() const = 0;
			virtual void SetSpecularHDR(const std::shared_ptr<ITexture>& pSpecularHDR) = 0;

			virtual const std::shared_ptr<ITexture>& GetSpecularBRDF() const = 0;
			virtual void SetSpecularBRDF(const std::shared_ptr<ITexture>& pSpecularBRDF) = 0;
		};

		class ITexture : public IResource
		{
		protected:
			ITexture() = default;
			virtual ~ITexture() = default;

		public:
			struct tKey {};
			using Key = PhantomType<tKey, const String::StringKey>;

			virtual Key GetKey() const = 0;

		public:
			static std::shared_ptr<ITexture> Create(const String::StringID& strName, ID3D11Texture2D* pTexture2D, const TextureDesc2D* pCustomDesc2D = nullptr);
			static std::shared_ptr<ITexture> Create(const String::StringID& strName, const TextureDesc1D& desc, D3D11_SUBRESOURCE_DATA* pData = nullptr);
			static std::shared_ptr<ITexture> Create(const String::StringID& strName, const TextureDesc2D& desc, D3D11_SUBRESOURCE_DATA* pData = nullptr);
			static std::shared_ptr<ITexture> Create(const String::StringID& strName, const TextureDesc3D& desc, D3D11_SUBRESOURCE_DATA* pData = nullptr);
			static std::shared_ptr<ITexture> Create(const std::string& strFilePath, bool isThreadLoad = true);

		public:
			virtual void CopySubresourceRegion(ThreadType emThreadID, uint32_t DstSubresource, uint32_t DstX, uint32_t DstY, uint32_t DstZ, ITexture* pSrcResource, uint32_t SrcSubresource, const D3D11_BOX* pSrcBox) = 0;

			virtual bool Map(ThreadType emThreadID, uint32_t Subresource, D3D11_MAP emMap, void** ppData) const = 0;
			virtual void Unmap(ThreadType emThreadID, uint32_t Subresource) const = 0;

		public:
			virtual ID3D11Texture1D* GetTexture1D() = 0;
			virtual ID3D11Texture2D* GetTexture2D() = 0;
			virtual ID3D11Texture3D* GetTexture3D() = 0;
			virtual ID3D11ShaderResourceView* GetShaderResourceView() = 0;
			virtual ID3D11ShaderResourceView** GetShaderResourceViewPtr() = 0;

			virtual const math::UInt2& GetSize() = 0;
			virtual const String::StringID& GetName() = 0;
		};

		class IVertexBuffer
		{
		public:
			enum Option
			{
				eNone = 0,
				eSaveRawValue,
				eSaveVertexPos,
				//eSaveVertexClipSpace,
			};

		public:
			IVertexBuffer() = default;
			virtual ~IVertexBuffer() = default;

			static IVertexBuffer* Create(EmVertexFormat::Type emVertexFormat, size_t nElementCount, const void* pData, D3D11_USAGE emUsage = D3D11_USAGE_DEFAULT, uint32_t nOptions = IVertexBuffer::Option::eNone);

		public:
			virtual ID3D11Buffer* const* GetBufferPtr() const = 0;
			virtual size_t GetFormatSize() const = 0;
			virtual EmVertexFormat::Type GetFormat() const = 0;
			virtual size_t GetVertexNum() const = 0;

			virtual bool Map(ThreadType emThreadID, uint32_t Subresource, D3D11_MAP emMap, void** ppData) const = 0;
			virtual void Unmap(ThreadType emThreadID, uint32_t Subresource) const = 0;

			virtual const void* GetRawValuePtr() const = 0;
			virtual const VertexPos* GetVertexPosPtr() const = 0;
			//virtual const VertexClipSpace* GetVertexClipSpace() const = 0;
		};

		class IIndexBuffer
		{
		public:
			enum Option
			{
				eNone = 0,
				eSaveRawValue,
			};

		public:
			IIndexBuffer() = default;
			virtual ~IIndexBuffer() = default;

			static IIndexBuffer* Create(size_t nElementCount, const uint32_t* pData, D3D11_USAGE emUsage = D3D11_USAGE_DEFAULT, uint32_t nOptions = IIndexBuffer::Option::eNone);

		public:
			virtual ID3D11Buffer* GetBuffer() const = 0;
			virtual size_t GetFormatSize() const = 0;
			virtual size_t GetFormat() const = 0;
			virtual size_t GetIndexNum() const = 0;

			virtual bool Map(ThreadType emThreadID, uint32_t Subresource, D3D11_MAP emMap, void** ppData) const = 0;
			virtual void Unmap(ThreadType emThreadID, uint32_t Subresource) const = 0;

			virtual const uint32_t* GetRawValuePtr() const = 0;
		};

		class IStructuredBuffer
		{
		public:
			IStructuredBuffer() = default;
			virtual ~IStructuredBuffer() = default;

			static IStructuredBuffer* Create(void* pData, size_t nNumElements, size_t nByteStride, bool isEnableCpuWrite = false, bool isEnableGpuWrite = true);

		public:
			virtual void UpdateSubresource(ThreadType emThreadID, uint32_t DstSubresource, const void* pSrcData, uint32_t SrcRowPitch) = 0;

		public:
			virtual ID3D11Buffer* GetBuffer() = 0;
			virtual ID3D11UnorderedAccessView* GetUnorderedAccessView() = 0;
			virtual ID3D11ShaderResourceView* GetShaderResourceView() = 0;
		};

		class IRenderTarget : public IResource
		{
		public:
			IRenderTarget() = default;
			virtual ~IRenderTarget() = default;

			static IRenderTarget* Create(ID3D11Texture2D* pTexture2D, const RenderTargetDesc2D* pRenderTargetDesc = nullptr);
			static IRenderTarget* Create(const RenderTargetDesc1D& renderTargetDesc);
			static IRenderTarget* Create(const RenderTargetDesc2D& renderTargetDesc);

		public:
			virtual const std::shared_ptr<ITexture>& GetTexture() = 0;
			virtual ID3D11RenderTargetView* GetRenderTargetView() = 0;
			virtual ID3D11RenderTargetView** GetRenderTargetViewPtr() = 0;
			virtual ID3D11UnorderedAccessView* GetUnorderedAccessView() = 0;

			virtual void SetMipLevel(uint32_t nMipLevel) = 0;
			virtual uint32_t GetMipLevel() = 0;

			virtual void SetClear(const math::Color& color) = 0;
			virtual void OnClear(IDeviceContext* pImmediateContext) = 0;

			virtual const RenderTargetDesc1D& GetDesc1D() = 0;
			virtual const RenderTargetDesc2D& GetDesc2D() = 0;
			virtual const RenderTargetKey& GetKey() = 0;

			virtual const math::UInt2& GetSize() = 0;
		};

		class IDepthStencil
		{
		public:
			IDepthStencil() = default;
			virtual ~IDepthStencil() = default;

			static IDepthStencil* Create(const DepthStencilDesc& depthStencilDesc);

		public:
			virtual const std::shared_ptr<ITexture>& GetTexture() const = 0;
			virtual ID3D11DepthStencilView* GetDepthStencilView() const = 0;
		};

		class ISamplerState
		{
		protected:
			ISamplerState() = default;
			virtual ~ISamplerState() = default;

		public:
			static ISamplerState* Create(const SamplerStateDesc& samplerStateDesc);

		public:
			virtual ID3D11SamplerState* GetInterface() = 0;
			virtual const SamplerStateDesc& GetDesc() = 0;
			virtual const SamplerStateKey& GetKey() = 0;
		};

		class IBlendState
		{
		protected:
			IBlendState() = default;
			virtual ~IBlendState() = default;

		public:
			static IBlendState* Create(const BlendStateDesc& blendStateDesc);

		public:
			virtual ID3D11BlendState* GetInterface() const = 0;
			virtual const BlendStateDesc& GetDesc() = 0;
			virtual const BlendStateKey& GetKey() = 0;
		};

		class IRasterizerState
		{
		protected:
			IRasterizerState() = default;
			virtual ~IRasterizerState() = default;

		public:
			static IRasterizerState* Create(const RasterizerStateDesc& rasterizerStateDesc);

		public:
			virtual ID3D11RasterizerState* GetInterface() const = 0;
			virtual const RasterizerStateDesc& GetDesc() = 0;
			virtual const RasterizerStateKey& GetKey() = 0;
		};

		class IDepthStencilState
		{
		protected:
			IDepthStencilState() = default;
			virtual ~IDepthStencilState() = default;

		public:
			static IDepthStencilState* Create(const DepthStencilStateDesc& depthStencilStateDesc);

		public:
			virtual ID3D11DepthStencilState* GetInterface() const = 0;
			virtual const DepthStencilStateDesc& GetDesc() = 0;
			virtual const DepthStencilStateKey& GetKey() = 0;
		};

		struct MaterialInfo
		{
			String::StringID strName;
			std::string strPath;

			math::Color colorAlbedo;
			math::Color colorEmissive;

			math::Vector4 f4PaddingRoughMetEmi;
			math::Vector4 f4SurSpecTintAniso;
			math::Vector4 f4SheenTintClearcoatGloss;

			float fStippleTransparencyFactor;
			float fTessellationFactor;
			bool isAlbedoAlphaChannelMaskMap;
			bool isVisible;
			bool isInsideTextureForder{ true };

			std::array<String::StringID, EmMaterial::TypeCount> strTextureNameArray;

			EmSamplerState::Type emSamplerState;
			EmBlendState::Type emBlendState;
			EmRasterizerState::Type emRasterizerState;
			EmDepthStencilState::Type emDepthStencilState;

			MaterialInfo();
			void Clear();
		};

		class IMaterial
		{
		protected:
			IMaterial() = default;
			virtual ~IMaterial() = default;

		public:
			static IMaterial* Create(const MaterialInfo* pInfo);
			static IMaterial* Create(const String::StringID& strName);
			static IMaterial* Clone(const IMaterial* pSource);

			static IMaterial* Create(const char* strFileName, const char* strFilePath);
			static void Destroy(IMaterial** ppMaterial);
			static bool SaveToFile(IMaterial* pMaterial, const char* strFilePath);

		public:
			virtual void LoadTexture() = 0;

			virtual const String::StringID& GetName() const = 0;
			virtual void SetName(const String::StringID& strName) = 0;

			virtual const std::string& GetPath() const = 0;
			virtual void SetPath(const std::string& strPath) = 0;

			virtual const math::Color& GetAlbedoColor() const = 0;
			virtual void SetAlbedoColor(const math::Color& color) = 0;

			virtual const math::Color& GetEmissiveColor() const = 0;
			virtual void SetEmissiveColor(const math::Color& color) = 0;

			virtual const String::StringID& GetTextureName(EmMaterial::Type emType) const = 0;
			virtual void SetTextureName(EmMaterial::Type emType, const String::StringID& strName) = 0;

			virtual const std::shared_ptr<ITexture>& GetTexture(EmMaterial::Type emType) const = 0;
			virtual void SetTexture(EmMaterial::Type emType, const std::shared_ptr<ITexture>& pTexture) = 0;

			virtual EmSamplerState::Type GetSamplerState() const = 0;
			virtual void SetSamplerState(EmSamplerState::Type pSamplerState) = 0;

			virtual EmBlendState::Type GetBlendState() const = 0;
			virtual void SetBlendState(EmBlendState::Type pBlendState) = 0;

			virtual EmRasterizerState::Type GetRasterizerState() const = 0;
			virtual void SetRasterizerState(EmRasterizerState::Type pRasterizerState) = 0;

			virtual EmDepthStencilState::Type GetDepthStencilState() const = 0;
			virtual void SetDepthStencilState(EmDepthStencilState::Type pDepthStencilState) = 0;

			virtual const math::Vector4& GetPaddingRoughMetEmi() const = 0;
			virtual void SetPaddingRoughMetEmi(const math::Vector4& f4PaddingRoughMetEmi) = 0;
			virtual const math::Vector4& GetSurSpecTintAniso() const = 0;
			virtual void SetSurSpecTintAniso(const math::Vector4& f4SurSpecTintAniso) = 0;
			virtual const math::Vector4& GetSheenTintClearcoatGloss() const = 0;
			virtual void SetSheenTintClearcoatGloss(const math::Vector4& f4SheenTintClearcoatGloss) = 0;

			virtual float GetDisplacement() const = 0;
			virtual void SetDisplacement(float fDisplacement) = 0;

			virtual float GetRoughness() const = 0;
			virtual void SetRoughness(float fRoughness) = 0;
			virtual float GetMetallic() const = 0;
			virtual void SetMetallic(float fMetallic) = 0;
			virtual float GetEmissive() const = 0;
			virtual void SetEmissive(float fEmissive) = 0;

			virtual float GetSubsurface() const = 0;
			virtual void SetSubsurface(float fSurface) = 0;
			virtual float GetSpecular() const = 0;
			virtual void SetSpecular(float fSpecular) = 0;
			virtual float GetSpecularTint() const = 0;
			virtual void SetSpecularTint(float fSpecularTint) = 0;
			virtual float GetAnisotropic() const = 0;
			virtual void SetAnisotropic(float fAnisotropic) = 0;

			virtual float GetSheen() const = 0;
			virtual void SetSheen(float fSheen) = 0;
			virtual float GetSheenTint() const = 0;
			virtual void SetSheenTint(float fSheenTint) = 0;
			virtual float GetClearcoat() const = 0;
			virtual void SetClearcoat(float fClearcoat) = 0;
			virtual float GetClearcoatGloss() const = 0;
			virtual void SetClearcoatGloss(float fClearcoatGloss) = 0;

			virtual float GetStippleTransparencyFactor() const = 0;
			virtual void SetStippleTransparencyFactor(float fStippleTransparencyFactor) = 0;

			virtual float GetTessellationFactor() const = 0;
			virtual void SetTessellationFactor(float fTessellationFactor) = 0;

			virtual bool IsAlbedoAlphaChannelMaskMap() const = 0;
			virtual void SetAlbedoAlphaChannelMaskMap(bool isAlbedoAlphaChannelMaskMap) = 0;

			virtual bool IsVisible() const = 0;
			virtual void SetVisible(bool isVisible) = 0;

			virtual bool IsLoadComplete() const = 0;

		public:
			virtual int GetReferenceCount() const = 0;
			virtual int IncreaseReference() = 0;
			virtual int DecreaseReference() = 0;
		};
	}
}

namespace std
{
	template <>
	struct hash<eastengine::graphics::ITexture::Key>
	{
		std::uint64_t operator()(const eastengine::graphics::ITexture::Key& key) const
		{
			return key.value.value;
		}
	};
}