#include "StdAfx.h"
#include "FbxImporter.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"
#include "CommonLib/Performance.h"

#include "DirectX/Vertex.h"

#include "Model.h"
#include "ModelNodeStatic.h"
#include "ModelNodeSkinned.h"
#include "Motion.h"

#include "Skeleton.h"

#include "FbxWriter.h"

ATG::ExportScene* g_pScene = nullptr;
ExportPath g_CurrentInputFileName;
ExportPath g_CurrentOutputFileName;

namespace EastEngine
{
	namespace Graphics
	{
		Math::Matrix ConvertMatrix(const fbxsdk::FbxMatrix& fbxMatrix)
		{
			Math::Matrix matResult;
			float* pFloats = reinterpret_cast<float*>(&matResult);
			const double* pDoubles = reinterpret_cast<const double*>(&fbxMatrix);
			for (int i = 0; i < 16; ++i)
			{
				pFloats[i] = static_cast<float>(pDoubles[i]);
			}

			return matResult;
		}

		using namespace ATG;

#define CONTENT_EXPORTER_TITLE CONTENT_EXPORTER_GLOBAL_TITLE " for FBX"

#define CONTENT_EXPORTER_MAJOR_VERSION 3
#define CONTENT_EXPORTER_MINOR_VERSION 0
#define CONTENT_EXPORTER_REVISION 0
#define MAKEVERSION(major, minor, rev) "" #major "." #minor "." #rev
#define CONTENT_EXPORTER_VERSION MAKEVERSION( CONTENT_EXPORTER_MAJOR_VERSION, CONTENT_EXPORTER_MINOR_VERSION, CONTENT_EXPORTER_REVISION )

#define CONTENT_EXPORTER_GLOBAL_TITLE "Samples Content Exporter"
#define CONTENT_EXPORTER_SETTINGS_TOKEN "SamplesContentExporter"
#define CONTENT_EXPORTER_VENDOR "Microsoft Advanced Technology Group"
#define CONTENT_EXPORTER_COPYRIGHT "Copyright (c) Microsoft Corporation.  All Rights Reserved."
#define CONTENT_EXPORTER_FILE_EXTENSION "xatg"
#define CONTENT_EXPORTER_FILE_FILTER "*." CONTENT_EXPORTER_FILE_EXTENSION
#define CONTENT_EXPORTER_FILE_FILTER_DESCRIPTION "XATG Samples Content File"
#define CONTENT_EXPORTER_BINARYFILE_EXTENSION "sdkmesh"
#define CONTENT_EXPORTER_BINARYFILE_FILTER "*." CONTENT_EXPORTER_BINARYFILE_EXTENSION
#define CONTENT_EXPORTER_BINARYFILE_FILTER_DESCRIPTION "SDK Mesh Binary File"

#ifdef _DEBUG
#define BUILD_FLAVOR "Debug"
#else
#define BUILD_FLAVOR "Release"
#endif

		class ConsoleOutListener : public ILogListener
		{
		protected:
			HANDLE m_hOut;
			WORD m_wDefaultConsoleTextAttributes;
			WORD m_wBackgroundAttributes;
		public:
			ConsoleOutListener()
			{
				m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
				CONSOLE_SCREEN_BUFFER_INFO csbi;
				GetConsoleScreenBufferInfo(m_hOut, &csbi);
				m_wDefaultConsoleTextAttributes = csbi.wAttributes;
				m_wBackgroundAttributes = m_wDefaultConsoleTextAttributes & 0x00F0;
			}
			virtual void LogMessage(const char* strMessage) override
			{
				puts(strMessage);
			}
			virtual void LogWarning(const char* strMessage) override
			{
				SetConsoleTextAttribute(m_hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | m_wBackgroundAttributes);
				LogMessage(strMessage);
				SetConsoleTextAttribute(m_hOut, m_wDefaultConsoleTextAttributes);
			}
			virtual void LogError(const char* strMessage) override
			{
				SetConsoleTextAttribute(m_hOut, FOREGROUND_RED | FOREGROUND_INTENSITY | m_wBackgroundAttributes);
				LogMessage(strMessage);
				SetConsoleTextAttribute(m_hOut, m_wDefaultConsoleTextAttributes);
			}
		};

		FBXTransformer::FBXTransformer()
			: m_fUnitScale(1.f)
			, m_isMaxConversion(false)
			, m_isFlipZ(true)
		{
		}

		void FBXTransformer::Initialize(fbxsdk::FbxScene* pScene)
		{
			ExportLog::LogMsg(4, "Identifying scene's coordinate system.");
			fbxsdk::FbxAxisSystem SceneAxisSystem = pScene->GetGlobalSettings().GetAxisSystem();

			// convert scene to Maya Y up coordinate system
			FbxAxisSystem::MayaYUp.ConvertScene(pScene);

			int nUpAxisSign = 0;
			fbxsdk::FbxAxisSystem::EUpVector UpVector = SceneAxisSystem.GetUpVector(nUpAxisSign);

			if (UpVector == FbxAxisSystem::eZAxis)
			{
				ExportLog::LogMsg(4, "Converting from Z-up axis system to Y-up axis system.");
				m_isMaxConversion = true;
			}
			else
			{
				m_isMaxConversion = false;
			}

			SetUnitScale(g_pScene->Settings().fExportScale);
			SetZFlip(g_pScene->Settings().bFlipZ);
		}

		void FBXTransformer::TransformMatrix(DirectX::XMFLOAT4X4* pDestMatrix, const DirectX::XMFLOAT4X4* pSrcMatrix) const
		{
			using namespace DirectX;

			XMFLOAT4X4 SrcMatrix;
			if (pSrcMatrix == pDestMatrix)
			{
				Memory::Copy(&SrcMatrix, sizeof(XMFLOAT4X4), pSrcMatrix);
				pSrcMatrix = &SrcMatrix;
			}
			Memory::Copy(pDestMatrix, sizeof(XMFLOAT4X4), pSrcMatrix);

			// What we're doing here is premultiplying by a left hand -> right hand matrix,
			// and then postmultiplying by a right hand -> left hand matrix.
			// The end result of those multiplications is that the third row and the third
			// column are negated (so element _33 is left alone).  So instead of actually
			// carrying out the multiplication, we just negate the 6 matrix elements.

			if (m_isFlipZ)
			{
				pDestMatrix->_13 = -pSrcMatrix->_13;
				pDestMatrix->_23 = -pSrcMatrix->_23;
				pDestMatrix->_43 = -pSrcMatrix->_43;

				pDestMatrix->_31 = -pSrcMatrix->_31;
				pDestMatrix->_32 = -pSrcMatrix->_32;
				pDestMatrix->_34 = -pSrcMatrix->_34;
			}

			// Apply the global unit scale to the translation components of the matrix.
			pDestMatrix->_41 *= m_fUnitScale;
			pDestMatrix->_42 *= m_fUnitScale;
			pDestMatrix->_43 *= m_fUnitScale;
		}

		void FBXTransformer::TransformPosition(DirectX::XMFLOAT3* pDestPosition, const DirectX::XMFLOAT3* pSrcPosition) const
		{
			using namespace DirectX;

			XMFLOAT3 SrcVector;
			if (pSrcPosition == pDestPosition)
			{
				SrcVector = *pSrcPosition;
				pSrcPosition = &SrcVector;
			}

			if (m_isMaxConversion)
			{
				pDestPosition->x = pSrcPosition->x * m_fUnitScale;
				pDestPosition->y = pSrcPosition->z * m_fUnitScale;
				pDestPosition->z = pSrcPosition->y * m_fUnitScale;
			}
			else
			{
				const float flipZ = m_isFlipZ ? -1.0f : 1.0f;

				pDestPosition->x = pSrcPosition->x * m_fUnitScale;
				pDestPosition->y = pSrcPosition->y * m_fUnitScale;
				pDestPosition->z = pSrcPosition->z * m_fUnitScale * flipZ;
			}
		}

		void FBXTransformer::TransformDirection(DirectX::XMFLOAT3* pDestDirection, const DirectX::XMFLOAT3* pSrcDirection) const
		{
			using namespace DirectX;

			XMFLOAT3 SrcVector;
			if (pSrcDirection == pDestDirection)
			{
				SrcVector = *pSrcDirection;
				pSrcDirection = &SrcVector;
			}

			if (m_isMaxConversion)
			{
				pDestDirection->x = pSrcDirection->x;
				pDestDirection->y = pSrcDirection->z;
				pDestDirection->z = pSrcDirection->y;
			}
			else
			{
				const float flipZ = m_isFlipZ ? -1.0f : 1.0f;

				pDestDirection->x = pSrcDirection->x;
				pDestDirection->y = pSrcDirection->y;
				pDestDirection->z = pSrcDirection->z * flipZ;
			}
		}

		float FBXTransformer::TransformLength(float fInputLength) const
		{
			return fInputLength * m_fUnitScale;
		}

		FBXImport::FBXImport()
			: m_pSDKManager(nullptr)
			, m_pImporter(nullptr)
			, m_pFBXScene(nullptr)
			, m_isBindPoseFixupRequired(false)
			, m_pConsoleOutListener(nullptr)
			, m_pDebugSpewListener(nullptr)
			, m_pManifest(nullptr)
		{
		}

		FBXImport::~FBXImport()
		{
			Release();
		}

