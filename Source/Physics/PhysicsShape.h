#pragma once

#include "PhysicsInterface.h"

namespace est
{
	namespace physics
	{
		class Shape : public IShape
		{
		public:
			Shape(physx::PxShape* pShape, const std::shared_ptr<IMaterial>* ppMaterial, size_t numMaterial);
			virtual ~Shape();

		public:
			virtual IGeometry::Type GetType() const override { return m_emGeometryType; }

			virtual void SetName(const string::StringID& name) override;
			virtual const string::StringID& GetName() const override { return m_name; }

		public:
			virtual void SetGeometry(const IGeometry* pGeometry) override;

			virtual void SetLocalPose(const Transform& pose) override;
			virtual Transform GetLocalPose() override;

			virtual void SetSimulationFilterData(const FilterData& filterData) override;
			virtual FilterData GetSimulationFilterData() const override;

			virtual void SetQueryFilterData(const FilterData& filterData) override;
			virtual FilterData GetQueryFilterData() const override;

			virtual void SetContactOffset(float contactOffset) override;
			virtual float GetContactOffset() const override;

			virtual void SetRestOffset(float restOffset) override;
			virtual float GetRestOffset() const override;

			virtual void SetFlag(Flag flag, bool isEnable) override;
			virtual void SetFlags(Flags flags) override;
			virtual Flags GetFlags() const override;

			virtual bool IsExclusive() const override;

		public:
			virtual uint32_t GetNumMaterials() const override;
			virtual std::shared_ptr<IMaterial> GetMaterial(uint32_t index) const override;

		public:
			physx::PxShape* GetInterface() const { return m_pShape; }

		private:
			physx::PxShape* m_pShape{ nullptr };
			IGeometry::Type m_emGeometryType{ IGeometry::Type::eInvalid };

			string::StringID m_name;

			std::vector<std::shared_ptr<IMaterial>> m_pMaterials;
		};
	}
}