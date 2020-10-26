#include "stdafx.h"
#include "Camera.h"

namespace est
{
	namespace graphics
	{
		FirstPersonCameraMan::FirstPersonCameraMan(Camera* pCamera, const DescMove& descMove)
			: m_descMove(descMove)
		{
			const math::Matrix matRotation = math::Matrix::CreateLookAt(pCamera->GetPosition(), pCamera->GetLookat(), pCamera->GetUp());

			math::Transform transform;
			matRotation.Decompose(transform.scale, transform.rotation, transform.position);

			m_rotation = transform.rotation.ToEularDegrees();
		}

		void FirstPersonCameraMan::Update(Camera* pCamera, float elapsedTime)
		{
			constexpr float epsilon = std::numeric_limits<float>::epsilon();

			const math::Matrix matView = pCamera->GetViewMatrix();
			math::float3 cameraPosition = pCamera->GetPosition();

			// move by direction
			const float elapsedMoveDistance = m_descMove.moveSpeed * elapsedTime;
			if (std::abs(m_moveByDirection.z) > epsilon)
			{
				const float offset = m_moveByDirection.z * elapsedMoveDistance;
				m_moveByDirection.z -= offset;

				math::float3 forward;
				matView.Forward().Normalize(forward);

				forward *= offset;

				cameraPosition += forward;
			}

			if (std::abs(m_moveByDirection.x) > epsilon)
			{
				const float offset = m_moveByDirection.x * elapsedMoveDistance;
				m_moveByDirection.x -= offset;

				math::float3 right;
				matView.Right().Normalize(right);

				right *= offset;

				cameraPosition += right;
			}

			if (std::abs(m_moveByDirection.y) > epsilon)
			{
				const float offset = m_moveByDirection.y * elapsedMoveDistance;
				m_moveByDirection.y -= offset;

				math::float3 up;
				matView.Up().Normalize(up);

				up *= offset;

				cameraPosition += up;
			}

			// move by axis
			if (std::abs(m_moveByAxis.z) > epsilon)
			{
				const float offset = m_moveByAxis.z * elapsedMoveDistance;
				m_moveByAxis.z -= offset;

				math::float3 forward = matView.Forward();
				forward.y = 0.f;
				forward.Normalize();
				forward *= offset;

				cameraPosition += forward;
			}

			if (std::abs(m_moveByAxis.x) > epsilon)
			{
				const float offset = m_moveByAxis.x * elapsedMoveDistance;
				m_moveByAxis.x -= offset;

				math::float3 right = matView.Right();
				right.y = 0.f;
				right.Normalize();
				right *= offset;

				cameraPosition += right;
			}

			if (std::abs(m_moveByAxis.y) > epsilon)
			{
				const float offset = m_moveByAxis.y * elapsedMoveDistance;
				m_moveByAxis.y -= offset;

				const math::float3 up = math::float3::Up * offset;

				cameraPosition += up;
			}

			// rotate by axis
			const float elapsedRotateAngle = m_descMove.rotateSpeed * elapsedTime;
			if (std::abs(m_rotateByAxis.x) > epsilon)
			{
				const float offset = m_rotateByAxis.x * elapsedRotateAngle;
				m_rotateByAxis.x -= offset;

				m_rotation.x += offset;
				m_rotation.x = fmod(m_rotation.x, 360.f);

				if (m_rotation.x < 0.f)
				{
					m_rotation.x += 360.f;
				}
			}

			if (std::abs(m_rotateByAxis.y) > epsilon)
			{
				const float offset = m_rotateByAxis.y * elapsedRotateAngle;
				m_rotateByAxis.y -= offset;

				m_rotation.y += offset;
				m_rotation.y = fmod(m_rotation.y, 360.f);

				if (m_rotation.y < 0.f)
				{
					m_rotation.y += 360.f;
				}
			}

			if (std::abs(m_rotateByAxis.z) > epsilon)
			{
				const float offset = m_rotateByAxis.z * elapsedRotateAngle;
				m_rotateByAxis.z -= offset;

				m_rotation.z += offset;
				m_rotation.z = fmod(m_rotation.z, 360.f);

				if (m_rotation.z < 0.f)
				{
					m_rotation.z += 360.f;
				}
			}

			const math::Matrix matRotate = math::Matrix::CreateFromYawPitchRoll(math::ToRadians(m_rotation.y), math::ToRadians(m_rotation.x), math::ToRadians(m_rotation.z));

			const float distance = math::float3::Distance(pCamera->GetLookat(), pCamera->GetPosition());
			const math::float3 lookat = (math::float3::Transform(math::float3::Forward, matRotate) * distance) + cameraPosition;
			const math::float3 up = math::float3::Transform(math::float3::Up, matRotate);

			pCamera->SetPosition(cameraPosition);
			pCamera->SetLookat(lookat);
			pCamera->SetUp(up);
		}

