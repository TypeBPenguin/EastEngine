#pragma once

namespace eastengine
{
	namespace graphics
	{
		struct MaterialInfo;
		class ILight;
	}

	namespace gameobject
	{
		class IActor;
	}
}

namespace Contents
{
	using namespace eastengine;

	class Sun
	{
	public:
		Sun();
		~Sun();

	public:
		bool Init(const String::StringID& strName, graphics::ILight* pLight, float fRadius, const math::Vector3& f3CenterAxis, const math::Vector3& f3Velocity, const math::Vector3& f3OrbitalPos);
		void Update(float fElapsedTime);

	public:
		const math::Vector3& GetCenterAxis() const { return m_f3CenterAxis; }
		void GetCenterAxis(const math::Vector3& f3CenterAxis) { m_f3CenterAxis = f3CenterAxis; }

		const math::Vector3& GetOrbitalRotateVelocity() const { return m_f3OrbitalRotateVelocity; }
		void GetOrbitalRotateVelocity(const math::Vector3& f3OrbitalRotateVelocity) { m_f3OrbitalRotateVelocity = f3OrbitalRotateVelocity; }

		const math::Vector3& GetOrbitalPos() const { return m_f3OrbitalPos; }
		void SetOrbitalPos(const math::Vector3& f3OrbitalPos) { m_f3OrbitalPos = f3OrbitalPos; }

		gameobject::IActor* GetActor() { return m_pActor; }
		graphics::ILight* GetLight() { return m_pLight; }

	private:
		gameobject::IActor* m_pActor;
		graphics::ILight* m_pLight;

		math::Vector3 m_f3CenterAxis;

		math::Vector3 m_f3OrbitalRotateVelocity;
		math::Vector3 m_f3OrbitalRotation;

		math::Vector3 m_f3OrbitalPos;
	};
}