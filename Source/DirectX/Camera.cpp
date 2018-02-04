#include "stdafx.h"
#include "Camera.h"

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Camera::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void SetView(const Math::Vector3& vEye, const Math::Vector3& vLookat, const Math::Vector3& vUp);
			void SetProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar);

			void Update(float fElapsedTime);

		public:
			void UpdateView(int nThreadID);
			void UpdateProjection(int nThreadID);
			void UpdateOrtho(int nThreadID);

			void UpdateFrustum(int nThreadID);

			void MoveFront(float fDist);
			void MoveForward(float fDist);
			void MoveSideward(float fDist);
			void MoveUpward(float fDist);

			void RotateAxisX(float fAngle);
			void RotateAxisY(float fAngle);
			void RotateAxisZ(float fAngle);

		public:
			void SetThirdView(const Math::Vector3& vLookat) { m_isThirdPerson = true; m_f3Eye += (vLookat - m_f3Lookat); m_f3Lookat = vLookat; }
			bool IsThirdPersonCamera() { return m_isThirdPerson; }

			void SetDistance(float fDistance) { m_f3Eye = m_f3Lookat - GetDir() * fDistance; m_fDistance = fDistance; }
			void AddDistance(float fOffset) { m_fAddDistance += fOffset; }

			void SetPosition(const Math::Vector3& vEye) { m_f3Eye = vEye; }
			const Math::Vector3& GetPosition() { return m_f3Eye; }

			void SetLookat(const Math::Vector3& vLookat) { m_f3Lookat = vLookat; }
			const Math::Vector3& GetLookat() { return m_f3Lookat; }

			void SetUp(const Math::Vector3& vUp) { m_f3Up = vUp; }
			const Math::Vector3& GetUp() { return m_f3Up; }

			Math::Vector3 GetDir() { Math::Vector3 v(m_f3Lookat - m_f3Eye); v.Normalize(); return v; }

			const Math::Matrix& GetViewMatrix(int nThreadID) { return m_matView[nThreadID]; }
			const Math::Matrix& GetProjMatrix(int nThreadID) { return m_matProj[nThreadID]; }
			const Math::Matrix& GetOrthoMatrix(int nThreadID) { return m_matOrtho[nThreadID]; }

			float GetFarClip(int nThreadID) const { return m_frustum[nThreadID].Far; }
			float GetNearClip(int nThreadID) const { return m_frustum[nThreadID].Near; }

		public:
			const Collision::Frustum& GetFrustum(int nThreadID) const { return m_frustum[nThreadID]; }

		private:
			bool m_isThirdPerson{ false };

			// View
			Math::Vector3 m_f3Eye;
			Math::Vector3 m_f3Lookat{ 0.f, 0.f, -1.f };
			Math::Vector3 m_f3Up{ 0.f, 1.f, 0.f };

			std::array<Math::Matrix, ThreadCount> m_matView;

			// Projection
			uint32_t m_nWidth{ 0 };
			uint32_t m_nHeight{ 0 };
			float m_fFov{ 0.f };		// Field Of View 보는 각도 시야각
			float m_fNear{ 0.f };		// 제일 가까운 시야
			float m_fFar{ 0.f };		// 제일 먼 시야

			std::array<Math::Matrix, ThreadCount> m_matProj;

			// Ortho
			std::array<Math::Matrix, ThreadCount> m_matOrtho;

			float m_fDistance{ 0.f };	// 3인칭 카메라 거리
			float m_fAddDistance{ 0.f };

			// Move
			float m_fMoveFront{ 0.f };
			float m_fMoveForward{ 0.f };
			float m_fMoveSideward{ 0.f };
			float m_fMoveUpward{ 0.f };

			// Rotate
			Math::Vector3 m_f3RotationOffset;
			Math::Vector3 m_f3Rotation;

			std::array<Collision::Frustum, ThreadCount> m_frustum;

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

		void Camera::Impl::SetView(const Math::Vector3& vEye, const Math::Vector3& vLookat, const Math::Vector3& vUp)
		{
			m_f3Eye = vEye;
			m_f3Lookat = vLookat;
			m_f3Up = vUp;

			m_fDistance = Math::Vector3::Distance(m_f3Eye, m_f3Lookat);

			UpdateView(ThreadType::eUpdate);
			UpdateView(ThreadType::eRender);
		}

		void Camera::Impl::SetProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar)
		{
			m_nWidth = nWidth;
			m_nHeight = nHeight;
			m_fFov = fFov;
			m_fNear = fNear;
			m_fFar = fFar;

			UpdateProjection(ThreadType::eUpdate);
			UpdateProjection(ThreadType::eRender);
		}

		void Camera::Impl::Update(float fElapsedTime)
		{
			const float fMin = 0.0001f;
			int nThreadID = GetThreadID(ThreadType::eUpdate);

			float fValue = fElapsedTime * 5.f;
			float fDeValue = 1.f - fValue;

			if (std::abs(m_fAddDistance) > fMin)
			{
				float fOffset = m_fAddDistance * fValue;
				m_fAddDistance *= fDeValue;

				m_fDistance += fOffset;
				m_fDistance = Math::Clamp(m_fDistance + fOffset, m_fMinDistance, m_fMaxDistance);

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

				Math::Vector3 vForward(m_matView[nThreadID].Forward());
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

				Math::Vector3 vForward(m_matView[nThreadID].Forward());
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

				Math::Vector3 vRight(m_matView[nThreadID].Right());
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

				Math::Vector3 vUp(m_matView[nThreadID].Up());
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

			float fDist = Math::Vector3::Distance(m_f3Lookat, m_f3Eye);
			Math::Matrix matRot = Math::Matrix::CreateFromYawPitchRoll(Math::ToRadians(m_f3Rotation.y), Math::ToRadians(m_f3Rotation.x), Math::ToRadians(m_f3Rotation.z));

			m_f3Lookat = (Math::Vector3::Transform(Math::Vector3::Forward, matRot) * fDist) + m_f3Eye;
			m_f3Up = Math::Vector3::Transform(Math::Vector3::Up, matRot);

			UpdateView(nThreadID);

			UpdateFrustum(nThreadID);
		}

		void Camera::Impl::UpdateView(int nThreadID)
		{
			m_matView[nThreadID] = Math::Matrix::CreateLookAt(m_f3Eye, m_f3Lookat, m_f3Up);
		}

		void Camera::Impl::UpdateProjection(int nThreadID)
		{
			m_matProj[nThreadID] = Math::Matrix::CreatePerspectiveFieldOfView(m_fFov, static_cast<float>(m_nWidth) / static_cast<float>(m_nHeight), m_fNear, m_fFar);
		}

		void Camera::Impl::UpdateOrtho(int nThreadID)
		{
			m_matOrtho[nThreadID] = Math::Matrix::CreateOrthographic(static_cast<float>(m_nWidth), static_cast<float>(m_nHeight), m_fNear, m_fFar);
		}

		void Camera::Impl::UpdateFrustum(int nThreadID)
		{
			Collision::Frustum::CreateFromMatrix(m_frustum[nThreadID], m_matProj[nThreadID]);
			m_frustum[nThreadID].Transform(m_matView[nThreadID].Invert());
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

		void Camera::SetView(const Math::Vector3& vEye, const Math::Vector3& vLookat, const Math::Vector3& vUp)
		{
			m_pImpl->SetView(vEye, vLookat, vUp);
		}

		void Camera::SetProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar)
		{
			m_pImpl->SetProjection(nWidth, nHeight, fFov, fNear, fFar);
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

		void Camera::SetThirdView(const Math::Vector3& vLookat)
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

		void Camera::SetPosition(const Math::Vector3& vEye)
		{
			m_pImpl->SetPosition(vEye);
		}

		const Math::Vector3& Camera::GetPosition()
		{
			return m_pImpl->GetPosition();
		}

		void Camera::SetLookat(const Math::Vector3& vLookat)
		{
			m_pImpl->SetLookat(vLookat);
		}

		const Math::Vector3& Camera::GetLookat()
		{
			return m_pImpl->GetLookat();
		}
		
		void Camera::SetUp(const Math::Vector3& vUp)
		{
			m_pImpl->SetUp(vUp);
		}

		const Math::Vector3& Camera::GetUp()
		{
			return m_pImpl->GetUp();
		}

		Math::Vector3 Camera::GetDir()
		{
			return m_pImpl->GetDir();
		}

		const Math::Matrix& Camera::GetViewMatrix(int nThreadID)
		{
			return m_pImpl->GetViewMatrix(nThreadID);
		}

		const Math::Matrix& Camera::GetProjMatrix(int nThreadID)
		{
			return m_pImpl->GetProjMatrix(nThreadID);
		}

		const Math::Matrix& Camera::GetOrthoMatrix(int nThreadID)
		{
			return m_pImpl->GetOrthoMatrix(nThreadID);
		}

		float Camera::GetFarClip(int nThreadID) const
		{
			return m_pImpl->GetFarClip(nThreadID);
		}

		float Camera::GetNearClip(int nThreadID) const
		{
			return m_pImpl->GetNearClip(nThreadID);
		}

		const Collision::Frustum& Camera::GetFrustum(int nThreadID) const
		{
			return m_pImpl->GetFrustum(nThreadID);
		}
	}
}