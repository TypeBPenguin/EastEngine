#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		struct MaterialInfo;
		class ILight;
	}

	namespace GameObject
	{
		class IActor;
	}
}

namespace Contents
{
	using namespace EastEngine;

	class Sun
	{
	public:
		Sun();
		~Sun();

	public:
		bool Init(const String::StringID& strName, Graphics::ILight* pLight, float fRadius, const Math::Vector3& f3CenterAxis, const Math::Vector3& f3Velocity, const Math::Vector3& f3OrbitalPos);
		void Update(float fElapsedTime);

	public:
		const Math::Vector3& GetCenterAxis() const { return m_f3CenterAxis; }
		void GetCenterAxis(const Math::Vector3& f3CenterAxis) { m_f3CenterAxis = f3CenterAxis; }

		const Math::Vector3& GetOrbitalRotateVelocity() const { return m_f3OrbitalRotateVelocity; }
		void GetOrbitalRotateVelocity(const Math::Vector3& f3OrbitalRotateVelocity) { m_f3OrbitalRotateVelocity = f3OrbitalRotateVelocity; }

		const Math::Vector3& GetOrbitalPos() const { return m_f3OrbitalPos; }
		void SetOrbitalPos(const Math::Vector3& f3OrbitalPos) { m_f3OrbitalPos = f3OrbitalPos; }

		GameObject::IActor* GetActor() { return m_pActor; }
		Graphics::ILight* GetLight() { return m_pLight; }

	private:
		GameObject::IActor* m_pActor;
		Graphics::ILight* m_pLight;

		Math::Vector3 m_f3CenterAxis;

		Math::Vector3 m_f3OrbitalRotateVelocity;
		Math::Vector3 m_f3OrbitalRotation;

		Math::Vector3 m_f3OrbitalPos;
	};
}