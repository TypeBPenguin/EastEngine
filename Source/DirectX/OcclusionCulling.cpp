#include "stdafx.h"
#include "OcclusionCulling.h"

#include "CullingThreadpool.h"

#include "Vertex.h"

#include "CameraManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		EmOcclusionCulling::Result CastResult(MaskedOcclusionCulling::CullingResult emResult)
		{
			switch (emResult)
			{
			case MaskedOcclusionCulling::VISIBLE:
				return EmOcclusionCulling::eVisible;
			case MaskedOcclusionCulling::OCCLUDED:
				return EmOcclusionCulling::eOccluded;
			case MaskedOcclusionCulling::VIEW_CULLED:
				return EmOcclusionCulling::eViewCulled;
			}

			return EmOcclusionCulling::eViewCulled;
		}

		SOcclusionCulling::SOcclusionCulling()
			: m_bInit(false)
			, m_pMaskedOcclusionCulling(nullptr)
			, m_pCullingThreadPool(nullptr)
		{
		}

		SOcclusionCulling::~SOcclusionCulling()
		{
			Release();
		}

		bool SOcclusionCulling::Init(uint32_t nWidth, uint32_t nHeight, float fNearClipPlane)
		{
			if (m_bInit == true)
				return true;

			m_bInit = true;

			m_pMaskedOcclusionCulling = MaskedOcclusionCulling::Create();
			//m_pMaskedOcclusionCulling->SetResolution(nWidth, nHeight);
			//m_pMaskedOcclusionCulling->SetNearClipPlane(fNearClipPlane);

			uint32_t nThreadCount = std::thread::hardware_concurrency();
			m_pCullingThreadPool = new CullingThreadpool(nThreadCount - 1, 3, 3, 8);
			m_pCullingThreadPool->SetBuffer(m_pMaskedOcclusionCulling);
			m_pCullingThreadPool->SetResolution(nWidth, nHeight);
			m_pCullingThreadPool->SetNearClipPlane(fNearClipPlane);

			return true;
		}

		void SOcclusionCulling::Release()
		{
			if (m_bInit == false)
				return;

			MaskedOcclusionCulling::Destroy(m_pMaskedOcclusionCulling);
			SafeDelete(m_pCullingThreadPool);

			m_bInit = false;
		}

		void SOcclusionCulling::ClearBuffer()
		{
			m_pCullingThreadPool->ClearBuffer();
			//m_pMaskedOcclusionCulling->ClearBuffer();
		}

		void SOcclusionCulling::Flush()
		{
			m_pCullingThreadPool->Flush();
		}

		void SOcclusionCulling::Start()
		{
			m_pCullingThreadPool->WakeThreads();
		}

		void SOcclusionCulling::End()
		{
			m_pCullingThreadPool->SuspendThreads();
		}

		void SOcclusionCulling::TransformVertices(const Math::Matrix& matWorld, const VertexPos* pVertexPos, VertexClipSpace* pVertexClipSpace_out, uint32_t nVertexCount)
		{
			MaskedOcclusionCulling::TransformVertices(&matWorld._11, &pVertexPos->pos.x, &pVertexClipSpace_out->pos.x, nVertexCount);
		}

		EmOcclusionCulling::Result SOcclusionCulling::RenderTriangles(const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount, const Math::Matrix* pMatModelToClipSpace)
		{
			if (pVertexClipSpace == nullptr || pIndexData == nullptr)
				return EmOcclusionCulling::eViewCulled;

			m_pCullingThreadPool->SetMatrix(reinterpret_cast<const float*>(pMatModelToClipSpace));

			m_pCullingThreadPool->RenderTriangles(&pVertexClipSpace->pos.x, pIndexData, nIndexCount / 3);
			return EmOcclusionCulling::eVisible;

			//return CastResult(m_pMaskedOcclusionCulling->RenderTriangles(&pVertexClipSpace->pos.x, pIndexData, nIndexCount / 3, reinterpret_cast<const float*>(pMatModelToClipSpace)));
		}

		EmOcclusionCulling::Result SOcclusionCulling::TestTriangles(const VertexClipSpace* pVertexClipSpace, const uint32_t* pIndexData, uint32_t nIndexCount, const Math::Matrix* pMatModelToClipSpace)
		{
			return CastResult(m_pMaskedOcclusionCulling->TestTriangles(&pVertexClipSpace->pos.x, pIndexData, nIndexCount / 3, reinterpret_cast<const float*>(pMatModelToClipSpace)));
		}
		
		static void WriteBMP(const char *filename, const unsigned char *data, int w, int h)
		{
			short header[] = { 0x4D42, 0, 0, 0, 0, 26, 0, 12, 0, (short)w, (short)h, 1, 24 };
			FILE* f = nullptr;
			fopen_s(&f, filename, "wb");
			fwrite(header, 1, sizeof(header), f);
			fwrite(data, 1, w * h * 3, f);
			fclose(f);
		}

		static void TonemapDepth(float *depth, unsigned char *image, int w, int h)
		{
			// Find min/max w coordinate (discard cleared pixels)
			float minW = FLT_MAX, maxW = 0.0f;
			for (int i = 0; i < w*h; ++i)
			{
				if (depth[i] > 0.0f)
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

		void SOcclusionCulling::Write()
		{
			float *perPixelZBuffer = new float[1920 * 1080];
			m_pMaskedOcclusionCulling->ComputePixelDepthBuffer(perPixelZBuffer);

			unsigned char *image = new unsigned char[1920 * 1080 * 3];
			TonemapDepth(perPixelZBuffer, image, 1920, 1080);
			WriteBMP("E:\\image.bmp", image, 1920, 1080);
			delete[] image;
			delete[] perPixelZBuffer;
		}
	}
}