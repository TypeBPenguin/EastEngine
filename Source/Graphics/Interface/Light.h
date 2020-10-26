#pragma once

#include "Resource.h"

#include "Shadow.h"
#include "CascadedShadows.h"

namespace sid
{
	RegisterStringID(DirectionalLight);
	RegisterStringID(PointLight);
	RegisterStringID(SpotLight);
}

namespace est
{
	namespace graphics
	{
		class IDirectionalLight;
		class IPointLight;
		class ISpotLight;

		struct DirectionalLightData
		{
			math::float3 color{ math::float3::One };
			float lightIntensity{ 1.f };

			math::float3 direction{ math::float3::Forward };
			float ambientIntensity{ 0 };

			math::float3 padding;
			float reflectionIntensity{ 0 };
		};

		struct PointLightData
		{
			math::float3 color{ math::float3::One };
			float lightIntensity{ 1.f };

			math::float3 position;
			float ambientIntensity{ 0.f };

			math::float3 padding;
			float reflectionIntensity{ 0.f };
		};

		struct SpotLightData
		{
			math::float3 color{ math::float3::One };
			float lightIntensity{ 1.f };

			math::float3 position;
			float ambientIntensity{ 0.f };

			math::float3 direction{ math::float3::Forward };
			float reflectionIntensity{ 0.f };

			math::float3 padding;
			float angle{ 0.f };
		};

		class ILight : public IResource
		{
			GraphicsResource(ILight);
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
				eMaxDirectionalLightCount = 8,
				eMaxPointLightCount = 64,
				eMaxSpotLightCount = 16,
			};

		protected:
			ILight() = default;
			virtual ~ILight() = default;

		public:
			virtual Type GetType() const = 0;
			virtual const string::StringID& GetName() const = 0;

			virtual float GetIntensity() const = 0;
			virtual void SetIntensity(float fDiffuseIntensity) = 0;

			virtual float GetAmbientIntensity() const = 0;
			virtual void SetAmbientIntensity(float fAmbientIntensity) = 0;

			virtual float GetReflectionIntensity() const = 0;
			virtual void SetReflectionIntensity(float fReflectionIntensity) = 0;

			virtual const math::float3& GetColor() const = 0;
			virtual void SetColor(const math::float3& color) = 0;

			virtual bool IsEnableShadow() const = 0;
			virtual void SetEnableShadow(bool isEnableShadow) = 0;
		};
		using LightPtr = std::shared_ptr<ILight>;

		class IDirectionalLight : public ILight
		{
			GraphicsResource(IDirectionalLight);
		protected:
			IDirectionalLight() = default;
			virtual ~IDirectionalLight() = default;

		public:
			virtual const string::StringID& GetResourceType() const override { return sid::DirectionalLight; }
			virtual Type GetType() const override { return Type::eDirectional; }

		public:
			virtual const math::float3& GetDirection() const = 0;
			virtual void SetDirection(const math::float3& f3Direction) = 0;

			virtual const DirectionalLightData& GetData() const = 0;
			virtual CascadedShadows& GetCascadedShadows() = 0;

			virtual void SetDepthMapResource(void* pResource) = 0;
			virtual void* GetDepthMapResource() const = 0;
		};
		using DirectionalLightPtr = std::shared_ptr<IDirectionalLight>;

		class IPointLight : public ILight
		{
			GraphicsResource(IPointLight);
		protected:
			IPointLight() = default;
			virtual ~IPointLight() = default;

		public:
			virtual const string::StringID& GetResourceType() const override { return sid::PointLight; }
			virtual Type GetType() const override { return Type::ePoint; }

		public:
			virtual const math::float3& GetPosition() const = 0;
			virtual void SetPosition(const math::float3& vPos) = 0;

			virtual const PointLightData& GetData() const = 0;
		};
		using PointLightPtr = std::shared_ptr<IPointLight>;

		class ISpotLight : public ILight
		{
			GraphicsResource(ISpotLight);
		protected:
			ISpotLight() = default;
			virtual ~ISpotLight() = default;

		public:
			virtual const string::StringID& GetResourceType() const override { return sid::SpotLight; }
			virtual Type GetType() const override { return Type::eSpot; }

		public:
			virtual const math::float3& GetPosition() const = 0;
			virtual void SetPosition(const math::float3& vPos) = 0;

			virtual const math::float3& GetDirection() const = 0;
			virtual void SetDirection(const math::float3& f3Direction) = 0;

			virtual float GetAngle() const = 0;
			virtual void SetAngle(float fAngle) = 0;

			virtual const SpotLightData& GetData() const = 0;
		};
		using SpotLightPtr = std::shared_ptr<ISpotLight>;
	}
}