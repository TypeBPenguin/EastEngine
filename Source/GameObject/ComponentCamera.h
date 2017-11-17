#pragma once

#include "ComponentInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Camera;
	}

	namespace GameObject
	{
		class ComponentCamera : public IComponent
		{
		public:
			ComponentCamera(IActor* pOwner);
			virtual ~ComponentCamera();

		public:
			void Init(Graphics::Camera* pMainCamera);
			void Init(Graphics::Camera* pMainCamera, float fLookAtHeight, float fThirdViewDistance = 10.f);

			virtual void Update(float fElapsedTime) override;

		private:
			Graphics::Camera* m_pMainCamera;
			bool m_isThirdView;
			float m_fHeight;
		};
	}
}