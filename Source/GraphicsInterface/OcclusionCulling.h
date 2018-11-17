#pragma once

#include "CommonLib/Singleton.h"
#include "GraphicsInterface.h"

namespace eastengine
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

		public:
			void Enable(bool isEnable);
			bool IsEnable() const;

			void ClearBuffer();
			void Flush();
			void WakeThreads();
			void SuspendThreads();

			void Update(const Camera* pCamera);
			const Collision::Frustum& GetCameraFrustum() const;

		public:
			void RenderTriangles(const math::Matrix& matWorld, const VertexPos* pVertices, const uint32_t* pIndices, size_t nIndexCount);
			Result TestTriangles(const math::Matrix& matWorld, const VertexPos* pVertices, const uint32_t* pIndices, size_t nIndexCount);
		
		public:
			Result TestRect(float xmin, float ymin, float xmax, float ymax, float wmin) const;
			Result TestRect(const Collision::AABB& aabb) const;

		public:
			void Write(const char* strPath = "C:\\image.bmp");

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}