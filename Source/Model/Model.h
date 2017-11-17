#pragma once

#include "../CommonLib/plf_colony.h"

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ModelInstance;

		class Model : public IModel
		{
		public:
			Model(uint32_t nReserveInstance);
			virtual ~Model();

		public:
			IModelInstance* CreateInstance();
			void DestroyInstance(IModelInstance** ppModelInstance);

			void Process();

		public:
			virtual void Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance) override;

			virtual void ChangeName(const String::StringID& strName) override;

			void LoadCompleteCallback(bool isSuccess);

		public:
			virtual const Math::Vector3& GetLocalPosition() override { return m_f3Pos; }
			virtual void SetLocalPosition(const Math::Vector3& f3Pos) override { m_f3Pos = f3Pos; m_isDirtyLocalMatrix = true; }
			virtual const Math::Vector3& GetLocalScale() override { return m_f3Scale; }
			virtual void SetLocalScale(const Math::Vector3& f3Scale) override { m_f3Scale = f3Scale; m_isDirtyLocalMatrix = true; }
			virtual const Math::Quaternion& GetLocalRotation() override { return m_quat; }
			virtual void SetLocalRotation(const Math::Quaternion& quat) override { m_quat = quat; m_isDirtyLocalMatrix = true; }

			virtual const String::StringID& GetName() override { return m_strModelName; }
			virtual const std::string& GetFilePath() override { return m_strFilePath; }

			virtual uint32_t GetNodeCount() override { return m_vecAllModelNode.size(); }
			virtual IModelNode* GetNode(uint32_t nIndex) override { return m_vecAllModelNode[nIndex]; }
			virtual IModelNode* GetNode(const String::StringID& strName) override;

			virtual bool IsVisible() override { return m_isVisible; }
			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }

			virtual ISkeleton* GetSkeleton() override { return m_pSkeleton; }

			virtual bool IsSkinningModel() const override { return m_pSkeleton != nullptr; }

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

			void SetSkeleton(ISkeleton* pSkeleton) { m_pSkeleton = pSkeleton; }
			bool Load(const ModelLoader& loader);
			bool LoadToFile(const char* strFilePath);
			void SetName(const String::StringID& strModelName) { m_strModelName = strModelName; }
			void SetFilePath(const std::string& strFilePath) { m_strFilePath = strFilePath; }

		protected:
			bool m_isVisible;
			bool m_isDirtyLocalMatrix;

			int m_nReferenceCount;

			ISkeleton* m_pSkeleton;

			Math::Vector3 m_f3Pos;
			Math::Vector3 m_f3Scale;
			Math::Quaternion m_quat;

			Math::Matrix m_matLocal;

			String::StringID m_strModelName;
			std::string	m_strFilePath;

			std::vector<IModelNode*> m_vecHierarchyModelNode;	// 계층 구조 노드
			std::vector<IModelNode*> m_vecAllModelNode;			// 모든 노드

			plf::colony<ModelInstance> m_clnModelInstance;
		};
	}
}