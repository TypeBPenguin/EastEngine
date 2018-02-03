#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		class Camera
		{
		public:
			Camera();
			~Camera();

		public:
			void InitView(const Math::Vector3& vEye, const Math::Vector3& vLookat, const Math::Vector3& vUp);
			void InitProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar);

			void Update(float fElapsedTime);

		public:
			void UpdateView() { m_matView = Math::Matrix::CreateLookAt(m_f3Eye, m_f3Lookat, m_f3Up); }
			void UpdateProjection() { m_matProj = Math::Matrix::CreatePerspectiveFieldOfView(m_fFov, (float)m_nWidth / (float)m_nHeight, m_fNear, m_fFar); }
			void UpdateOrtho() { m_matOrtho = Math::Matrix::CreateOrthographic((float)m_nWidth, (float)m_nHeight, m_fNear, m_fFar); }

			void UpdateFrustum();

			void MoveFront(float fDist) { m_fMoveFront += fDist; m_fMoveFront = Math::Min(s_fMaxMoveSpeed, m_fMoveFront); m_fMoveFront = Math::Max(-s_fMaxMoveSpeed, m_fMoveFront); }
			void MoveForward(float fDist) { m_fMoveForward += fDist; m_fMoveForward = Math::Min(s_fMaxMoveSpeed, m_fMoveForward); m_fMoveForward = Math::Max(-s_fMaxMoveSpeed, m_fMoveForward); }
			void MoveSideward(float fDist) { m_fMoveSideward += fDist; m_fMoveSideward = Math::Min(s_fMaxMoveSpeed, m_fMoveSideward); m_fMoveSideward = Math::Max(-s_fMaxMoveSpeed, m_fMoveSideward); }
			void MoveUpward(float fDist) { m_fMoveUpward += fDist; m_fMoveUpward = Math::Min(s_fMaxMoveSpeed, m_fMoveUpward); m_fMoveUpward = Math::Max(-s_fMaxMoveSpeed, m_fMoveUpward); }

			void RotateAxisX(float fAngle) { m_f3RotationOffset.x += fAngle; m_f3RotationOffset.x = Math::Clamp(m_f3RotationOffset.x, -s_fMaxRotSpeed, s_fMaxRotSpeed); }
			void RotateAxisY(float fAngle) { m_f3RotationOffset.y += fAngle; m_f3RotationOffset.y = Math::Clamp(m_f3RotationOffset.y, -s_fMaxRotSpeed, s_fMaxRotSpeed); }
			void RotateAxisZ(float fAngle) { m_f3RotationOffset.z += fAngle; m_f3RotationOffset.z = Math::Clamp(m_f3RotationOffset.z, -s_fMaxRotSpeed, s_fMaxRotSpeed); }

			void SetThirdView(const Math::Vector3& vLookat) { m_isThirdPerson = true; m_f3Eye += (vLookat - m_f3Lookat); m_f3Lookat = vLookat; }

			void SetDistance(float fDistance) { m_f3Eye = m_f3Lookat - GetDir() * fDistance; m_fDistance = fDistance; }
			void AddDistance(float fOffset) { m_fAddDistance += fOffset; }

			bool IsThirdPersonCamera() { return m_isThirdPerson; }

			void SetPosition(const Math::Vector3& vEye) { m_f3Eye = vEye; }
			const Math::Vector3& GetPosition() { return m_f3Eye; }

			void SetLookat(const Math::Vector3& vLookat) { m_f3Lookat = vLookat; }
			const Math::Vector3& GetLookat() { return m_f3Lookat; }

			void SetUp(const Math::Vector3& vUp) { m_f3Up = vUp; }
			const Math::Vector3& GetUp() { return m_f3Up; }

			Math::Vector3 GetDir() { Math::Vector3 v(m_f3Lookat - m_f3Eye); v.Normalize(); return v; }

			const Math::Matrix& GetViewMatrix() { return m_matView; }
			const Math::Matrix& GetProjMatrix() { return m_matProj; }
			const Math::Matrix& GetOrthoMatrix() { return m_matOrtho; }

			Collision::EmContainment::Type IsFrustumContains(const Collision::AABB& box) { return m_frustum.Contains(box); }
			Collision::EmContainment::Type IsFrustumContains(const Collision::OBB& box) { return m_frustum.Contains(box); }
			Collision::EmContainment::Type IsFrustumContains(const Collision::Sphere& sphere) { return m_frustum.Contains(sphere); }
			Collision::EmContainment::Type IsFrustumContains(const Math::Vector3& vPos) { return m_frustum.Contains(vPos); }
			const Collision::Frustum& GetFrustum() const { return m_frustum; }

			float GetFarClip() const { return m_frustum.Far; }
			float GetNearClip() const { return m_frustum.Near; }

		private:
			bool m_isThirdPerson;

			// View
			Math::Vector3 m_f3Eye;
			Math::Vector3 m_f3Lookat;
			Math::Vector3 m_f3Up;

			Math::Matrix m_matView;

			// Projection
			uint32_t m_nWidth;
			uint32_t m_nHeight;
			float m_fFov;		// Field Of View 보는 각도 시야각
			float m_fNear;		// 제일 가까운 시야
			float m_fFar;		// 제일 먼 시야

			Math::Matrix m_matProj;

			// Ortho
			Math::Matrix m_matOrtho;

			float m_fDistance;	// 3인칭 카메라 거리
			float m_fAddDistance;

			// Move
			float m_fMoveFront;
			float m_fMoveForward;
			float m_fMoveSideward;
			float m_fMoveUpward;

			// Rotate
			Math::Vector3 m_f3RotationOffset;
			Math::Vector3 m_f3Rotation;

			Collision::Frustum m_frustum;

			static float s_fMaxMoveSpeed;
			static float s_fMaxRotSpeed;
			static float s_fMinDistance;
			static float s_fMaxDistance;
		};
	}
}