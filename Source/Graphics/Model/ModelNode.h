#pragma once

#include "CommonLib/FileStream.h"

#include "ModelInterface.h"

namespace est
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
			ModelNode(Type emType, const wchar_t* filePath, BinaryReader& binaryReader);
			virtual ~ModelNode() = 0;

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, ITransformInstance* pTransformInstance, bool isModelVisible) const = 0;

		public:
			virtual bool IsVisible() const override { return m_isVisible; }
			virtual void SetVisible(bool isVisible) override { m_isVisible = isVisible; }

			virtual float GetDistanceFromCamera() const override { return m_fDistanceFromCamera; }
			virtual void SetDistanceFromCamera(float fDist) override { m_fDistanceFromCamera = fDist; }

			virtual const string::StringID& GetName() const override { return m_nodeName; }
			virtual const string::StringID& GetAttachedBoneName() const override { return m_attachedBoneName; }

			virtual const string::StringID& GetParentName() const override { return m_parentNodeName; }

			virtual VertexBufferPtr GetVertexBuffer(LOD emLod = eLv0) const override;
			virtual IndexBufferPtr GetIndexBuffer(LOD emLod = eLv0) const override;

			virtual void GetRawVertices(const VertexPos** ppVertices, size_t& vertexCount) const override;
			virtual void GetRawIndices(const uint32_t** ppIndices, size_t& indexCount) const override;

			virtual uint32_t GetChildNodeCount() const override { return static_cast<uint32_t>(m_childNodes.size()); }
			virtual IModelNode* GetChildNode(uint32_t index) const override { return m_childNodes[index].get(); }

			virtual uint32_t GetMaterialCount() const override { return static_cast<uint32_t>(m_materials.size()); }
			virtual MaterialPtr GetMaterial(uint32_t index) const override { return m_materials[index]; }
			virtual MaterialPtr GetMaterial(const string::StringID& strMaterialName, uint32_t& nMaterialID_out) const override;

			virtual uint32_t GetModelSubsetCount(LOD emLod = eLv0) const override { assert(emLod <= m_emMaxLod); return static_cast<uint32_t>(m_modelSubsets[emLod].size()); }
			virtual const ModelSubset* GetModelSubset(uint32_t index, LOD emLod = eLv0) const override { assert(emLod <= m_emMaxLod); return &m_modelSubsets[emLod][index]; }

			virtual void SetOriginAABB(const collision::AABB& aabb) override;
			virtual const collision::AABB& GetOriginAABB() const override { return m_originAABB; }

			virtual LOD GetLOD() const override { return m_emLod; }
			virtual void SetLOD(LOD emLod = eLv0) override { m_emLod = std::min(emLod, m_emMaxLod); }

		public:
			void SetNodeName(const string::StringID& strNodeName) { m_nodeName = strNodeName; }
			void SetAttachedBoneName(const string::StringID& strNodeName) { m_attachedBoneName = strNodeName; }

			void SetParentName(const string::StringID& parentName) { m_parentNodeName = parentName; }
			void AddChildNode(std::unique_ptr<ModelNode> pChildNode) { m_childNodes.emplace_back(std::move(pChildNode)); }

			void SetVertexBuffer(const VertexBufferPtr& pVertexBuffer, LOD emLod = eLv0);
			void SetIndexBuffer(const IndexBufferPtr& pIndexBuffer, LOD emLod = eLv0);

			void AddMaterial(const MaterialPtr& pMaterial);
			void AddMaterialArray(const MaterialPtr* ppMaterials, size_t count);
			void AddModelSubset(ModelSubset& modelSubset, LOD emLod = eLv0);
			void AddModelSubsets(const std::vector<ModelSubset>& modelSubsets, LOD emLod = eLv0);

			void SetRawVertices(const VertexPos* pVertices, size_t vertexCount) { m_rawVertices.clear(); m_rawVertices.assign(pVertices, pVertices + vertexCount); }
			void SetRawIndices(const uint32_t* pIndices, size_t indexCount) { m_rawIndices.clear(); m_rawIndices.assign(pIndices, pIndices + indexCount); }

			void SetRawVertices(std::vector<VertexPos>&& source) { m_rawVertices = std::move(source); }
			void SetRawIndices(std::vector<uint32_t>&& source) { m_rawIndices = std::move(source); }

		protected:
			bool m_isVisible{ true };
			string::StringID m_nodeName;
			string::StringID m_attachedBoneName{ sid::None };
			string::StringID m_parentNodeName{ sid::None };

			std::vector<std::unique_ptr<ModelNode>> m_childNodes;

			std::vector<MaterialPtr> m_materials;
			std::vector<ModelSubset> m_modelSubsets[eMaxLod];

			std::array<VertexBufferPtr, eMaxLod> m_pVertexBuffer{ nullptr };
			std::array<IndexBufferPtr, eMaxLod> m_pIndexBuffer{ nullptr };

			std::vector<VertexPos> m_rawVertices;
			std::vector<uint32_t> m_rawIndices;

			LOD m_emLod{ LOD::eLv0 };
			LOD m_emMaxLod{ LOD::eLv0 };

			float m_fDistanceFromCamera{ 0.f };

			collision::AABB m_originAABB;
		};
	}
}