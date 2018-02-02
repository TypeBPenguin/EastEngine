#pragma once

#include "CommonLib/Singleton.h"

class MaskedOcclusionCulling;
class CullingThreadpool;

namespace EastEngine
{
	namespace Graphics
	{
		class OcclusionCulling : public Singleton<OcclusionCulling>
		{
			friend Singleton<OcclusionCulling>;
		private:
			OcclusionCulling();
			virtual ~OcclusionCulling();

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};

		struct VertexPos;
		struct VertexClipSpace;

		namespace EmOcclusionCulling
		{
			enum Result
			{
				eVisible = 0,
				eOccluded = 1,
				eViewCulled = 2,
			};
		}

		class SOcclusionCulling : public Singleton<SOcclusionCulling>
		{
			friend Singleton<SOcclusionCulling>;
		private:
			SOcclusionCulling();
			virtual ~SOcclusionCulling();

		public:
			bool Init(uint32_t nWidth, uint32_t nHeight, float fNearClipPlane);
			void Release();

			void ClearBuffer();
			void Flush();
			void Start();
			void End();

			void TransformVertices(const Math::Matrix& matWorld, const VertexPos* pVertexPos, VertexClipSpace* pVertexClipSpace_out, uint32_t nVertexCount);
			EmOcclusionCulling::Result RenderTriangles(const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount, const Math::Matrix* pMatModelToClipSpace = nullptr);
			EmOcclusionCulling::Result TestTriangles(const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount, const Math::Matrix* pMatModelToClipSpace = nullptr);

			void Write();

		private:
			bool m_bInit;
			MaskedOcclusionCulling* m_pMaskedOcclusionCulling;
			CullingThreadpool* m_pCullingThreadPool;
		};
	}
}