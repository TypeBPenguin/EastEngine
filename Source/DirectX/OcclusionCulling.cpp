#include "stdafx.h"
#include "OcclusionCulling.h"

#include "CommonLib/FileStream.h"

#include "CullingThreadpool.h"
#include "CameraManager.h"
#include "Vertex.h"

namespace EastEngine
{
	namespace Graphics
	{
		class OcclusionCulling::Impl
		{
		public:
			Impl(uint32_t nWidth, uint32_t nHeight, float fNearClipPlane);
			~Impl();

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
			void Write(const char* strPath);

		private:
			Result CastResult(MaskedOcclusionCulling::CullingResult emResult);
			Math::Matrix GetClipSpaceMatrix(const Math::Matrix& matWorld) const;

		private:
			std::mutex m_mutex;

			MaskedOcclusionCulling * m_pMaskedOcclusionCulling;
			std::unique_ptr<CullingThreadpool> m_pCullingThreadPool;
		};

		OcclusionCulling::Impl::Impl(uint32_t nWidth, uint32_t nHeight, float fNearClipPlane)
			: m_pMaskedOcclusionCulling{ MaskedOcclusionCulling::Create() }
		{
			const uint32_t nThreadCount = std::thread::hardware_concurrency() - 1;
			m_pCullingThreadPool = std::make_unique<CullingThreadpool>(nThreadCount, 2, nThreadCount);
			m_pCullingThreadPool->SetBuffer(m_pMaskedOcclusionCulling);
			m_pCullingThreadPool->SetResolution(nWidth, nHeight);
			m_pCullingThreadPool->SetNearClipPlane(fNearClipPlane);

			ClearBuffer();
		}

		OcclusionCulling::Impl::~Impl()
		{
			MaskedOcclusionCulling::Destroy(m_pMaskedOcclusionCulling);
		}

		void OcclusionCulling::Impl::ClearBuffer()
		{
			m_pCullingThreadPool->ClearBuffer();
		}

		void OcclusionCulling::Impl::Flush()
		{
			m_pCullingThreadPool->Flush();
		}

		void OcclusionCulling::Impl::Start()
		{
			m_pCullingThreadPool->WakeThreads();
		}

		void OcclusionCulling::Impl::End()
		{
			m_pCullingThreadPool->SuspendThreads();
		}

		void OcclusionCulling::Impl::TransformVertices(const Math::Matrix& matWorld, const VertexPos* pVertexPos, VertexClipSpace* pVertexClipSpace_out, uint32_t nVertexCount)
		{
			MaskedOcclusionCulling::TransformVertices(&matWorld._11, &pVertexPos->pos.x, &pVertexClipSpace_out->pos.x, nVertexCount);
		}

		OcclusionCulling::Result OcclusionCulling::Impl::RenderTriangles(const Math::Matrix& matWorld, const VertexPos* pVertexPos, size_t nVertexCount, const uint32_t* pIndexData, uint32_t nIndexCount)
		{
			if (pVertexPos == nullptr || pIndexData == nullptr || (nIndexCount % 3 != 0))
				return OcclusionCulling::eViewCulled;

			const Math::Matrix matSwapXY =
			{
				0.f, 1.f, 0.f, 0.f,
				1.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f,
			};

			//Math::Matrix matClipSpace = GetClipSpaceMatrix(matWorld);

			std::unique_lock<std::mutex> lock(m_mutex);

			m_pCullingThreadPool->SetMatrix(&matWorld._11);
			m_pCullingThreadPool->RenderTriangles(&pVertexPos->pos.x, pIndexData, nIndexCount / 3, MaskedOcclusionCulling::BACKFACE_CCW);
			return OcclusionCulling::eVisible;
		}

