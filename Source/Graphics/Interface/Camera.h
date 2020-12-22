#pragma once

namespace est
{
	namespace graphics
	{
		class Camera;

		class ICameraMan
		{
		public:
			ICameraMan() = default;
			virtual ~ICameraMan() = default;

		public:
			virtual void Update(Camera* pCamera, float elapsedTime) = 0;
		};

		class FirstPersonCameraMan : public ICameraMan
		{
		public:
			struct DescMove
			{
				float moveSpeed{ 5.f };
				float rotateSpeed{ 5.f };
			};

		public:
			FirstPersonCameraMan(Camera* pCamera, const DescMove& descMove);
			virtual ~FirstPersonCameraMan() = default;

		public:
			virtual void Update(Camera* pCamera, float elapsedTime) override;

		public:
			const DescMove& GetDescMove() const { return m_descMove; }
			void SetDescMove(const DescMove& descMove) { m_descMove = descMove; }

			const math::float3& GetMoveByDirection() const { return m_moveByDirection; }
			const math::float3& GetMoveByAxis() const { return m_moveByAxis; }
			const math::float3& GetRotateByAxis() const { return m_rotateByAxis; }

			const math::float3& GetRotation() const { return m_rotation; }
			void SetRotation(const math::float3& rotation) { m_rotation = rotation; }

		public:
			void MoveForward(float moveDist) { m_moveByDirection.z += moveDist; }
			void MoveSideward(float moveDist) { m_moveByDirection.x += moveDist; }
			void MoveUpward(float moveDist) { m_moveByDirection.y += moveDist; }

			void MoveAxisX(float moveDist) { m_moveByAxis.x += moveDist; }
			void MoveAxisY(float moveDist) { m_moveByAxis.y += moveDist; }
			void MoveAxisZ(float moveDist) { m_moveByAxis.z += moveDist; }

			void RotateAxisX(float degree) { m_rotateByAxis.x += degree; }
			void RotateAxisY(float degree) { m_rotateByAxis.y += degree; }
			void RotateAxisZ(float degree) { m_rotateByAxis.z += degree; }

		private:
			DescMove m_descMove;

			math::float3 m_moveByDirection;
			math::float3 m_moveByAxis;
			math::float3 m_rotateByAxis;

			math::float3 m_rotation;
		};

		class ThirdPersonCameraMan : public ICameraMan
		{
		public:
			struct DescMove
			{
				float moveSpeed{ 5.f };
				float rotateSpeed{ 2.5f };

				float minDistance{ 1.f };
				float maxDistance{ 20.f };
			};

		public:
			ThirdPersonCameraMan(Camera* pCamera, const DescMove& descMove);
			virtual ~ThirdPersonCameraMan() = default;

		public:
			virtual void Update(Camera* pCamera, float elapsedTime) override;

		public:
			const DescMove& GetDescMove() const { return m_descMove; }
			void SetDescMove(const DescMove& descMove) { m_descMove = descMove; }

			float GetMoveDistance() const { return m_moveDistance; }
			float GetDistance() const { return m_distance; }

			const math::float3& GetTargetPosition() const { return m_targetPosition; }
			const math::float3& GetRotateByAxis() const { return m_rotateByAxis; }

			const math::float3& GetRotation() const { return m_rotation; }
			void SetRotation(const math::float3& rotation) { m_rotation = rotation; }

		public:
			void RotateAxisX(float degree) { m_rotateByAxis.x += degree; }
			void RotateAxisY(float degree) { m_rotateByAxis.y += degree; }
			void RotateAxisZ(float degree) { m_rotateByAxis.z += degree; }

			void SetDistance(float distance) { m_moveDistance = distance - m_distance; }
			void MoveDistance(float distance) { m_moveDistance += distance; }

			void SetTargetPosition(const math::float3& targetPosition) { m_targetPosition = targetPosition; }

		private:
			DescMove m_descMove;

			float m_moveDistance{ 0.f };
			float m_distance{ 5.f };

			math::float3 m_targetPosition;
			math::float3 m_rotateByAxis;

			math::float3 m_rotation;
		};

		class Camera
		{
		public:
			struct DescView
			{
				math::float3 position;
				math::float3 lookat{ math::float3::Forward };
				math::float3 up{ math::float3::Up };
			};

			struct DescProjection
			{
				uint32_t width{ 0 };
				uint32_t height{ 0 };
				float fov{ math::PIDIV4 };
				float nearClip{ 0.1f };
				float farClip{ 1000.f };
				bool isReverseUpDown{ false };
			};

			struct DescOrthographic
			{
				uint32_t width{ 0 };
				uint32_t height{ 0 };
				float nearClip{ 0.1f };
				float farClip{ 1000.f };
			};

		public:
			Camera() = default;
			virtual ~Camera() = default;

		public:
			void Update(float elapsedTime);

		public:
			void SetView(const DescView& descView) { m_descView = descView; m_isDirtyView = true; }
			const DescView& GetView() const { return m_descView; }

			void SetProjection(const DescProjection& descProjection) { m_descProjection = descProjection; m_isDirtyProjection = true; }
			const DescProjection& GetProjection() const { return m_descProjection; }

			void SetOrthographic(const DescOrthographic& descOrthographic) { m_descOrthographic = descOrthographic; m_isDirtyOrtho = true; }
			const DescOrthographic& GetOrthographic() const { return m_descOrthographic; }

			void SetPosition(const math::float3& position) { m_descView.position = position; m_isDirtyView = true; }
			const math::float3& GetPosition() const { return m_descView.position; }

			void SetLookat(const math::float3& lookat) { m_descView.lookat = lookat; m_isDirtyView = true; }
			const math::float3& GetLookat() const { return m_descView.lookat; }

			void SetUp(const math::float3& up) { m_descView.up = up; m_isDirtyView = true; }
			const math::float3& GetUp() const { return m_descView.up; }

			const math::Matrix& GetViewMatrix() { return UpdateView(); }
			const math::Matrix& GetProjectionMatrix() { return UpdateProjection(); }
			const math::Matrix& GetOrthoMatrix() { return UpdateOrtho(); }

			math::float3 GetDirection() const { math::float3 direction{ m_descView.lookat - m_descView.position }; direction.Normalize(); return direction; }

		public:
			const collision::Frustum& GetFrustum() { return UpdateFrustum(); }

			void SetCameraMan(std::unique_ptr<ICameraMan> pCameraMan) { m_pCameraMan = std::move(pCameraMan); }
			ICameraMan* GetCameraMan() const { return m_pCameraMan.get(); }

			void SetFirstPersonCameraMan(const FirstPersonCameraMan::DescMove& descMove = {}) { m_pCameraMan = std::make_unique<FirstPersonCameraMan>(this, descMove); }
			void SetThirdPersonCameraMan(const ThirdPersonCameraMan::DescMove& descMove = {}) { m_pCameraMan = std::make_unique<ThirdPersonCameraMan>(this, descMove); }

		private:
			const math::Matrix& UpdateView();
			const math::Matrix& UpdateProjection();
			const math::Matrix& UpdateOrtho();
			const collision::Frustum& UpdateFrustum();

		private:
			DescView m_descView;
			DescProjection m_descProjection;
			DescOrthographic m_descOrthographic;

			bool m_isDirtyView{ false };
			bool m_isDirtyProjection{ false };
			bool m_isDirtyOrtho{ false };

			math::Matrix m_viewMatrix;
			math::Matrix m_projectionMatrix;
			math::Matrix m_orthographicMatrix;

			collision::Frustum m_frustum;

			std::unique_ptr<ICameraMan> m_pCameraMan;
		};
	}
}