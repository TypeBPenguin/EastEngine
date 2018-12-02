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
			void SetView(const math::float3& vEye, const math::float3& vLookat, const math::float3& vUp);
			void SetProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar, bool isUpsideDown = false);

			void Update(float elapsedTime);

		public:
			void MoveFront(float fDist);
			void MoveForward(float fDist);
			void MoveSideward(float fDist);
			void MoveUpward(float fDist);

			void RotateAxisX(float fAngle);
			void RotateAxisY(float fAngle);
			void RotateAxisZ(float fAngle);

		public:
			void SetThirdView(const math::float3& vLookat);
			bool IsThirdPersonCamera();

			void SetDistance(float fDistance);
			void AddDistance(float fOffset);

			void SetPosition(const math::float3& vEye);
			const math::float3& GetPosition() const;

			void SetLookat(const math::float3& vLookat);
			const math::float3& GetLookat() const;

			void SetUp(const math::float3& vUp);
			const math::float3& GetUp() const;

			math::float3 GetDir() const;

			const math::Matrix& GetViewMatrix() const;
			const math::Matrix& GetProjMatrix() const;
			const math::Matrix& GetOrthoMatrix() const;

			float GetFOV() const;
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