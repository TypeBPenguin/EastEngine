#include "stdafx.h"
#include "ComponentCamera.h"

#include "Graphics/Interface/Camera.h"

#include "Input/InputInterface.h"

#include "GameObject.h"

namespace est
{
	namespace gameobject
	{
		ComponentCamera::ComponentCamera(IActor* pOwner)
			: IComponent(pOwner, IComponent::eCamera)
			, m_isThirdView(false)
		{
		}

		ComponentCamera::~ComponentCamera()
		{
		}

		void ComponentCamera::Init(graphics::Camera* pMainCamera)
		{
			m_pMainCamera = pMainCamera;
		}

		void ComponentCamera::Init(graphics::Camera* pMainCamera, float fLookAtHeight, float fThirdViewDistance)
		{
		}

		void ComponentCamera::Update(float elapsedTime)
		{
		}
	}
}