		bool FBXImport::Initialize()
		{
			ExportLog::ClearListeners();

			SafeDelete(m_pConsoleOutListener);
			m_pConsoleOutListener = new ConsoleOutListener;

			ExportLog::AddListener(m_pConsoleOutListener);

#ifdef _DEBUG
			SafeDelete(m_pDebugSpewListener);
			m_pDebugSpewListener = new ATG::DebugSpewListener;

			ExportLog::AddListener(m_pDebugSpewListener);
#endif

			ExportLog::SetLogLevel(1);
			ExportLog::EnableLogging(true);

#ifdef _DEBUG
			std::string strExporterName = String::Format("%s version %d.%d.%d (DEBUG)", CONTENT_EXPORTER_TITLE, CONTENT_EXPORTER_MAJOR_VERSION, CONTENT_EXPORTER_MINOR_VERSION, CONTENT_EXPORTER_REVISION);
#else
			std::string strExporterName = String::Format("%s version %d.%d.%d", CONTENT_EXPORTER_TITLE, CONTENT_EXPORTER_MAJOR_VERSION, CONTENT_EXPORTER_MINOR_VERSION, CONTENT_EXPORTER_REVISION);
#endif

			ExportLog::LogMsg(0, "----------------------------------------------------------");
			ExportLog::LogMsg(0, strExporterName.c_str());
			ExportLog::LogMsg(0, CONTENT_EXPORTER_VENDOR);
			ExportLog::LogMsg(0, CONTENT_EXPORTER_COPYRIGHT);
			ExportLog::LogMsg(0, "----------------------------------------------------------");

			g_pScene = new ATG::ExportScene;
			g_pScene->SetDCCTransformer(&m_fbxTransformer);

			ExportLog::LogMsg(9, "Microsoft C++ compiler version %d", _MSC_VER);
			ExportLog::LogMsg(9, "FBX SDK version %s", FBXSDK_VERSION_STRING);

			ExportLog::LogMsg(9, "DirectXMath version %d", DIRECTX_MATH_VERSION);

			ExportCoreSettings& InitialSettings = g_pScene->Settings();
			InitialSettings = ATG::ExportCoreSettings();

			if (InitialSettings.bForceIndex32Format && (InitialSettings.dwFeatureLevel <= D3D_FEATURE_LEVEL_9_1))
			{
				ExportLog::LogWarning("32-bit index buffers not supported on Feature Level 9.1");
			}

			if (InitialSettings.bCompressVertexData && 
				(InitialSettings.dwNormalCompressedType == D3DDECLTYPE_DXGI_R10G10B10A2_UNORM ||
					InitialSettings.dwNormalCompressedType == D3DDECLTYPE_DXGI_R11G11B10_FLOAT ||
					InitialSettings.dwNormalCompressedType == D3DDECLTYPE_DXGI_R8G8B8A8_SNORM) &&
				(InitialSettings.dwFeatureLevel < D3D_FEATURE_LEVEL_10_0))
			{
				ExportLog::LogWarning("R11G11B10_FLOAT/10:10:10:2/R8G8B8A8 Signed in vertex normals not supported on Feature Level 9.x");
			}

			if (InitialSettings.bCompressVertexData &&
				(InitialSettings.dwNormalCompressedType == D3DDECLTYPE_XBOX_R10G10B10_SNORM_A2_UNORM) &&
				(InitialSettings.dwFeatureLevel < D3D_FEATURE_LEVEL_11_1))
			{
				ExportLog::LogWarning("10:10:10 Signed A2 only supported on Xbox One");
			}

			if (InitialSettings.bExportColors &&
				(InitialSettings.dwVertexColorType == D3DDECLTYPE_DXGI_R10G10B10A2_UNORM || InitialSettings.dwVertexColorType == D3DDECLTYPE_DXGI_R11G11B10_FLOAT) &&
				(InitialSettings.dwFeatureLevel < D3D_FEATURE_LEVEL_10_0))
			{
				ExportLog::LogWarning("R11G11B10_FLOAT/10:10:10:2 vertex colors not supported on Feature Level 9.x");
			}

			if (m_pSDKManager == nullptr)
			{
				m_pSDKManager = fbxsdk::FbxManager::Create();
				if (m_pSDKManager == nullptr)
					return false;

				fbxsdk::FbxIOSettings* ios = fbxsdk::FbxIOSettings::Create(m_pSDKManager, IOSROOT);
				if (ios == nullptr)
					return false;

				m_pSDKManager->SetIOSettings(ios);
			}

			if (m_pImporter == nullptr)
			{
				m_pImporter = fbxsdk::FbxImporter::Create(m_pSDKManager, "");
				if (m_pImporter == nullptr)
					return false;
			}

			if (m_pFBXScene != nullptr)
			{
				m_pFBXScene->Clear();
			}

			m_pFBXScene = fbxsdk::FbxScene::Create(m_pSDKManager, "");
			if (m_pFBXScene == nullptr)
				return false;

			m_pManifest = new ATG::ExportManifest;

			m_BindPoseMap.clear();
			m_umapMotionOffsetMarix.clear();

			return true;
		}

		void FBXImport::Release()
		{
			SafeDelete(m_pManifest);
			SafeDelete(m_pConsoleOutListener);
			SafeDelete(m_pDebugSpewListener);
			SafeDelete(g_pScene);

			if (m_pFBXScene != nullptr)
			{
				m_pFBXScene->Clear();
				m_pFBXScene = nullptr;
			}

			if (m_pImporter != nullptr)
			{
				m_pImporter->Destroy();
				m_pImporter = nullptr;
			}

			if (m_pSDKManager != nullptr)
			{
				m_pSDKManager->Destroy();
				m_pSDKManager = nullptr;
			}
		}

		bool FBXImport::LoadModel(IModel* pModel, const char* strFilePath, float fScale)
		{
			if (pModel == nullptr || strFilePath == nullptr)
				return false;

			Performance::Counter counter;
			counter.Start();

			if (Initialize() == false)
			{
				ExportLog::LogError("Failed to initialize FBX");
				return false;
			}
			ExportLog::LogMsg(4, "FBX has been initialized.");

			ModelSettings();

			g_pScene->Settings().fExportScale = fScale;

			g_pScene->Statistics().StartExport();
			g_pScene->Statistics().StartSceneParse();

			if (ImportFile(strFilePath) == false)
			{
				ExportLog::LogError("Could not load file \"%s\".", strFilePath);
				return false;
			}

			g_pScene->Statistics().StartSave();

			bool isExportMaterials = g_pScene->Settings().bExportMaterials;

			ExportTextureConverter::ProcessScene(g_pScene, m_pManifest, "", true);
			WriteModel(pModel, m_umapMotionOffsetMarix);
			if (isExportMaterials)
			{
				ExportTextureConverter::PerformTextureFileOperations(m_pManifest);
			}

			g_pScene->Statistics().EndExport();
			g_pScene->Statistics().FinalReport();

			bool isFoundErrors = false;
			if (ExportLog::GenerateLogReport())
			{
				isFoundErrors = true;
			}

			Release();
			ExportLog::ResetCounters();

			counter.End();

			PRINT_LOG("FBX Import, Model[%s] : %lf", strFilePath, counter.Count());

			return true;
		}
		
		bool FBXImport::LoadMotion(IMotion* pMotion, const char* strFilePath, float fScale)
		{
			if (pMotion == nullptr || strFilePath == nullptr)
				return false;

			Performance::Counter counter;
			counter.Start();

			if (Initialize() == false)
			{
				ExportLog::LogError("Failed to initialize FBX");
				return false;
			}
			ExportLog::LogMsg(4, "FBX has been initialized.");

			MotionSettings();
			
			g_pScene->Settings().fExportScale = fScale;
			
			g_pScene->Statistics().StartExport();
			g_pScene->Statistics().StartSceneParse();
			
			if (ImportFile(strFilePath) == false)
			{
				ExportLog::LogError("Could not load file \"%s\".", strFilePath);
				return false;
			}
			
			g_pScene->Statistics().StartSave();
			
			WriteMotion(pMotion);
			
			g_pScene->Statistics().EndExport();
			g_pScene->Statistics().FinalReport();
			
			bool isFoundErrors = false;
			if (ExportLog::GenerateLogReport())
			{
				isFoundErrors = true;
			}
			
			Release();
			ExportLog::ResetCounters();

			counter.End();

			PRINT_LOG("FBX Import, Motion[%s] : %lf", strFilePath, counter.Count());

			return true;
		}

		void FBXImport::ModelSettings()
		{
			g_pScene->Settings().bExportAnimations = false;
			g_pScene->Settings().bExportCameras = false;
			g_pScene->Settings().bExportLights = false;
			g_pScene->Settings().bExportMaterials = true;
			g_pScene->Settings().bExportMeshes = true;
			g_pScene->Settings().bExportScene = true;
			g_pScene->Settings().bForceIndex32Format = true;
			g_pScene->Settings().bOptimizeVCache = true;
		}

		void FBXImport::MotionSettings()
		{
			g_pScene->Settings().bExportAnimations = true;
			g_pScene->Settings().bCompressVertexData = false;
			g_pScene->Settings().bComputeVertexTangentSpace = false;
			g_pScene->Settings().bExportBinormal = false;
			g_pScene->Settings().bExportCameras = false;
			g_pScene->Settings().bExportLights = false;
			g_pScene->Settings().bExportMaterials = false;
			g_pScene->Settings().bExportNormals = false;
			g_pScene->Settings().bExportColors = false;
			g_pScene->Settings().bExportSkinWeights = false;
			g_pScene->Settings().bForceIndex32Format = false;
			g_pScene->Settings().bExportMeshes = false;
			g_pScene->Settings().bExportScene = false;
			g_pScene->Settings().iMaxUVSetCount = 0;
			g_pScene->Settings().bRenameAnimationsToFileName = true;
			g_pScene->Settings().bOptimizeAnimations = true;
		}

		bool FBXImport::ImportFile(const char* strFileName)
		{
			assert(m_pSDKManager != nullptr);
			assert(m_pImporter != nullptr);
			assert(m_pFBXScene != nullptr);

			g_pScene->Information().ExporterName = "EsatEngine";
			int nMajorVersion = 0;
			int nMinorVersion = 0;
			int nRevision = 0;
			m_pSDKManager->GetFileFormatVersion(nMajorVersion, nMinorVersion, nRevision);

			std::string strTemp = String::Format("FBX SDK %d.%d.%d", nMajorVersion, nMinorVersion, nRevision);
			g_pScene->Information().DCCNameAndVersion = strTemp.c_str();

			ExportLog::LogMsg(2, "Compiled against %s", strTemp.c_str());
			ExportLog::LogMsg(1, "Loading FBX file \"%s\"...", strFileName);

			int nFileFormat = -1;
			bool isSuccess = m_pImporter->Initialize(strFileName, nFileFormat, m_pSDKManager->GetIOSettings());

			if (isSuccess == false)
			{
				ExportLog::LogError("Could not initialize FBX importer.");
				return false;
			}

			isSuccess = m_pImporter->Import(m_pFBXScene);

			if (isSuccess == false)
			{
				ExportLog::LogError("Could not load FBX file \"%s\".", strFileName);
				return false;
			}

			ExportLog::LogMsg(1, "FBX file \"%s\" was successfully loaded.", strFileName);
			m_pImporter->GetFileVersion(nMajorVersion, nMinorVersion, nRevision);
			ExportLog::LogMsg(2, "FBX file version: %d.%d.%d", nMajorVersion, nMinorVersion, nRevision);

			ExportLog::LogMsg(2, "Parsing scene.");

			FBXTransformer* pTransformer = reinterpret_cast<FBXTransformer*>(g_pScene->GetDCCTransformer());
			pTransformer->Initialize(m_pFBXScene);

			SetBindPose();
			m_isBindPoseFixupRequired = false;

			assert(m_pFBXScene->GetRootNode() != nullptr);
			Math::Matrix matIdentity;
			ParseNode(m_pFBXScene->GetRootNode(), g_pScene, matIdentity);

			if (m_isBindPoseFixupRequired)
			{
				ExportLog::LogMsg(2, "Fixing up frames with updated bind pose.");
				FixupNode(g_pScene, matIdentity);
			}

			if (g_pScene->Settings().bExportAnimations)
			{
				ParseAnimation(m_pFBXScene);
				if (g_pScene->Settings().bRenameAnimationsToFileName)
				{
					std::string AnimName = File::GetFileNameWithoutExtension(strFileName);

					size_t nAnimCount = g_pScene->GetAnimationCount();
					for (size_t i = 0; i < nAnimCount; ++i)
					{
						std::string strCurrentAnimName;
						if (i > 0)
						{
							strCurrentAnimName = String::Format("%s%Iu", AnimName.c_str(), i);
						}
						else
						{
							strCurrentAnimName = AnimName;
						}
						ExportAnimation* pAnim = g_pScene->GetAnimation(i);
						ExportLog::LogMsg(4, "Renaming animation \"%s\" to \"%s\".", pAnim->GetName().SafeString(), strCurrentAnimName.c_str());
						pAnim->SetName(strCurrentAnimName.c_str());
					}
				}
			}

			return true;
		}

