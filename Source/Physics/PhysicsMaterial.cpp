#include "stdafx.h"
#include "PhysicsMaterial.h"

namespace est
{
	namespace physics
	{
		Material::Material(physx::PxMaterial* pMaterial)
			: m_pMaterial(pMaterial)
		{
		}
		
		Material::~Material()
		{
			if (m_pMaterial != nullptr)
			{
				m_pMaterial->release();
				m_pMaterial = nullptr;
			}
		}
	}
}