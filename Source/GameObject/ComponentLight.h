#pragma once

#include "ComponentInterface.h"

namespace est
{
	namespace graphics
	{
		class ILight;
	}

	namespace gameobject
	{
		class ComponentLight : public IComponent
		{
		public:
			ComponentLight(IActor* pOwner);
			virtual ~ComponentLight();

		public:
			virtual void Update(float elapsedTime, float lodThreshold) override;

		private:
			std::list<graphics::ILight*> m_listLights;
		};
	}
}