		void FBXImport::SetBindPose()
		{
			assert(m_pFBXScene != nullptr);

			std::vector<fbxsdk::FbxPose*> vecBindPoses;

			int nPoseCount = m_pFBXScene->GetPoseCount();
			for (int i = 0; i < nPoseCount; ++i)
			{
				fbxsdk::FbxPose* pPose = m_pFBXScene->GetPose(i);
				int nNodeCount = pPose->GetCount();
				ExportLog::LogMsg(4, "Found %spose: \"%s\" with %d nodes", pPose->IsBindPose() ? "bind " : "", pPose->GetName(), nNodeCount);
				for (int j = 0; j < nNodeCount; ++j)
				{
					fbxsdk::FbxNode* pPoseNode = pPose->GetNode(j);
					ExportLog::LogMsg(5, "Pose node %d: %s", j, pPoseNode->GetName());
				}

				if (pPose->IsBindPose())
				{
					vecBindPoses.push_back(pPose);
				}
			}

			if (vecBindPoses.empty())
			{
				if (g_pScene->Settings().bExportAnimations)
				{
					ExportLog::LogWarning("No valid bind pose found; will export scene using the default pose.");
				}
				return;
			}

			size_t nBindPoseCount = vecBindPoses.size();
			for (size_t i = 0; i < nBindPoseCount; ++i)
			{
				FbxPose* pPose = vecBindPoses[i];
				int nNodeCount = pPose->GetCount();
				for (int j = 0; j < nNodeCount; ++j)
				{
					fbxsdk::FbxNode* pNode = pPose->GetNode(j);
					const fbxsdk::FbxMatrix& matNode = pPose->GetMatrix(j);

					auto iter = m_BindPoseMap.find(pNode->GetName());
					if (iter != m_BindPoseMap.end())
					{
						const fbxsdk::FbxMatrix& matExisting = iter->second;
						if (matExisting != matNode)
						{
							ExportLog::LogWarning("Node \"%s\" found in more than one bind pose, with conflicting transforms.", pNode->GetName());
						}
					}

					m_BindPoseMap[pNode->GetName()] = matNode;
				}
			}

			ExportLog::LogMsg(3, "Created bind pose map with %Iu nodes.", m_BindPoseMap.size());
		}

		Math::Matrix FBXImport::ParseTransform(fbxsdk::FbxNode* pNode, ATG::ExportFrame* pFrame, const Math::Matrix& matParentWorld, const bool isWarnings)
		{
			Math::Matrix matWorld;
			Math::Matrix matLocal;
			bool isProcessDefaultTransform = true;

			if (m_BindPoseMap.empty() == false)
			{
				auto iter = m_BindPoseMap.find(pNode->GetName());
				if (iter != m_BindPoseMap.end())
				{
					const FbxMatrix& PoseMatrix = iter->second;
					matWorld = ConvertMatrix(PoseMatrix);

					matLocal = matWorld * matParentWorld.Invert();
					isProcessDefaultTransform = false;
				}
			}

			if (isProcessDefaultTransform == true)
			{
				FbxVector4 Translation;
				if (pNode->LclTranslation.IsValid())
				{
					Translation = pNode->LclTranslation.Get();
				}

				FbxVector4 Rotation;
				if (pNode->LclRotation.IsValid())
				{
					Rotation = pNode->LclRotation.Get();
				}

				FbxVector4 Scale;
				if (pNode->LclScaling.IsValid())
				{
					Scale = pNode->LclScaling.Get();
				}

				FbxMatrix matTransform(Translation, Rotation, Scale);
				matLocal = ConvertMatrix(matTransform);
				matWorld = matParentWorld * matLocal;
			}

			pFrame->Transform().Initialize(*reinterpret_cast<DirectX::XMFLOAT4X4*>(&matLocal));

			const Math::Vector3& Scale = *reinterpret_cast<const Math::Vector3*>(&pFrame->Transform().Scale());
			if (isWarnings == true &&
				(Math::IsEqual(Scale.x, Scale.y) == false ||
					Math::IsEqual(Scale.y, Scale.z) == false ||
					Math::IsEqual(Scale.x, Scale.z) == false))
			{
				ExportLog::LogWarning("Non-uniform scale found on node \"%s\".", pFrame->GetName().SafeString());
			}

			const ExportTransform& Transform = pFrame->Transform();
			ExportLog::LogMsg(5, "Node transform for \"%s\": Translation <%0.3f %0.3f %0.3f> Rotation <%0.3f %0.3f %0.3f %0.3f> Scale <%0.3f %0.3f %0.3f>",
				pFrame->GetName().SafeString(),
				Transform.Position().x,
				Transform.Position().y,
				Transform.Position().z,
				Transform.Orientation().x,
				Transform.Orientation().y,
				Transform.Orientation().z,
				Transform.Orientation().w,
				Transform.Scale().x,
				Transform.Scale().y,
				Transform.Scale().z);

			return matWorld;
		}

		void FBXImport::ParseNode(FbxNode* pNode, ATG::ExportFrame* pParentFrame, const Math::Matrix& matParentWorld)
		{
			ExportLog::LogMsg(2, "Parsing node \"%s\".", pNode->GetName());

			ExportFrame* pFrame = new ExportFrame(pNode->GetName());
			pFrame->SetDCCObject(pNode);

			Math::Matrix matWorld = ParseTransform(pNode, pFrame, matParentWorld);
			pParentFrame->AddChild(pFrame);

			if (pNode->GetSubdiv())
			{
				ParseSubDiv(pNode, pNode->GetSubdiv(), pFrame);
			}
			else if (pNode->GetMesh())
			{
				ParseMesh(pNode, pNode->GetMesh(), pFrame, false);
			}

			ParseCamera(pNode->GetCamera(), pFrame);
			ParseLight(pNode->GetLight(), pFrame);

			uint32_t nChildCount = pNode->GetChildCount();
			for (uint32_t i = 0; i < nChildCount; ++i)
			{
				ParseNode(pNode->GetChild(i), pFrame, matWorld);
			}
		}

		void FBXImport::ParseCamera(FbxCamera* pFbxCamera, ATG::ExportFrame* pParentFrame)
		{
			if (pFbxCamera == nullptr || g_pScene->Settings().bExportCameras == false)
				return;

			ExportLog::LogMsg(2, "Parsing camera \"%s\".", pFbxCamera->GetName());

			ExportCamera* pCamera = new ExportCamera(pFbxCamera->GetName());
			pCamera->SetDCCObject(pFbxCamera);

			pCamera->fNearClip = static_cast<float>(pFbxCamera->NearPlane.Get());
			pCamera->fFarClip = static_cast<float>(pFbxCamera->FarPlane.Get());
			pCamera->fFieldOfView = static_cast<float>(pFbxCamera->FieldOfView.Get());
			pCamera->fFocalLength = static_cast<float>(pFbxCamera->FocalLength.Get());

			pParentFrame->AddCamera(pCamera);
		}

		void FBXImport::ParseLight(FbxLight* pFbxLight, ATG::ExportFrame* pParentFrame)
		{
			if (pFbxLight == nullptr || g_pScene->Settings().bExportLights == false)
				return;

			ExportLog::LogMsg(2, "Parsing light \"%s\".", pFbxLight->GetName());

			switch (pFbxLight->LightType.Get())
			{
			case FbxLight::ePoint:
			case FbxLight::eSpot:
			case FbxLight::eDirectional:
				break;
			case FbxLight::eArea:
			case FbxLight::eVolume:
				ExportLog::LogWarning("Ignores area and volume lights");
				return;
			default:
				ExportLog::LogWarning("Could not determine light type, ignored.");
				return;
			}

			ExportLight* pLight = new ExportLight(pFbxLight->GetName());
			pLight->SetDCCObject(pFbxLight);
			pParentFrame->AddLight(pLight);

			fbxsdk::FbxDouble3 colorRGB = pFbxLight->Color.Get();
			float fIntensity = static_cast<float>(pFbxLight->Intensity.Get());
			fIntensity *= 0.01f;

			pLight->Color.x = static_cast<float>(colorRGB[0]) * fIntensity;
			pLight->Color.y = static_cast<float>(colorRGB[1]) * fIntensity;
			pLight->Color.z = static_cast<float>(colorRGB[2]) * fIntensity;
			pLight->Color.w = 1.f;

			switch (pFbxLight->DecayType.Get())
			{
			case FbxLight::eNone:
				pLight->Falloff = ExportLight::LF_NONE;
				pLight->fRange = 20.0f;
				break;
			case FbxLight::eLinear:
				pLight->Falloff = ExportLight::LF_LINEAR;
				pLight->fRange = 4.0f * fIntensity;
				break;
			case FbxLight::eQuadratic:
			case FbxLight::eCubic:
				pLight->Falloff = ExportLight::LF_SQUARED;
				pLight->fRange = 2.0f * sqrtf(fIntensity);
				break;
			default:
				ExportLog::LogWarning("Could not determine light decay type, using None");
				pLight->Falloff = ExportLight::LF_NONE;
				pLight->fRange = 20.0f;
				break;
			}

			pLight->fRange *= g_pScene->Settings().fLightRangeScale;

			ExportLog::LogMsg(4, "Light color (multiplied by intensity): <%0.2f %0.2f %0.2f> intensity: %0.2f falloff: %0.2f", pLight->Color.x, pLight->Color.y, pLight->Color.z, fIntensity, pLight->fRange);

			switch (pFbxLight->LightType.Get())
			{
			case FbxLight::ePoint:
				pLight->Type = ExportLight::LT_POINT;
				break;
			case FbxLight::eSpot:
				pLight->Type = ExportLight::LT_SPOT;
				pLight->fOuterAngle = (float)pFbxLight->OuterAngle.Get();
				pLight->fInnerAngle = (float)pFbxLight->InnerAngle.Get();
				pLight->SpotFalloff = pLight->Falloff;
				break;
			case FbxLight::eDirectional:
				pLight->Type = ExportLight::LT_DIRECTIONAL;
				break;
			}
		}

