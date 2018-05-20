#include "stdafx.h"
#include "ComponentLight.h"

namespace eastengine
{
	namespace gameobject
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
