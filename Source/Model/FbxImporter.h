#pragma once

#include "../CommonLib/Singleton.h"

namespace fbxsdk
{
	class FbxManager;
	class FbxImporter;
	class FbxScene;
	class FbxNode;
	class FbxAnimStack;
	class FbxAnimLayer;
}

namespace EastEngine
{
	namespace Graphics
	{
		struct VertexPosTexNor;

		class IMaterial;

		class IModel;
		class IModelNode;
		class IMotion;
		class ISkeleton;

		class SFbxImporter : public Singleton<SFbxImporter>
		{
			friend Singleton<SFbxImporter>;
		private:
			SFbxImporter();
			virtual ~SFbxImporter();

		public:
			bool Init();
			void Release();

			bool LoadModel(IModel* pModel, const char* strFilePath, float fScaleFactor);
			bool LoadMotion(IMotion* pMotion, const char* strFilePath, float fScaleFactor);

		private:
			bool initSdkObjects();
			bool initImporter(const char* strFilePath, float fScaleFactor);

		private:
			void createModel(fbxsdk::FbxNode* pNode, IModel* pModel, IModelNode* pParentNode);
			IModelNode* buildNode(fbxsdk::FbxNode* pNode);
			IModelNode* createModelNode(fbxsdk::FbxNode* pNode);
			IModelNode* createStaticNode(fbxsdk::FbxNode* pNode);
			IModelNode* createSkinnedNode(fbxsdk::FbxNode* pNode);

			void createSkeleton(fbxsdk::FbxNode* pNode, IModel* pModel, class ISkeleton* pSkeleton, const String::StringID& strParentName = "");

			void loadGeometry(fbxsdk::FbxNode* pNode, std::vector<VertexPosTexNor>& vecVertices, std::vector<uint32_t>& vecIndicecs,
				std::vector<int>& vecAttributeList, boost::unordered_map<int, std::vector<int>>* pUmapControlPointVertex = nullptr);

			void loadMaterial(fbxsdk::FbxNode* pNode, std::vector<IMaterial*>& vecMaterial);

		private:
			void createMotion(fbxsdk::FbxScene* pScene, IMotion* pMotion);
			void createMotion(fbxsdk::FbxAnimStack* pAnimStack, fbxsdk::FbxNode* pNode, IMotion* pMotion);
			void createMotion(fbxsdk::FbxAnimLayer* pAnimLayer, fbxsdk::FbxNode* pNode, IMotion* pMotion, const char* strTakeName);

		private:
			bool m_isInit;

			fbxsdk::FbxManager* m_pSdkManager;
			fbxsdk::FbxImporter* m_pImporter;
			fbxsdk::FbxScene* m_pScene;

			float m_fScaleFactor;

			boost::unordered_map<String::StringID, Math::Matrix> m_umapOffsetMatrix;
			boost::unordered_map<String::StringID, Math::Matrix> m_umapTransform;
		};
	}
}