		void FBXImport::FixupNode(ATG::ExportFrame* pFrame, const Math::Matrix& matParentWorld)
		{
			fbxsdk::FbxNode* pNode = reinterpret_cast<fbxsdk::FbxNode*>(pFrame->GetDCCObject());

			Math::Matrix matWorld;
			if (pNode != nullptr)
			{
				ExportLog::LogMsg(4, "Fixing up frame \"%s\".", pFrame->GetName().SafeString());
				matWorld = ParseTransform(pNode, pFrame, matParentWorld, false);
			}
			else
			{
				matWorld = matParentWorld;
			}

			size_t nChildCount = pFrame->GetChildCount();
			for (size_t i = 0; i < nChildCount; ++i)
			{
				FixupNode(pFrame->GetChildByIndex(i), matWorld);
			}
		}

		void FBXImport::ParseAnimNode(FbxNode* pNode, std::vector<AnimationScanNode>& scanlist, DWORD dwFlags, int nParentIndex, bool isIncludeNode)
		{
			int nCurrentIndex = nParentIndex;

			if (isIncludeNode == false)
			{
				const char* strNodeName = pNode->GetName();
				if (_stricmp(strNodeName, g_pScene->Settings().strAnimationRootNodeName) == 0)
				{
					isIncludeNode = true;
				}
			}

			if (isIncludeNode == true)
			{
				nCurrentIndex = static_cast<int>(scanlist.size());

				// add node to anim list
				AnimationScanNode asn;
				asn.nParentIndex = nParentIndex;
				asn.pNode = pNode;
				asn.dwFlags = dwFlags;
				scanlist.push_back(asn);
			}

			int nChildCount = pNode->GetChildCount();
			for (int i = 0; i < nChildCount; ++i)
			{
				ParseAnimNode(pNode->GetChild(i), scanlist, dwFlags, nCurrentIndex, isIncludeNode);
			}
		}

		void FBXImport::AddKey(AnimationScanNode& asn, const AnimationScanNode* pParent, const FbxAMatrix& matFBXGlobal, float fTime)
		{
			Math::Matrix matGlobal = ConvertMatrix(matFBXGlobal);

			asn.matGlobal = matGlobal;

			Math::Matrix matLocal = matGlobal;
			if (pParent != nullptr)
			{
				matLocal = matGlobal * pParent->matGlobal.Invert();
			}

			Math::Matrix matLocalFinal = matLocal;
			g_pScene->GetDCCTransformer()->TransformMatrix(reinterpret_cast<DirectX::XMFLOAT4X4*>(&matLocalFinal), reinterpret_cast<const DirectX::XMFLOAT4X4*>(&matLocal));

			Math::Vector3 f3Scale;
			Math::Quaternion quatRotation;
			Math::Vector3 f3Translation;
			matLocalFinal.Decompose(f3Scale, quatRotation, f3Translation);

			asn.pTrack->TransformTrack.AddKey(fTime, 
				*reinterpret_cast<const DirectX::XMFLOAT3*>(&f3Translation), 
				*reinterpret_cast<const DirectX::XMFLOAT4*>(&quatRotation),
				*reinterpret_cast<const DirectX::XMFLOAT3*>(&f3Scale));
		}

		void FBXImport::CaptureAnimation(std::vector<AnimationScanNode>& scanlist, ATG::ExportAnimation* pAnim, FbxScene* pFbxScene)
		{
			const float fDeltaTime = pAnim->fSourceSamplingInterval;
			const float fStartTime = pAnim->fStartTime;
			const float fEndTime = pAnim->fEndTime;
			float fCurrentTime = fStartTime;

			const size_t nNodeCount = scanlist.size();

			ExportLog::LogMsg(2, "Capturing animation data from %Iu nodes, from time %0.3f to %0.3f, at an interval of %0.3f seconds.", nNodeCount, fStartTime, fEndTime, fDeltaTime);

			while (fCurrentTime <= fEndTime)
			{
				FbxTime CurrentTime;
				CurrentTime.SetSecondDouble(fCurrentTime);
				for (size_t i = 0; i < nNodeCount; ++i)
				{
					AnimationScanNode& asn = scanlist[i];

#if (FBXSDK_VERSION_MAJOR > 2014 || ((FBXSDK_VERSION_MAJOR==2014) && (FBXSDK_VERSION_MINOR>1) ) )
					auto pAnimEvaluator = pFbxScene->GetAnimationEvaluator();
#else
					auto pAnimEvaluator = pFbxScene->GetEvaluator();
#endif

					fbxsdk::FbxAMatrix matGlobal = pAnimEvaluator->GetNodeGlobalTransform(asn.pNode, CurrentTime);
					AnimationScanNode* pParent = nullptr;
					if (asn.nParentIndex >= 0)
					{
						pParent = &scanlist[asn.nParentIndex];
					}

					AddKey(asn, pParent, matGlobal, fCurrentTime - fStartTime);
				}

				fCurrentTime += fDeltaTime;
			}
		}

		void FBXImport::ParseAnimStack(FbxScene* pFbxScene, FbxString* strAnimStackName)
		{
			// TODO - Ignore "Default"? FBXSDK_TAKENODE_DEFAULT_NAME

			fbxsdk::FbxAnimStack* curAnimStack = pFbxScene->FindMember<fbxsdk::FbxAnimStack>(strAnimStackName->Buffer());
			if (curAnimStack == nullptr)
				return;

#if (FBXSDK_VERSION_MAJOR > 2014 || ((FBXSDK_VERSION_MAJOR==2014) && (FBXSDK_VERSION_MINOR>1) ) )
			pFbxScene->GetAnimationEvaluator()->Reset();
#else
			pFbxScene->GetEvaluator()->SetContext(curAnimStack);
#endif

			fbxsdk::FbxTakeInfo* pTakeInfo = pFbxScene->GetTakeInfo(*strAnimStackName);

			ExportLog::LogMsg(2, "Parsing animation \"%s\"", strAnimStackName->Buffer());

			ExportAnimation* pAnim = new ExportAnimation;
			pAnim->SetName(strAnimStackName->Buffer());
			pAnim->SetDCCObject(pTakeInfo);
			g_pScene->AddAnimation(pAnim);

			FbxTime FrameTime;
			FrameTime.SetTime(0, 0, 0, 1, 0, pFbxScene->GetGlobalSettings().GetTimeMode());

			float fFrameTime = static_cast<float>(FrameTime.GetSecondDouble());
			float fSampleTime = fFrameTime / static_cast<float>(g_pScene->Settings().iAnimSampleCountPerFrame);
			assert(fSampleTime > 0);

			float fStartTime, fEndTime;
			if (pTakeInfo != nullptr)
			{
				fStartTime = static_cast<float>(pTakeInfo->mLocalTimeSpan.GetStart().GetSecondDouble());
				fEndTime = static_cast<float>(pTakeInfo->mLocalTimeSpan.GetStop().GetSecondDouble());
			}
			else
			{
				fbxsdk::FbxTimeSpan tlTimeSpan;
				pFbxScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(tlTimeSpan);

				fStartTime = static_cast<float>(tlTimeSpan.GetStart().GetSecondDouble());
				fEndTime = static_cast<float>(tlTimeSpan.GetStop().GetSecondDouble());

				ExportLog::LogWarning("Animation take \"%s\" has no takeinfo; using defaults.", pAnim->GetName().SafeString());
			}

			pAnim->fStartTime = fStartTime;
			pAnim->fEndTime = fEndTime;
			pAnim->fSourceFrameInterval = fFrameTime;
			pAnim->fSourceSamplingInterval = fSampleTime;

			bool isIncludeAllNodes = true;
			if (strlen(g_pScene->Settings().strAnimationRootNodeName) > 0)
			{
				isIncludeAllNodes = false;
			}

			std::vector<AnimationScanNode> scanlist;
			ParseAnimNode(pFbxScene->GetRootNode(), scanlist, 0, -1, isIncludeAllNodes);

			size_t nTrackCount = scanlist.size();
			for (size_t i = 0; i < nTrackCount; ++i)
			{
				const char* strTrackName = scanlist[i].pNode->GetName();
				ExportLog::LogMsg(4, "Track: %s", strTrackName);
				ExportAnimationTrack* pTrack = new ExportAnimationTrack;
				pTrack->SetName(strTrackName);
				pTrack->TransformTrack.pSourceFrame = g_pScene->FindFrameByDCCObject(scanlist[i].pNode);
				pAnim->AddTrack(pTrack);
				scanlist[i].pTrack = pTrack;
			}

			CaptureAnimation(scanlist, pAnim, pFbxScene);

			pAnim->Optimize();
		}

		void FBXImport::ParseAnimation(fbxsdk::FbxScene* pFbxScene)
		{
			assert(pFbxScene != nullptr);

			// set animation quality settings
			ExportAnimation::SetAnimationExportQuality(g_pScene->Settings().iAnimPositionExportQuality, g_pScene->Settings().iAnimOrientationExportQuality, 50);

			FbxArray<FbxString*> AnimStackNameArray;
			pFbxScene->FillAnimStackNameArray(AnimStackNameArray);

			uint32_t nAnimStackCount = static_cast<uint32_t>(AnimStackNameArray.GetCount());
			for (uint32_t i = 0; i < nAnimStackCount; ++i)
			{
				ParseAnimStack(pFbxScene, AnimStackNameArray.GetAt(i));
			}
		}

