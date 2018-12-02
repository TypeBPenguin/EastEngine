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
		bool Init(const string::StringID& strName, graphics::ILight* pLight, float fRadius, const math::float3& f3CenterAxis, const math::float3& f3Velocity, const math::float3& f3OrbitalPos);
		void Update(float elapsedTime);

	public:
		const math::float3& GetCenterAxis() const { return m_f3CenterAxis; }
		void GetCenterAxis(const math::float3& f3CenterAxis) { m_f3CenterAxis = f3CenterAxis; }

		const math::float3& GetOrbitalRotateVelocity() const { return m_f3OrbitalRotateVelocity; }
		void GetOrbitalRotateVelocity(const math::float3& f3OrbitalRotateVelocity) { m_f3OrbitalRotateVelocity = f3OrbitalRotateVelocity; }

		const math::float3& GetOrbitalPos() const { return m_f3OrbitalPos; }
		void SetOrbitalPos(const math::float3& f3OrbitalPos) { m_f3OrbitalPos = f3OrbitalPos; }

		gameobject::IActor* GetActor() { return m_pActor; }
		graphics::ILight* GetLight() { return m_pLight; }

	private:
		gameobject::IActor* m_pActor;
		graphics::ILight* m_pLight;

		math::float3 m_f3CenterAxis;

		math::float3 m_f3OrbitalRotateVelocity;
		math::float3 m_f3OrbitalRotation;

		math::float3 m_f3OrbitalPos;
	};
}