#pragma once

#include "ComponentInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;
	}

	namespace gameobject
	{
		class ComponentCamera : public IComponent
		{
		public:
			ComponentCamera(IActor* pOwner);
			virtual ~ComponentCamera();

		public:
			void Init(graphics::Camera* pMainCamera);
			void Init(graphics::Camera* pMainCamera, float fLookAtHeight, float fThirdViewDistance = 10.f);

			virtual void Update(float fElapsedTime) override;

		private:
			graphics::Camera* m_pMainCamera;
			bool m_isThirdView;
			float m_fHeight;
		};
	}
}