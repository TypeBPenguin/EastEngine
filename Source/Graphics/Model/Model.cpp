#include "stdafx.h"
#include "Model.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"
#include "CommonLib/Timer.h"

#include "ModelManager.h"

#include "ModelNodeStatic.h"
#include "ModelNodeSkinned.h"

#include "ModelInstance.h"

#include "GeometryModel.h"

#include "XpsImporter.h"

namespace sid
{
	RegisterStringID(est_CustomStaticModel);
	RegisterStringID(est_Cube);
	RegisterStringID(est_Box);
	RegisterStringID(est_Sphere);
	RegisterStringID(est_GeoSphere);
	RegisterStringID(est_Cylinder);
	RegisterStringID(est_Cone);
	RegisterStringID(est_Torus);
	RegisterStringID(est_Tetrahedron);
	RegisterStringID(est_Octahedron);
	RegisterStringID(est_Dodecahedron);
	RegisterStringID(est_Icosahedron);
	RegisterStringID(est_Teapot);
	RegisterStringID(est_Hexagon);
	RegisterStringID(est_Capsule);
	RegisterStringID(est_Grid);
	RegisterStringID(est_Plane);
	RegisterStringID(NoParent);
};

namespace est
{
	namespace graphics
	{
		const string::StringID& GetGeometryTypeName(ModelLoader::GeometryType emGeometryType)
		{
			static string::StringID strGeometryType[] =
			{
				sid::est_CustomStaticModel,
				sid::est_Cube,
				sid::est_Box,
				sid::est_Sphere,
				sid::est_GeoSphere,
				sid::est_Cylinder,
				sid::est_Cone,
				sid::est_Torus,
				sid::est_Tetrahedron,
				sid::est_Octahedron,
				sid::est_Dodecahedron,
				sid::est_Icosahedron,
				sid::est_Teapot,
				sid::est_Hexagon,
				sid::est_Capsule,
				sid::est_Grid,
				sid::est_Plane,
			};

			return strGeometryType[emGeometryType];
		}

		Model::Model(Key key)
			: m_key(key)
		{
		}

		Model::Model(Model&& source) noexcept
			: m_key(std::move(source.m_key))
			, m_isVisible(std::move(source.m_isVisible))
			, m_isDirtyLocalMatrix(std::move(source.m_isDirtyLocalMatrix))
			, m_skeleton(std::move(source.m_skeleton))
			, m_transform(std::move(source.m_transform))
			, m_matLocal(std::move(source.m_matLocal))
			, m_modelName(std::move(source.m_modelName))
			, m_filePath(std::move(source.m_filePath))
			, m_hierarchyModelNodes(std::move(source.m_hierarchyModelNodes))
			, m_modelNodes(std::move(source.m_modelNodes))
			, m_modelInstances(std::move(source.m_modelInstances))
		{
		}

		Model::~Model()
		{
			m_modelInstances.clear();

			m_hierarchyModelNodes.clear();
			m_modelNodes.clear();
		}

		Model& Model::operator=(Model&& source) noexcept
		{
			m_key = std::move(source.m_key);
			m_isVisible = std::move(source.m_isVisible);
			m_isDirtyLocalMatrix = std::move(source.m_isDirtyLocalMatrix);
			m_skeleton = std::move(source.m_skeleton);
			m_transform = std::move(source.m_transform);
			m_matLocal = std::move(source.m_matLocal);
			m_modelName = std::move(source.m_modelName);
			m_filePath = std::move(source.m_filePath);
			m_hierarchyModelNodes = std::move(source.m_hierarchyModelNodes);
			m_modelNodes = std::move(source.m_modelNodes);
			m_modelInstances = std::move(source.m_modelInstances);

			return *this;
		}

		void Model::AddInstance(ModelInstance* pModelInstance)
		{
			m_modelInstances.emplace_back(pModelInstance);

			if (GetState() == IResource::eComplete)
			{
				pModelInstance->LoadCompleteCallback(true);
			}
			else if (GetState() == IResource::eInvalid)
			{
				pModelInstance->LoadCompleteCallback(false);
			}
		}

		bool Model::RemoveInstance(ModelInstance* pModelInstance)
		{
			auto iter = std::find(m_modelInstances.begin(), m_modelInstances.end(), pModelInstance);
			if (iter == m_modelInstances.end())
				return false;

			m_modelInstances.erase(iter);
			return true;
		}