		void FBXImport::FixupGenericMaterial(ATG::ExportMaterial* pMaterial)
		{
			ExportMaterialParameter OutputParam;
			OutputParam.ParamType = MPT_TEXTURE2D;
			OutputParam.bInstanceParam = true;

			ExportMaterialParameter* pParam = pMaterial->FindParameter("DiffuseTexture");
			if (pParam == nullptr)
			{
				OutputParam.ValueString = ExportMaterial::GetDefaultDiffuseMapTextureName();
				if (*OutputParam.ValueString)
				{
					ExportLog::LogMsg(2, "Material \"%s\" has no diffuse texture.  Assigning a default diffuse texture.", pMaterial->GetName().SafeString());
					OutputParam.Name = "DiffuseTexture";
					pMaterial->AddParameter(OutputParam);
				}
			}
			else if (g_ExportCoreSettings.bMaterialColors)
			{
				auto pColor = pMaterial->FindParameter("DiffuseColor");

				if (pColor && pColor->ValueFloat[0] == 0 && pColor->ValueFloat[1] == 0 && pColor->ValueFloat[2] == 0)
				{
					ExportLog::LogWarning("Material \"%s\" has a black DiffuseColor which will modulate a DiffuseTexture to black. Set a DiffuseColor or use -materialcolors-.", pMaterial->GetName().SafeString());
				}
			}

			pParam = pMaterial->FindParameter("NormalMapTexture");
			if (pParam == nullptr)
			{
				OutputParam.ValueString = ExportMaterial::GetDefaultNormalMapTextureName();
				if (*OutputParam.ValueString)
				{
					ExportLog::LogMsg(2, "Material \"%s\" has no normal map texture.  Assigning a default normal map texture.", pMaterial->GetName().SafeString());
					OutputParam.Name = "NormalMapTexture";
					pMaterial->AddParameter(OutputParam);
				}
			}

			pParam = pMaterial->FindParameter("SpecularMapTexture");
			if (pParam == nullptr)
			{
				if (g_ExportCoreSettings.bUseEmissiveTexture)
				{
					pParam = pMaterial->FindParameter("EmissiveMapTexture");
					if (pParam)
					{
						// Copy emissive to specular (SDKMESH's material doesn't have an emissive texture slot)
						ExportLog::LogMsg(4, "EmissiveMapTexture encoded as SpecularMapTexture in material \"%s\".", pMaterial->GetName().SafeString());
						OutputParam.Name = "SpecularMapTexture";
						OutputParam.ValueString = pParam->ValueString;
						pMaterial->AddParameter(OutputParam);
					}
				}

				if (pParam == nullptr)
				{
					OutputParam.ValueString = ExportMaterial::GetDefaultSpecularMapTextureName();
					if (*OutputParam.ValueString)
					{
						ExportLog::LogMsg(2, "Material \"%s\" has no specular map texture.  Assigning a default specular map texture.", pMaterial->GetName().SafeString());
						OutputParam.Name = "SpecularMapTexture";
						pMaterial->AddParameter(OutputParam);
					}
				}
			}

			ATG::MaterialParameterList* pParamList = pMaterial->GetParameterList();
			std::stable_sort(pParamList->begin(), pParamList->end(), [](ATG::ExportMaterialParameter A, ATG::ExportMaterialParameter B)
			{
				if (A.ParamType == MPT_TEXTURE2D && B.ParamType != MPT_TEXTURE2D)
					return true;

				return false;
			});
		}

		void FBXImport::AddTextureParameter(ATG::ExportMaterial* pMaterial, const char* strParamName, uint32_t nIndex, const char* strFileName, DWORD dwFlags)
		{
			ExportMaterialParameter OutputParam;
			if (nIndex == 0)
			{
				OutputParam.Name = strParamName;
			}
			else
			{
				OutputParam.Name = String::Format("%s%u", strParamName, nIndex).c_str();
			}

			ExportLog::LogMsg(4, "Material parameter \"%s\" = \"%s\"", OutputParam.Name.SafeString(), strFileName);
			OutputParam.ValueString = strFileName;
			OutputParam.ParamType = MPT_TEXTURE2D;
			OutputParam.bInstanceParam = true;
			OutputParam.Flags = dwFlags;
			pMaterial->AddParameter(OutputParam);
		}

		bool FBXImport::ExtractTextures(fbxsdk::FbxProperty Property, const char* strParameterName, ATG::ExportMaterial* pMaterial, DWORD dwFlags)
		{
			auto CheckUVSettings = [](fbxsdk::FbxFileTexture* texture, ATG::ExportMaterial* pMaterial)
			{
				if (texture->GetSwapUV())
				{
					ExportLog::LogWarning("Material \"%s\" has swapped UVs which are not exported as such", pMaterial->GetName().SafeString());
				}

				if (texture->GetWrapModeU() != FbxTexture::eRepeat ||
					texture->GetWrapModeV() != FbxTexture::eRepeat)
				{
					ExportLog::LogWarning("Material \"%s\" has set to clamp wrap U/V mode which is not exported", pMaterial->GetName().SafeString());
				}

				fbxsdk::FbxVector2& uvScaling = texture->GetUVScaling();
				fbxsdk::FbxVector2& uvTrans = texture->GetUVTranslation();
				if (uvScaling[0] != 1.0 ||
					uvScaling[1] != 1.0 ||
					uvTrans[0] != 0 ||
					uvTrans[1] != 0)
				{
					ExportLog::LogWarning("Material \"%s\" has UV transforms which are not exported", pMaterial->GetName().SafeString());
				}
			};

			bool bResult = false;
			int nLayeredTextureCount = Property.GetSrcObjectCount<FbxLayeredTexture>();
			if (nLayeredTextureCount > 0)
			{
				uint32_t nTextureIndex = 0;
				for (int i = 0; i < nLayeredTextureCount; ++i)
				{
					auto pFbxLayeredTexture = FbxCast<FbxLayeredTexture>(Property.GetSrcObject<FbxLayeredTexture>(i));
					int nTextureCount = pFbxLayeredTexture->GetSrcObjectCount<FbxFileTexture>();
					for (int j = 0; j < nTextureCount; ++j)
					{
						fbxsdk::FbxFileTexture* pFbxTexture = FbxCast<FbxFileTexture>(pFbxLayeredTexture->GetSrcObject<FbxFileTexture>(j));
						if (pFbxTexture == nullptr)
							continue;

						CheckUVSettings(pFbxTexture, pMaterial);

						AddTextureParameter(pMaterial, strParameterName, nTextureIndex, pFbxTexture->GetFileName(), dwFlags);
						++nTextureIndex;
						bResult = true;
					}
				}
			}
			else
			{
				int nTextureCount = Property.GetSrcObjectCount<FbxFileTexture>();
				for (int i = 0; i < nTextureCount; ++i)
				{
					fbxsdk::FbxFileTexture* pFbxTexture = FbxCast<FbxFileTexture>(Property.GetSrcObject<FbxFileTexture>(i));
					if (pFbxTexture == nullptr)
						continue;

					CheckUVSettings(pFbxTexture, pMaterial);

					AddTextureParameter(pMaterial, strParameterName, i, pFbxTexture->GetFileName(), dwFlags);
					bResult = true;
				}
			}

			return bResult;
		}

