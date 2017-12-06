#include "stdafx.h"
#include "RenderSubsets.h"

namespace EastEngine
{
	namespace Graphics
	{
		RenderSubsetLine::RenderSubsetLine(IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, const Math::Matrix& matWorld)
			: pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), matWorld(matWorld)
		{
		}

		RenderSubsetLineSegment::RenderSubsetLineSegment(const Math::Vector3& f3StartPoint, const Math::Color& colorStartPoint, const Math::Vector3& f3EndPoint, const Math::Color& colorEndPoint)
		{
			vertexLineSegment[0] = VertexPosCol(f3StartPoint, colorStartPoint);
			vertexLineSegment[1] = VertexPosCol(f3EndPoint, colorEndPoint);
		}

		RenderSubsetVertex::RenderSubsetVertex(IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, const Math::Matrix* pWorldMatrix, const Math::Color& color)
			: pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pWorldMatrix(pWorldMatrix), color(color)
		{
		}

		RenderSubsetStatic::RenderSubsetStatic()
		{
		}

		RenderSubsetStatic::RenderSubsetStatic(void* pKey, IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, IMaterial* pMaterial
			, const Math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount
			, float fDepth, const Collision::Sphere& boundingSphere)
			: pKey(pKey), pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMaterial(pMaterial)
			, matWorld(matWorld), nStartIndex(nStartIndex), nIndexCount(nIndexCount)
			, fDepth(fDepth), boundingSphere(boundingSphere)
		{
		}

		RenderSubsetSkinned::RenderSubsetSkinned()
		{
		}

		RenderSubsetSkinned::RenderSubsetSkinned(void* pKey, IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, IMaterial* pMaterial
			, const Math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount, uint32_t nVTFID
			, float fDepth)
			: pKey(pKey), pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMaterial(pMaterial)
			, matWorld(matWorld), nStartIndex(nStartIndex), nIndexCount(nIndexCount)
			, nVTFID(nVTFID)
			, fDepth(fDepth)
		{
		}

		RenderSubsetHeightField::RenderSubsetHeightField()
		{
		}

		RenderSubsetHeightField::RenderSubsetHeightField(void* pKey, IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, IMaterial* pMaterial
			, const Math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount
			, float fDepth, const Collision::Sphere& boundingSphere)
			: pKey(pKey), pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMaterial(pMaterial)
			, matWorld(matWorld), nStartIndex(nStartIndex), nIndexCount(nIndexCount)
			, fDepth(fDepth), boundingSphere(boundingSphere)
		{
		}

		RenderSubsetSky::RenderSubsetSky()
		{
		}

		RenderSubsetSky::RenderSubsetSky(IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, Math::Matrix* pMatrix, Math::Color* pColorApex, Math::Color* pColorCenter)
			: pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMatrix(pMatrix), pColorApex(pColorApex), pColorCenter(pColorCenter)
		{
		}

		RenderSubsetSkyEffect::RenderSubsetSkyEffect()
		{
		}

		RenderSubsetSkyEffect::RenderSubsetSkyEffect(IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, Math::Matrix* pMatrix, const std::shared_ptr<ITexture>& pTexEffect)
			: pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMatrix(pMatrix), pTexEffect(pTexEffect)
		{
		}

		RenderSubsetSkyCloud::RenderSubsetSkyCloud()
		{
		}

		RenderSubsetSkyCloud::RenderSubsetSkyCloud(IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, Math::Matrix* pMatrix, const std::shared_ptr<ITexture>&, const std::shared_ptr<ITexture>& pTexCloudBlend, float fBlend)
			: pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMatrix(pMatrix), pTexCloud(pTexCloud), pTexCloudBlend(pTexCloudBlend), fBlend(fBlend)
		{
		}

		RenderSubsetParticleEmitter::RenderSubsetParticleEmitter()
		{
		}