		bool Model::IsReadyToDestroy(double gameTime)
		{
			if (m_modelInstances.empty() == false)
			{
				m_destroyWaitTime = 0.0;
				return false;
			}

			if (math::IsZero(m_destroyWaitTime) == true)
			{
				m_destroyWaitTime = gameTime + 120.0;
				return false;
			}

			return m_destroyWaitTime < gameTime;
		}

		void Model::Ready()
		{
			if (GetState() != IResource::eComplete)
				return;

			if (m_isDirtyLocalMatrix == true)
			{
				m_matLocal = m_transform.Compose();
				m_isDirtyLocalMatrix = false;
			}
		}

		void Model::Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, ITransformInstance* pTransformInstance)
		{
			TRACER_EVENT(__FUNCTIONW__);
			for (auto& pModelNode : m_hierarchyModelNodes)
			{
				pModelNode->Update(elapsedTime, matParent, pSkeletonInstance, pMaterialInstance, pTransformInstance, m_isVisible);
			}
		}

		IModelNode* Model::GetNode(const string::StringID& name) const
		{
			auto iter = std::find_if(m_modelNodes.begin(), m_modelNodes.end(), [&](IModelNode* pNode)
			{
				return pNode->GetName() == name;
			});

			if (iter != m_modelNodes.end())
				return *iter;

			return nullptr;
		}

		void Model::LoadCompleteCallback(bool isSuccess)
		{
			SetState(isSuccess == true ? IResource::eComplete : IResource::eInvalid);

			for (auto& pInstance : m_modelInstances)
			{
				pInstance->LoadCompleteCallback(isSuccess);
			}
		}

		void Model::ChangeName(const string::StringID& name)
		{
			SetName(name);
		}

		void Model::AddNode(std::unique_ptr<ModelNode> pNode, const string::StringID& nodeName)
		{
			if (GetNode(nodeName) == nullptr)
			{
				m_modelNodes.emplace_back(pNode.get());

				pNode->SetNodeName(nodeName);
				m_hierarchyModelNodes.emplace_back(std::move(pNode));
			}
			else
			{
				LOG_ERROR(L"failed to add model node, already added node[%s]", nodeName.c_str());
			}
		}

		void Model::AddNode(std::unique_ptr<ModelNode> pNode, const string::StringID& nodeName, const string::StringID& parentNodeName)
		{
			ModelNode* pParentNode = static_cast<ModelNode*>(GetNode(parentNodeName));
			if (pParentNode == nullptr)
			{
				LOG_ERROR(L"failed to add model node, not exists parent node[%s]", parentNodeName.c_str());
				AddNode(std::move(pNode), nodeName);
			}
			else
			{
				if (GetNode(nodeName) == nullptr)
				{
					m_modelNodes.emplace_back(pNode.get());
					pParentNode->AddChildNode(std::move(pNode));
				}
				else
				{
					LOG_ERROR(L"failed to add model node, already added node[%s]", nodeName.c_str());
				}
			}
		}

