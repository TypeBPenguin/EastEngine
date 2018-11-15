#pragma once

namespace eastengine
{
	namespace graphics
	{
		class IDirectionalLight;
		class IPointLight;
		class ISpotLight;

		struct DirectionalLightData
		{
			math::float3 f3Color;
			float fLightIntensity = 0.f;

			math::float3 f3Dir;
			float fAmbientIntensity = 0.f;

			math::float3 padding;
			float fReflectionIntensity = 0.f;

			void Set(const math::Color& color, const math::float3& direction, float lightIntensity, float ambientIntensity, float reflectionIntensity)
			{
				f3Color.x = color.r;
				f3Color.y = color.g;
				f3Color.z = color.b;

				f3Dir = direction;
				fLightIntensity = lightIntensity;
				fAmbientIntensity = ambientIntensity;
				fReflectionIntensity = reflectionIntensity;
			}
		};

		struct PointLightData
		{
			math::float3 f3Color;
			float fLightIntensity = 0.f;

			math::float3 f3Pos;
			float fAmbientIntensity = 0.f;

			math::float3 padding;
			float fReflectionIntensity = 0.f;

			void Set(const math::Color& color, const math::float3& pos, float lightIntensity, float ambientIntensity, float reflectionIntensity)
			{
				f3Color.x = color.r;
				f3Color.y = color.g;
				f3Color.z = color.b;

				f3Pos = pos;
				fLightIntensity = lightIntensity;
				fAmbientIntensity = ambientIntensity;
				fReflectionIntensity = reflectionIntensity;
			}
		};

		struct SpotLightData
		{
			math::float3 f3Color;
			float fLightIntensity = 0.f;

			math::float3 f3Pos;
			float fAmbientIntensity = 0.f;

			math::float3 f3Dir;
			float fReflectionIntensity = 0.f;

			math::float3 padding;
			float fAngle = 0.f;

			void Set(const math::Color& color, const math::float3& position, const math::float3& direction, float lightIntensity, float ambientIntensity, float reflectionIntensity, float angle)
			{
				f3Color.x = color.r;
				f3Color.y = color.g;
				f3Color.z = color.b;

				f3Pos = position;
				f3Dir = direction;
				fLightIntensity = lightIntensity;
				fAmbientIntensity = ambientIntensity;
				fReflectionIntensity = reflectionIntensity;
				fAngle = angle;
			}
		};

		class ILight
		{
		public:
			enum Type
			{
				eDirectional = 0,
				ePoint,
				eSpot,

				eCount,
			};

			enum Capacity
			{
				eMaxDirectionalLightCount = 32,
				eMaxPointLightCount = 1024,
				eMaxSpotLightCount = 128,
			};

		public:
			ILight();
			virtual ~ILight() = 0;

			static IDirectionalLight* CreateDirectionalLight(const string::StringID& strName, const math::float3& f3Direction, const math::Color& color, float fIntensity, float fAmbientIntensity = 0.f, float fReflectionIntensity = 0.f);
			static IPointLight* CreatePointLight(const string::StringID& strName, const math::float3& f3Position, const math::Color& color, float fIntensity, float fAmbientIntensity = 0.f, float fReflectionIntensity = 0.f);
			static ISpotLight* CreateSpotLight(const string::StringID& strName, const math::float3& f3Position, const math::float3& f3Direction, float fAngle, const math::Color& color, float fIntensity, float fAmbientIntensity = 0.f, float fReflectionIntensity = 0.f);

		public:
			virtual void Update(float fElapsedTime) = 0;

		public:
			virtual const string::StringID& GetName() const = 0;

			virtual float GetIntensity() const = 0;
			virtual void SetIntensity(float fDiffuseIntensity) = 0;

			virtual float GetAmbientIntensity() const = 0;
			virtual void SetAmbientIntensity(float fAmbientIntensity) = 0;

			virtual float GetReflectionIntensity() const = 0;
			virtual void SetReflectionIntensity(float fReflectionIntensity) = 0;

			virtual const math::Color& GetColor() const = 0;
			virtual void SetColor(const math::Color& color) = 0;

			virtual bool IsEnableShadow() const = 0;
			virtual void SetEnableShadow(bool isEnableShadow) = 0;

			virtual Type GetType() const = 0;
		};

		class IDirectionalLight : public ILight
		{
		public:
			IDirectionalLight() = default;
			virtual ~IDirectionalLight() = default;

		public:
			virtual Type GetType() const override { return Type::eDirectional; }

		public:
			virtual const math::float3& GetDirection() const = 0;
			virtual void SetDirection(const math::float3& f3Direction) = 0;
		};

		class IPointLight : public ILight
		{
		public:
			IPointLight() = default;
			virtual ~IPointLight() = default;

		public:
			virtual Type GetType() const override { return Type::ePoint; }

		public:
			virtual const math::float3& GetPosition() const = 0;
			virtual void SetPosition(const math::float3& vPos) = 0;
		};

		class ISpotLight : public ILight
		{
		public:
			ISpotLight() = default;
			virtual ~ISpotLight() = default;

		public:
			virtual Type GetType() const override { return Type::eSpot; }

		public:
			virtual const math::float3& GetPosition() const = 0;
			virtual void SetPosition(const math::float3& vPos) = 0;

			virtual const math::float3& GetDirection() const = 0;
			virtual void SetDirection(const math::float3& f3Direction) = 0;

			virtual float GetAngle() const = 0;
			virtual void SetAngle(float fAngle) = 0;
		};
	}
}