		RenderSubsetParticleEmitter::RenderSubsetParticleEmitter(IBlendState* pBlendState, const Math::Vector3& f3Pos, VertexPosTexCol* pVertices, uint32_t nVertexCount, const std::shared_ptr<ITexture>& pTexture)
			: pBlendState(pBlendState), f3Pos(f3Pos), pVertices(pVertices), nVertexCount(nVertexCount), pTexture(pTexture)
		{
		}

		RenderSubsetParticleDecal::RenderSubsetParticleDecal()
		{
		}

		RenderSubsetParticleDecal::RenderSubsetParticleDecal(const Math::Matrix& matWVP, const Math::Matrix& matWorld, IMaterial* pMaterial)
			: matWVP(matWVP), matWorld(matWorld), pMaterial(pMaterial)
		{
		}

		RenderSubsetUIText::RenderSubsetUIText()
			: pSpriteFont(nullptr), vOrigin(0.f, 0.f), color(Math::Color::Black), fRot(0.f), fScale(0.f), fDepth(0.f), emSpriteEffects(EmSprite::eNone)
			, pRenderTargetView(nullptr)
		{
			SetRectEmpty(&rect);
		}

		RenderSubsetUIText::RenderSubsetUIText(std::shared_ptr<Graphics::ISpriteFont> pSpriteFont, const std::wstring& wstrText, Math::Rect& rect, Math::Vector2& vOrigin, Math::Color color, float fRot, float fScale, float fDepth, EmSprite::Effects emSpriteEffects, IRenderTarget* pRenderTargetView)
			: pSpriteFont(pSpriteFont), wstrText(wstrText), rect(rect), vOrigin(vOrigin), color(color), fRot(fRot), fScale(fScale), fDepth(fDepth), emSpriteEffects(emSpriteEffects)
			, pRenderTargetView(pRenderTargetView)
		{
		}

		RenderSubsetUISprite::RenderSubsetUISprite()
			: pTexture(nullptr), sourceRect(nullptr), vOrigin(0.f, 0.f), emSpriteEffects(EmSprite::eNone), color(Math::Color::Black)
			, fRot(0.f), fDepth(0.f), pRenderTargetView(nullptr)
		{
			SetRectEmpty(&destRect);
		}

		RenderSubsetUISprite::RenderSubsetUISprite(const std::shared_ptr<ITexture>& pTexture, Math::Rect destRect, Math::Rect* sourceRect, Math::Vector2 vOrigin, Math::Color color, float fRot, float fDepth, EmSprite::Effects emSpriteEffects, IRenderTarget* pRenderTargetView)
			: pTexture(pTexture), destRect(destRect), sourceRect(sourceRect), color(color), vOrigin(vOrigin), fRot(fRot), fDepth(fDepth), emSpriteEffects(emSpriteEffects)
			, pRenderTargetView(pRenderTargetView)
		{
		}

		RenderSubsetUIPanel::RenderSubsetUIPanel()
			: pTexture(nullptr), sourceRect(nullptr), vOrigin(0.f, 0.f), emSpriteEffects(EmSprite::eNone), color(Math::Color::Black)
			, fRot(0.f), fDepth(0.f), pRenderTargetView(nullptr)
			, bNeedRender(false)
		{
			SetRectEmpty(&destRect);
		}

		RenderSubsetUIPanel::RenderSubsetUIPanel(const std::shared_ptr<ITexture>& pTexture, Math::Rect destRect, Math::Rect* sourceRect, Math::Vector2 vOrigin, Math::Color color, float fRot, float fDepth, EmSprite::Effects emSpriteEffects, IRenderTarget* pRenderTargetView)
			: pTexture(pTexture), destRect(destRect), sourceRect(sourceRect), color(color), vOrigin(vOrigin), fRot(fRot), fDepth(fDepth), emSpriteEffects(emSpriteEffects)
			, pRenderTargetView(pRenderTargetView)
			, bNeedRender(false)
		{
		}
	}
}