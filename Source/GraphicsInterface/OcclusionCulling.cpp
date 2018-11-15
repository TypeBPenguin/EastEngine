#include "stdafx.h"
#include "OcclusionCulling.h"

#include "CullingThreadpool.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/Lock.h"

#include "Camera.h"
#include "Vertex.h"

namespace eastengine
{
	namespace graphics
	{
		static_assert(OcclusionCulling::eVisible == MaskedOcclusionCulling::VISIBLE, "mismatch result type");
		static_assert(OcclusionCulling::eOccluded == MaskedOcclusionCulling::OCCLUDED, "mismatch result type");
		static_assert(OcclusionCulling::eViewCulled == MaskedOcclusionCulling::VIEW_CULLED, "mismatch result type");

		inline __m128d& operator+=(__m128d& v1, const __m128d& v2) { return (v1 = _mm_add_pd(v1, v2)); }
		inline __m128d& operator-=(__m128d& v1, const __m128d& v2) { return (v1 = _mm_sub_pd(v1, v2)); }
		inline __m128d& operator*=(__m128d& v1, const __m128d& v2) { return (v1 = _mm_mul_pd(v1, v2)); }
		inline __m128d& operator/=(__m128d& v1, const __m128d& v2) { return (v1 = _mm_div_pd(v1, v2)); }
		inline __m128d operator+(const __m128d& v1, const __m128d& v2) { return _mm_add_pd(v1, v2); }
		inline __m128d operator-(const __m128d& v1, const __m128d& v2) { return _mm_sub_pd(v1, v2); }
		inline __m128d operator*(const __m128d& v1, const __m128d& v2) { return _mm_mul_pd(v1, v2); }
		inline __m128d operator/(const __m128d& v1, const __m128d& v2) { return _mm_div_pd(v1, v2); }
		inline __m128& operator+=(__m128& v1, const __m128& v2) { return (v1 = _mm_add_ps(v1, v2)); }
		inline __m128& operator-=(__m128& v1, const __m128& v2) { return (v1 = _mm_sub_ps(v1, v2)); }
		inline __m128& operator*=(__m128& v1, const __m128& v2) { return (v1 = _mm_mul_ps(v1, v2)); }
		inline __m128& operator/=(__m128& v1, const __m128& v2) { return (v1 = _mm_div_ps(v1, v2)); }
		inline __m128 operator+(const __m128& v1, const __m128& v2) { return _mm_add_ps(v1, v2); }
		inline __m128 operator-(const __m128& v1, const __m128& v2) { return _mm_sub_ps(v1, v2); }
		inline __m128 operator*(const __m128& v1, const __m128& v2) { return _mm_mul_ps(v1, v2); }
		inline __m128 operator/(const __m128& v1, const __m128& v2) { return _mm_div_ps(v1, v2); }

		const float OccludeeSizeThreshold = 0.01f;

		class OcclusionCulling::Impl
		{
		public:
			Impl(uint32_t width, uint32_t height);
			~Impl();

		public:
			void Enable(bool isEnable) { m_isEnable = isEnable; }
			bool IsEnable() const { return m_isEnable; }

			void ClearBuffer();
			void Flush();
			void WakeThreads();
			void SuspendThreads();

			void Update(const Camera* pCamera);
			const Collision::Frustum& GetCameraFrustum() const { return m_cameraFrustum; }

		public:
			void RenderTriangles(const math::Matrix& matWorld, const VertexPos* pVertices, const uint32_t* pIndices, int indexCount);
			Result TestTriangles(const math::Matrix& matWorld, const VertexPos* pVertices, const uint32_t* pIndices, int indexCount);

		public:
			Result TestRect(float xmin, float ymin, float xmax, float ymax, float wmin) const;
			Result TestRect(const Collision::AABB& aabb) const;

		public:
			void Write(const char* strPath);

		private:
			void WriteBMP(const char *filename, const unsigned char *data, int w, int h);
			void TonemapDepth(float *depth, unsigned char *image, int w, int h);
			void SaveBMP();

			void SetFov(float fov)
			{
				if (math::IsSame(m_cameraFov, fov) == false)
				{
					m_cameraFov = fov;
					m_radiusThreshold = OccludeeSizeThreshold * OccludeeSizeThreshold * m_cameraFov;
				}
			}

			void SetNearClip(float nearClip)
			{
				if (math::IsSame(m_cameraNearClip, nearClip) == false)
				{
					m_cameraNearClip = nearClip;
					m_pCullingThreadPool->SetNearClipPlane(m_cameraNearClip);
				}
			}