		OcclusionCulling::Result OcclusionCulling::Impl::RenderTriangles(const Math::Matrix& matWorld, const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount)
		{
			if (pVertexClipSpace == nullptr || pIndexData == nullptr || (nIndexCount % 3 != 0))
				return OcclusionCulling::eViewCulled;

			const Math::Matrix matSwapXY =
			{
				0.f, 1.f, 0.f, 0.f,
				1.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f,
			};

			//Math::Matrix matClipSpace = GetClipSpaceMatrix(matWorld);

			std::unique_lock<std::mutex> lock(m_mutex);

			m_pCullingThreadPool->SetMatrix(&matWorld._11);
			m_pCullingThreadPool->RenderTriangles(&pVertexClipSpace->pos.x, pIndexData, nIndexCount / 3, MaskedOcclusionCulling::BACKFACE_CCW);
			return OcclusionCulling::eVisible;
		}

		OcclusionCulling::Result OcclusionCulling::Impl::TestTriangles(const Math::Matrix& matWorld, const VertexPos* pVertexPos, size_t nVertexCount, const uint32_t* pIndexData, uint32_t nIndexCount)
		{
			if (pVertexPos == nullptr || pIndexData == nullptr || (nIndexCount % 3 != 0))
				return OcclusionCulling::eViewCulled;

			const Math::Matrix matSwapXY =
			{
				0.f, 1.f, 0.f, 0.f,
				1.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f,
			};

			//Math::Matrix matClipSpace = GetClipSpaceMatrix(matWorld);

			std::unique_lock<std::mutex> lock(m_mutex);

			m_pCullingThreadPool->SetMatrix(&matWorld._11);
			return CastResult(m_pCullingThreadPool->TestTriangles(&pVertexPos->pos.x, pIndexData, nIndexCount / 3, MaskedOcclusionCulling::BACKFACE_CCW));
		}

		OcclusionCulling::Result OcclusionCulling::Impl::TestTriangles(const Math::Matrix& matWorld, const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount)
		{
			if (pVertexClipSpace == nullptr || pIndexData == nullptr || (nIndexCount % 3 != 0))
				return OcclusionCulling::eViewCulled;

			const Math::Matrix matSwapXY =
			{
				0.f, 1.f, 0.f, 0.f,
				1.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f,
			};

			//Math::Matrix matClipSpace = GetClipSpaceMatrix(matWorld);

			std::unique_lock<std::mutex> lock(m_mutex);

			m_pCullingThreadPool->SetMatrix(&matWorld._11);
			return CastResult(m_pCullingThreadPool->TestTriangles(&pVertexClipSpace->pos.x, pIndexData, nIndexCount / 3, MaskedOcclusionCulling::BACKFACE_CCW));
		}

		static void WriteBMP(const char *filename, const unsigned char *data, int w, int h)
		{
			short header[] = { 0x4D42, 0, 0, 0, 0, 26, 0, 12, 0, (short)w, (short)h, 1, 24 };
			FILE* f = nullptr;
			fopen_s(&f, filename, "wb");

			fwrite(header, 1, sizeof(header), f);
#if USE_D3D == 1
			// Flip image because Y axis of Direct3D points in the opposite direction of bmp. If the library 
			// is configured for OpenGL (USE_D3D 0) then the Y axes would match and this wouldn't be required.
			for (int y = 0; y < h; ++y)
			{
				fwrite(&data[(h - y - 1) * w * 3], 1, w * 3, f);
			}
#else
			fwrite(data, 1, w * h * 3, f);
#endif
			fclose(f);
		}

		static void TonemapDepth(float *depth, unsigned char *image, int w, int h)
		{
			// Find min/max w coordinate (discard cleared pixels)
			float minW = std::numeric_limits<float>::max();
			float maxW = std::numeric_limits<float>::min();
			for (int i = 0; i < w*h; ++i)
			{
				if (depth[i] > 0.f)
				{
					minW = std::min(minW, depth[i]);
					maxW = std::max(maxW, depth[i]);
				}
			}

			// Tonemap depth values
			for (int i = 0; i < w*h; ++i)
			{
				int intensity = 0;
				if (depth[i] > 0)
					intensity = (unsigned char)(223.0*(depth[i] - minW) / (maxW - minW) + 32.0);

				image[i * 3 + 0] = (unsigned char)intensity;
				image[i * 3 + 1] = (unsigned char)intensity;
				image[i * 3 + 2] = (unsigned char)intensity;
			}
		}

