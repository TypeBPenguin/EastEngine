#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		struct MaterialInfo;
		class ITexture;
		class IBlendState;

		namespace EmParticle
		{
			enum Type
			{
				eEmitter = 0,
				eDecal,

				TypeCount,
			};
		}

		struct ParticleInstanceInfo
		{
			Math::Vector3 f3Pos;
			Math::Vector3 f3Velocity;
			float fCreationTime = 0.f;

			DWORD color = 0;
			DWORD targetColor = 0;
			float fColorChangeSpeed = 0.f;

			std::shared_ptr<ITexture> pTexture = nullptr;
		};

		struct ParticleEmitterAttributes
		{
			uint32_t nMaxParticles = 0;
			uint32_t nNumToRelease = 0;

			float fReleaseInterval = 0.f;
			float fLifeCycle = 0.f;
			float fSize = 0.f;

			Math::Color color = Math::Color::White;
			Math::Vector3 f3Pos;
			Math::Vector3 f3Velocity;
			float fVelocityVar = 0.f;

			Math::Vector3 f3Gravity;
			Math::Vector3 f3Wind;
			bool isAirResistance = false;
			bool isRandomColor = false;

			IBlendState* pBlendState = nullptr;
			std::string strTexFile;

			float fEndTime = 0.f;
		};

		struct ParticleDecalAttributes
		{
			String::StringID strDecalName;

			Math::Vector3 f3Pos;
			Math::Vector3 f3Scale = Math::Vector3::One;
			Math::Quaternion quatRot = Math::Quaternion::Identity;

			MaterialInfo* pMaterialInfo;

			float fEndTime = 0.f;
		};

		class IParticle
		{
		protected:
			IParticle(EmParticle::Type emEffectType);
			virtual ~IParticle() = default;

		public:
			static void Destroy(IParticle** ppParticle);

			virtual void Update(float fElapsedTime, const Math::Matrix& matView, const Math::Matrix& matViewProjection, const Collision::Frustum& frustum) = 0;

		public:
			EmParticle::Type GetType() const { return m_emEffectType; }

			bool IsStart() const { return m_isStart; }
			void Start() { m_isStart = true; m_isPause = false; }

			bool IsStop() const { return m_isStart; }
			void Stop() { m_isStart = false; }

			bool IsPause() const { return m_isPause; }
			void Pause() { m_isPause = true; }

			bool IsAlive() const { return m_isAlive; }
			void SetAlive(bool isAlive) { m_isAlive = isAlive; }

		private:
			EmParticle::Type m_emEffectType;

			bool m_isStart;
			bool m_isPause;
			bool m_isAlive;
		};

		class IParticleEmitter : public IParticle
		{
		protected:
			IParticleEmitter();
			virtual ~IParticleEmitter() = default;

		public:
			static IParticleEmitter* Create(const ParticleEmitterAttributes& attributes);
			static void Destroy(IParticleEmitter** ppParticle);
		};

		class IParticleDecal : public IParticle
		{
		protected:
			IParticleDecal();
			virtual ~IParticleDecal() = default;

		public:
			static IParticleDecal* Create(const ParticleDecalAttributes& attributes);
			static void Destroy(IParticleDecal** ppParticle);
		};
	}
}