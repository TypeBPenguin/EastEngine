#include "stdafx.h"
#include "ComponentCamera.h"

#include "../DirectX/Camera.h"
#include "../Input/InputDevice.h"

#include "ActorInterface.h"

namespace EastEngine
{
	namespace GameObject
	{
		ComponentCamera::ComponentCamera(IActor* pOwner)
			: IComponent(pOwner, EmComponent::eCamera)
			, m_isThirdView(false)
		{
		}

		ComponentCamera::~ComponentCamera()
		{
		}

		void ComponentCamera::Init(Graphics::Camera* pMainCamera)
		{
			m_pMainCamera = pMainCamera;
		}

		void ComponentCamera::Init(Graphics::Camera* pMainCamera, float fLookAtHeight, float fThirdViewDistance)
		{
			m_pMainCamera = pMainCamera;

			m_isThirdView = true;
			m_fHeight = fLookAtHeight;
						
			m_pMainCamera->SetDistance(fThirdViewDistance);
		}

		void ComponentCamera::Update(float fElapsedTime)
		{
			if (m_pMainCamera == nullptr)
				return;

			Math::Vector3 f3Eye = m_pMainCamera->GetPosition();
			Math::Vector3 f3Lookat = m_pMainCamera->GetLookat();
			Math::Vector3 f3Dir = f3Lookat - f3Eye;

			f3Eye = m_pOwner->GetPosition();
			f3Eye.y += m_fHeight;
			f3Lookat = f3Eye + f3Dir;

			m_pMainCamera->SetPosition(f3Eye);
			m_pMainCamera->SetLookat(f3Lookat);

			if (m_isThirdView)
			{
				m_pMainCamera->SetThirdView(f3Eye);
			}

			Input::InputDevice* pInputDevice = Input::InputDevice::GetInstance();
			float dx = static_cast<float>(pInputDevice->GetMoveX() * 0.25f);
			float dy = static_cast<float>(pInputDevice->GetMoveY() * 0.25f);
			float dz = static_cast<float>(pInputDevice->GetMoveWheel()) * 0.01f;
			bool bX = fabsf(dx) > 0.f;
			bool bY = fabsf(dy) > 0.f;
			if (pInputDevice->IsMousePress(Input::EmMouse::eRight))
			{
				if (bX)
				{
					m_pMainCamera->RotateAxisY(dx);
				}

				if (bY)
				{
					m_pMainCamera->RotateAxisX(dy);
				}
			}

			// ÈÙÁ¶ÀÛ
			if (dz != 0.f)
			{
				m_pMainCamera->AddDistance(-dz);
			}

			m_pMainCamera->UpdateView();
		}
	}
}