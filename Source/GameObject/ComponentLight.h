#pragma once

#include "ComponentInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ILight;
	}

	namespace GameObject
	{
		class ComponentLight : public IComponent
		{
		public:
			ComponentLight(IActor* pOwner);
			virtual ~ComponentLight();

		public:
			virtual void Update(float fElapsedTime) override;

		private:
			std::list<Graphics::ILight*> m_listLights;
		};
	}
}