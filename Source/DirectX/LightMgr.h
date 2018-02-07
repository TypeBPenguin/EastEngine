#pragma once

#include "CommonLib/Singleton.h"

#include "Light.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IStructuredBuffer;

		class LightManager : public Singleton<LightManager>
		{
			friend Singleton<LightManager>;
		private:
			LightManager();
			virtual ~LightManager();

		public:
			void Update(float fElapsedTime);
			void Synchronize();

		public:
			bool AddLight(ILight* pLight);
			void Remove(ILight* pLight);
			void Remove(EmLight::Type emType, size_t nIndex);
			void RemoveAll();
			ILight* GetLight(EmLight::Type emType, size_t nIndex);
			size_t GetLightCount(EmLight::Type emType);

		public:
			IStructuredBuffer* GetLightBuffer(EmLight::Type emType);
			uint32_t GetLightCountInView(EmLight::Type emType);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;

		};
	}
}