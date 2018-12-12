#pragma once

#include "CommonLib/FileStream.h"

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
			ModelNode(LOD emMaxLod = LOD::eLv0);
			ModelNode(Type emType, const char* filePath, BinaryReader& binaryReader);
			virtual ~ModelNode() = 0;

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) const = 0;

		public:
			virtual bool IsVisible() const override { return m_isVisible; }
			virtual void SetVisible(bool isVisible) override { m_isVisible = isVisible; }

			virtual float GetDistanceFromCamera() const override { return m_fDistanceFromCamera; }
			virtual void SetDistanceFromCamera(float fDist) override { m_fDistanceFromCamera = fDist; }

			virtual const string::StringID& GetName() const override { return m_nodeName; }
			virtual const string::StringID& GetAttachedBoneName() const override { return m_attachedBoneName; }

			virtual const string::StringID& GetParentName() const override { return m_parentNodeName; }

			virtual IVertexBuffer* GetVertexBuffer(LOD emLod = eLv0) const override;
			virtual IIndexBuffer* GetIndexBuffer(LOD emLod = eLv0) const override;

			virtual void GetRawVertices(const VertexPos** ppVertices, size_t& vertexCount) const override;
			virtual void GetRawIndices(const uint32_t** ppIndices, size_t& indexCount) const override;

			virtual uint32_t GetChildNodeCount() const override { return static_cast<uint32_t>(m_vecChildModelNode.size()); }
			virtual IModelNode* GetChildNode(uint32_t index) const override { return m_vecChildModelNode[index]; }

			virtual uint32_t GetMaterialCount() const override { return static_cast<uint32_t>(m_vecMaterial.size()); }
			virtual IMaterial* GetMaterial(uint32_t index) const override { return m_vecMaterial[index]; }
			virtual IMaterial* GetMaterial(const string::StringID& strMaterialName, uint32_t& nMaterialID_out) const override;

			virtual uint32_t GetModelSubsetCount(LOD emLod = eLv0) const override { assert(emLod <= m_emMaxLod); return static_cast<uint32_t>(m_vecModelSubsets[emLod].size()); }
			virtual const ModelSubset* GetModelSubset(uint32_t index, LOD emLod = eLv0) const override { assert(emLod <= m_emMaxLod); return &m_vecModelSubsets[emLod][index]; }

			virtual void SetOriginAABB(const Collision::AABB& aabb) override;
			virtual const Collision::AABB& GetOriginAABB() const override { return m_originAABB; }

			virtual LOD GetLOD() const override { return m_emLod; }
			virtual void SetLOD(LOD emLod = eLv0) override { m_emLod = std::min(emLod, m_emMaxLod); }

		public:
			void SetNodeName(const string::StringID& strNodeName) { m_nodeName = strNodeName; }
			void SetAttachedBoneName(const string::StringID& strNodeName) { m_attachedBoneName = strNodeName; }

			void SetParentName(const string::StringID& parentName) { m_parentNodeName = parentName; }
			void AddChildNode(IModelNode* pChildNode) { m_vecChildModelNode.push_back(pChildNode); }

			void SetVertexBuffer(IVertexBuffer* pVertexBuffer, LOD emLod = eLv0);
			void SetIndexBuffer(IIndexBuffer* pIndexBuffer, LOD emLod = eLv0);

			void AddMaterial(IMaterial* pMaterial);
			void AddMaterialArray(IMaterial** ppMaterials, size_t count);
			void AddModelSubset(ModelSubset& modelSubset, LOD emLod = eLv0);
			void AddModelSubsets(const std::vector<ModelSubset>& vecModelSubsets, LOD emLod = eLv0);

			void SetRawVertices(const VertexPos* pVertices, size_t vertexCount) { m_rawVertices.clear(); m_rawVertices.assign(pVertices, pVertices + vertexCount); }
			void SetRawIndices(const uint32_t* pIndices, size_t indexCount) { m_rawIndices.clear(); m_rawIndices.assign(pIndices, pIndices + indexCount); }

			void SetRawVertices(std::vector<VertexPos>&& source) { m_rawVertices = std::move(source); }
			void SetRawIndices(std::vector<uint32_t>&& source) { m_rawIndices = std::move(source); }

		protected:
			bool m_isVisible{ true };
			string::StringID m_nodeName;
			string::StringID m_attachedBoneName{ StrID::None };
			string::StringID m_parentNodeName{ StrID::None };

			std::vector<IModelNode*> m_vecChildModelNode;

			std::vector<IMaterial*> m_vecMaterial;
			std::vector<ModelSubset> m_vecModelSubsets[eMaxLod];

			std::array<IVertexBuffer*, eMaxLod> m_pVertexBuffer{ nullptr };
			std::array<IIndexBuffer*, eMaxLod> m_pIndexBuffer{ nullptr };

			std::vector<VertexPos> m_rawVertices;
			std::vector<uint32_t> m_rawIndices;

			LOD m_emLod{ LOD::eLv0 };
			LOD m_emMaxLod{ LOD::eLv0 };

			float m_fDistanceFromCamera{ 0.f };

			Collision::AABB m_originAABB;
		};
	}
}