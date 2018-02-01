#pragma once

#include "ModelInterface.h"
#include "Skeleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ModelInstance;

		class Model : public IModel
		{
		public:
			Model(Key key);
			virtual ~Model();

		public:
			virtual Key GetKey() const override { return m_key; }

		public:
			void AddInstance(ModelInstance* pModelInstance);
			bool RemoveInstance(ModelInstance* pModelInstance);
			bool IsHasInstance() const;

			void Ready();

		public:
			virtual void Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance) override;

			virtual void ChangeName(const String::StringID& strName) override;

			void LoadCompleteCallback(bool isSuccess);

		public:
			virtual const Math::Vector3& GetLocalPosition() const override { return m_f3Pos; }
			virtual void SetLocalPosition(const Math::Vector3& f3Pos) override { m_f3Pos = f3Pos; m_isDirtyLocalMatrix = true; }
			virtual const Math::Vector3& GetLocalScale() const override { return m_f3Scale; }
			virtual void SetLocalScale(const Math::Vector3& f3Scale) override { m_f3Scale = f3Scale; m_isDirtyLocalMatrix = true; }
			virtual const Math::Quaternion& GetLocalRotation() const override { return m_quat; }
			virtual void SetLocalRotation(const Math::Quaternion& quat) override { m_quat = quat; m_isDirtyLocalMatrix = true; }

			virtual const Math::Matrix& GetLocalMatrix() const override { return m_matLocal; }

			virtual const String::StringID& GetName() const override { return m_strModelName; }
			virtual const std::string& GetFilePath() const override { return m_strFilePath; }

			virtual size_t GetNodeCount() const override { return m_vecModelNodes.size(); }
			virtual IModelNode* GetNode(size_t nIndex) const override { return m_vecModelNodes[nIndex]; }
			virtual IModelNode* GetNode(const String::StringID& strName) const override;

			virtual bool IsVisible() const override { return m_isVisible; }
			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }

			virtual ISkeleton* GetSkeleton() override { return &m_skeleton; }

		public:
			virtual int GetReferenceCount() const override { return m_nReferenceCount; }
			virtual int IncreaseReference() override
			{
				++m_nReferenceCount;
				SetAlive(true);

				return m_nReferenceCount;
			}
			virtual int DecreaseReference() override { return --m_nReferenceCount; }

		public:
			void AddNode(IModelNode* pNode, const String::StringID& strNodeName, bool isRootNode);

			bool Load(const ModelLoader& loader);
			bool LoadToFile(const char* strFilePath);
			void SetName(const String::StringID& strModelName) { m_strModelName = strModelName; }
			void SetFilePath(const std::string& strFilePath) { m_strFilePath = strFilePath; }

		private:
			const Key m_key;
			int m_nReferenceCount;

			bool m_isVisible;
			bool m_isDirtyLocalMatrix;

			Skeleton m_skeleton;

			Math::Vector3 m_f3Pos;
			Math::Vector3 m_f3Scale;
			Math::Quaternion m_quat;

			Math::Matrix m_matLocal;

			String::StringID m_strModelName;
			std::string	m_strFilePath;

			std::vector<IModelNode*> m_vecHierarchyModelNodes;	// 계층 구조 노드
			std::vector<IModelNode*> m_vecModelNodes;			// 모든 노드

			std::vector<ModelInstance*> m_vecModelInstances;
		};
	}
}