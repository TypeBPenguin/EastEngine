#pragma once

#include "DirectX/Vertex.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IRenderTarget;
		class IVertexBuffer;
		class IIndexBuffer;
		class ITexture;
		class IBlendState;
		class IMaterial;
		class ISpriteFont;
		class ISpriteBatch;

		struct RenderSubsetLine
		{
			IVertexBuffer* pVertexBuffer = nullptr;
			IIndexBuffer* pIndexBuffer = nullptr;
			Math::Matrix matWorld;

			RenderSubsetLine(IVertexBuffer* pIVertexBuffer, IIndexBuffer* pIIndexBuffer, const Math::Matrix& matWorld);
		};

		struct RenderSubsetLineSegment
		{
			VertexPosCol vertexLineSegment[2];

			RenderSubsetLineSegment(const Math::Vector3& f3StartPoint, const Math::Color& colorStartPoint, const Math::Vector3& f3EndPoint, const Math::Color& colorEndPoint);
		};

		struct RenderSubsetVertex
		{
			IVertexBuffer* pVertexBuffer = nullptr;
			IIndexBuffer* pIndexBuffer = nullptr;
			Math::Matrix* pWorldMatrix = nullptr;

			RenderSubsetVertex(IVertexBuffer* pIVertexBuffer, IIndexBuffer* pIIndexBuffer, Math::Matrix* pWorldMatrix);
		};

		struct RenderSubsetStatic
		{
			void* pKey = nullptr;
			IVertexBuffer* pVertexBuffer = nullptr;
			IIndexBuffer* pIndexBuffer = nullptr;
			IMaterial* pMaterial = nullptr;
			Math::Matrix matWorld;
			uint32_t nStartIndex = 0;
			uint32_t nIndexCount = 0;
			float fDepth = 0.f;
			Collision::Sphere boundingSphere;

			RenderSubsetStatic();
			RenderSubsetStatic(void* pKey, IVertexBuffer* pIVertexBuffer, IIndexBuffer* pIIndexBuffer, IMaterial* pMaterial
				, const Math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount
				, float fDepth, const Collision::Sphere& boundingSphere);
		};

		struct RenderSubsetSkinned
		{
			void* pKey = nullptr;
			IVertexBuffer* pVertexBuffer = nullptr;
			IIndexBuffer* pIndexBuffer = nullptr;
			IMaterial* pMaterial = nullptr;
			Math::Matrix matWorld;
			uint32_t nStartIndex = 0;
			uint32_t nIndexCount = 0;
			uint32_t nVTFID = 0;
			float fDepth = 0.f;

			RenderSubsetSkinned();
			RenderSubsetSkinned(void* pKey, IVertexBuffer* pIVertexBuffer, IIndexBuffer* pIIndexBuffer, IMaterial* pMaterial
				, const Math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount, uint32_t nVTFID
				, float fDepth);
		};

		struct RenderSubsetTerrain
		{
			IVertexBuffer* pVertexBuffer = nullptr;

			std::shared_ptr<ITexture> pTexHeightField;
			std::shared_ptr<ITexture> pTexLayerdef;
			std::shared_ptr<ITexture> pTexRockBump;
			std::shared_ptr<ITexture> pTexRockMicroBump;
			std::shared_ptr<ITexture> pTexRockDiffuse;
			std::shared_ptr<ITexture> pTexSandBump;
			std::shared_ptr<ITexture> pTexSandMicroBump;
			std::shared_ptr<ITexture> pTexSandDiffuse;
			std::shared_ptr<ITexture> pTexGrassDiffuse;
			std::shared_ptr<ITexture> pTexSlopeDiffuse;
			//std::shared_ptr<ITexture> pTexDepthMap;

			float fHeightFieldSize = 0.f;

			Math::Matrix matWorld;
			float fHalfSpaceCullSign = 0.f;
			float fHalfSpaceCullPosition = 0.f;
		};

		struct RenderSubsetSky
		{
			IVertexBuffer* pVertexBuffer = nullptr;
			IIndexBuffer* pIndexBuffer = nullptr;
			Math::Matrix* pMatrix = nullptr;
			Math::Color* pColorApex = nullptr;
			Math::Color* pColorCenter = nullptr;

			RenderSubsetSky();
			RenderSubsetSky(IVertexBuffer* pIVertexBuffer, IIndexBuffer* pIIndexBuffer, Math::Matrix* pMatrix, Math::Color* pColorApex, Math::Color* pColorCenter);
		};

		struct RenderSubsetSkyEffect
		{
			IVertexBuffer* pVertexBuffer = nullptr;
			IIndexBuffer* pIndexBuffer = nullptr;
			Math::Matrix* pMatrix = nullptr;
			std::shared_ptr<ITexture> pTexEffect;

			RenderSubsetSkyEffect();
			RenderSubsetSkyEffect(IVertexBuffer* pIVertexBuffer, IIndexBuffer* pIIndexBuffer, Math::Matrix* pMatrix, const std::shared_ptr<ITexture>& pTexEffect);
		};

		struct RenderSubsetSkyCloud
		{
			IVertexBuffer* pVertexBuffer = nullptr;
			IIndexBuffer* pIndexBuffer = nullptr;
			Math::Matrix* pMatrix = nullptr;
			std::shared_ptr<ITexture> pTexCloud;
			std::shared_ptr<ITexture> pTexCloudBlend;
			float fBlend = 0.f;

			RenderSubsetSkyCloud();
			RenderSubsetSkyCloud(IVertexBuffer* pIVertexBuffer, IIndexBuffer* pIIndexBuffer, Math::Matrix* pMatrix, const std::shared_ptr<ITexture>& pTexCloud, const std::shared_ptr<ITexture>& pTexCloudBlend, float fBlend);
		};

		struct RenderSubsetParticleEmitter
		{
			IBlendState* pBlendState = nullptr;
			Math::Vector3 f3Pos;
			VertexPosTexCol* pVertices = nullptr;
			uint32_t nVertexCount = 0;
			std::shared_ptr<ITexture> pTexture;

			RenderSubsetParticleEmitter();
			RenderSubsetParticleEmitter(IBlendState* pBlendState, const Math::Vector3& f3Pos, VertexPosTexCol* pVertices, uint32_t nVertexCount, const std::shared_ptr<ITexture>& pTexture);
		};

		struct RenderSubsetParticleDecal
		{
			Math::Matrix matWVP;
			Math::Matrix matWorld;
			IMaterial* pMaterial = nullptr;

			RenderSubsetParticleDecal();
			RenderSubsetParticleDecal(const Math::Matrix& matWVP, const Math::Matrix& matWorld, IMaterial* pMaterial);
		};

		struct RenderSubsetUIText
		{
			std::shared_ptr<Graphics::ISpriteFont> pSpriteFont = nullptr;
			Math::Rect rect;
			Math::Vector2 vOrigin;
			EmSprite::Effects emSpriteEffects;
			Math::Color color;
			float fRot;
			float fScale;
			float fDepth;
			std::wstring wstrText;
			IRenderTarget* pRenderTargetView = nullptr;

			RenderSubsetUIText();
			RenderSubsetUIText(std::shared_ptr<Graphics::ISpriteFont> pSpriteFont, const std::wstring& wstrText, Math::Rect& rect, Math::Vector2& vOrigin, Math::Color color = Math::Color::White, float fRot = 0.f, float fScale = 1.f, float fDepth = 0.f, EmSprite::Effects emSpriteEffects = EmSprite::eNone, IRenderTarget* pRenderTargetView = nullptr);

			void Set(std::shared_ptr<Graphics::ISpriteFont> _pSpriteFont, const std::wstring& _wstrText, Math::Rect& _rect, Math::Vector2& _vOrigin, Math::Color _color = Math::Color::White, float _fRot = 0.f, float _fScale = 1.f, float _fDepth = 0.f, EmSprite::Effects _emSpriteEffects = EmSprite::eNone, IRenderTarget* _pRenderTargetView = nullptr)
			{
				pSpriteFont = _pSpriteFont;
				wstrText = _wstrText;
				rect = _rect;
				vOrigin = _vOrigin;
				color = _color;
				fRot = _fRot;
				fScale = _fScale;
				fDepth = _fDepth;
				emSpriteEffects = _emSpriteEffects;
				pRenderTargetView = _pRenderTargetView;
			}
		};

		struct RenderSubsetUISprite
		{
			std::shared_ptr<ITexture> pTexture;
			Math::Rect destRect;
			Math::Rect* sourceRect;
			Math::Vector2 vOrigin;
			EmSprite::Effects emSpriteEffects;
			Math::Color color;
			float fRot;
			float fDepth;
			IRenderTarget* pRenderTargetView;

			RenderSubsetUISprite();
			RenderSubsetUISprite(const std::shared_ptr<ITexture>& pTexture, Math::Rect destRect, Math::Rect* sourceRect, Math::Vector2 vOrigin, Math::Color color, float fRot = 0.f, float fDepth = 1.f, EmSprite::Effects emSpriteEffects = EmSprite::eNone, IRenderTarget* pRenderTargetView = nullptr);
		};

		// UIText와 UISprite는 패널의 렌더 타깃에 그리게 되는데
		// 이때 렌더 큐에 쌓아서 Sprite를 그리고 Text 그리는데
		// 패널을 그려줄 때 Sprite로 그리게 되면 렌더링 순서가 꼬이게 된다.
		// 그래서 Panel 전용 렌더피스를 만들어서 Text와 Sprite를 다 그리고 난 후에
		// 패널의 렌더 타깃을 화면에 그려주어야 한다.
		struct RenderSubsetUIPanel
		{
			std::shared_ptr<ITexture> pTexture;
			Math::Rect destRect;
			Math::Rect* sourceRect;
			Math::Vector2 vOrigin;
			EmSprite::Effects emSpriteEffects;
			Math::Color color;
			float fRot;
			float fDepth;
			IRenderTarget* pRenderTargetView;
			bool bNeedRender;

			RenderSubsetUIPanel();
			RenderSubsetUIPanel(const std::shared_ptr<ITexture>& pTexture, Math::Rect destRect, Math::Rect* sourceRect, Math::Vector2 vOrigin, Math::Color color, float fRot = 0.f, float fDepth = 1.f, EmSprite::Effects emSpriteEffects = EmSprite::eNone, IRenderTarget* pRenderTargetView = nullptr);
		};

		struct RenderGroupUI
		{
			IRenderTarget* pRenderTargetView = nullptr;
			std::vector<RenderSubsetUISprite> vecSprite;
			std::vector<RenderSubsetUIText> vecText;
		};
	}
}