		void OcclusionCulling::Impl::Write(const char* strPath)
		{
			std::unique_ptr<float[]> perPixelZBuffer = std::make_unique<float[]>(1920 * 1080);
			m_pMaskedOcclusionCulling->ComputePixelDepthBuffer(perPixelZBuffer.get(), false);

			std::unique_ptr<unsigned char[]> image = std::make_unique<unsigned char[]>(1920 * 1080 * 3);
			TonemapDepth(perPixelZBuffer.get(), image.get(), 1920, 1080);
			WriteBMP(strPath, image.get(), 1920, 1080);
		}

		OcclusionCulling::Result OcclusionCulling::Impl::CastResult(MaskedOcclusionCulling::CullingResult emResult)
		{
			switch (emResult)
			{
			case MaskedOcclusionCulling::VISIBLE:
				return OcclusionCulling::eVisible;
			case MaskedOcclusionCulling::OCCLUDED:
				return OcclusionCulling::eOccluded;
			case MaskedOcclusionCulling::VIEW_CULLED:
				return OcclusionCulling::eViewCulled;
			}

			return OcclusionCulling::eViewCulled;
		}

		Math::Matrix OcclusionCulling::Impl::GetClipSpaceMatrix(const Math::Matrix& matWorld) const
		{
			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();

			const Math::Matrix matViewport
			(
				1.f, 0.f, 0.f, 0.f,
				0.f, -1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f
			);

			return matWorld * pCamera->GetViewMatrix() * pCamera->GetProjMatrix() * matViewport;
		}

		OcclusionCulling::OcclusionCulling()
		{
		}

		OcclusionCulling::~OcclusionCulling()
		{
		}

		void OcclusionCulling::Initialize(uint32_t nWidth, uint32_t nHeight, float fNearClipPlane)
		{
			m_pImpl = std::make_unique<Impl>(nWidth, nHeight, fNearClipPlane);
		}

		void OcclusionCulling::ClearBuffer()
		{
			m_pImpl->ClearBuffer();
		}

		void OcclusionCulling::Flush()
		{
			m_pImpl->Flush();
		}

		void OcclusionCulling::Start()
		{
			m_pImpl->Start();
		}

		void OcclusionCulling::End()
		{
			m_pImpl->End();
		}

		void OcclusionCulling::TransformVertices(const Math::Matrix& matWorld, const VertexPos* pVertexPos, VertexClipSpace* pVertexClipSpace_out, uint32_t nVertexCount)
		{
			m_pImpl->TransformVertices(matWorld, pVertexPos, pVertexClipSpace_out, nVertexCount);
		}

		OcclusionCulling::Result OcclusionCulling::RenderTriangles(const Math::Matrix& matWorld, const VertexPos* pVertexPos, size_t nVertexCount, const uint32_t* pIndexData, uint32_t nIndexCount)
		{
			return m_pImpl->RenderTriangles(matWorld, pVertexPos, nVertexCount, pIndexData, nIndexCount);
		}

		OcclusionCulling::Result OcclusionCulling::RenderTriangles(const Math::Matrix& matWorld, const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount)
		{
			return m_pImpl->RenderTriangles(matWorld, pVertexClipSpace, pIndexData, nIndexCount);
		}

		OcclusionCulling::Result OcclusionCulling::TestTriangles(const Math::Matrix& matWorld, const VertexPos* pVertexPos, size_t nVertexCount, const uint32_t* pIndexData, uint32_t nIndexCount)
		{
			return m_pImpl->TestTriangles(matWorld, pVertexPos, nVertexCount, pIndexData, nIndexCount);
		}

		OcclusionCulling::Result OcclusionCulling::TestTriangles(const Math::Matrix& matWorld, const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount)
		{
			return m_pImpl->TestTriangles(matWorld, pVertexClipSpace, pIndexData, nIndexCount);
		}

		void OcclusionCulling::Write(const char* strPath)
		{
			m_pImpl->Write(strPath);
		}
	}
}