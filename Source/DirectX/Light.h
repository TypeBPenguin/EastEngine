#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		struct ShadowConfig;
		struct CascadedShadowsConfig;

		class IDepthStencil;
		class ITexture;
		class IRasterizerState;
		class ISamplerState;
		class IDirectionalLight;
		class IPointLight;
		class ISpotLight;
		class IShadowMap;
		class IShadowCubeMap;
		class ICascadedShadows;

		namespace EmLight
		{
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
		}

		class ILight
		{
		public:
			ILight();
			virtual ~ILight() = 0;

			static IDirectionalLight* CreateDirectionalLight(const String::StringID& strName, const Math::Vector3& f3Direction, const Math::Color& color, float fIntensity, float fAmbientIntensity = 0.f, float fReflectionIntensity = 0.f, CascadedShadowsConfig* pCascadeConfig = nullptr);
			static IPointLight* CreatePointLight(const String::StringID& strName, const Math::Vector3& f3Position, const Math::Color& color, float fIntensity, float fAmbientIntensity = 0.f, float fReflectionIntensity = 0.f, ShadowConfig* pShadowConfig = nullptr);
			static ISpotLight* CreateSpotLight(const String::StringID& strName, const Math::Vector3& f3Position, const Math::Vector3& f3Direction, float fAngle, const Math::Color& color, float fIntensity, float fAmbientIntensity = 0.f, float fReflectionIntensity = 0.f, ShadowConfig* pShadowConfig = nullptr);

		public:
			virtual void Update(float fElapsedTime) = 0;

		public:
			virtual const String::StringID& GetName() const = 0;

			virtual float GetIntensity() const = 0;
			virtual void SetIntensity(float fDiffuseIntensity) = 0;

			virtual float GetAmbientIntensity() const = 0;
			virtual void SetAmbientIntensity(float fAmbientIntensity) = 0;

			virtual float GetReflectionIntensity() const = 0;
			virtual void SetReflectionIntensity(float fReflectionIntensity) = 0;

			virtual const Math::Color& GetColor() const = 0;
			virtual void SetColor(const Math::Color& color) = 0;

			virtual bool IsEnableShadow() const = 0;
			virtual void SetEnableShadow(bool isEnableShadow) = 0;

			virtual EmLight::Type GetType() const = 0;
		};

		class IDirectionalLight : public ILight
		{
		public:
			IDirectionalLight() = default;
			virtual ~IDirectionalLight() = default;

		public:
			virtual EmLight::Type GetType() const override { return EmLight::Type::eDirectional; }

		public:
			virtual ICascadedShadows* GetCascadedShadow() const = 0;

			virtual const Math::Vector3& GetDirection() const = 0;
			virtual void SetDirection(const Math::Vector3& f3Direction) = 0;
		};

		class IPointLight : public ILight
		{
		public:
			IPointLight() = default;
			virtual ~IPointLight() = default;

		public:
			virtual EmLight::Type GetType() const override { return EmLight::Type::ePoint; }

		public:
			virtual IShadowCubeMap* GetShadowCubeMap() const = 0;

			virtual const Math::Vector3& GetPosition() const = 0;
			virtual void SetPosition(const Math::Vector3& vPos) = 0;
		};

		class ISpotLight : public ILight
		{
		public:
			ISpotLight() = default;
			virtual ~ISpotLight() = default;

		public:
			virtual EmLight::Type GetType() const override { return EmLight::Type::eSpot; }

		public:
			virtual IShadowMap* GetShadowMap() const = 0;

			virtual const Math::Vector3& GetPosition() const = 0;
			virtual void SetPosition(const Math::Vector3& vPos) = 0;

			virtual const Math::Vector3& GetDirection() const = 0;
			virtual void SetDirection(const Math::Vector3& f3Direction) = 0;

			virtual float GetAngle() const = 0;
			virtual void SetAngle(float fAngle) = 0;
		};

		class IShadow
		{
		public:
			enum EmBufferFormat
			{
				eR32 = 0,
				eR24G8,
				eR16,
				eR8,
			};

		public:
			IShadow() = default;
			virtual ~IShadow() = default;

		public:
			virtual void Update() = 0;

		public:
			virtual Math::Int2 GetPCFBlurSize() const = 0;
			virtual void SetPCFBlurSize(int nPCFBlurSize) = 0;

			virtual Math::Vector2 GetTexelOffset() = 0;

			virtual int GetBufferSize() const = 0;

			virtual float GetDepthBias() const = 0;
			virtual void SetDepthBias(float fDepthBias) = 0;

			virtual IDepthStencil* GetDepthStencil() const = 0;
			virtual const std::shared_ptr<ITexture>& GetShadowMap() const = 0;

			virtual ISamplerState* GetSamplerPCF() const = 0;
			virtual ISamplerState* GetSamplerPoint() const = 0;
			virtual IRasterizerState* GetRasterizerShadow() const = 0;
		};

		class IShadowMap : public IShadow
		{
		public:
			IShadowMap() = default;
			virtual ~IShadowMap() = default;

		public:
			virtual const Math::Matrix& GetViewMatrix() const = 0;
			virtual const Math::Matrix& GetProjectionMatrix() const = 0;
			virtual const Math::Viewport& GetViewport() const = 0;
			virtual const Collision::Frustum& GetFrustum() const = 0;
		};

		class IShadowCubeMap : public IShadow
		{
		public:
			enum EmDirection
			{
				eRight = 0,
				eLeft,
				eUp,
				eDown,
				eFront,
				eBack,

				DirectionCount,
			};

		public:
			IShadowCubeMap() = default;
			virtual ~IShadowCubeMap() = default;

		public:
			virtual const Math::Matrix& GetViewMatrix(EmDirection emDirection) const = 0;
			virtual const Math::Matrix& GetProjectionMatrix() const = 0;
			virtual const Math::Viewport& GetViewport() const = 0;
			virtual const Collision::Frustum& GetFrustum(EmDirection emDirection) const = 0;

			virtual float GetFarPlane() const = 0;
		};

		struct ShadowConfig
		{
			IShadow::EmBufferFormat emBufferFormat = IShadow::EmBufferFormat::eR32;
			int nBufferSize = 1024;
		};

		struct CascadedShadowsConfig : public ShadowConfig
		{
			enum
			{
				eMinLevel = 1,
				eMaxLevel = 8,
			};

			uint32_t nLevel = eMinLevel;
			float fCascadeDistance = 256.f;
		};

		class ICascadedShadows : public IShadow
		{
		public:
			ICascadedShadows() = default;
			virtual ~ICascadedShadows() = default;

		public:
			virtual uint32_t GetCascadeLevel() const = 0;

			virtual const Math::Matrix& GetViewMatrix(uint32_t nLevel) = 0;
			virtual const Math::Matrix& GetProjectionMatrix(uint32_t nLevel) = 0;
			virtual const Math::Viewport& GetViewport(uint32_t nLevel) = 0;
			virtual const Collision::Frustum& GetFrustum(uint32_t nLevel) = 0;

			virtual const Math::Vector2& GetSplitDepths(uint32_t nLevel) = 0;
		};
	}
}