		private:
			math::uint2 m_screenSize;

			math::Matrix m_matViewProjection;

			float m_cameraFov{ 0.f };
			float m_cameraNearClip{ 0.f };
			Collision::Frustum m_cameraFrustum;

			float m_radiusThreshold{ 0.f };
			MaskedOcclusionCulling* m_pMaskedOcclusionCulling{ nullptr };
			std::unique_ptr<CullingThreadpool> m_pCullingThreadPool;

			enum State
			{
				eInitialized = 0,
				eSuspend,
				eRunning,
				ePause,
				eStop,
			};
			State m_emState{ eInitialized };

			bool m_isEnable{ true };

			std::string m_savePath;

			std::mutex m_mutex;
			std::condition_variable m_condition;
			std::condition_variable m_condition_suspend;

			std::thread m_renderWaitThread;

			struct RenderWaitData
			{
				math::Matrix matWVP;
				const VertexPos* pVertices{ nullptr };
				const uint32_t* pIndices{ nullptr };
				int indexCount{ 0 };

				RenderWaitData() = default;
				RenderWaitData(const math::Matrix& matWVP, const VertexPos* pVertices, const uint32_t* pIndices, int indexCount)
					: matWVP(matWVP)
					, pVertices(pVertices)
					, pIndices(pIndices)
					, indexCount(indexCount)
				{
				}
			};
			std::queue<RenderWaitData> m_renderWaitDatas;
		};

		OcclusionCulling::Impl::Impl(uint32_t width, uint32_t height)
			: m_pMaskedOcclusionCulling{ MaskedOcclusionCulling::Create() }
			, m_screenSize(width, height)
		{
			const uint32_t nThreadCount = std::thread::hardware_concurrency() - 1;
			m_pCullingThreadPool = std::make_unique<CullingThreadpool>(nThreadCount, 10, 6, 128);
			m_pCullingThreadPool->SetBuffer(m_pMaskedOcclusionCulling);
			m_pCullingThreadPool->SetResolution(width, height);

			m_renderWaitThread = std::thread([&]()
			{
				while (true)
				{
					RenderWaitData renderWaitData;
					{
						std::unique_lock<std::mutex> lock(m_mutex);
						m_condition.wait(lock, [&]()
						{
							return m_emState == State::eStop || m_renderWaitDatas.empty() == false;
						});

						if (m_emState == State::eStop)
							break;

						renderWaitData = m_renderWaitDatas.front();
						m_renderWaitDatas.pop();
					}

					m_pCullingThreadPool->SetMatrix(&renderWaitData.matWVP._11);
					m_pCullingThreadPool->SetVertexLayout({ sizeof(VertexPos), 4, 8 });
					m_pCullingThreadPool->RenderTriangles(reinterpret_cast<const float*>(renderWaitData.pVertices), renderWaitData.pIndices, renderWaitData.indexCount / 3, MaskedOcclusionCulling::BACKFACE_CCW, MaskedOcclusionCulling::CLIP_PLANE_ALL);

					m_condition_suspend.notify_all();
				}
			});
		}

		OcclusionCulling::Impl::~Impl()
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_emState = State::eStop;
				m_renderWaitDatas = std::queue<RenderWaitData>();
			}
			m_condition.notify_all();

