#include "stdafx.h"
#include "ComponentLight.h"

namespace est
{
	namespace gameobject
	{
		ComponentLight::ComponentLight(IActor* pOwner)
			: IComponent(pOwner, IComponent::eLight)
		{
		}

		ComponentLight::~ComponentLight()
		{
		}
	}
}
