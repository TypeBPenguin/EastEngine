#pragma once

#include "CommonLib/Singleton.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera : public Singleton<Camera>
		{
			friend Singleton<Camera>;
		private:
			Camera();
			virtual ~Camera();

		public:
			void SetView(const math::Vector3& vEye, const math::Vector3& vLookat, const math::Vector3& vUp);
			void SetProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar, bool isUpsideDown = false);

			void Update(float fElapsedTime);

		public:
			void MoveFront(float fDist);
			void MoveForward(float fDist);
			void MoveSideward(float fDist);
			void MoveUpward(float fDist);

			void RotateAxisX(float fAngle);
			void RotateAxisY(float fAngle);
			void RotateAxisZ(float fAngle);

		public:
			void SetThirdView(const math::Vector3& vLookat);
			bool IsThirdPersonCamera();

			void SetDistance(float fDistance);
			void AddDistance(float fOffset);

			void SetPosition(const math::Vector3& vEye);
			const math::Vector3& GetPosition();

			void SetLookat(const math::Vector3& vLookat);
			const math::Vector3& GetLookat();

			void SetUp(const math::Vector3& vUp);
			const math::Vector3& GetUp();

			math::Vector3 GetDir();

			const math::Matrix& GetViewMatrix();
			const math::Matrix& GetProjMatrix();
			const math::Matrix& GetOrthoMatrix();

			float GetFarClip() const;
			float GetNearClip() const;

		public:
			const Collision::Frustum& GetFrustum() const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}