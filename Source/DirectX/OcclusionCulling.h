#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		struct VertexPos;
		struct VertexClipSpace;

		class OcclusionCulling : public Singleton<OcclusionCulling>
		{
			friend Singleton<OcclusionCulling>;
		private:
			OcclusionCulling();
			virtual ~OcclusionCulling();

		public:
			enum Result
			{
				eVisible = 0,
				eOccluded = 1,
				eViewCulled = 2,
			};

		public:
			void Initialize(uint32_t nWidth, uint32_t nHeight, float fNearClipPlane);

		public:
			void ClearBuffer();
			void Flush();
			void Start();
			void End();

		public:
			void TransformVertices(const Math::Matrix& matWorld, const VertexPos* pVertexPos, VertexClipSpace* pVertexClipSpace_out, uint32_t nVertexCount);
			Result RenderTriangles(const Math::Matrix& matWorld, const VertexPos* pVertexPos, size_t nVertexCount, const uint32_t* pIndexData, uint32_t nIndexCount);
			Result RenderTriangles(const Math::Matrix& matWorld, const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount);
			Result TestTriangles(const Math::Matrix& matWorld, const VertexPos* pVertexPos, size_t nVertexCount, const uint32_t* pIndexData, uint32_t nIndexCount);
			Result TestTriangles(const Math::Matrix& matWorld, const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount);

		public:
			void Write(const char* strPath = "C:\\image.bmp");

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}