#pragma once

#include "CommonLib/Singleton.h"
#include "GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		struct VertexPos;

		class Camera;

		class OcclusionCulling : public Singleton<OcclusionCulling>
		{
			friend Singleton<OcclusionCulling>;
		private:
			OcclusionCulling();
			virtual ~OcclusionCulling();

		public:
			enum Result
			{
				eVisible = 0x0,
				eOccluded = 0x1,
				eViewCulled = 0x3,
			};

		public:
			void Initialize(uint32_t nWidth, uint32_t nHeight);
			void Release();

		public:
			void Enable(bool isEnable);
			bool IsEnable() const;

			void ClearBuffer();
			void Flush();
			void WakeThreads();
			void SuspendThreads();

			void Update(Camera* pCamera);
			const collision::Frustum& GetCameraFrustum() const;

		public:
			void RenderTriangles(const math::Matrix& worldMatrix, const VertexPos* pVertices, const uint32_t* pIndices, size_t indexCount);
			Result TestTriangles(const math::Matrix& worldMatrix, const VertexPos* pVertices, const uint32_t* pIndices, size_t indexCount);
		
		public:
			Result TestRect(float xmin, float ymin, float xmax, float ymax, float wmin) const;
			Result TestRect(const collision::AABB& aabb) const;

		public:
			void Write(const wchar_t* path);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}