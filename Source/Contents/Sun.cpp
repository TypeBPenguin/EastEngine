#include "stdafx.h"
#include "Sun.h"

#include "GameObject/GameObject.h"
#include "GameObject/ComponentModel.h"
#include "GameObject/ComponentTimer.h"

#include "DirectX/Light.h"
#include "Model/ModelLoader.h"

namespace Contents
{
	Sun::Sun()
		: m_pActor(nullptr)
		, m_pLight(nullptr)
	{
	}

	Sun::~Sun()
	{
		gameobject::IActor::Destroy(&m_pActor);

		m_pLight = nullptr;
	}

	bool Sun::Init(const string::StringID& strName, graphics::ILight* pLight, float fRadius, const math::Vector3& f3CenterAxis, const math::Vector3& f3Velocity, const math::Vector3& f3OrbitalPos)
	{
		m_f3CenterAxis = f3CenterAxis;
		m_f3OrbitalRotateVelocity = f3Velocity;
		m_f3OrbitalPos = f3OrbitalPos;

		graphics::MaterialInfo material;
		material.strName = pLight->GetName();
		material.colorAlbedo = pLight->GetColor();
		material.colorEmissive = pLight->GetColor();
		material.f4PaddingRoughMetEmi.w = pLight->GetIntensity() / 10.f;

		graphics::ModelLoader loader;
		loader.InitSphere(pLight->GetName(), &material, fRadius * 2.f, 32);

		m_pActor = gameobject::IActor::Create(strName);

		auto pCompModel = static_cast<gameobject::ComponentModel*>(m_pActor->CreateComponent(gameobject::EmComponent::eModel));
		pCompModel->Init(&loader);

		pCompModel->GetModelInstance()->SetVisible(false);

		auto pCompTimer = static_cast<gameobject::ComponentTimer*>(m_pActor->CreateComponent(gameobject::EmComponent::eTimer));
		pCompTimer->StartTimeAction([&](uint32_t nEventID, float fElapsedTime, float fProcessTime)
		{
			Update(fElapsedTime);
		}, 0, 0);

		m_pLight = pLight;

		return true;
	}

	void Sun::Update(float fElapsedTime)
	{
		//math::Matrix matCenterAxis = math::Matrix::CreateTranslation(m_f3CenterAxis);
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
		//math::Matrix matOrbitalRotate = math::Matrix::CreateFromYawPitchRoll(m_f3OrbitalRotation.y, m_f3OrbitalRotation.x, m_f3OrbitalRotation.z);
		//
		//math::Matrix matOrbitalPos = math::Matrix::CreateTranslation(m_f3OrbitalPos);
		//
		//math::Matrix matFinalPos = matOrbitalPos * (matCenterAxis * matOrbitalRotate);
		//
		//SetPosition(matFinalPos.Translation());

		if (m_pLight->GetType() == graphics::EmLight::ePoint)
		{
			graphics::IPointLight* pPointLight = static_cast<graphics::IPointLight*>(m_pLight);
			pPointLight->SetPosition(m_pActor->GetPosition());
		}
		else if (m_pLight->GetType() == graphics::EmLight::eSpot)
		{
			graphics::ISpotLight* pPointLight = static_cast<graphics::ISpotLight*>(m_pLight);
			pPointLight->SetPosition(m_pActor->GetPosition());

			static float fTime = 0.f;
			fTime += fElapsedTime * 0.1f;
			pPointLight->SetAngle(std::abs(math::Sin(fTime) * 90.f));
		}
	}
}