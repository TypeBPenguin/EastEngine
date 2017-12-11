#include "stdafx.h"
#include "Camera.h"

namespace EastEngine
{
	namespace Graphics
	{
		float Camera::s_fMaxMoveSpeed = 10.f;
		float Camera::s_fMaxRotSpeed = 10.f;
		float Camera::s_fMinDistance = 1.f;
		float Camera::s_fMaxDistance = 50.f;

		Camera::Camera()
			: m_isThirdPerson(false)
			, m_nWidth(0)
			, m_nHeight(0)
			, m_fFov(0.f)
			, m_fNear(0.f)
			, m_fFar(0.f)
			, m_fDistance(0.f)
			, m_fAddDistance(0.f)
			, m_fMoveFront(0.f)
			, m_fMoveForward(0.f)
			, m_fMoveSideward(0.f)
			, m_fMoveUpward(0.f)
		{
		}

		Camera::~Camera()
		{
		}

		void Camera::InitView(const Math::Vector3& vEye, const Math::Vector3& vLookat, const Math::Vector3& vUp)
		{
			m_f3Eye = vEye;
			m_f3Lookat = vLookat;
			m_f3Up = vUp;

			m_fDistance = Math::Vector3::Distance(m_f3Eye, m_f3Lookat);

			UpdateView();
		}

		void Camera::InitProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar)
		{
			m_nWidth = nWidth;
			m_nHeight = nHeight;
			m_fFov = fFov;
			m_fNear = fNear;
			m_fFar = fFar;

			UpdateProjection();
		}

		void Camera::Update(float fElapsedTime)
		{
			float fValue = fElapsedTime * 5.f;
			float fDeValue = 1.f - fValue;

			if (std::abs(m_fAddDistance) > 0.001f)
			{
				float fOffset = m_fAddDistance * fValue;
				m_fAddDistance *= fDeValue;

				m_fDistance += fOffset;
				m_fDistance = Math::Clamp(m_fDistance + fOffset, s_fMinDistance, s_fMaxDistance);

				SetDistance(m_fDistance);
			}
			else
			{
				m_fAddDistance = 0.f;
			}

			if (std::abs(m_fMoveFront) > 0.001f)
			{
				float fOffset = m_fMoveFront * fValue;
				m_fMoveFront *= fDeValue;

				Math::Vector3 vForward(m_matView.Forward());
				vForward.Normalize();

				vForward.y = 0.f;

				m_f3Eye = m_f3Eye + vForward * fOffset;
				m_f3Lookat = m_f3Lookat + vForward * fOffset;
			}
			else
			{
				m_fMoveFront = 0.f;
			}

			if (std::abs(m_fMoveForward) > 0.001f)
			{
				float fOffset = m_fMoveForward * fValue;
				m_fMoveForward *= fDeValue;

				Math::Vector3 vForward(m_matView.Forward());
				vForward.Normalize();

				m_f3Eye = m_f3Eye + vForward * fOffset;
				m_f3Lookat = m_f3Lookat + vForward * fOffset;
			}
			else
			{
				m_fMoveForward = 0.f;
			}

			if (std::abs(m_fMoveSideward) > 0.001f)
			{
				float fOffset = m_fMoveSideward * fValue;
				m_fMoveSideward *= fDeValue;

				Math::Vector3 vRight(m_matView.Right());
				vRight.Normalize();

				m_f3Eye = m_f3Eye + vRight * fOffset;
				m_f3Lookat = m_f3Lookat + vRight * fOffset;
			}
			else
			{
				m_fMoveSideward = 0.f;
			}

			if (std::abs(m_fMoveUpward) > 0.001f)
			{
				float fOffset = m_fMoveUpward * fValue;
				m_fMoveUpward *= fDeValue;

				Math::Vector3 vUp(m_matView.Up());
				vUp.Normalize();

				m_f3Eye = m_f3Eye + vUp * fOffset;
				m_f3Lookat = m_f3Lookat + vUp * fOffset;
			}
			else
			{
				m_fMoveUpward = 0.f;
			}

			if (std::abs(m_f3RotationOffset.x) > 0.001f)
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

			if (std::abs(m_f3RotationOffset.y) > 0.001f)
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

			if (std::abs(m_f3RotationOffset.z) > 0.001f)
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

			UpdateView();

			Collision::Frustum::CreateFromMatrix(m_frustum, m_matProj);
			m_frustum.Transform(m_matView.Invert());
		}
	}
}