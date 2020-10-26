#include "stdafx.h"
#include "PhysicsShape.h"

#include "PhysicsUtil.h"

namespace est
{
	namespace physics
	{
		Shape::Shape(physx::PxShape* pShape, const std::shared_ptr<IMaterial>* ppMaterial, size_t numMaterial)
			: m_pShape(pShape)
		{
			m_pShape->userData = this;

			m_pMaterials.reserve(numMaterial);
			for (size_t i = 0; i < numMaterial; ++i)
			{
				m_pMaterials.emplace_back(ppMaterial[i]);
			}
		}

		Shape::~Shape()
		{
			m_pMaterials.clear();

			if (m_pShape != nullptr)
			{
				m_pShape->release();
				m_pShape->userData = nullptr;
				m_pShape = nullptr;
			}
		}

		void Shape::SetName(const string::StringID& name)
		{
			m_name = name;

			const std::string multiName = string::WideToMulti(m_name.c_str());
			m_pShape->setName(multiName.c_str());
		}

		void Shape::SetGeometry(const IGeometry* pGeometry)
		{
			switch (pGeometry->GetType())
			{
			case IGeometry::eSphere:
			{
				const SphereGeometry* pSphereGeometry = static_cast<const SphereGeometry*>(pGeometry);
				const physx::PxSphereGeometry geometry(pSphereGeometry->radius);
				m_pShape->setGeometry(geometry);
			}
			break;
			case IGeometry::ePlane:
			{
				m_pShape->setGeometry(physx::PxPlaneGeometry());
			}
			break;
			case IGeometry::eCapsule:
			{
				const CapsuleGeometry* pCapsuleGeometry = static_cast<const CapsuleGeometry*>(pGeometry);
				const physx::PxCapsuleGeometry geometry(pCapsuleGeometry->radius, pCapsuleGeometry->halfHeight);
				m_pShape->setGeometry(geometry);
			}
			break;
			case IGeometry::eBox:
			{
				const BoxGeometry* pBoxGeometry = static_cast<const BoxGeometry*>(pGeometry);
				const physx::PxBoxGeometry geometry(Convert<const physx::PxVec3>(pBoxGeometry->halfExtents));
				m_pShape->setGeometry(geometry);
			}
			break;
			case IGeometry::eConvexMesh:
			{
				const ConvexMeshGeometry* pConvexMeshGeometry = static_cast<const ConvexMeshGeometry*>(pGeometry);

				physx::PxConvexMesh* pConvexMesh = nullptr;
				if (pConvexMeshGeometry->pIndices == nullptr || pConvexMeshGeometry->numIndices == 0)
				{
					pConvexMesh = util::CreateConvexMesh(pConvexMeshGeometry->pVertices, pConvexMeshGeometry->numVertices);
				}
				else
				{
					pConvexMesh = util::CreateConvexMesh(pConvexMeshGeometry->pVertices, pConvexMeshGeometry->numVertices, pConvexMeshGeometry->pIndices, pConvexMeshGeometry->numIndices);
				}

				const physx::PxMeshScale meshScale(Convert<const physx::PxVec3>(pConvexMeshGeometry->scale), Convert<const physx::PxQuat>(pConvexMeshGeometry->rotation));
				const physx::PxConvexMeshGeometry geometry(pConvexMesh, meshScale, Convert<const physx::PxConvexMeshGeometryFlags>(pConvexMeshGeometry->flags));
				m_pShape->setGeometry(geometry);
			}
			break;
			case IGeometry::eTriangleMesh:
			{
				const TriangleMeshGeometry* pTriangleMeshGeometry = static_cast<const TriangleMeshGeometry*>(pGeometry);
				physx::PxTriangleMesh* pTriangleMesh = util::CreateTriangleMesh(pTriangleMeshGeometry->pVertices, pTriangleMeshGeometry->numVertices, pTriangleMeshGeometry->pIndices, pTriangleMeshGeometry->numIndices);

				const physx::PxMeshScale meshScale(Convert<const physx::PxVec3>(pTriangleMeshGeometry->scale), Convert<const physx::PxQuat>(pTriangleMeshGeometry->rotation));
				physx::PxTriangleMeshGeometry geometry(pTriangleMesh, meshScale, Convert<const physx::PxMeshGeometryFlags>(pTriangleMeshGeometry->meshFlags));
				m_pShape->setGeometry(geometry);
			}
			break;
			case IGeometry::eHeightField:
			{
				const HeightFieldGeometry* pHeightFieldGeometry = static_cast<const HeightFieldGeometry*>(pGeometry);
				physx::PxHeightField* pHeightField = util::CreateHeightField(pHeightFieldGeometry);
				physx::PxHeightFieldGeometry geometry(pHeightField, Convert<const physx::PxMeshGeometryFlags>(pHeightFieldGeometry->meshFlags), pHeightFieldGeometry->heightScale, pHeightFieldGeometry->rowScale, pHeightFieldGeometry->columnScale);
				m_pShape->setGeometry(geometry);
			}
			break;
			default:
				throw_line("unknown geometry type");
				break;
			}
		}

		void Shape::SetLocalPose(const Transform& pose)
		{
			m_pShape->setLocalPose(Convert<const physx::PxTransform>(pose));
		}

		Transform Shape::GetLocalPose()
		{
			const physx::PxTransform transform = m_pShape->getLocalPose();
			return Convert<const Transform>(transform);
		}

		void Shape::SetSimulationFilterData(const FilterData& filterData)
		{
			m_pShape->setSimulationFilterData(Convert<const physx::PxFilterData>(filterData));
		}

		FilterData Shape::GetSimulationFilterData() const
		{
			const physx::PxFilterData filterData = m_pShape->getSimulationFilterData();
			return Convert<const FilterData>(filterData);
		}

		void Shape::SetQueryFilterData(const FilterData& filterData)
		{
			m_pShape->setQueryFilterData(Convert<const physx::PxFilterData>(filterData));
		}

		FilterData Shape::GetQueryFilterData() const
		{
			const physx::PxFilterData filterData = m_pShape->getQueryFilterData();
			return Convert<const FilterData>(filterData);
		}

		void Shape::SetContactOffset(float contactOffset)
		{
			m_pShape->setContactOffset(contactOffset);
		}

		float Shape::GetContactOffset() const
		{
			return m_pShape->getContactOffset();
		}

		void Shape::SetRestOffset(float restOffset)
		{
			m_pShape->setRestOffset(restOffset);
		}

		float Shape::GetRestOffset() const
		{
			return m_pShape->getRestOffset();
		}

		void Shape::SetFlag(Flag flag, bool isEnable)
		{
			m_pShape->setFlag(Convert<physx::PxShapeFlag::Enum>(flag), isEnable);
		}

		void Shape::SetFlags(Flags flags)
		{
			m_pShape->setFlags(Convert<physx::PxShapeFlags>(flags));
		}

		Shape::Flags Shape::GetFlags() const
		{
			const physx::PxShapeFlags flags = m_pShape->getFlags();
			return Convert<const Shape::Flags>(flags);
		}

		bool Shape::IsExclusive() const
		{
			return m_pShape->isExclusive();
		}

		uint32_t Shape::GetNumMaterials() const
		{
			return static_cast<uint32_t>(m_pMaterials.size());
		}

		std::shared_ptr<IMaterial> Shape::GetMaterial(uint32_t index) const
		{
			if (index >= GetNumMaterials())
				return nullptr;

			return m_pMaterials[index];
		}
	}
}