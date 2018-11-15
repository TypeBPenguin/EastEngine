#include "stdafx.h"
#include "Camera.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void SetView(const math::float3& vEye, const math::float3& vLookat, const math::float3& vUp);
			void SetProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar, bool isUpsideDown);

			void Update(float fElapsedTime);

		public:
			void UpdateView();
			void UpdateProjection();
			void UpdateOrtho();

			void UpdateFrustum();

			void MoveFront(float fDist);
			void MoveForward(float fDist);
			void MoveSideward(float fDist);
			void MoveUpward(float fDist);

			void RotateAxisX(float fAngle);
			void RotateAxisY(float fAngle);
			void RotateAxisZ(float fAngle);

		public:
			void SetThirdView(const math::float3& vLookat) { m_isThirdPerson = true; m_f3Eye += (vLookat - m_f3Lookat); m_f3Lookat = vLookat; }
			bool IsThirdPersonCamera() { return m_isThirdPerson; }

			void SetDistance(float fDistance) { m_f3Eye = m_f3Lookat - GetDir() * fDistance; m_fDistance = fDistance; }
			void AddDistance(float fOffset) { m_fAddDistance += fOffset; }

			void SetPosition(const math::float3& vEye) { m_f3Eye = vEye; }
			const math::float3& GetPosition() { return m_f3Eye; }

			void SetLookat(const math::float3& vLookat) { m_f3Lookat = vLookat; }
			const math::float3& GetLookat() { return m_f3Lookat; }

			void SetUp(const math::float3& vUp) { m_f3Up = vUp; }
			const math::float3& GetUp() { return m_f3Up; }

			math::float3 GetDir() { math::float3 v(m_f3Lookat - m_f3Eye); v.Normalize(); return v; }

			const math::Matrix& GetViewMatrix() { return m_matView; }
			const math::Matrix& GetProjMatrix() { return m_matProj; }
			const math::Matrix& GetOrthoMatrix() { return m_matOrtho; }

			float GetFOV() const { return m_fFov; }
			float GetFarClip() const { return m_frustum.Far; }
			float GetNearClip() const { return m_frustum.Near; }

		public:
			const Collision::Frustum& GetFrustum() const { return m_frustum; }

		private:
			bool m_isThirdPerson{ false };

			// View
			math::float3 m_f3Eye;
			math::float3 m_f3Lookat{ 0.f, 0.f, -1.f };
			math::float3 m_f3Up{ 0.f, 1.f, 0.f };

			math::Matrix m_matView;

			// Projection
			uint32_t m_nWidth{ 0 };
			uint32_t m_nHeight{ 0 };
			float m_fFov{ 0.f };		// Field Of View 보는 각도 시야각
			float m_fNear{ 0.f };		// 제일 가까운 시야
			float m_fFar{ 0.f };		// 제일 먼 시야

			math::Matrix m_matProj;

			// Ortho
			math::Matrix m_matOrtho;

			float m_fDistance{ 0.f };	// 3인칭 카메라 거리
			float m_fAddDistance{ 0.f };

			// Move
			float m_fMoveFront{ 0.f };
			float m_fMoveForward{ 0.f };
			float m_fMoveSideward{ 0.f };
			float m_fMoveUpward{ 0.f };

			// Rotate
			math::float3 m_f3RotationOffset;
			math::float3 m_f3Rotation;

			Collision::Frustum m_frustum;

			float m_fMaxMoveSpeed{ 10.f };
			float m_fMaxRotSpeed{ 10.f };
			float m_fMinDistance{ 1.f };
			float m_fMaxDistance{ 50.f };
		};

		Camera::Impl::Impl()
		{
		}

		Camera::Impl::~Impl()
		{
		}

		void Camera::Impl::SetView(const math::float3& vEye, const math::float3& vLookat, const math::float3& vUp)
		{
			m_f3Eye = vEye;
			m_f3Lookat = vLookat;
			m_f3Up = vUp;

			m_fDistance = math::float3::Distance(m_f3Eye, m_f3Lookat);

			UpdateView();
		}

		void Camera::Impl::SetProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar, bool isUpsideDown)
		{
			m_nWidth = nWidth;
			m_nHeight = nHeight;
			m_fFov = fFov;
			m_fNear = fNear;
			m_fFar = fFar;

			UpdateProjection();

			if (isUpsideDown == true)
			{
				m_matProj.m[1][1] *= -1;
			}
		}

		void Camera::Impl::Update(float fElapsedTime)
		{
			TRACER_EVENT("Camera::Update");
			const float fMin = 0.0001f;

			float fValue = fElapsedTime * 5.f;
			float fDeValue = 1.f - fValue;

			if (std::abs(m_fAddDistance) > fMin)
			{
				float fOffset = m_fAddDistance * fValue;
				m_fAddDistance *= fDeValue;

				m_fDistance += fOffset;
				m_fDistance = std::clamp(m_fDistance + fOffset, m_fMinDistance, m_fMaxDistance);

				SetDistance(m_fDistance);
			}
			else
			{
				m_fAddDistance = 0.f;
			}

			if (std::abs(m_fMoveFront) > fMin)
			{
				float fOffset = m_fMoveFront * fValue;
				m_fMoveFront *= fDeValue;

				math::float3 vForward(m_matView.Forward());
				vForward.Normalize();

				vForward.y = 0.f;

				m_f3Eye = m_f3Eye + vForward * fOffset;
				m_f3Lookat = m_f3Lookat + vForward * fOffset;
			}
			else
			{
				m_fMoveFront = 0.f;
			}

			if (std::abs(m_fMoveForward) > fMin)
			{
				float fOffset = m_fMoveForward * fValue;
				m_fMoveForward *= fDeValue;

				math::float3 vForward(m_matView.Forward());
				vForward.Normalize();

				m_f3Eye = m_f3Eye + vForward * fOffset;
				m_f3Lookat = m_f3Lookat + vForward * fOffset;
			}
			else
			{
				m_fMoveForward = 0.f;
			}

			if (std::abs(m_fMoveSideward) > fMin)
			{
				float fOffset = m_fMoveSideward * fValue;
				m_fMoveSideward *= fDeValue;

				math::float3 vRight(m_matView.Right());
				vRight.Normalize();

				m_f3Eye = m_f3Eye + vRight * fOffset;
				m_f3Lookat = m_f3Lookat + vRight * fOffset;
			}
			else
			{
				m_fMoveSideward = 0.f;
			}

			if (std::abs(m_fMoveUpward) > fMin)
			{
				float fOffset = m_fMoveUpward * fValue;
				m_fMoveUpward *= fDeValue;

				math::float3 vUp(m_matView.Up());
				vUp.Normalize();

				m_f3Eye = m_f3Eye + vUp * fOffset;
				m_f3Lookat = m_f3Lookat + vUp * fOffset;
			}
			else
			{
				m_fMoveUpward = 0.f;
			}

			if (std::abs(m_f3RotationOffset.x) > fMin)
			{
				float fOffset = m_f3RotationOffset.x * fValue * 5.f;
				m_f3RotationOffset.x *= fDeValue;

				m_f3Rotation.x += fOffset;

				if (m_f3Rotation.x < 0.f)
				{
					m_f3Rotation.x += 360.f;
				}
				else if (m_f3Rotation.x > 360.f)
				{
					m_f3Rotation.x -= 360.f;
				}
			}
			else
			{
				m_f3RotationOffset.x = 0.f;
			}

			if (std::abs(m_f3RotationOffset.y) > fMin)
			{
				float fOffset = m_f3RotationOffset.y * fValue * 5.f;
				m_f3RotationOffset.y *= fDeValue;

				m_f3Rotation.y += fOffset;

				if (m_f3Rotation.y < 0.f)
				{
					m_f3Rotation.y += 360.f;
				}
				else if (m_f3Rotation.y > 360.f)
				{
					m_f3Rotation.y -= 360.f;
				}
			}
			else
			{
				m_f3RotationOffset.y = 0.f;
			}

			if (std::abs(m_f3RotationOffset.z) > fMin)
			{
				float fOffset = m_f3RotationOffset.z * fValue * 5.f;
				m_f3RotationOffset.z *= fDeValue;

				m_f3Rotation.z += fOffset;

				if (m_f3Rotation.z < 0.f)
				{
					m_f3Rotation.z += 360.f;
				}
				else if (m_f3Rotation.z > 360.f)
				{
					m_f3Rotation.z -= 360.f;
				}
			}
			else
			{
				m_f3RotationOffset.z = 0.f;
			}

			float fDist = math::float3::Distance(m_f3Lookat, m_f3Eye);
			math::Matrix matRot = math::Matrix::CreateFromYawPitchRoll(math::ToRadians(m_f3Rotation.y), math::ToRadians(m_f3Rotation.x), math::ToRadians(m_f3Rotation.z));

			m_f3Lookat = (math::float3::Transform(math::float3::Forward, matRot) * fDist) + m_f3Eye;
			m_f3Up = math::float3::Transform(math::float3::Up, matRot);

			UpdateView();

			UpdateFrustum();
		}

		void Camera::Impl::UpdateView()
		{
			m_matView = math::Matrix::CreateLookAt(m_f3Eye, m_f3Lookat, m_f3Up);
		}

		void Camera::Impl::UpdateProjection()
		{
			m_matProj = math::Matrix::CreatePerspectiveFieldOfView(m_fFov, static_cast<float>(m_nWidth) / static_cast<float>(m_nHeight), m_fNear, m_fFar);
		}

		void Camera::Impl::UpdateOrtho()
		{
			m_matOrtho = math::Matrix::CreateOrthographic(static_cast<float>(m_nWidth), static_cast<float>(m_nHeight), m_fNear, m_fFar);
		}

		void Camera::Impl::UpdateFrustum()
		{
			Collision::Frustum::CreateFromMatrix(m_frustum, m_matProj);
			m_frustum.Transform(m_matView.Invert());
		}

		void Camera::Impl::MoveFront(float fDist)
		{
			m_fMoveFront += fDist;
			m_fMoveFront = std::clamp(m_fMoveFront, -m_fMaxMoveSpeed, m_fMaxMoveSpeed);
		}

		void Camera::Impl::MoveForward(float fDist)
		{
			m_fMoveForward += fDist;
			m_fMoveForward = std::clamp(m_fMoveForward, -m_fMaxMoveSpeed, m_fMaxMoveSpeed);
		}

		void Camera::Impl::MoveSideward(float fDist)
		{
			m_fMoveSideward += fDist;
			m_fMoveSideward = std::clamp(m_fMoveSideward, -m_fMaxMoveSpeed, m_fMaxMoveSpeed);
		}

		void Camera::Impl::MoveUpward(float fDist)
		{
			m_fMoveUpward += fDist;
			m_fMoveUpward = std::clamp(m_fMoveUpward, -m_fMaxMoveSpeed, m_fMaxMoveSpeed);
		}
		
		void Camera::Impl::RotateAxisX(float fAngle)
		{
			m_f3RotationOffset.x += fAngle;
			m_f3RotationOffset.x = std::clamp(m_f3RotationOffset.x, -m_fMaxRotSpeed, m_fMaxRotSpeed);
		}

		void Camera::Impl::RotateAxisY(float fAngle)
		{
			m_f3RotationOffset.y += fAngle;
			m_f3RotationOffset.y = std::clamp(m_f3RotationOffset.y, -m_fMaxRotSpeed, m_fMaxRotSpeed);
		}

		void Camera::Impl::RotateAxisZ(float fAngle)
		{
			m_f3RotationOffset.z += fAngle;
			m_f3RotationOffset.z = std::clamp(m_f3RotationOffset.z, -m_fMaxRotSpeed, m_fMaxRotSpeed);
		}

		Camera::Camera()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		Camera::~Camera()
		{
		}

		void Camera::SetView(const math::float3& vEye, const math::float3& vLookat, const math::float3& vUp)
		{
			m_pImpl->SetView(vEye, vLookat, vUp);
		}

		void Camera::SetProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar, bool isUpsideDown)
		{
			m_pImpl->SetProjection(nWidth, nHeight, fFov, fNear, fFar, isUpsideDown);
		}

		void Camera::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}

		void Camera::MoveFront(float fDist)
		{
			m_pImpl->MoveFront(fDist);
		}

		void Camera::MoveForward(float fDist)
		{
			m_pImpl->MoveForward(fDist);
		}

		void Camera::MoveSideward(float fDist)
		{
			m_pImpl->MoveSideward(fDist);
		}

		void Camera::MoveUpward(float fDist)
		{
			m_pImpl->MoveUpward(fDist);
		}

		void Camera::RotateAxisX(float fAngle)
		{
			m_pImpl->RotateAxisX(fAngle);
		}

		void Camera::RotateAxisY(float fAngle)
		{
			m_pImpl->RotateAxisY(fAngle);
		}

		void Camera::RotateAxisZ(float fAngle)
		{
			m_pImpl->RotateAxisZ(fAngle);
		}

		void Camera::SetThirdView(const math::float3& vLookat)
		{
			m_pImpl->SetThirdView(vLookat);
		}

		bool Camera::IsThirdPersonCamera()
		{
			return m_pImpl->IsThirdPersonCamera();
		}

		void Camera::SetDistance(float fDistance)
		{
			m_pImpl->SetDistance(fDistance);
		}

		void Camera::AddDistance(float fOffset)
		{
			m_pImpl->AddDistance(fOffset);
		}

		void Camera::SetPosition(const math::float3& vEye)
		{
			m_pImpl->SetPosition(vEye);
		}

		const math::float3& Camera::GetPosition() const
		{
			return m_pImpl->GetPosition();
		}

		void Camera::SetLookat(const math::float3& vLookat)
		{
			m_pImpl->SetLookat(vLookat);
		}

		const math::float3& Camera::GetLookat() const
		{
			return m_pImpl->GetLookat();
		}
		
		void Camera::SetUp(const math::float3& vUp)
		{
			m_pImpl->SetUp(vUp);
		}

		const math::float3& Camera::GetUp() const
		{
			return m_pImpl->GetUp();
		}

		math::float3 Camera::GetDir() const
		{
			return m_pImpl->GetDir();
		}

		const math::Matrix& Camera::GetViewMatrix() const
		{
			return m_pImpl->GetViewMatrix();
		}

		const math::Matrix& Camera::GetProjMatrix() const
		{
			return m_pImpl->GetProjMatrix();
		}

		const math::Matrix& Camera::GetOrthoMatrix() const
		{
			return m_pImpl->GetOrthoMatrix();
		}

		float Camera::GetFOV() const
		{
			return m_pImpl->GetFOV();
		}

		float Camera::GetFarClip() const
		{
			return m_pImpl->GetFarClip();
		}

		float Camera::GetNearClip() const
		{
			return m_pImpl->GetNearClip();
		}

		const Collision::Frustum& Camera::GetFrustum() const
		{
			return m_pImpl->GetFrustum();
		}
	}
}