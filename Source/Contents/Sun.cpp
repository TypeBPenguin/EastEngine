#include "stdafx.h"
#include "Sun.h"

#include "../GameObject/ActorInterface.h"
#include "../GameObject/ComponentModel.h"
#include "../GameObject/ComponentTimer.h"

#include "../DirectX/Light.h"
#include "../Model/ModelLoader.h"

namespace Contents
{
	Sun::Sun()
		: m_pActor(nullptr)
		, m_pLight(nullptr)
	{
	}

	Sun::~Sun()
	{
		GameObject::IActor::Destroy(&m_pActor);

		m_pLight = nullptr;
	}

	bool Sun::Init(const String::StringID& strName, Graphics::ILight* pLight, float fRadius, const Math::Vector3& f3CenterAxis, const Math::Vector3& f3Velocity, const Math::Vector3& f3OrbitalPos)
	{
		m_f3CenterAxis = f3CenterAxis;
		m_f3OrbitalRotateVelocity = f3Velocity;
		m_f3OrbitalPos = f3OrbitalPos;

		Graphics::MaterialInfo material;
		material.strName = pLight->GetName();
		material.colorAlbedo = pLight->GetColor();
		material.colorEmissive = pLight->GetColor();
		material.f4DisRoughMetEmi.w = pLight->GetIntensity() / 10.f;

		Graphics::ModelLoader loader;
		loader.InitSphere(pLight->GetName(), &material, fRadius * 2.f, 32);

		m_pActor = GameObject::IActor::Create(strName);

		auto pCompModel = static_cast<GameObject::ComponentModel*>(m_pActor->CreateComponent(GameObject::EmComponent::eModel));
		pCompModel->Init(&loader);

		pCompModel->GetModelInstance()->SetVisible(false);

		auto pCompTimer = static_cast<GameObject::ComponentTimer*>(m_pActor->CreateComponent(GameObject::EmComponent::eTimer));
		pCompTimer->StartTimer([&](float fElapsedTime, float fProcessTime)
		{
			Update(fElapsedTime);
		}, 0, 0);

		m_pLight = pLight;

		return true;
	}

	void Sun::Update(float fElapsedTime)
	{
		//Math::Matrix matCenterAxis = Math::Matrix::CreateTranslation(m_f3CenterAxis);
		//
		//m_f3OrbitalRotation += m_f3OrbitalRotateVelocity * fElapsedTime;
		//
		//auto ResetRot = [](float& fRot)
		//{
		//	if (fRot >= 360.f)
		//	{
		//		fRot -= 360.f;
		//	}
		//	else if (fRot <= -360.f)
		//	{
		//		fRot += 360.f;
		//	}
		//};
		//
		//ResetRot(m_f3OrbitalRotation.x);
		//ResetRot(m_f3OrbitalRotation.y);
		//ResetRot(m_f3OrbitalRotation.z);
		//
		//Math::Matrix matOrbitalRotate = Math::Matrix::CreateFromYawPitchRoll(m_f3OrbitalRotation.y, m_f3OrbitalRotation.x, m_f3OrbitalRotation.z);
		//
		//Math::Matrix matOrbitalPos = Math::Matrix::CreateTranslation(m_f3OrbitalPos);
		//
		//Math::Matrix matFinalPos = matOrbitalPos * (matCenterAxis * matOrbitalRotate);
		//
		//SetPosition(matFinalPos.Translation());

		if (m_pLight->GetType() == Graphics::EmLight::ePoint)
		{
			Graphics::IPointLight* pPointLight = static_cast<Graphics::IPointLight*>(m_pLight);
			pPointLight->SetPosition(m_pActor->GetPosition());
		}
		else if (m_pLight->GetType() == Graphics::EmLight::eSpot)
		{
			Graphics::ISpotLight* pPointLight = static_cast<Graphics::ISpotLight*>(m_pLight);
			pPointLight->SetPosition(m_pActor->GetPosition());

			static float fTime = 0.f;
			fTime += fElapsedTime * 0.1f;
			pPointLight->SetAngle(std::abs(Math::Sin(fTime) * 90.f));
		}
	}
}