		ATG::ExportMaterial* FBXImport::ParseMaterial(fbxsdk::FbxSurfaceMaterial* pFbxMaterial)
		{
			if (pFbxMaterial == nullptr)
				return nullptr;

			ATG::ExportMaterial* pExistingMaterial = g_pScene->FindMaterial(pFbxMaterial);
			if (pExistingMaterial != nullptr)
			{
				ExportLog::LogMsg(4, "Found existing material \"%s\".", pFbxMaterial->GetName());
				return pExistingMaterial;
			}

			ExportLog::LogMsg(2, "Parsing material \"%s\".", pFbxMaterial->GetName());

			bool isRenameMaterial = false;
			ExportString MaterialName(pFbxMaterial->GetName());
			ExportMaterial* pSameNameMaterial = nullptr;
			uint32_t nRenameIndex = 0;
			do
			{
				pSameNameMaterial = g_pScene->FindMaterial(MaterialName);
				if (pSameNameMaterial)
				{
					isRenameMaterial = true;
					MaterialName = String::Format("%s_%u", pFbxMaterial->GetName(), nRenameIndex++).c_str();
				}
			} while (pSameNameMaterial != nullptr);

			if (isRenameMaterial == true)
			{
				ExportLog::LogMsg(2, "Found duplicate material name; renaming material \"%s\" to \"%s\".", pFbxMaterial->GetName(), MaterialName.SafeString());
			}

			ExportMaterial* pMaterial = new ExportMaterial(MaterialName);
			pMaterial->SetDCCObject(pFbxMaterial);
			pMaterial->SetDefaultMaterialName(g_pScene->Settings().strDefaultMaterialName);

			if (g_ExportCoreSettings.bMaterialColors)
			{
				fbxsdk::FbxSurfaceLambert* pFbxLambert = FbxCast<fbxsdk::FbxSurfaceLambert>(pFbxMaterial);
				if (pFbxLambert != nullptr)
				{
					// Diffuse Color
					{
						FbxDouble3 color = pFbxLambert->Diffuse.Get();
						double factor = pFbxLambert->DiffuseFactor.Get();

						ExportMaterialParameter OutputParam;
						OutputParam.Name = "DiffuseColor";
						OutputParam.ValueFloat[0] = static_cast<float>(color[0] * factor);
						OutputParam.ValueFloat[1] = static_cast<float>(color[1] * factor);
						OutputParam.ValueFloat[2] = static_cast<float>(color[2] * factor);
						OutputParam.ValueFloat[3] = static_cast<float>(1.0 - pFbxLambert->TransparencyFactor.Get());
						OutputParam.ParamType = MPT_FLOAT4;
						OutputParam.bInstanceParam = true;
						OutputParam.Flags = 0;
						pMaterial->AddParameter(OutputParam);

						ExportLog::LogMsg(4, "Material parameter \"%s\" = \"%f %f %f %f\"", OutputParam.Name.SafeString(),
							OutputParam.ValueFloat[0], OutputParam.ValueFloat[1], OutputParam.ValueFloat[2], OutputParam.ValueFloat[3]);
					}

					// Ambient Color
					{
						FbxDouble3 color = pFbxLambert->Ambient.Get();
						double factor = pFbxLambert->AmbientFactor.Get();

						ExportMaterialParameter OutputParam;
						OutputParam.Name = "AmbientColor";
						OutputParam.ValueFloat[0] = static_cast<float>(color[0] * factor);
						OutputParam.ValueFloat[1] = static_cast<float>(color[1] * factor);
						OutputParam.ValueFloat[2] = static_cast<float>(color[2] * factor);
						OutputParam.ParamType = MPT_FLOAT3;
						OutputParam.bInstanceParam = true;
						OutputParam.Flags = 0;
						pMaterial->AddParameter(OutputParam);

						ExportLog::LogMsg(4, "Material parameter \"%s\" = \"%f %f %f\"", OutputParam.Name.SafeString(),
							OutputParam.ValueFloat[0], OutputParam.ValueFloat[1], OutputParam.ValueFloat[2]);
					}

					// Emissive Color
					{
						FbxDouble3 color = pFbxLambert->Emissive.Get();
						double factor = pFbxLambert->EmissiveFactor.Get();

						ExportMaterialParameter OutputParam;
						OutputParam.Name = "EmissiveColor";
						OutputParam.ValueFloat[0] = static_cast<float>(color[0] * factor);
						OutputParam.ValueFloat[1] = static_cast<float>(color[1] * factor);
						OutputParam.ValueFloat[2] = static_cast<float>(color[2] * factor);
						OutputParam.ParamType = MPT_FLOAT3;
						OutputParam.bInstanceParam = true;
						OutputParam.Flags = 0;
						pMaterial->AddParameter(OutputParam);

						ExportLog::LogMsg(4, "Material parameter \"%s\" = \"%f %f %f\"", OutputParam.Name.SafeString(),
							OutputParam.ValueFloat[0], OutputParam.ValueFloat[1], OutputParam.ValueFloat[2]);
					}

					fbxsdk::FbxSurfacePhong* pFbxPhong = FbxCast<fbxsdk::FbxSurfacePhong>(pFbxLambert);
					if (pFbxPhong)
					{
						// Specular Color
						{
							FbxDouble3 color = pFbxPhong->Specular.Get();
							double factor = pFbxPhong->SpecularFactor.Get();

							ExportMaterialParameter OutputParam;
							OutputParam.Name = "SpecularColor";
							OutputParam.ValueFloat[0] = static_cast<float>(color[0] * factor);
							OutputParam.ValueFloat[1] = static_cast<float>(color[1] * factor);
							OutputParam.ValueFloat[2] = static_cast<float>(color[2] * factor);
							OutputParam.ParamType = MPT_FLOAT3;
							OutputParam.bInstanceParam = true;
							OutputParam.Flags = 0;
							pMaterial->AddParameter(OutputParam);

							ExportLog::LogMsg(4, "Material parameter \"%s\" = \"%f %f %f\"", OutputParam.Name.SafeString(),
								OutputParam.ValueFloat[0], OutputParam.ValueFloat[1], OutputParam.ValueFloat[2]);
						}

						// Specular Power
						{
							ExportMaterialParameter OutputParam;
							OutputParam.Name = "SpecularPower";
							OutputParam.ValueFloat[0] = static_cast<float>(pFbxPhong->Shininess.Get());
							OutputParam.ParamType = MPT_FLOAT;
							OutputParam.bInstanceParam = true;
							OutputParam.Flags = 0;
							pMaterial->AddParameter(OutputParam);

							ExportLog::LogMsg(4, "Material parameter \"%s\" = \"%f\"", OutputParam.Name.SafeString(), OutputParam.ValueFloat[0]);
						}
					}
				}
			}

			enum ParameterPostOperations
			{
				PPO_Nothing = 0,
				PPO_TransparentMaterial = 1,
			};

			struct TextureParameterExtraction
			{
				const char* strFbxPropertyName;
				const char* strParameterName;
				uint32_t nPostOperations;
				uint32_t nParameterFlags;
			};

			TextureParameterExtraction ExtractionList[] =
			{
				{ FbxSurfaceMaterial::sTransparentColor,   "AlphaTexture",                 PPO_TransparentMaterial,    ExportMaterialParameter::EMPF_ALPHACHANNEL },
				{ FbxSurfaceMaterial::sDiffuse,            "DiffuseTexture",               PPO_Nothing,                ExportMaterialParameter::EMPF_DIFFUSEMAP },
				{ FbxSurfaceMaterial::sAmbient,            "AOTexture",	                   PPO_Nothing,                ExportMaterialParameter::EMPF_AOMAP },
				{ FbxSurfaceMaterial::sBump,               "NormalMapTexture",             PPO_Nothing,                0 /*ExportMaterialParameter::EMPF_BUMPMAP*/ },
				{ FbxSurfaceMaterial::sNormalMap,          "NormalMapTexture",             PPO_Nothing,                ExportMaterialParameter::EMPF_NORMALMAP },
				{ FbxSurfaceMaterial::sSpecular,           "SpecularMapTexture",           PPO_Nothing,                ExportMaterialParameter::EMPF_SPECULARMAP },
				{ FbxSurfaceMaterial::sEmissive,           "EmissiveMapTexture",           PPO_Nothing,                0 },
			};

			for (uint32_t nExtractionIndex = 0; nExtractionIndex < ARRAYSIZE(ExtractionList); ++nExtractionIndex)
			{
				const TextureParameterExtraction& tpe = ExtractionList[nExtractionIndex];

				fbxsdk::FbxProperty Property = pFbxMaterial->FindProperty(tpe.strFbxPropertyName);
				if (Property.IsValid() == false)
					continue;

				bool isFound = ExtractTextures(Property, tpe.strParameterName, pMaterial, tpe.nParameterFlags);
				if (isFound)
				{
					if (tpe.nPostOperations & PPO_TransparentMaterial)
					{
						ExportLog::LogMsg(4, "Material \"%s\" is transparent.", pMaterial->GetName().SafeString());
						pMaterial->SetTransparent(true);
					}
				}
			}

			FixupGenericMaterial(pMaterial);

			bool isSuccess = g_pScene->AddMaterial(pMaterial);
			assert(isSuccess);
			if (isSuccess == false)
			{
				ExportLog::LogError("Could not add material \"%s\" to scene.", pMaterial->GetName().SafeString());
			}
			g_pScene->Statistics().MaterialsExported++;

			return pMaterial;
		}

		void FBXImport::SkinData::Alloc(size_t nCount, DWORD nStride)
		{
			nVertexCount = nCount;
			nVertexStride = nStride;

			size_t nBufferSize = nVertexCount * nVertexStride;
			pBoneIndices.reset(new byte[nBufferSize]);
			ZeroMemory(pBoneIndices.get(), sizeof(byte) * nBufferSize);

			pBoneWeights.reset(new float[nBufferSize]);
			ZeroMemory(pBoneWeights.get(), sizeof(float) * nBufferSize);
		}

		byte* FBXImport::SkinData::GetIndices(size_t nIndex)
		{
			assert(nIndex < nVertexCount);
			return pBoneIndices.get() + (nIndex * nVertexStride);
		}

		float* FBXImport::SkinData::GetWeights(size_t nIndex)
		{
			assert(nIndex < nVertexCount);
			return pBoneWeights.get() + (nIndex * nVertexStride);
		}

		void FBXImport::SkinData::InsertWeight(size_t nIndex, uint32_t nBoneIndex, float fBoneWeight)
		{
			assert(nBoneIndex < 256);

			byte* pIndices = GetIndices(nIndex);
			float* pWeights = GetWeights(nIndex);

			for (uint32_t i = 0; i < nVertexStride; ++i)
			{
				if (fBoneWeight > pWeights[i])
				{
					for (uint32_t j = (nVertexStride - 1); j > i; --j)
					{
						pIndices[j] = pIndices[j - 1];
						pWeights[j] = pWeights[j - 1];
					}

					pIndices[i] = static_cast<byte>(nBoneIndex);
					pWeights[i] = fBoneWeight;
					break;
				}
			}
		}

		void FBXImport::CaptureBindPoseMatrix(fbxsdk::FbxNode* pNode, const fbxsdk::FbxMatrix& matBindPose)
		{
			auto iter = m_BindPoseMap.find(pNode->GetName());
			if (iter != m_BindPoseMap.end())
			{
				const FbxMatrix& matExisting = iter->second;
				if (matExisting != matBindPose)
				{
					// found the bind pose matrix, but it is different than what we prevoiusly encountered
					m_BindPoseMap.emplace(pNode->GetName(), matBindPose);
					m_isBindPoseFixupRequired = true;
					ExportLog::LogMsg(4, "Updating bind pose matrix for frame \"%s\"", pNode->GetName());
				}
			}
			else
			{
				// have not encountered this frame in the bind pose yet
				m_BindPoseMap.emplace(pNode->GetName(), matBindPose);
				m_isBindPoseFixupRequired = true;
				ExportLog::LogMsg(4, "Adding bind pose matrix for frame \"%s\"", pNode->GetName());
			}
		}

		bool FBXImport::ParseMeshSkinning(fbxsdk::FbxMesh* pMesh, SkinData* pSkinData)
		{
			int nDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
			if (nDeformerCount == 0)
				return false;

			ExportLog::LogMsg(4, "Parsing skin weights on mesh %s", pMesh->GetName());

			const int nVertexCount = pMesh->GetControlPointsCount();
			const int nStride = 4;
			pSkinData->Alloc(nVertexCount, nStride);

			for (int nDeformerIndex = 0; nDeformerIndex < nDeformerCount; ++nDeformerIndex)
			{
				auto pSkin = reinterpret_cast<FbxSkin*>(pMesh->GetDeformer(nDeformerIndex, FbxDeformer::eSkin));
				int nClusterCount = pSkin->GetClusterCount();

				for (int nClusterIndex = 0; nClusterIndex < nClusterCount; ++nClusterIndex)
				{
					fbxsdk::FbxCluster* pCluster = pSkin->GetCluster(nClusterIndex);
					int nClusterSize = pCluster->GetControlPointIndicesCount();
					if (nClusterSize == 0)
						continue;

					fbxsdk::FbxNode* pLink = pCluster->GetLink();

					uint32_t nBoneIndex = pSkinData->GetBoneCount();
					pSkinData->InfluenceNodes.push_back(pLink);
					ExportLog::LogMsg(4, "Influence %u: %s", nBoneIndex, pLink->GetName());

					FbxAMatrix matXBindPose;
					pCluster->GetTransformLinkMatrix(matXBindPose);
					
					FbxAMatrix matReferenceGlobalInitPosition;
					pCluster->GetTransformMatrix(matReferenceGlobalInitPosition);

					FbxMatrix matBindPose = matReferenceGlobalInitPosition.Inverse() * matXBindPose;

					Math::Matrix matMotionOffset = ConvertMatrix(matXBindPose.Inverse() * matReferenceGlobalInitPosition);
					g_pScene->GetDCCTransformer()->TransformMatrix(reinterpret_cast<DirectX::XMFLOAT4X4*>(&matMotionOffset), reinterpret_cast<const DirectX::XMFLOAT4X4*>(&matMotionOffset));
					
					m_umapMotionOffsetMarix.emplace(pLink->GetName(), matMotionOffset);

					CaptureBindPoseMatrix(pLink, matBindPose);

					int* pIndices = pCluster->GetControlPointIndices();
					double* pWeights = pCluster->GetControlPointWeights();

					for (int i = 0; i < nClusterSize; ++i)
					{
						pSkinData->InsertWeight(pIndices[i], nBoneIndex, static_cast<float>(pWeights[i]));
					}
				}
			}

			return true;
		}

