#pragma once

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IVertexBuffer;
		class IIndexBuffer;
		class IMaterial;
		class ISkeletonInstance;
		
		class ModelNode : public IModelNode
		{
		public:
			ModelNode(EmModelNode::Type emModelNodeType);
			virtual ~ModelNode() = 0;

		public:
			virtual void Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) = 0;

		public:
			virtual EmModelNode::Type GetType() const override { return m_emModelNodeType; }

			virtual bool IsVisible() const override { return m_isVisible; }
			virtual void SetVisible(bool isVisible) override { m_isVisible = isVisible; }

			virtual float GetDistanceFromCamera() const override { return m_fDistanceFromCamera; }
			virtual void SetDistanceFromCamera(float fDist) override { m_fDistanceFromCamera = fDist; }

			virtual const String::StringID& GetName() const override { return m_strNodeName; }
			virtual const String::StringID& GetAttachedBoneName() const override { return m_strAttachedBoneName; }

			virtual IModelNode* GetParentNode() override { return m_pParentNode; }

			virtual IVertexBuffer* GetVertexBuffer(uint32_t nLod = 0) override;
			virtual IIndexBuffer* GetIndexBuffer(uint32_t nLod = 0) override;

			virtual const Math::Matrix& GetWorldMatrix() override { return m_matWorld; }
			virtual const Math::Matrix* GetWorldMatrixPtr() override { return &m_matWorld; }

			virtual size_t GetChildNodeCount() const override { return m_vecChildModelNode.size(); }
			virtual IModelNode* GetChildNode(size_t nIndex) override { return m_vecChildModelNode[nIndex]; }

			virtual size_t GetMaterialCount() const override { return m_vecMaterial.size(); }
			virtual IMaterial* GetMaterial(size_t nIndex) override { return m_vecMaterial[nIndex]; }

			virtual size_t GetModelSubsetCount(uint32_t nLod = 0) const override { return m_vecModelSubsets[nLod].size(); }
			virtual ModelSubset* GetModelSubset(size_t nIndex, uint32_t nLod = 0) override { return &m_vecModelSubsets[nLod][nIndex]; }

			virtual void BuildBoundingBox(const Collision::AABB& aabb) override;
			virtual void UpdateBoundingBox() override;

			virtual const Collision::AABB& GetAABB() override { return m_boundingAABB; }
			virtual const Collision::OBB& GetOBB() override { return m_boundingOBB; }
			virtual const Collision::Sphere& GetSphere() override { return m_boundingSphere; }

			virtual uint32_t GetLOD() override { return m_nLod; }
			virtual void SetLOD(uint32_t nLod) override { m_nLod = nLod; }

			void SetNodeName(const String::StringID& strNodeName) { m_strNodeName = strNodeName; }
			void SetAttachedBoneName(const String::StringID& strNodeName) { m_strAttachedBoneName = strNodeName; }

			void SetParentNode(IModelNode* pModelNode) { m_pParentNode = pModelNode; }
			void AddChildNode(IModelNode* pChildNode) { m_vecChildModelNode.push_back(pChildNode); }

			void SetVertexBuffer(IVertexBuffer* pVertexBuffer, uint32_t nLod = 0);
			void SetIndexBuffer(IIndexBuffer* pIndexBuffer, uint32_t nLod = 0);

			void AddMaterial(IMaterial* pMaterial);
			void AddMaterialArray(IMaterial** ppMaterials, uint32_t nCount);
			void AddModelSubset(ModelSubset& modelSubset, uint32_t nLod = 0);
			void AddModelSubsets(const std::vector<ModelSubset>& vecModelSubsets, uint32_t nLod = 0);

		protected:
			bool m_isVisible;
			String::StringID m_strNodeName;
			String::StringID m_strAttachedBoneName;

			IModelNode* m_pParentNode;
			std::vector<IModelNode*> m_vecChildModelNode;

			Math::Matrix m_matWorld;

			std::vector<IMaterial*> m_vecMaterial;
			std::vector<ModelSubset> m_vecModelSubsets[eMaxLod];

			std::array<IVertexBuffer*, eMaxLod> m_pVertexBuffer;
			std::array<IIndexBuffer*, eMaxLod> m_pIndexBuffer;

			uint32_t m_nLod;
			uint32_t m_nLodMax;

			float m_fDistanceFromCamera;

			Collision::AABB m_originAABB;

			Collision::AABB m_boundingAABB;
			Collision::OBB m_boundingOBB;
			Collision::Sphere m_boundingSphere;

		private:
			EmModelNode::Type m_emModelNodeType;
		};
	}
}