			ClearBuffer();
			MaskedOcclusionCulling::Destroy(m_pMaskedOcclusionCulling);
		}

		void OcclusionCulling::Impl::ClearBuffer()
		{
			if (m_emState != ePause)
			{
				{
					std::unique_lock<std::mutex> lock(m_mutex);
					m_condition_suspend.wait(lock, [&]()
					{
						return m_renderWaitDatas.empty() == true;
					});
				}
				m_pCullingThreadPool->ClearBuffer();
			}
		}

		void OcclusionCulling::Impl::Flush()
		{
			if (m_emState != ePause)
			{
				{
					std::unique_lock<std::mutex> lock(m_mutex);
					m_condition_suspend.wait(lock, [&]()
					{
						return m_renderWaitDatas.empty() == true;
					});
				}
				m_pCullingThreadPool->Flush();
			}
		}

		void OcclusionCulling::Impl::WakeThreads()
		{
			if (m_emState != ePause)
			{
				{
					std::lock_guard<std::mutex> lock(m_mutex);
					m_emState = State::eRunning;
				}
				m_pCullingThreadPool->WakeThreads();
			}
		}

		void OcclusionCulling::Impl::SuspendThreads()
		{
			if (m_emState != ePause)
			{
				{
					std::unique_lock<std::mutex> lock(m_mutex);
					m_emState = State::eSuspend;

					m_condition_suspend.wait(lock, [&]()
					{
						return m_renderWaitDatas.empty() == true;
					});
				}
				m_pCullingThreadPool->SuspendThreads();
			}
		}

		void OcclusionCulling::Impl::Update(const Camera* pCamera)
		{
			SaveBMP();

			if (m_isEnable == false && m_emState != State::ePause)
			{
				m_emState = State::ePause;
			}
			else if (m_isEnable == true && m_emState == State::ePause)
			{
				m_emState = State::eSuspend;
			}

			SetFov(pCamera->GetFOV());
			SetNearClip(pCamera->GetNearClip());

			m_cameraFrustum = pCamera->GetFrustum();
			m_matViewProjection = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();
		}

		void OcclusionCulling::Impl::RenderTriangles(const math::Matrix& matWorld, const VertexPos* pVertices, const uint32_t* pIndices, int indexCount)
		{
			if (pVertices == nullptr || pIndices == nullptr || (indexCount % 3 != 0))
				return;

			if (m_emState == ePause)
				return;

			if (m_emState == State::eSuspend)
			{
				LOG_WARNING("falied to RenderTriangles, OcclusionCulling thread is suspend mode now");
				return;
			}

			const math::Matrix matWVP = matWorld * m_matViewProjection;

			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_renderWaitDatas.emplace(matWVP, pVertices, pIndices, indexCount);
			}
			m_condition.notify_all();
		}

		OcclusionCulling::Result OcclusionCulling::Impl::TestTriangles(const math::Matrix& matWorld, const VertexPos* pVertices, const uint32_t* pIndices, int indexCount)
		{
			if (pVertices == nullptr || pIndices == nullptr || (indexCount % 3 != 0))
				return OcclusionCulling::eViewCulled;

			if (m_emState == ePause)
				return OcclusionCulling::eVisible;

			const math::Matrix matWVP = matWorld * m_matViewProjection;

			m_pCullingThreadPool->SetMatrix(&matWorld._11);
			m_pCullingThreadPool->SetVertexLayout({ sizeof(VertexPos), 4, 8 });
			return static_cast<Result>(m_pCullingThreadPool->TestTriangles(reinterpret_cast<const float*>(pVertices), pIndices, indexCount / 3, MaskedOcclusionCulling::BACKFACE_CCW, MaskedOcclusionCulling::CLIP_PLANE_ALL));
		}

		OcclusionCulling::Result OcclusionCulling::Impl::TestRect(float xmin, float ymin, float xmax, float ymax, float wmin) const
		{
			if (m_emState == ePause)
				return OcclusionCulling::eVisible;

			return static_cast<Result>(m_pCullingThreadPool->TestRect(xmin, ymin, xmax, ymax, wmin));
		}

		OcclusionCulling::Result OcclusionCulling::Impl::TestRect(const Collision::AABB& aabb) const
		{
			if (m_emState == ePause)
				return OcclusionCulling::eVisible;

			// 0 = use min corner, 1 = use max corner
			static const uint32_t sBBxInd[AABB_VERTICES]{ 1, 0, 0, 1, 1, 1, 0, 0 };
			static const uint32_t sBByInd[AABB_VERTICES]{ 1, 1, 1, 1, 0, 0, 0, 0 };
			static const uint32_t sBBzInd[AABB_VERTICES]{ 1, 1, 0, 0, 0, 1, 1, 0 };

			__m128 cumulativeMatrix[4];
			cumulativeMatrix[0] = _mm_loadu_ps(&m_matViewProjection._r0.x);
			cumulativeMatrix[1] = _mm_loadu_ps(&m_matViewProjection._r1.x);
			cumulativeMatrix[2] = _mm_loadu_ps(&m_matViewProjection._r2.x);
			cumulativeMatrix[3] = _mm_loadu_ps(&m_matViewProjection._r3.x);

			const float w = aabb.Center.x * cumulativeMatrix[0].m128_f32[3] +
				aabb.Center.y * cumulativeMatrix[1].m128_f32[3] +
				aabb.Center.z * cumulativeMatrix[2].m128_f32[3] +
				cumulativeMatrix[3].m128_f32[3];

			if (w > 1.f && (aabb.Extents.LengthSquared() < w * m_radiusThreshold))
				return Result::eViewCulled;

			// w ends up being garbage, but it doesn't matter - we ignore it anyway.
			const __m128 vCenter = _mm_loadu_ps(&aabb.Center.x);
			const __m128 vHalf = _mm_loadu_ps(&aabb.Extents.x);

			const __m128 vMin = _mm_sub_ps(vCenter, vHalf);
			const __m128 vMax = _mm_add_ps(vCenter, vHalf);

			// transforms
			__m128 xRow[2], yRow[2], zRow[2];
			xRow[0] = _mm_shuffle_ps(vMin, vMin, 0x00) * cumulativeMatrix[0];
			xRow[1] = _mm_shuffle_ps(vMax, vMax, 0x00) * cumulativeMatrix[0];
			yRow[0] = _mm_shuffle_ps(vMin, vMin, 0x55) * cumulativeMatrix[1];
			yRow[1] = _mm_shuffle_ps(vMax, vMax, 0x55) * cumulativeMatrix[1];
			zRow[0] = _mm_shuffle_ps(vMin, vMin, 0xaa) * cumulativeMatrix[2];
			zRow[1] = _mm_shuffle_ps(vMax, vMax, 0xaa) * cumulativeMatrix[2];

			__m128 zAllIn = _mm_castsi128_ps(_mm_set1_epi32(~0));
			__m128 screenMin = _mm_set1_ps(FLT_MAX);
			__m128 screenMax = _mm_set1_ps(-FLT_MAX);

			// Find the minimum of each component
			const __m128 minvert = _mm_add_ps(cumulativeMatrix[3], _mm_add_ps(_mm_add_ps(_mm_min_ps(xRow[0], xRow[1]), _mm_min_ps(yRow[0], yRow[1])), _mm_min_ps(zRow[0], zRow[1])));
			const float minW = minvert.m128_f32[3];
			if (minW < 0.00000001f)
				return Result::eVisible;

			for (UINT i = 0; i < AABB_VERTICES; i++)
			{
				// Transform the vertex
				__m128 vert = cumulativeMatrix[3];
				vert += xRow[sBBxInd[i]];
				vert += yRow[sBByInd[i]];
				vert += zRow[sBBzInd[i]];

				// We have inverted z; z is in front of near plane iff z <= w.
				__m128 vertZ = _mm_shuffle_ps(vert, vert, 0xaa); // vert.zzzz
				__m128 vertW = _mm_shuffle_ps(vert, vert, 0xff); // vert.wwww

				// project
				__m128 xformedPos = _mm_div_ps(vert, vertW);

				// update bounds
				screenMin = _mm_min_ps(screenMin, xformedPos);
				screenMax = _mm_max_ps(screenMax, xformedPos);
			}

			return TestRect(screenMin.m128_f32[0], screenMin.m128_f32[1], screenMax.m128_f32[0], screenMax.m128_f32[1], minW);

			/*
			// 아래 코드는 위 avx 코드에 대한 해석
			// 우리 수학 라이브러리로 TestRect 를 사용하는 경우
			// avx 코드를 사용하는 편이 아래의 코드보다 약 10 ~ 15% 가량 더 빠르다.
			// 하지만 0.00009(sec) -> 0.000075(sec) 차이로 매우 미묘한 차이
			// TestRect 하는 수가 많아지면 avx 사용의 효과도 커지겠지 ~_~
			math::float4 center(aabb.Center.x, aabb.Center.y, aabb.Center.z, 1.f);
			center = math::float4::Transform(center, m_matViewProjection);
			if (center.w > 1.f && (aabb.Extents.LengthSquared() < center.w * m_radiusThreshold))
				return Result::eViewCulled;
			
			math::float3 corners[Collision::AABB::CORNER_COUNT];
			aabb.GetCorners(corners);
			
			math::float4 max(std::numeric_limits<float>::min());
			math::float4 min(std::numeric_limits<float>::max());
			
			float minW = std::numeric_limits<float>::max();
			
			for (int i = 0; i < AABB_VERTICES; ++i)
			{
				const math::float4 corner(corners[i].x, corners[i].y, corners[i].z, 1.f);
				math::float4 pos = math::float4::Transform(corner, m_matViewProjection);
			
				minW = std::min(minW, pos.w);
			
				pos /= pos.w;
			
				max = math::float4::Max(pos, max);
				min = math::float4::Min(pos, min);
			}
			
			if (minW < 0.00000001f)
				return Result::eVisible;
			
			return TestRect(min.x, min.y, max.x, max.y, minW);
			*/
		}

		void OcclusionCulling::Impl::TonemapDepth(float *depth, unsigned char *image, int w, int h)
		{
			// Find min/max w coordinate (discard cleared pixels)
			float minW = std::numeric_limits<float>::max();
			float maxW = std::numeric_limits<float>::min();
			for (int i = 0; i < w * h; ++i)
			{
				if (depth[i] > 0.0f)
				{
					minW = std::min(minW, depth[i]);
					maxW = std::max(maxW, depth[i]);
				}
			}

			// Tonemap depth values
			for (int i = 0; i < w * h; ++i)
			{
				int intensity = 0;
				if (depth[i] > 0)
				{
					intensity = (unsigned char)(223.0*(depth[i] - minW) / (maxW - minW) + 32.0);
				}

				image[i * 3 + 0] = (unsigned char)intensity;
				image[i * 3 + 1] = (unsigned char)intensity;
				image[i * 3 + 2] = (unsigned char)intensity;
			}
		}

		void OcclusionCulling::Impl::WriteBMP(const char *filename, const unsigned char *data, int w, int h)
		{
			const short header[] = { 0x4D42, 0, 0, 0, 0, 26, 0, 12, 0, static_cast<short>(w), static_cast<short>(h), 1, 24 };

			FILE* f = nullptr;
			if (fopen_s(&f, filename, "wb") == 0)
			{
				fwrite(header, 1, sizeof(header), f);
				fwrite(data, 1, w * h * 3, f);
				fclose(f);
			}
			else
			{
				printf("\nError trying to save to %s", filename);
			}
		}

		void OcclusionCulling::Impl::SaveBMP()
		{
			if (m_savePath.empty() == true)
				return;

			std::vector<float> perPixelZBuffer;
			perPixelZBuffer.resize(m_screenSize.x * m_screenSize.y);

			m_pMaskedOcclusionCulling->ComputePixelDepthBuffer(perPixelZBuffer.data(), true);

			std::vector<unsigned char> image;
			image.resize(m_screenSize.x * m_screenSize.y * 3);

			const math::int2 screenSize(static_cast<int>(m_screenSize.x), static_cast<int>(m_screenSize.y));
			TonemapDepth(perPixelZBuffer.data(), image.data(), screenSize.x, screenSize.y);
			WriteBMP(m_savePath.c_str(), image.data(), screenSize.x, screenSize.y);

			m_savePath.clear();
		}

		void OcclusionCulling::Impl::Write(const char* strPath)
		{
			m_savePath = strPath;
		}

		OcclusionCulling::OcclusionCulling()
		{
		}

		OcclusionCulling::~OcclusionCulling()
		{
		}

		void OcclusionCulling::Initialize(uint32_t width, uint32_t height)
		{
			m_pImpl = std::make_unique<Impl>(width, height);
		}

		void OcclusionCulling::Enable(bool isEnable)
		{
			m_pImpl->Enable(isEnable);
		}

		bool OcclusionCulling::IsEnable() const
		{
			return m_pImpl->IsEnable();
		}

		void OcclusionCulling::ClearBuffer()
		{
			m_pImpl->ClearBuffer();
		}

		void OcclusionCulling::Flush()
		{
			m_pImpl->Flush();
		}

		void OcclusionCulling::WakeThreads()
		{
			m_pImpl->WakeThreads();
		}

		void OcclusionCulling::SuspendThreads()
		{
			m_pImpl->SuspendThreads();
		}

		void OcclusionCulling::Update(const Camera* pCamera)
		{
			m_pImpl->Update(pCamera);
		}

		const Collision::Frustum& OcclusionCulling::GetCameraFrustum() const
		{
			return m_pImpl->GetCameraFrustum();
		}

		void OcclusionCulling::RenderTriangles(const math::Matrix& matWorld, const VertexPos* pVertices, const uint32_t* pIndices, size_t indexCount)
		{
			assert(indexCount <= std::numeric_limits<int>::max());
			m_pImpl->RenderTriangles(matWorld, pVertices, pIndices, static_cast<int>(indexCount));
		}

		OcclusionCulling::Result OcclusionCulling::TestTriangles(const math::Matrix& matWorld, const VertexPos* pVertices, const uint32_t* pIndices, size_t indexCount)
		{
			assert(indexCount <= std::numeric_limits<int>::max());
			return m_pImpl->TestTriangles(matWorld, pVertices, pIndices, static_cast<int>(indexCount));
		}

		OcclusionCulling::Result OcclusionCulling::TestRect(float xmin, float ymin, float xmax, float ymax, float wmin) const
		{
			return m_pImpl->TestRect(xmin, ymin, xmax, ymax, wmin);
		}

		OcclusionCulling::Result OcclusionCulling::TestRect(const Collision::AABB& aabb) const
		{
			return m_pImpl->TestRect(aabb);
		}

		void OcclusionCulling::Write(const char* strPath)
		{
			m_pImpl->Write(strPath);
		}
	}
}