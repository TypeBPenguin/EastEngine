#pragma once

#include "ModelInterface.h"
#include "Skeleton.h"

namespace eastengine
{
	namespace graphics
	{
		class ModelInstance;

		class Model : public IModel
		{
		public:
			Model(Key key);
			Model(const Model& source);
			virtual ~Model();

		public:
			virtual Key GetKey() const override { return m_key; }

		public:
			void AddInstance(ModelInstance* pModelInstance);
			bool RemoveInstance(ModelInstance* pModelInstance);
			bool IsHasInstance() const;

			void Ready();

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance) override;

			virtual void ChangeName(const string::StringID& strName) override;

			void LoadCompleteCallback(bool isSuccess);

		public:
			virtual const math::float3& GetLocalPosition() const override { return m_transform.position; }
			virtual void SetLocalPosition(const math::float3& f3Pos) override { m_transform.position = f3Pos; m_isDirtyLocalMatrix = true; }
			virtual const math::float3& GetLocalScale() const override { return m_transform.scale; }
			virtual void SetLocalScale(const math::float3& f3Scale) override { m_transform.scale = f3Scale; m_isDirtyLocalMatrix = true; }
			virtual const math::Quaternion& GetLocalRotation() const override { return m_transform.rotation; }
			virtual void SetLocalRotation(const math::Quaternion& quat) override { m_transform.rotation = quat; m_isDirtyLocalMatrix = true; }

			virtual const math::Matrix& GetLocalMatrix() const override { return m_matLocal; }

			virtual const string::StringID& GetName() const override { return m_strModelName; }
			virtual const std::string& GetFilePath() const override { return m_strFilePath; }

			virtual uint32_t GetNodeCount() const override { return static_cast<uint32_t>(m_vecModelNodes.size()); }
			virtual IModelNode* GetNode(uint32_t nIndex) const override { return m_vecModelNodes[nIndex]; }
			virtual IModelNode* GetNode(const string::StringID& strName) const override;

			virtual bool IsVisible() const override { return m_isVisible; }
			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }

			virtual ISkeleton* GetSkeleton() override { return &m_skeleton; }

		public:
			void AddNode(IModelNode* pNode, const string::StringID& strNodeName, bool isRootNode);

			bool Load(const ModelLoader& loader);
			bool LoadFile(const char* strFilePath);
			void SetName(const string::StringID& strModelName) { m_strModelName = strModelName; }
			void SetFilePath(const std::string& strFilePath) { m_strFilePath = strFilePath; }

		private:
			Key m_key;

			bool m_isVisible{ true };
			bool m_isDirtyLocalMatrix{ true };

			Skeleton m_skeleton;

			math::Transform m_transform;

			math::Matrix m_matLocal;

			string::StringID m_strModelName;
			std::string	m_strFilePath;

			std::vector<IModelNode*> m_vecHierarchyModelNodes;	// 계층 구조 노드
			std::vector<IModelNode*> m_vecModelNodes;			// 모든 노드

			std::vector<ModelInstance*> m_vecModelInstances;
		};
	}
}