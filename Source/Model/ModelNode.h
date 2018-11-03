#pragma once

#include "ModelInterface.h"

namespace eastengine
{
	namespace graphics
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
			virtual void Update(float fElapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) const = 0;

		public:
			virtual EmModelNode::Type GetType() const override { return m_emModelNodeType; }

			virtual bool IsVisible() const override { return m_isVisible; }
			virtual void SetVisible(bool isVisible) override { m_isVisible = isVisible; }

			virtual float GetDistanceFromCamera() const override { return m_fDistanceFromCamera; }
			virtual void SetDistanceFromCamera(float fDist) override { m_fDistanceFromCamera = fDist; }

			virtual const string::StringID& GetName() const override { return m_strNodeName; }
			virtual const string::StringID& GetAttachedBoneName() const override { return m_strAttachedBoneName; }

			virtual IModelNode* GetParentNode() const override { return m_pParentNode; }

			virtual IVertexBuffer* GetVertexBuffer(uint32_t nLod = 0) const override;
			virtual IIndexBuffer* GetIndexBuffer(uint32_t nLod = 0) const override;

			virtual uint32_t GetChildNodeCount() const override { return static_cast<uint32_t>(m_vecChildModelNode.size()); }
			virtual IModelNode* GetChildNode(uint32_t nIndex) const override { return m_vecChildModelNode[nIndex]; }

			virtual uint32_t GetMaterialCount() const override { return static_cast<uint32_t>(m_vecMaterial.size()); }
			virtual IMaterial* GetMaterial(uint32_t nIndex) const override { return m_vecMaterial[nIndex]; }
			virtual IMaterial* GetMaterial(const string::StringID& strMaterialName, uint32_t& nMaterialID_out) const override;

			virtual uint32_t GetModelSubsetCount(uint32_t nLod = 0) const override { return static_cast<uint32_t>(m_vecModelSubsets[nLod].size()); }
			virtual const ModelSubset* GetModelSubset(uint32_t nIndex, uint32_t nLod = 0) const override { return &m_vecModelSubsets[nLod][nIndex]; }

			virtual void SetOriginAABB(const Collision::AABB& aabb) override;
			virtual const Collision::AABB& GetOriginAABB() const override { return m_originAABB; }

			virtual uint32_t GetLOD() const override { return m_nLod; }
			virtual void SetLOD(uint32_t nLod) override { m_nLod = nLod; }

			void SetNodeName(const string::StringID& strNodeName) { m_strNodeName = strNodeName; }
			void SetAttachedBoneName(const string::StringID& strNodeName) { m_strAttachedBoneName = strNodeName; }

			void SetParentNode(IModelNode* pModelNode) { m_pParentNode = pModelNode; }
			void AddChildNode(IModelNode* pChildNode) { m_vecChildModelNode.push_back(pChildNode); }

			void SetVertexBuffer(IVertexBuffer* pVertexBuffer, uint32_t nLod = 0);
			void SetIndexBuffer(IIndexBuffer* pIndexBuffer, uint32_t nLod = 0);

			void AddMaterial(IMaterial* pMaterial);
			void AddMaterialArray(IMaterial** ppMaterials, size_t nCount);
			void AddModelSubset(ModelSubset& modelSubset, uint32_t nLod = 0);
			void AddModelSubsets(const std::vector<ModelSubset>& vecModelSubsets, uint32_t nLod = 0);

		protected:
			bool m_isVisible;
			string::StringID m_strNodeName;
			string::StringID m_strAttachedBoneName;

			IModelNode* m_pParentNode;
			std::vector<IModelNode*> m_vecChildModelNode;

			std::vector<IMaterial*> m_vecMaterial;
			std::vector<ModelSubset> m_vecModelSubsets[eMaxLod];

			std::array<IVertexBuffer*, eMaxLod> m_pVertexBuffer;
			std::array<IIndexBuffer*, eMaxLod> m_pIndexBuffer;

			uint32_t m_nLod;
			uint32_t m_nLodMax;

			float m_fDistanceFromCamera;

			Collision::AABB m_originAABB;

		private:
			EmModelNode::Type m_emModelNodeType;
		};
	}
}