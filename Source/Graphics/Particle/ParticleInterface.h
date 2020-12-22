#pragma once

#include "Graphics/Interface/Define.h"

namespace est
{
	namespace graphics
	{
		enum ParticleType
		{
			eEmitter = 0,
			eDecal,

			TypeCount,
		};

		struct ParticleInstance
		{
			math::float3 position;
			math::float3 velocity;
			float creationTime{ 0.f };

			math::Color color{ math::Color::Transparent };
			math::Color targetColor{ math::Color::Transparent };
			float colorChangeSpeed{ 0.f };

			bool isAlive{ false };
		};

		struct ParticleEmitterAttributes
		{
			size_t maxParticles{ 0 };
			size_t numToRelease{ 0 };

			float releaseInterval{ 0.f };
			float lifeCycle{ 0.f };
			float size{ 0.f };
			float endTime{ 0.f };

			math::Color color{ math::Color::White };
			math::float3 position;
			math::float3 velocity;
			float velocityIntensity{ 0.f };

			math::float3 gravity;
			math::float3 wind;
			bool isAirResistance{ false };
			bool isRandomColor{ false };

			BlendState::Type blendState{ BlendState::eOff };

			std::wstring textureFilePath;
			bool isEnableAsyncLoadTexture{ false };
		};

		struct ParticleDecalAttributes
		{
			float endTime{ 0.f };
			math::Transform transform;
			IMaterial::Data materialData;
		};

		class IParticle
		{
		protected:
			IParticle() = default;
			virtual ~IParticle() = default;

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matInvView, const math::Matrix& matViewProjection, const collision::Frustum& frustum) = 0;

		public:
			enum State
			{
				eStop = 0,
				eRunning,
				ePause,
			};

		public:
			virtual ParticleType GetType() const = 0;

		public:
			State GetState() const { return m_emState; }
			void Stop() { m_emState = State::eStop; }
			void Start() { m_emState = State::eRunning; }
			void Pause() { m_emState = State::ePause; }

		private:
			State m_emState{ State::eStop };
		};
		using ParticlePtr = std::shared_ptr<IParticle>;

		class IParticleEmitter : public IParticle
		{
		protected:
			IParticleEmitter() = default;
			virtual ~IParticleEmitter() = default;

		public:
			virtual ParticleType GetType() const { return ParticleType::eEmitter; }
		};
		using ParticleEmitterPtr = std::shared_ptr<IParticleEmitter>;

		class IParticleDecal : public IParticle
		{
		protected:
			IParticleDecal() = default;
			virtual ~IParticleDecal() = default;

		public:
			virtual ParticleType GetType() const { return ParticleType::eDecal; }

		public:
			virtual const math::float3& GetScale() const = 0;
			virtual void SetScale(const math::float3& scale) = 0;
			virtual const math::float3& GetPosition() const = 0;
			virtual void SetPosition(const math::float3& position) = 0;
			virtual const math::Quaternion& GetRotation() const = 0;
			virtual void SetRotation(const math::Quaternion& rotate) = 0;
		};
		using ParticleDecalPtr = std::shared_ptr<IParticleDecal>;

		ParticleEmitterPtr CreateParticle(const ParticleEmitterAttributes& attributes);
		ParticleDecalPtr CreateParticle(const ParticleDecalAttributes& attributes);

		void DestroyParticle(ParticleEmitterPtr& pParticle);
		void DestroyParticle(ParticleDecalPtr& pParticle);
	}
}