		void FBXImport::ParseMesh(fbxsdk::FbxNode* pNode, fbxsdk::FbxMesh* pFbxMesh, ATG::ExportFrame* pParentFrame, bool isSubDProcess, const char* strSuffix)
		{
			if (g_pScene->Settings().bExportMeshes == false)
				return;

			if (pNode == nullptr || pFbxMesh == nullptr)
				return;

			const char* strName = pFbxMesh->GetName();
			if (strName == nullptr || strName[0] == '\0')
			{
				strName = pParentFrame->GetName().SafeString();
			}

			if (strSuffix == nullptr)
			{
				strSuffix = "";
			}

			std::string strDecoratedName = String::Format("%s_%s%s", g_pScene->Settings().strMeshNameDecoration, strName, strSuffix);
			ExportMesh* pMesh = new ExportMesh(strDecoratedName.c_str());
			pMesh->SetDCCObject(pFbxMesh);

			bool isSmoothMesh = false;

			fbxsdk::FbxMesh::ESmoothness Smoothness = pFbxMesh->GetMeshSmoothness();
			if (Smoothness != FbxMesh::eHull && g_pScene->Settings().bConvertMeshesToSubD)
			{
				isSubDProcess = true;
				isSmoothMesh = true;
			}

			ExportLog::LogMsg(2, "Parsing %s mesh \"%s\", renamed to \"%s\"", isSmoothMesh ? "smooth" : "poly", strName, strDecoratedName.c_str());

			SkinData skindata;
			bool isSkinnedMesh = ParseMeshSkinning(pFbxMesh, &skindata);
			if (isSkinnedMesh == true)
			{
				uint32_t nBoneCount = skindata.GetBoneCount();
				for (uint32_t i = 0; i < nBoneCount; ++i)
				{
					pMesh->AddInfluence(skindata.InfluenceNodes[i]->GetName());
				}
			}

			bool isExportColors = g_pScene->Settings().bExportColors;
			pMesh->SetVertexColorCount(0);

			// Vertex normals and tangent spaces
			if (g_pScene->Settings().bExportNormals == false)
			{
				pMesh->SetVertexNormalCount(0);
			}
			else if (g_pScene->Settings().bComputeVertexTangentSpace == true)
			{
				if (g_pScene->Settings().bExportBinormal)
				{
					pMesh->SetVertexNormalCount(3);
				}
				else
				{
					pMesh->SetVertexNormalCount(2);
				}
			}
			else
			{
				pMesh->SetVertexNormalCount(1);
			}

			int nLayerCount = pFbxMesh->GetLayerCount();
			ExportLog::LogMsg(4, "%u layers in FBX mesh", nLayerCount);

			if (nLayerCount == 0 || !pFbxMesh->GetLayer(0)->GetNormals())
			{
				ExportLog::LogMsg(4, "Generating normals...");
				pFbxMesh->InitNormals();
#if (FBXSDK_VERSION_MAJOR >= 2015)
				pFbxMesh->GenerateNormals();
#else
				pFbxMesh->ComputeVertexNormals();
#endif
			}

			uint32_t nVertexColorCount = 0;
			uint32_t nUVSetCount = 0;
			FbxLayerElementVertexColor* pVertexColorSet = nullptr;
			FbxLayerElementMaterial* pMaterialSet = nullptr;
			std::vector<FbxLayerElementUV*> VertexUVSets;
			for (int nLayerIndex = 0; nLayerIndex < nLayerCount; ++nLayerIndex)
			{
				if (pFbxMesh->GetLayer(nLayerIndex)->GetVertexColors() && isExportColors == true)
				{
					if (nVertexColorCount == 0)
					{
						nVertexColorCount++;
						pVertexColorSet = pFbxMesh->GetLayer(nLayerIndex)->GetVertexColors();
					}
					else
					{
						ExportLog::LogWarning("Only one vertex color set is allowed; ignoring additional vertex color sets.");
					}
				}

				fbxsdk::FbxLayerElementUV* pUVs = pFbxMesh->GetLayer(nLayerIndex)->GetUVs();
				if (pUVs != nullptr)
				{
					nUVSetCount++;
					VertexUVSets.push_back(pUVs);
				}

				fbxsdk::FbxLayerElementMaterial* pMaterials = pFbxMesh->GetLayer(nLayerIndex)->GetMaterials();
				if (pMaterials != nullptr)
				{
					if (pMaterialSet != nullptr)
					{
						ExportLog::LogWarning("Multiple material layers detected on mesh %s.  Some will be ignored.", pMesh->GetName().SafeString());
					}
					pMaterialSet = pMaterials;
				}
			}

			std::vector<ExportMaterial*> MaterialList;
			for (int nMaterial = 0; nMaterial < pNode->GetMaterialCount(); ++nMaterial)
			{
				fbxsdk::FbxSurfaceMaterial* pMat = pNode->GetMaterial(nMaterial);
				if (pMat == nullptr)
					continue;

				ATG::ExportMaterial* pMaterial = ParseMaterial(pMat);
				MaterialList.push_back(pMaterial);
			}

			ExportLog::LogMsg(4, "Found %u UV sets", nUVSetCount);
			nUVSetCount = std::min<uint32_t>(nUVSetCount, g_pScene->Settings().iMaxUVSetCount);
			ExportLog::LogMsg(4, "Using %u UV sets", nUVSetCount);

			pMesh->SetVertexColorCount(nVertexColorCount);
			pMesh->SetVertexUVCount(nUVSetCount);
			// TODO: Does FBX only support 2D texture coordinates?
			pMesh->SetVertexUVDimension(2);

			uint32_t nMeshOptimizationFlags = 0;
			if (g_pScene->Settings().bCompressVertexData)
			{
				nMeshOptimizationFlags |= ExportMesh::COMPRESS_VERTEX_DATA;
			}

			int nPolyCount = pFbxMesh->GetPolygonCount();
			// Assume that polys are usually quads.
			g_MeshTriangleAllocator.SetSizeHint(nPolyCount * 2);

			int nVertexCount = pFbxMesh->GetControlPointsCount();
			fbxsdk::FbxVector4* pVertexPositions = pFbxMesh->GetControlPoints();

			if (isSkinnedMesh == true)
			{
				assert(skindata.nVertexCount == static_cast<size_t>(nVertexCount));
			}

			ExportLog::LogMsg(4, "%u vertices, %u polygons", nVertexCount, nPolyCount);

			uint32_t nNonConformingSubDPolys = 0;

			// Compute total transformation
			FbxAMatrix vertMatrix;
			FbxAMatrix normMatrix;
			{	
				fbxsdk::FbxVector4 trans = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
				fbxsdk::FbxVector4 rot = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
				fbxsdk::FbxVector4 scale = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

				FbxAMatrix geom;
				geom.SetT(trans);
				geom.SetR(rot);
				geom.SetS(scale);

				if (g_pScene->Settings().bExportAnimations || !g_pScene->Settings().bApplyGlobalTrans)
				{
					vertMatrix = geom;
				}
				else
				{
					vertMatrix = pNode->EvaluateGlobalTransform() * geom;
				}

				// Calculate the normal transform matrix (inverse-transpose)
				normMatrix = vertMatrix;
				normMatrix = normMatrix.Inverse();
				normMatrix = normMatrix.Transpose();
			}

			const bool isInvertTexVCoord = g_pScene->Settings().bInvertTexVCoord;

			// Loop over polygons.
			int basePolyIndex = 0;
			for (int nPolyIndex = 0; nPolyIndex < nPolyCount; ++nPolyIndex)
			{
				// Triangulate each polygon into one or more triangles.
				int nPolySize = pFbxMesh->GetPolygonSize(nPolyIndex);
				assert(nPolySize >= 3);
				int nTriangleCount = nPolySize - 2;
				assert(nTriangleCount > 0);

				if (nPolySize > 4)
				{
					++nNonConformingSubDPolys;
				}

				int nMaterialIndex = 0;
				if (pMaterialSet != nullptr)
				{
					switch (pMaterialSet->GetMappingMode())
					{
					case FbxLayerElement::eByPolygon:
						switch (pMaterialSet->GetReferenceMode())
						{
						case FbxLayerElement::eDirect:
							nMaterialIndex = nPolyIndex;
							break;
						case FbxLayerElement::eIndex:
						case FbxLayerElement::eIndexToDirect:
							nMaterialIndex = pMaterialSet->GetIndexArray().GetAt(nPolyIndex);
							break;
						}
					}
				}

				int nCornerIndices[3] = {};
				// Loop over triangles in the polygon.
				for (int nTriangleIndex = 0; nTriangleIndex < nTriangleCount; ++nTriangleIndex)
				{
					nCornerIndices[0] = pFbxMesh->GetPolygonVertex(nPolyIndex, 0);
					nCornerIndices[1] = pFbxMesh->GetPolygonVertex(nPolyIndex, nTriangleIndex + 1);
					nCornerIndices[2] = pFbxMesh->GetPolygonVertex(nPolyIndex, nTriangleIndex + 2);

					//ExportLog::LogMsg( 4, "Poly %d Triangle %d: %d %d %d", nPolyIndex, nTriangleIndex, nCornerIndices[0], nCornerIndices[1], nCornerIndices[2] );

					FbxVector4 vNormals[3] = {};

					int nVertIndex[3] = { 0, static_cast<int>(nTriangleIndex + 1), static_cast<int>(nTriangleIndex + 2) };
					pFbxMesh->GetPolygonVertexNormal(nPolyIndex, nVertIndex[0], vNormals[0]);
					pFbxMesh->GetPolygonVertexNormal(nPolyIndex, nVertIndex[1], vNormals[1]);
					pFbxMesh->GetPolygonVertexNormal(nPolyIndex, nVertIndex[2], vNormals[2]);

					// Build the raw triangle.
					ATG::ExportMeshTriangle* pTriangle = g_MeshTriangleAllocator.GetNewTriangle();

					// Store polygon index
					pTriangle->PolygonIndex = nPolyIndex;

					// Store material subset index
					pTriangle->SubsetIndex = nMaterialIndex;

					for (int nCornerIndex = 0; nCornerIndex < 3; ++nCornerIndex)
					{
						const int& dwDCCIndex = nCornerIndices[nCornerIndex];
						// Store DCC vertex index (this helps the mesh reduction/VB generation code)
						pTriangle->Vertex[nCornerIndex].DCCVertexIndex = dwDCCIndex;

						// Store vertex position
						auto finalPos = vertMatrix.MultT(pVertexPositions[dwDCCIndex]);

						pTriangle->Vertex[nCornerIndex].Position.x = static_cast<float>(finalPos.mData[0]);
						pTriangle->Vertex[nCornerIndex].Position.y = static_cast<float>(finalPos.mData[1]);
						pTriangle->Vertex[nCornerIndex].Position.z = static_cast<float>(finalPos.mData[2]);

						// Store vertex normal
						fbxsdk::FbxVector4 finalNorm = vNormals[nCornerIndex];
						finalNorm.mData[3] = 0.0;
						finalNorm = normMatrix.MultT(finalNorm);
						finalNorm.Normalize();

						pTriangle->Vertex[nCornerIndex].Normal.x = static_cast<float>(finalNorm.mData[0]);
						pTriangle->Vertex[nCornerIndex].Normal.y = static_cast<float>(finalNorm.mData[1]);
						pTriangle->Vertex[nCornerIndex].Normal.z = static_cast<float>(finalNorm.mData[2]);

						// Store UV sets
						for (uint32_t nUVIndex = 0; nUVIndex < nUVSetCount; ++nUVIndex)
						{
							// Crack apart the FBX dereferencing system for UV coordinates
							FbxLayerElementUV* pUVSet = VertexUVSets[nUVIndex];
							FbxVector2 Value(0, 0);
							switch (pUVSet->GetMappingMode())
							{
							case FbxLayerElement::eByControlPoint:
								switch (pUVSet->GetReferenceMode())
								{
								case FbxLayerElement::eDirect:
									Value = pUVSet->GetDirectArray().GetAt(dwDCCIndex);
									break;
								case FbxLayerElement::eIndex:
								case FbxLayerElement::eIndexToDirect:
								{
									int nTargetUVIndex = pUVSet->GetIndexArray().GetAt(dwDCCIndex);
									Value = pUVSet->GetDirectArray().GetAt(nTargetUVIndex);
								}
								break;
								}
								break;
							case FbxLayerElement::eByPolygonVertex:
								switch (pUVSet->GetReferenceMode())
								{
								case FbxLayerElement::eDirect:
									Value = pUVSet->GetDirectArray().GetAt(basePolyIndex + nVertIndex[nCornerIndex]);
									break;
								case FbxLayerElement::eIndex:
								case FbxLayerElement::eIndexToDirect:
								{
									int nTargetUVIndex = pUVSet->GetIndexArray().GetAt(basePolyIndex + nVertIndex[nCornerIndex]);
#ifdef _DEBUG
									if (nUVIndex == 0)
									{
										// Warning: pFbxMesh->GetTextureUVIndex only works for the first layer of the mesh
										int nTargetUVIndex2 = pFbxMesh->GetTextureUVIndex(nPolyIndex, nVertIndex[nCornerIndex]);
										assert(nTargetUVIndex == nTargetUVIndex2);
									}
#endif
									Value = pUVSet->GetDirectArray().GetAt(nTargetUVIndex);
								}
								break;
								}
								break;
							}

							// Store a single UV set
							pTriangle->Vertex[nCornerIndex].TexCoords[nUVIndex].x = static_cast<float>(Value.mData[0]);
							if (isInvertTexVCoord == true)
							{
								pTriangle->Vertex[nCornerIndex].TexCoords[nUVIndex].y = 1.0f - static_cast<float>(Value.mData[1]);
							}
							else
							{
								pTriangle->Vertex[nCornerIndex].TexCoords[nUVIndex].y = static_cast<float>(Value.mData[1]);
							}
						}

						// Store vertex color set
						if (nVertexColorCount > 0 && pVertexColorSet != nullptr)
						{
							// Crack apart the FBX dereferencing system for Color coordinates
							FbxColor Value(1, 1, 1, 1);
							switch (pVertexColorSet->GetMappingMode())
							{
							case FbxLayerElement::eByControlPoint:
								switch (pVertexColorSet->GetReferenceMode())
								{
								case FbxLayerElement::eDirect:
									Value = pVertexColorSet->GetDirectArray().GetAt(dwDCCIndex);
									break;
								case FbxLayerElement::eIndex:
								case FbxLayerElement::eIndexToDirect:
								{
									int iColorIndex = pVertexColorSet->GetIndexArray().GetAt(dwDCCIndex);
									Value = pVertexColorSet->GetDirectArray().GetAt(iColorIndex);
								}
								break;
								}
								break;
							case FbxLayerElement::eByPolygonVertex:
								switch (pVertexColorSet->GetReferenceMode())
								{
								case FbxLayerElement::eDirect:
									Value = pVertexColorSet->GetDirectArray().GetAt(basePolyIndex + nVertIndex[nCornerIndex]);
									break;
								case FbxLayerElement::eIndex:
								case FbxLayerElement::eIndexToDirect:
								{
									int iColorIndex = pVertexColorSet->GetIndexArray().GetAt(basePolyIndex + nVertIndex[nCornerIndex]);
									Value = pVertexColorSet->GetDirectArray().GetAt(iColorIndex);
								}
								break;
								}
								break;
							}

							// Store a single vertex color set
							pTriangle->Vertex[nCornerIndex].Color.x = static_cast<float>(Value.mRed);
							pTriangle->Vertex[nCornerIndex].Color.y = static_cast<float>(Value.mGreen);
							pTriangle->Vertex[nCornerIndex].Color.z = static_cast<float>(Value.mBlue);
							pTriangle->Vertex[nCornerIndex].Color.w = static_cast<float>(Value.mAlpha);
						}

						// Store skin weights
						if (isSkinnedMesh == true)
						{
							Memory::Copy(&pTriangle->Vertex[nCornerIndex].BoneIndices, sizeof(DirectX::PackedVector::XMUBYTE4), skindata.GetIndices(dwDCCIndex));
							Memory::Copy(&pTriangle->Vertex[nCornerIndex].BoneWeights, sizeof(DirectX::XMFLOAT4), skindata.GetWeights(dwDCCIndex));
						}
					}

					// Add raw triangle to the mesh.
					pMesh->AddRawTriangle(pTriangle);
				}

				basePolyIndex += nPolySize;
			}

			if (isSubDProcess == true)
			{
				nMeshOptimizationFlags |= ExportMesh::FORCE_SUBD_CONVERSION;
			}

			if (g_pScene->Settings().bCleanMeshes)
			{
				nMeshOptimizationFlags |= ExportMesh::CLEAN_MESHES;
			}

			if (g_pScene->Settings().bOptimizeVCache)
			{
				nMeshOptimizationFlags |= ExportMesh::CLEAN_MESHES | ExportMesh::VCACHE_OPT;
			}

			pMesh->Optimize(nMeshOptimizationFlags);

			ExportModel* pModel = new ExportModel(pMesh);
			size_t nMaterialCount = MaterialList.size();
			if (pMesh->GetSubDMesh() == nullptr)
			{
				for (size_t nSubset = 0; nSubset < nMaterialCount; ++nSubset)
				{
					ATG::ExportMaterial* pMaterial = MaterialList[nSubset];
					ATG::ExportIBSubset* pSubset = pMesh->GetSubset(nSubset);
					std::string strUniqueSubsetName = String::Format("subset%Iu_%s", nSubset, pMaterial->GetName().SafeString());
					pSubset->SetName(strUniqueSubsetName.c_str());
					pModel->SetSubsetBinding(pSubset->GetName(), pMaterial);
				}
			}
			else
			{
				ExportSubDProcessMesh* pSubDMesh = pMesh->GetSubDMesh();
				size_t nSubsetCount = pSubDMesh->GetSubsetCount();
				for (size_t nSubset = 0; nSubset < nSubsetCount; ++nSubset)
				{
					ExportSubDPatchSubset* pSubset = pSubDMesh->GetSubset(nSubset);
					assert(pSubset != nullptr);
					assert(pSubset->iOriginalMeshSubset < static_cast<int>(nMaterialCount));
					ATG::ExportMaterial* pMaterial = MaterialList[pSubset->iOriginalMeshSubset];
					std::string strUniqueSubsetName = String::Format("subset%Iu_%s", nSubset, pMaterial->GetName().SafeString());
					pSubset->Name = strUniqueSubsetName.c_str();
					pModel->SetSubsetBinding(pSubset->Name, pMaterial, true);
				}
			}

			if (isSubDProcess == true && (nNonConformingSubDPolys > 0))
			{
				ExportLog::LogWarning("Encountered %u polygons with 5 or more sides in mesh \"%s\", which were subdivided into quad and triangle patches.  Mesh appearance may have been affected.", nNonConformingSubDPolys, pMesh->GetName().SafeString());
			}

			// update statistics
			if (pMesh->GetSubDMesh() != nullptr)
			{
				g_pScene->Statistics().SubDMeshesProcessed++;
				g_pScene->Statistics().SubDQuadsProcessed += pMesh->GetSubDMesh()->GetQuadPatchCount();
				g_pScene->Statistics().SubDTrisProcessed += pMesh->GetSubDMesh()->GetTrianglePatchCount();
			}
			else
			{
				g_pScene->Statistics().TrisExported += pMesh->GetIB()->GetIndexCount() / 3;
				g_pScene->Statistics().VertsExported += pMesh->GetVB()->GetVertexCount();
				g_pScene->Statistics().MeshesExported++;
			}

			pParentFrame->AddModel(pModel);
			g_pScene->AddMesh(pMesh);
		}

