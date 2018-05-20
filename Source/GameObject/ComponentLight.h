#pragma once

#include "ComponentInterface.h"

namespace eastengine
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
			virtual void Update(float fElapsedTime) override;

		private:
			std::list<graphics::ILight*> m_listLights;
		};
	}
}