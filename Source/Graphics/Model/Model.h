#pragma once

#include "ModelInterface.h"
#include "Skeleton.h"

namespace est
{
	namespace graphics
	{
		class ModelNode;
		class ModelInstance;

		class Model : public IModel
		{
		public:
			Model(Key key);
			Model(Model&& source) noexcept;
			virtual ~Model();

			Model& operator=(Model&& source) noexcept;

		public:
			virtual Key GetKey() const override { return m_key; }

		public:
			void AddInstance(ModelInstance* pModelInstance);
			bool RemoveInstance(ModelInstance* pModelInstance);
			bool IsReadyToDestroy(double gameTime);

			void Ready();

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, ITransformInstance* pTransformInstance) override;

			virtual void ChangeName(const string::StringID& name) override;

			void LoadCompleteCallback(bool isSuccess);

		public:
			virtual const math::float3& GetLocalPosition() const override { return m_transform.position; }
			virtual void SetLocalPosition(const math::float3& f3Pos) override { m_transform.position = f3Pos; m_isDirtyLocalMatrix = true; }
			virtual const math::float3& GetLocalScale() const override { return m_transform.scale; }
			virtual void SetLocalScale(const math::float3& f3Scale) override { m_transform.scale = f3Scale; m_isDirtyLocalMatrix = true; }
			virtual const math::Quaternion& GetLocalRotation() const override { return m_transform.rotation; }
			virtual void SetLocalRotation(const math::Quaternion& quat) override { m_transform.rotation = quat; m_isDirtyLocalMatrix = true; }

			virtual const math::Matrix& GetLocalMatrix() const override { return m_matLocal; }

			virtual const string::StringID& GetName() const override { return m_modelName; }
			virtual const std::wstring& GetFilePath() const override { return m_filePath; }

			virtual uint32_t GetNodeCount() const override { return static_cast<uint32_t>(m_modelNodes.size()); }
			virtual IModelNode* GetNode(uint32_t index) const override { return m_modelNodes[index]; }
			virtual IModelNode* GetNode(const string::StringID& name) const override;

			virtual bool IsVisible() const override { return m_isVisible; }
			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }

			virtual ISkeleton* GetSkeleton() override { return &m_skeleton; }

		public:
			void AddNode(std::unique_ptr<ModelNode> pNode, const string::StringID& nodeName);
			void AddNode(std::unique_ptr<ModelNode> pNode, const string::StringID& nodeName, const string::StringID& parentNodeName);

			bool Load(const ModelLoader& loader);
			bool LoadFile(const wchar_t* filePath);
			void SetName(const string::StringID& modelName) { m_modelName = modelName; }
			void SetFilePath(const std::wstring& filePath) { m_filePath = filePath; }

		private:
			Key m_key;

			bool m_isVisible{ true };
			bool m_isDirtyLocalMatrix{ true };
			double m_destroyWaitTime{ 0.0 };

			Skeleton m_skeleton;

			math::Transform m_transform;

			math::Matrix m_matLocal;

			string::StringID m_modelName;
			std::wstring m_filePath;

			std::vector<std::unique_ptr<ModelNode>> m_hierarchyModelNodes;	// 계층 구조 노드
			std::vector<IModelNode*> m_modelNodes;			// 모든 노드

			std::vector<ModelInstance*> m_modelInstances;
		};
	}
}