		void FBXImport::ParseSubDiv(FbxNode* pNode, FbxSubDiv* pFbxSubD, ATG::ExportFrame* pParentFrame)
		{
			if (g_pScene->Settings().bExportMeshes == false)
				return;

			if (pFbxSubD == nullptr)
				return;

			const char* strName = pFbxSubD->GetName();
			if (strName == nullptr || strName[0] == '\0')
			{
				strName = pParentFrame->GetName().SafeString();
			}

			int nLevelCount = pFbxSubD->GetLevelCount();
			ExportLog::LogMsg(2, "Parsing subdivision surface \"%s\" with %u levels", strName, nLevelCount);
			if (nLevelCount == 0)
			{
				ExportLog::LogWarning("Subdivision surface \"%s\" has no levels.", strName);
				return;
			}

			FbxMesh* pLevelMesh = nullptr;
			int nCurrentLevel = nLevelCount - 1;
			while (!pLevelMesh && nCurrentLevel > 0)
			{
				pLevelMesh = pFbxSubD->GetMesh(nCurrentLevel);
				if (pLevelMesh == nullptr)
				{
					--nCurrentLevel;
				}
			}

			if (pLevelMesh == nullptr)
			{
				pLevelMesh = pFbxSubD->GetBaseMesh();
			}

			assert(pLevelMesh != nullptr);

			ExportLog::LogMsg(3, "Parsing level %u", nCurrentLevel);
			std::string strSuffix = String::Format("_level%u", nCurrentLevel);
			ParseMesh(pNode, pLevelMesh, pParentFrame, true, strSuffix.c_str());
		}
	}
}