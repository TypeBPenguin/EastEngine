#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Camera : public Singleton<Camera>
		{
			friend Singleton<Camera>;
		private:
			Camera();
			virtual ~Camera();

		public:
			void SetView(const Math::Vector3& vEye, const Math::Vector3& vLookat, const Math::Vector3& vUp);
			void SetProjection(uint32_t nWidth, uint32_t nHeight, float fFov, float fNear, float fFar);

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
			void SetThirdView(const Math::Vector3& vLookat);
			bool IsThirdPersonCamera();

			void SetDistance(float fDistance);
			void AddDistance(float fOffset);

			void SetPosition(const Math::Vector3& vEye);
			const Math::Vector3& GetPosition();

			void SetLookat(const Math::Vector3& vLookat);
			const Math::Vector3& GetLookat();

			void SetUp(const Math::Vector3& vUp);
			const Math::Vector3& GetUp();

			Math::Vector3 GetDir();

			const Math::Matrix& GetViewMatrix(int nThreadID);
			const Math::Matrix& GetProjMatrix(int nThreadID);
			const Math::Matrix& GetOrthoMatrix(int nThreadID);

			float GetFarClip(int nThreadID) const;
			float GetNearClip(int nThreadID) const;

		public:
			const Collision::Frustum& GetFrustum(int nThreadID) const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}