		bool Model::Load(const ModelLoader& loader)
		{
			bool isSuccess = false;

			switch (loader.GetLoadModelType())
			{
			case ModelLoader::eFbx:
			{
				Stopwatch sw;
				sw.Start();
				isSuccess = ModelManager::GetInstance()->LoadModelFBX(this, loader.GetFilePath().c_str(), loader.GetScaleFactor(), loader.IsFlipZ());
				sw.Stop();

				if (isSuccess == true)
				{
					LOG_MESSAGE(L"FBX Model Load Complete : %lf[%s]", sw.Elapsed(), loader.GetFilePath().c_str());
				}
				else
				{
					LOG_ERROR(L"Can't load Model[FBX] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case ModelLoader::eObj:
			{
				// fbx에 obj를 넣어도 된다.
				Stopwatch sw;
				sw.Start();
				isSuccess = ModelManager::GetInstance()->LoadModelFBX(this, loader.GetFilePath().c_str(), loader.GetScaleFactor(), loader.IsFlipZ());
				sw.Stop();

				if (isSuccess == true)
				{
					LOG_MESSAGE(L"Obj Model Load Complete : %lf[%s]", sw.Elapsed(), loader.GetFilePath().c_str());
				}
				else
				{
					LOG_ERROR(L"Can't load Model[Obj] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case ModelLoader::eXps:
			{
				Stopwatch sw;
				sw.Start();
				isSuccess = XPSImport::LoadModel(this, loader.GetFilePath().c_str(), loader.GetDevideKeywords(), loader.GetDevideKeywordCount());
				sw.Stop();

				if (isSuccess == true)
				{
					LOG_MESSAGE(L"Xps Model Load Complete : %lf[%s]", sw.Elapsed(), loader.GetFilePath().c_str());
				}
				else
				{
					LOG_ERROR(L"Can't load Model[XPS] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case ModelLoader::eEast:
			{
				Stopwatch sw;
				sw.Start();
				isSuccess = LoadFile(loader.GetFilePath().c_str());
				sw.Stop();

				if (isSuccess == true)
				{
					LOG_MESSAGE(L"Emod Model Load Complete : %lf[%s]", sw.Elapsed(), loader.GetFilePath().c_str());
				}
				else
				{
					LOG_ERROR(L"Can't load Model[East] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case ModelLoader::eGeometry:
			{
				std::vector<VertexPosTexNor> vertices;
				std::vector<uint32_t> indices;

				collision::AABB aabb;

				auto SetModelNode = [&](const ModelLoader& loader, const string::StringID& nodeName, const VertexBufferPtr& pVertexBuffer, const IndexBufferPtr& pIndexBuffer, const std::vector<VertexPos>& rawVertices, const std::vector<uint32_t>& rawIndices, const collision::AABB& aabb)
				{
					std::unique_ptr<ModelNodeStatic> pModelStatic = std::make_unique<ModelNodeStatic>();
					pModelStatic->SetVertexBuffer(pVertexBuffer);
					pModelStatic->SetIndexBuffer(pIndexBuffer);

					if (rawVertices.empty() == false && rawIndices.empty() == false)
					{
						pModelStatic->SetRawVertices(rawVertices.data(), rawVertices.size());
						pModelStatic->SetRawIndices(rawIndices.data(), rawIndices.size());
					}

					pModelStatic->SetOriginAABB(aabb);

					MaterialPtr pMaterial = CreateMaterial(&loader.GetMaterial());
					pModelStatic->AddMaterial(pMaterial);

					ModelSubset modelSubset;
					modelSubset.materialID = 0;
					modelSubset.startIndex = 0;
					modelSubset.indexCount = pIndexBuffer->GetIndexCount();
					pModelStatic->AddModelSubset(modelSubset);

					AddNode(std::move(pModelStatic), nodeName);
				};

				switch (loader.GetLoadGeometryType())
				{
				case ModelLoader::eCube:
				{
					const LoadInfoCube* pLoadInfo = loader.GetCubeInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateCube(vertices, indices, pLoadInfo->size, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->size);
					}
				}
				break;
				case ModelLoader::eBox:
				{
					const LoadInfoBox* pLoadInfo = loader.GetBoxInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateBox(vertices, indices, pLoadInfo->halfExtents, loader.IsRightHandCoords(), loader.IsInvertNormal());

						aabb.Extents = pLoadInfo->halfExtents;
					}
				}
				break;
				case ModelLoader::eSphere:
				{
					const LoadInfoSphere* pLoadInfo = loader.GetSphereInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateSphere(vertices, indices, pLoadInfo->diameter, pLoadInfo->tessellation, loader.IsRightHandCoords(), loader.IsInvertNormal());

						aabb.Extents = math::float3(pLoadInfo->diameter * 0.5f);
					}
				}
				break;
				case ModelLoader::eGeoSphere:
				{
					const LoadInfoGeoSphere* pLoadInfo = loader.GetGeoSphereInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateGeoSphere(vertices, indices, pLoadInfo->diameter, pLoadInfo->tessellation, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->diameter * 0.5f);
					}
				}
				break;
				case ModelLoader::eCylinder:
				{
					const LoadInfoCylinder* pLoadInfo = loader.GetCylinderInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateCylinder(vertices, indices, pLoadInfo->height, pLoadInfo->diameter, pLoadInfo->tessellation, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->diameter * 0.5f, pLoadInfo->height, pLoadInfo->diameter * 0.5f);
					}
				}
				break;
				case ModelLoader::eCone:
				{
					const LoadInfoCone* pLoadInfo = loader.GetConeInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateCone(vertices, indices, pLoadInfo->diameter, pLoadInfo->height, pLoadInfo->tessellation, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->diameter * 0.5f, pLoadInfo->height, pLoadInfo->diameter * 0.5f);
					}
				}
				break;
				case ModelLoader::eTorus:
				{
					const LoadInfoTorus* pLoadInfo = loader.GetTorusInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateTorus(vertices, indices, pLoadInfo->diameter, pLoadInfo->thickness, pLoadInfo->tessellation, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->diameter * 0.5f, pLoadInfo->thickness, pLoadInfo->diameter * 0.5f);
					}
				}
				break;
				case ModelLoader::eTetrahedron:
				{
					const LoadInfoTetrahedron* pLoadInfo = loader.GetTetrahedronInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateTetrahedron(vertices, indices, pLoadInfo->size, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->size);
					}
				}
				break;
				case ModelLoader::eOctahedron:
				{
					const LoadInfoOctahedron* pLoadInfo = loader.GetOctahedronInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateOctahedron(vertices, indices, pLoadInfo->size, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->size);
					}
				}
				break;
				case ModelLoader::eDodecahedron:
				{
					const LoadInfoDodecahedron* pLoadInfo = loader.GetDodecahedronInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateDodecahedron(vertices, indices, pLoadInfo->size, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->size);
					}
				}
				break;
				case ModelLoader::eIcosahedron:
				{
					const LoadInfoIcosahedron* pLoadInfo = loader.GetIcosahedronInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateIcosahedron(vertices, indices, pLoadInfo->size, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->size);
					}
				}
				break;
				case ModelLoader::eTeapot:
				{
					const LoadInfoTeapot* pLoadInfo = loader.GetTeapotInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateTeapot(vertices, indices, pLoadInfo->size, pLoadInfo->tessellation, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->size);
					}
				}
				break;
				case ModelLoader::eHexagon:
				{
					const LoadInfoHexagon* pLoadInfo = loader.GetHexagonInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateHexagon(vertices, indices, pLoadInfo->radius, loader.IsRightHandCoords());

						aabb.Extents = math::float3(pLoadInfo->radius);
					}
				}
				break;
				case ModelLoader::eCapsule:
				{
					const LoadInfoCapsule* pLoadInfo = loader.GetCapsuleInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateCapsule(vertices, indices, pLoadInfo->radius, pLoadInfo->height, pLoadInfo->subdivisionHeight, pLoadInfo->segments);

						aabb.Extents = math::float3(pLoadInfo->radius, pLoadInfo->height, pLoadInfo->radius);
					}
				}
				break;
				case ModelLoader::eGrid:
				{
					const LoadInfoGrid* pLoadInfo = loader.GetGridInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreateGrid(vertices, indices, pLoadInfo->gridSizeX, pLoadInfo->gridSizeZ, pLoadInfo->blockCountWidth, pLoadInfo->blockCountLength);

						aabb.Extents = math::float3(0.5f * pLoadInfo->gridSizeX, 0.1f, 0.5f * pLoadInfo->gridSizeZ);
					}
				}
				break;
				case ModelLoader::ePlane:
				{
					const LoadInfoPlane* pLoadInfo = loader.GetPlaneInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = geometry::CreatePlane(vertices, indices, pLoadInfo->eachLengthX, pLoadInfo->eachLengthZ, pLoadInfo->totalCountX, pLoadInfo->totalCountZ);

						aabb.Extents = math::float3(0.5f * pLoadInfo->eachLengthX * pLoadInfo->totalCountX, 0.1f, 0.5f * pLoadInfo->eachLengthZ * pLoadInfo->totalCountZ);
					}
				}
				break;
				case ModelLoader::eCustomStaticModel:
					break;
				}

				if (isSuccess == true)
				{
					VertexBufferPtr pVertexBuffer = CreateVertexBuffer(reinterpret_cast<const uint8_t*>(vertices.data()), static_cast<uint32_t>(vertices.size()), sizeof(VertexPosTexNor), true);
					IndexBufferPtr pIndexBuffer = CreateIndexBuffer(reinterpret_cast<const uint8_t*>(indices.data()), static_cast<uint32_t>(indices.size()), sizeof(uint32_t), true);

					const size_t vertexCount = vertices.size();
					std::vector<VertexPos> rawVertices;

					rawVertices.resize(vertices.size());
					for (size_t i = 0; i < vertexCount; ++i)
					{
						rawVertices[i].pos = vertices[i].pos;
					}

					SetModelNode(loader, GetGeometryTypeName(loader.GetLoadGeometryType()), pVertexBuffer, pIndexBuffer, rawVertices, indices, aabb);
				}
				else
				{
					LOG_ERROR(L"Can't load Model, GeometryType : %s", GetGeometryTypeName(loader.GetLoadGeometryType()).c_str());
				}
			}
			break;
			default:
				return false;
			}

			return isSuccess;
		}

		bool Model::LoadFile(const wchar_t* filePath)
		{
			const std::wstring fileExtension = file::GetFileExtension(filePath);
			if (fileExtension != L".emod")
			{
				LOG_ERROR(L"Invalid Extension, Required[%s] != Request[%s]", ".emod", fileExtension.c_str());
				return false;
			}

			// 좀 더 구조적으로 쉽고 간편한 Save Load 방식이 필요함
			// Stream 은 빨라서 좋지만, 데이터 규격이 달라지면 기존 데이터를 사용할 수 없게됨
			// 또는 확실한 버전 관리로, 버전별 Save Load 로직을 구현한다면 Stream 으로도 문제없음
			file::Stream file;
			if (file.Open(filePath, file::eReadBinary) == false)
			{
				LOG_WARNING(L"Can't open to file : %s", filePath);
				return false;
			}

			BinaryReader binaryReader = file.GetBinaryReader();

			{
				const std::wstring onlyPath = file::GetFilePath(filePath);

				const string::StringID modelName = binaryReader.ReadString();
				SetName(modelName);

				m_transform.position = binaryReader;
				m_transform.scale = binaryReader;
				m_transform.rotation = binaryReader;

				m_isDirtyLocalMatrix = true;

				const uint32_t nodeCount = binaryReader;
				for (uint32_t i = 0; i < nodeCount; ++i)
				{
					std::unique_ptr<ModelNode> pNode = nullptr;

					const IModelNode::Type emType = binaryReader;
					switch (emType)
					{
					case IModelNode::eStatic:
						pNode = std::make_unique<ModelNodeStatic>(onlyPath.c_str(), binaryReader);
						break;
					case IModelNode::eSkinned:
						pNode = std::make_unique<ModelNodeSkinned>(onlyPath.c_str(), binaryReader);
						break;
					default:
						LOG_WARNING(L"잘못된타입임돠, 데이터 포맷이 바뀐 듯 함돠.");
						break;
					}

					const string::StringID& parentNodeName = pNode->GetParentName();
					if (parentNodeName == sid::NoParent || parentNodeName == sid::None)
					{
						const string::StringID name = pNode->GetName();
						AddNode(std::move(pNode), name);
					}
					else
					{
						const string::StringID name = pNode->GetName();
						AddNode(std::move(pNode), name, parentNodeName);
					}
				}

				// Skeleton
				const bool isHasSkeleton = binaryReader ;
				if (isHasSkeleton == true)
				{
					m_skeleton.LoadFile(binaryReader);
				}

				for (const auto& pNode : m_modelNodes)
				{
					const uint32_t nMaterialCount = pNode->GetMaterialCount();
					for (uint32_t i = 0; i < nMaterialCount; ++i)
					{
						MaterialPtr pMaterial = pNode->GetMaterial(i);
						pMaterial->LoadTexture();
					}

					const uint32_t nBoneCount = m_skeleton.GetBoneCount();
					if (nBoneCount > 0)
					{
						if (pNode->GetType() != IModelNode::eSkinned)
							continue;

						ModelNodeSkinned* pNodeSkinned = static_cast<ModelNodeSkinned*>(pNode);

						uint32_t nIncludBoneCount = pNodeSkinned->GetBoneCount();

						std::vector<string::StringID> vecBoneNames;
						vecBoneNames.resize(nIncludBoneCount);

						for (uint32_t j = 0; j < nIncludBoneCount; ++j)
						{
							vecBoneNames[j] = pNodeSkinned->GetBoneName(j);
						}

						m_skeleton.SetSkinnedList(pNodeSkinned->GetName(), &vecBoneNames.front(), vecBoneNames.size());
					}
				}
			}

			return true;
		}
	}
}