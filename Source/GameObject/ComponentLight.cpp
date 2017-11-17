#include "stdafx.h"
#include "ComponentLight.h"

namespace EastEngine
{
	namespace GameObject
	{
		ComponentLight::ComponentLight(IActor* pOwner)
			: IComponent(pOwner, EmComponent::eLight)
		{
		}

		ComponentLight::~ComponentLight()
		{
		}
	}
}