		ThirdPersonCameraMan::ThirdPersonCameraMan(Camera* pCamera, const DescMove& descMove)
			: m_descMove(descMove)
		{
			const math::Matrix matRotation = math::Matrix::CreateLookAt(pCamera->GetPosition(), pCamera->GetLookat(), pCamera->GetUp());

			math::Transform transform;
			matRotation.Decompose(transform.scale, transform.rotation, transform.position);

			m_rotation = transform.rotation.ToEularDegrees();

			const float distance = math::float3::Distance(pCamera->GetPosition(), pCamera->GetLookat());
			SetDistance(distance);
		}
		
		void ThirdPersonCameraMan::Update(Camera* pCamera, float elapsedTime)
		{
			constexpr float epsilon = std::numeric_limits<float>::epsilon();

			const math::Matrix matView = pCamera->GetViewMatrix();

			// move by direction
			const float elapsedMoveDistance = m_descMove.moveSpeed * elapsedTime;
			if (std::abs(m_moveDistance) > epsilon)
			{
				const float offset = m_moveDistance * elapsedMoveDistance;
				m_moveDistance -= offset;

				m_distance += offset;
				m_distance = std::clamp(m_distance, m_descMove.minDistance, m_descMove.maxDistance);
			}

			// rotate by axis
			const float elapsedRotateAngle = m_descMove.rotateSpeed * elapsedTime;
			if (std::abs(m_rotateByAxis.x) > epsilon)
			{
				const float offset = m_rotateByAxis.x * elapsedRotateAngle;
				m_rotateByAxis.x -= offset;

				m_rotation.x += offset;
				m_rotation.x = fmod(m_rotation.x, 360.f);

				if (m_rotation.x < 0.f)
				{
					m_rotation.x += 360.f;
				}
			}

			if (std::abs(m_rotateByAxis.y) > epsilon)
			{
				const float offset = m_rotateByAxis.y * elapsedRotateAngle;
				m_rotateByAxis.y -= offset;

				m_rotation.y += offset;
				m_rotation.y = fmod(m_rotation.y, 360.f);

				if (m_rotation.y < 0.f)
				{
					m_rotation.y += 360.f;
				}
			}

			if (std::abs(m_rotateByAxis.z) > epsilon)
			{
				const float offset = m_rotateByAxis.z * elapsedRotateAngle;
				m_rotateByAxis.z -= offset;

				m_rotation.z += offset;
				m_rotation.z = fmod(m_rotation.z, 360.f);

				if (m_rotation.z < 0.f)
				{
					m_rotation.z += 360.f;
				}
			}

			const math::Matrix matRotate = math::Matrix::CreateFromYawPitchRoll(math::ToRadians(m_rotation.y), math::ToRadians(m_rotation.x), math::ToRadians(m_rotation.z));

			const math::float3 cameraPosition = (math::float3::Transform(math::float3::Forward, matRotate) * -m_distance) + m_targetPosition;
			const math::float3 up = math::float3::Transform(math::float3::Up, matRotate);

			pCamera->SetPosition(cameraPosition);
			pCamera->SetLookat(m_targetPosition);
			pCamera->SetUp(up);
		}

		void Camera::Update(float elapsedTime)
		{
			if (m_pCameraMan != nullptr)
			{
				m_pCameraMan->Update(this, elapsedTime);
			}
		}

		const math::Matrix& Camera::UpdateView()
		{
			if (m_isInvalidView == true)
			{
				m_matView = math::Matrix::CreateLookAt(m_descView.position, m_descView.lookat, m_descView.up);
				m_isInvalidView = false;
			}
			return m_matView;
		}

		const math::Matrix& Camera::UpdateProjection()
		{
			if (m_isInvalidProjection == true)
			{
				m_matProjection = math::Matrix::CreatePerspectiveFieldOfView(m_descProjection.fov, static_cast<float>(m_descProjection.width) / static_cast<float>(m_descProjection.height), m_descProjection.nearClip, m_descProjection.farClip);

				if (m_descProjection.isReverseUpDown == true)
				{
					m_matProjection.m[1][1] *= -1;
				}
				m_isInvalidProjection = false;
			}
			return m_matProjection;
		}

		const math::Matrix& Camera::UpdateOrtho()
		{
			if (m_isInvalidOrtho == true)
			{
				m_matOrtho = math::Matrix::CreateOrthographic(static_cast<float>(m_descProjection.width), static_cast<float>(m_descProjection.height), m_descProjection.nearClip, m_descProjection.farClip);
				m_isInvalidOrtho = false;
			}
			return m_matOrtho;
		}

		const collision::Frustum& Camera::UpdateFrustum()
		{
			collision::Frustum::CreateFromMatrix(m_frustum, GetProjectionMatrix());
			m_frustum.Transform(GetViewMatrix().Invert());

			return m_frustum;
		}
	}
}