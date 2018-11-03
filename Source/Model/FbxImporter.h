#pragma once

#define FBXSDK_NEW_API

#pragma warning(push)
#pragma warning( disable : 4616 6011 )
#include <fbxsdk.h>
#pragma warning(pop)

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

#include "CommonLib/Lock.h"

#undef USE_OPENEXR

#include "ExternLib/FBXExporter/ExportObjects/ExportXmlParser.h"
#include "ExternLib/FBXExporter/ExportObjects/ExportPath.h"
#include "ExternLib/FBXExporter/ExportObjects/ExportMaterial.h"
#include "ExternLib/FBXExporter/ExportObjects/ExportObjects.h"

namespace DirectX
{
	struct XMFLOAT4X4;
	struct XMFLOAT3;
}

namespace eastengine
{
	namespace graphics
	{
		class Model;
		class Motion;

		class FBXTransformer : public ATG::IDCCTransformer
		{
		public:
			FBXTransformer();
			~FBXTransformer() = default;

		public:
			void Initialize(fbxsdk::FbxScene* pScene);

			virtual void TransformMatrix(DirectX::XMFLOAT4X4* pDestMatrix, const DirectX::XMFLOAT4X4* pSrcMatrix) const override;
			virtual void TransformPosition(DirectX::XMFLOAT3* pDestPosition, const DirectX::XMFLOAT3* pSrcPosition) const override;
			virtual void TransformDirection(DirectX::XMFLOAT3* pDestDirection, const DirectX::XMFLOAT3* pSrcDirection) const override;
			virtual float TransformLength(float fInputLength) const override;

		public:
			// Sets unit scale for exporting all geometry - works with characters too.
			void SetUnitScale(const float fScale) { m_fUnitScale = fScale; }
			void SetZFlip(const bool bFlip) { m_isFlipZ = bFlip; }

		protected:
			float m_fUnitScale;
			bool m_isMaxConversion;
			bool m_isFlipZ;
		};

		class FBXImport
		{
		public:
			FBXImport();
			~FBXImport();

		public:
			bool Initialize();
			void Release();

		public:
			bool LoadModel(Model* pModel, const char* strFilePath, float fScale, bool isFlipZ);
			bool LoadMotion(Motion* pMotion, const char* strFilePath, float fScale);

		private:
			void ModelSettings();
			void MotionSettings();

			bool ImportFile(const char* strFileName);

		private:
			void SetBindPose();

			math::Matrix ParseTransform(fbxsdk::FbxNode* pNode, ATG::ExportFrame* pFrame, const math::Matrix& matParentWorld, const bool isWarnings = true);
			void ParseNode(fbxsdk::FbxNode* pNode, ATG::ExportFrame* pParentFrame, const math::Matrix& matParentWorld);
			void ParseCamera(fbxsdk::FbxCamera* pFbxCamera, ATG::ExportFrame* pParentFrame);
			void ParseLight(fbxsdk::FbxLight* pFbxLight, ATG::ExportFrame* pParentFrame);
			void FixupNode(ATG::ExportFrame* pFrame, const math::Matrix& matParentWorld);

			struct AnimationScanNode
			{
				int nParentIndex = 0;
				fbxsdk::FbxNode* pNode = nullptr;
				ATG::ExportAnimationTrack* pTrack = nullptr;
				DWORD dwFlags = 0;
				math::Matrix matGlobal;
			};

			void ParseAnimNode(FbxNode* pNode, std::vector<AnimationScanNode>& scanlist, DWORD nFlags, int nParentIndex, bool isIncludeNode);
			void AddKey(AnimationScanNode& asn, const AnimationScanNode* pParent, const FbxAMatrix& matFBXGlobal, float fTime);

			void CaptureAnimation(std::vector<AnimationScanNode>& scanlist, ATG::ExportAnimation* pAnim, FbxScene* pFbxScene);
			void ParseAnimStack(FbxScene* pFbxScene, FbxString* strAnimStackName);
			void ParseAnimation(fbxsdk::FbxScene* pFbxScene);

			void FixupGenericMaterial(ATG::ExportMaterial* pMaterial);
			void AddTextureParameter(ATG::ExportMaterial* pMaterial, const char* strParamName, uint32_t nIndex, const char* strFileName, DWORD nFlags);
			bool ExtractTextures(fbxsdk::FbxProperty Property, const char* strParameterName, ATG::ExportMaterial* pMaterial, DWORD nFlags);
			ATG::ExportMaterial* ParseMaterial(fbxsdk::FbxSurfaceMaterial* pFbxMaterial);

			struct SkinData
			{
				std::vector<fbxsdk::FbxNode*> InfluenceNodes;
				size_t nVertexCount = 0;
				uint32_t nVertexStride = 0;
				std::unique_ptr<byte[]> pBoneIndices;
				std::unique_ptr<float[]> pBoneWeights;

				void Alloc(size_t nCount, DWORD nStride);
				byte* GetIndices(size_t nIndex);
				float* GetWeights(size_t nIndex);

				uint32_t GetBoneCount() const { return static_cast<uint32_t>(InfluenceNodes.size()); }

				void InsertWeight(size_t nIndex, uint32_t nBoneIndex, float fBoneWeight);
			};

			void CaptureBindPoseMatrix(fbxsdk::FbxNode* pNode, const fbxsdk::FbxMatrix& matBindPose);
			bool ParseMeshSkinning(fbxsdk::FbxMesh* pMesh, SkinData* pSkinData);
			void ParseMesh(fbxsdk::FbxNode* pNode, fbxsdk::FbxMesh* pFbxMesh, ATG::ExportFrame* pParentFrame, bool isSubDProcess = false, const char* strSuffix = nullptr);
			void ParseSubDiv(fbxsdk::FbxNode* pNode, fbxsdk::FbxSubDiv* pFbxSubD, ATG::ExportFrame* pParentFrame);

		private:
			thread::SRWLock m_srwLock;

			fbxsdk::FbxManager* m_pSDKManager{ nullptr };
			fbxsdk::FbxImporter* m_pImporter{ nullptr };
			fbxsdk::FbxScene* m_pFBXScene{ nullptr };

			std::unordered_map<eastengine::string::StringID, eastengine::math::Matrix> m_umapMotionOffsetMarix;

			std::unordered_map<string::StringID, fbxsdk::FbxMatrix> m_BindPoseMap;
			bool m_isBindPoseFixupRequired{ false };

			class ConsoleOutListener* m_pConsoleOutListener{ nullptr };
			class ATG::DebugSpewListener* m_pDebugSpewListener{ nullptr };
			FBXTransformer m_fbxTransformer;
			ATG::ExportManifest* m_pManifest{ nullptr };
		};
	}
}