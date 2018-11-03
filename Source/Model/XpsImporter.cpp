#include "stdafx.h"
#include "XpsImporter.h"

#include "Model.h"
#include "ModelNodeSkinned.h"
#include "Skeleton.h"

#include "Motion.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

namespace eastengine
{
	namespace graphics
	{
		namespace XPSImport
		{
			struct XPS_Bone
			{
				std::string name;
				uint16_t parentIndex = 0;
				float defaultPositionX = 0.f;
				float defaultPositionY = 0.f;
				float defaultPositionZ = 0.f;
			};

			struct XPS_Texture
			{
				std::string filename;
				uint32_t uvLayer = 0;
			};

			struct XPS_Vertex
			{
				math::Vector3 vertex;
				math::Vector3 normal;
				uint8_t color[4] = {};
				std::vector<math::Vector2> texCoord;	// [numUVLayers]
				std::vector<math::Vector4> tangent;		// [numUVLayers]
				uint16_t boneIndex[4] = {};				// Only if the model has bones
				float boneWeight[4] = {};				// Only if the model has bones
			};

			struct XPS_Mesh
			{
				std::string name;
				uint32_t numUVLayers;
				uint32_t numTextures;
				std::vector<XPS_Texture> textures; // [numTextures];
				uint32_t numVertices;
				std::vector<XPS_Vertex> vertices;	// [numVertices];
				uint32_t numElements;
				std::vector<uint32_t> elements;	// [numElements];
			};

			std::string GetString(file::Stream& file)
			{
				if (file.GetOpenMode() == 0)
				{
					std::string strTemp;
					file >> strTemp;
					return strTemp;
				}
				else
				{
					uint8_t nNameLength = 0;
					file.Read(&nNameLength);

					std::string strTemp;
					strTemp.resize(nNameLength);
					file.Read(strTemp.data(), nNameLength);

					return strTemp;
				}
			}

			bool LoadModel(Model* pModel, const char* strFilePath, const std::string* pStrDevideModels, size_t nKeywordCount)
			{
				file::Stream file;
				if (file.Open(strFilePath, file::eRead | file::eBinary) == false)
					return false;

				std::string strPath = file::GetFilePath(strFilePath);
				std::string strFileName = file::GetFileName(strFilePath);

				uint32_t nBoneCount = 0;
				file.Read(&nBoneCount);

				enum
				{
					eXPS_MagicNumber = 323232,
				};

				bool isReadTangentData = true;

				if (nBoneCount == eXPS_MagicNumber)
				{
					isReadTangentData = false;

					uint16_t nTemp16 = 0;
					file.Read(&nTemp16);
					file.Read(&nTemp16);

					std::string strTemp;
					strTemp = GetString(file);

					uint32_t nTemp32 = 0;
					file.Read(&nTemp32);

					strTemp = GetString(file);
					strTemp = GetString(file);

					uint8_t length = 0;
					file.Read(&length);

					int8_t byte = 0;
					file.Read(&byte);

					if (byte > 2)
					{
						file.Seekg(-1, 1);
					}

					strTemp.clear();
					strTemp.resize(length);
					file.Read(strTemp.data(), length);

					int nReadSize = 0;
					//if (strTemp.back() != -68)
					//{
					//	nReadSize = 1029;
					//
					//	while (true)
					//	{
					//		file.Read(&byte);
					//
					//		if (byte == -68)
					//		{
					//			file.Read(&nTemp16);
					//			break;
					//		}
					//	}
					//}
					//else
					{
						nReadSize = 1028;

						file.Read(&nTemp32);
					}

					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);

					uint32_t nPoseDataCount = 0;
					file.Read(&nPoseDataCount);

					std::vector<std::string> vecS;
					vecS.resize(nPoseDataCount);
					for (uint32_t i = 0; i < nPoseDataCount; ++i)
					{
						file.ReadLine(vecS[i]);
					}

					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);
					file.Read(&nTemp32);

					strTemp.clear();
					strTemp.resize(nReadSize);
					file.Read(strTemp.data(), nReadSize);

					//uint8_t nTemp8 = 0;
					//file.Read(&nTemp8);

					file.Read(&nBoneCount);
				}

				std::vector<XPS_Bone> vecBones;
				std::vector<XPS_Mesh> vecMeshs;

				vecBones.resize(nBoneCount);

				for (uint32_t i = 0; i < nBoneCount; ++i)
				{
					XPS_Bone& bone = vecBones[i];

					bone.name = GetString(file);

					file.Read(&bone.parentIndex);
					file.Read(&bone.defaultPositionX);
					file.Read(&bone.defaultPositionY);
					file.Read(&bone.defaultPositionZ);

					bone.defaultPositionZ *= -1.f;
				}

				uint32_t nMeshCount = 0;
				file.Read(&nMeshCount);

				vecMeshs.resize(nMeshCount);

				for (uint32_t i = 0; i < nMeshCount; ++i)
				{
					XPS_Mesh& mesh = vecMeshs[i];

					mesh.name = GetString(file);

					file.Read(&mesh.numUVLayers);
					file.Read(&mesh.numTextures);

					mesh.textures.resize(mesh.numTextures);
					for (uint32_t j = 0; j < mesh.numTextures; ++j)
					{
						XPS_Texture& texture = mesh.textures[j];

						texture.filename = GetString(file);

						file.Read(&texture.uvLayer);
					}

					file.Read(&mesh.numVertices);

					mesh.vertices.resize(mesh.numVertices);
					for (uint32_t j = 0; j < mesh.numVertices; ++j)
					{
						XPS_Vertex& vertex = mesh.vertices[j];

						file.Read(&vertex.vertex.x, 3);
						vertex.vertex.z *= -1.f;

						file.Read(&vertex.normal.x, 3);
						vertex.normal.z *= -1.f;

						file.Read(vertex.color, 4);

						vertex.texCoord.resize(mesh.numUVLayers);
						for (uint32_t k = 0; k < mesh.numUVLayers; ++k)
						{
							file.Read(&vertex.texCoord[k].x, 2);
						}

						if (isReadTangentData == true)
						{
							vertex.tangent.resize(mesh.numUVLayers);
							for (uint32_t k = 0; k < mesh.numUVLayers; ++k)
							{
								file.Read(&vertex.tangent[k].x, 4);
							}
						}

						file.Read(vertex.boneIndex, 4);
						file.Read(vertex.boneWeight, 4);
					}

					file.Read(&mesh.numElements);
					mesh.numElements *= 3;

					mesh.elements.resize(mesh.numElements);

					file.Read(mesh.elements.data(), static_cast<uint32_t>(mesh.elements.size()));
				}

				{
					ModelNodeSkinned* pSkinnedNode = new ModelNodeSkinned;
					pSkinnedNode->SetNodeName(strFileName.c_str());
					pSkinnedNode->SetVisible(true);

					std::vector<VertexPosTexNorWeiIdx> vecVertices;
					std::vector<uint32_t> vecIndices;
					std::vector<ModelSubset> vecModelSubset;
					std::vector<IMaterial*> vecMaterial;

					bool isEnableMeshOptimize = true;
					if (isEnableMeshOptimize == true)
					{
						uint32_t nVertexCount = 0;
						uint32_t nIndexCount = 0;

						struct MeshOptimizer
						{
							string::StringID strName;
							std::vector<int> vecMeshIndex;
							uint32_t numVertices = 0;
							uint32_t numElements = 0;
						};

						std::map<std::tuple<std::string, std::string, std::string>, MeshOptimizer> umapMesh;
						for (uint32_t i = 0; i < nMeshCount; ++i)
						{
							XPS_Mesh& mesh = vecMeshs[i];

							nVertexCount += mesh.numVertices;
							nIndexCount += mesh.numElements;
							
							std::string strAlbedo;
							std::string strNormal;
							std::string strClassify = mesh.name;

							if (mesh.numTextures == 1)
							{
								strAlbedo = file::GetFileName(mesh.textures[0].filename);
							}
							else if (mesh.numTextures == 3 || mesh.numTextures == 4)
							{
								strAlbedo = file::GetFileName(mesh.textures[0].filename);
								strNormal = file::GetFileName(mesh.textures[2].filename);
							}

							for (size_t j = 0; j < nKeywordCount; ++j)
							{
								if (strstr(mesh.name.c_str(), pStrDevideModels[j].c_str()) != nullptr)
								{
									strClassify = pStrDevideModels[j];
									break;
								}
							}

							std::tuple<std::string, std::string, std::string> key = std::make_tuple(strAlbedo, strNormal, strClassify);
							auto iter = umapMesh.find(key);
							if (iter != umapMesh.end())
							{
								MeshOptimizer& meshOptimizer = iter->second;
								meshOptimizer.vecMeshIndex.emplace_back(i);
								meshOptimizer.numVertices += mesh.numVertices;
								meshOptimizer.numElements += mesh.numElements;
							}
							else
							{
								auto iter_result = umapMesh.emplace(key, MeshOptimizer());
								MeshOptimizer& meshOptimizer = iter_result.first->second;
								meshOptimizer.vecMeshIndex.emplace_back(i);
								meshOptimizer.numVertices += mesh.numVertices;
								meshOptimizer.numElements += mesh.numElements;
								meshOptimizer.strName = mesh.name.c_str();
							}
						}

						vecModelSubset.resize(umapMesh.size());
						vecMaterial.resize(umapMesh.size());
						vecVertices.resize(nVertexCount);
						vecIndices.resize(nIndexCount);

						uint32_t nStartVertex = 0;
						uint32_t nStartIndex = 0;

						int nIndex = 0;
						for (auto iter : umapMesh)
						{
							const std::tuple<std::string, std::string, std::string> key = iter.first;
							const MeshOptimizer& meshOptimizer = iter.second;

							vecModelSubset[nIndex].strName = meshOptimizer.strName;
							vecModelSubset[nIndex].nStartIndex = nStartIndex;
							vecModelSubset[nIndex].nIndexCount = meshOptimizer.numElements;
							vecModelSubset[nIndex].nMaterialID = nIndex;

							MaterialInfo materianInfo;
							materianInfo.strPath = strPath;
							materianInfo.strName = meshOptimizer.strName;
							materianInfo.strTextureNameArray[EmMaterial::eAlbedo] = std::get<0>(key).c_str();
							materianInfo.strTextureNameArray[EmMaterial::eNormal] = std::get<1>(key).c_str();
							vecMaterial[nIndex] = CreateMaterial(&materianInfo);

							++nIndex;

							const std::vector<int>& vecMeshIndex = meshOptimizer.vecMeshIndex;
							size_t nSize = vecMeshIndex.size();
							for (size_t i = 0; i < nSize; ++i)
							{
								XPS_Mesh& mesh = vecMeshs[vecMeshIndex[i]];

								for (uint32_t j = 0; j < mesh.numVertices; ++j)
								{
									vecVertices[nStartVertex + j].pos = mesh.vertices[j].vertex;
									vecVertices[nStartVertex + j].uv = mesh.vertices[j].texCoord[0];
									vecVertices[nStartVertex + j].normal = mesh.vertices[j].normal;
									vecVertices[nStartVertex + j].boneWeight.x = mesh.vertices[j].boneWeight[0];
									vecVertices[nStartVertex + j].boneWeight.y = mesh.vertices[j].boneWeight[1];
									vecVertices[nStartVertex + j].boneWeight.z = mesh.vertices[j].boneWeight[2];

									vecVertices[nStartVertex + j].boneIndices[0] = mesh.vertices[j].boneIndex[0];
									vecVertices[nStartVertex + j].boneIndices[1] = mesh.vertices[j].boneIndex[1];
									vecVertices[nStartVertex + j].boneIndices[2] = mesh.vertices[j].boneIndex[2];
									vecVertices[nStartVertex + j].boneIndices[3] = mesh.vertices[j].boneIndex[3];
								}

								for (uint32_t j = 0; j < mesh.numElements; ++j)
								{
									vecIndices[nStartIndex + j] = nStartVertex + mesh.elements[j];
								}

								nStartVertex += mesh.numVertices;
								nStartIndex += mesh.numElements;
							}
						}
					}
					else
					{
						vecModelSubset.resize(nMeshCount);
						vecMaterial.resize(nMeshCount);

						uint32_t nVertexCount = 0;
						uint32_t nIndexCount = 0;
						for (uint32_t i = 0; i < nMeshCount; ++i)
						{
							XPS_Mesh& mesh = vecMeshs[i];

							nVertexCount += mesh.numVertices;
							nIndexCount += mesh.numElements;
						}

						vecVertices.resize(nVertexCount);
						vecIndices.resize(nIndexCount);

						uint32_t nStartVertex = 0;
						uint32_t nStartIndex = 0;

						for (uint32_t i = 0; i < nMeshCount; ++i)
						{
							XPS_Mesh& mesh = vecMeshs[i];

							vecModelSubset[i].strName = mesh.name.c_str();
							vecModelSubset[i].nStartIndex = nStartIndex;
							vecModelSubset[i].nIndexCount = mesh.numElements;
							vecModelSubset[i].nMaterialID = i;

							MaterialInfo materianInfo;
							materianInfo.strPath = strPath;
							materianInfo.strName.Format("material_%s", mesh.name.c_str());

							if (mesh.numTextures == 1)
							{
								materianInfo.strTextureNameArray[EmMaterial::eAlbedo] = file::GetFileName(mesh.textures[0].filename).c_str();
							}
							else if (mesh.numTextures == 3)
							{
								materianInfo.strTextureNameArray[EmMaterial::eAlbedo] = file::GetFileName(mesh.textures[0].filename).c_str();
								materianInfo.strTextureNameArray[EmMaterial::eNormal] = file::GetFileName(mesh.textures[1].filename).c_str();
							}
							else if (mesh.numTextures == 4)
							{
								materianInfo.strTextureNameArray[EmMaterial::eAlbedo] = file::GetFileName(mesh.textures[0].filename).c_str();
								materianInfo.strTextureNameArray[EmMaterial::eNormal] = file::GetFileName(mesh.textures[2].filename).c_str();
							}

							vecMaterial[i] = CreateMaterial(&materianInfo);

							for (uint32_t j = 0; j < mesh.numVertices; ++j)
							{
								vecVertices[nStartVertex + j].pos = mesh.vertices[j].vertex;
								vecVertices[nStartVertex + j].uv = mesh.vertices[j].texCoord[0];
								vecVertices[nStartVertex + j].normal = mesh.vertices[j].normal;
								vecVertices[nStartVertex + j].boneWeight.x = mesh.vertices[j].boneWeight[0];
								vecVertices[nStartVertex + j].boneWeight.y = mesh.vertices[j].boneWeight[1];
								vecVertices[nStartVertex + j].boneWeight.z = mesh.vertices[j].boneWeight[2];

								vecVertices[nStartVertex + j].boneIndices[0] = mesh.vertices[j].boneIndex[0];
								vecVertices[nStartVertex + j].boneIndices[1] = mesh.vertices[j].boneIndex[1];
								vecVertices[nStartVertex + j].boneIndices[2] = mesh.vertices[j].boneIndex[2];
								vecVertices[nStartVertex + j].boneIndices[3] = mesh.vertices[j].boneIndex[3];
							}

							for (uint32_t j = 0; j < mesh.numElements; ++j)
							{
								vecIndices[nStartIndex + j] = nStartVertex + mesh.elements[j];
							}

							nStartVertex += mesh.numVertices;
							nStartIndex += mesh.numElements;
						}
					}


					IVertexBuffer* pVertexBuffer = CreateVertexBuffer(reinterpret_cast<uint8_t*>(vecVertices.data()), static_cast<uint32_t>(sizeof(VertexPosTexNorWeiIdx) * vecVertices.size()), static_cast<uint32_t>(vecVertices.size()));
					pSkinnedNode->SetVertexBuffer(pVertexBuffer);
					ReleaseResource(&pVertexBuffer);

					IIndexBuffer* pIndexBuffer = CreateIndexBuffer(reinterpret_cast<uint8_t*>(vecIndices.data()), static_cast<uint32_t>(sizeof(VertexPosTexNorWeiIdx) * vecIndices.size()), static_cast<uint32_t>(vecIndices.size()));
					pSkinnedNode->SetIndexBuffer(pIndexBuffer);
					ReleaseResource(&pIndexBuffer);

					pSkinnedNode->AddModelSubsets(vecModelSubset);

					pSkinnedNode->AddMaterialArray(vecMaterial.data(), vecMaterial.size());
					for (auto& pMaterial : vecMaterial)
					{
						ReleaseResource(&pMaterial);
					}

					Collision::AABB aabb;
					Collision::AABB::CreateFromPoints(aabb, vecVertices.size(), &vecVertices[0].pos, VertexPosTexNorWeiIdx::Size());

					pSkinnedNode->SetOriginAABB(aabb);

					pModel->AddNode(pSkinnedNode, pSkinnedNode->GetName(), true);

					std::unordered_map<uint16_t, std::pair<string::StringID, math::Matrix>> umapBones;
					umapBones.reserve(nBoneCount);

					std::vector<string::StringID> vecBoneNames;
					vecBoneNames.resize(nBoneCount);

					Skeleton* pSkeleton = static_cast<Skeleton*>(pModel->GetSkeleton());

					for (uint32_t i = 0; i < nBoneCount; ++i)
					{
						XPS_Bone& bone = vecBones[i];

						vecBoneNames[i] = bone.name.c_str();

						math::Matrix matDefaultMotionData = math::Matrix::CreateTranslation(bone.defaultPositionX, bone.defaultPositionY, bone.defaultPositionZ);
						math::Matrix matParent;

						string::StringID strParentBoneName;
						auto iter = umapBones.find(bone.parentIndex);
						if (iter != umapBones.end())
						{
							strParentBoneName = iter->second.first;
							matParent = iter->second.second;
						}

						math::Matrix matDefault = matParent.Invert() * matDefaultMotionData;
						math::Matrix matMotionOffset = matDefaultMotionData.Invert();

						string::StringID strBoneName = bone.name.c_str();
						if (strParentBoneName.empty() == false)
						{
							pSkeleton->CreateBone(strParentBoneName, strBoneName, matMotionOffset, matDefault);
						}
						else
						{
							pSkeleton->CreateBone(strBoneName, matMotionOffset, matDefault);
						}

						umapBones.emplace(static_cast<uint16_t>(i), std::make_pair(strBoneName, matDefaultMotionData));
					}

					pSkinnedNode->SetBoneNameList(vecBoneNames);
				}

				Skeleton* pSkeleton = static_cast<Skeleton*>(pModel->GetSkeleton());

				const uint32_t nNodeCount = pModel->GetNodeCount();
				for (uint32_t i = 0; i < nNodeCount; ++i)
				{
					IModelNode* pModelNode = pModel->GetNode(i);
					if (pModelNode == nullptr)
						continue;

					if (pModelNode->GetType() != EmModelNode::eSkinned)
						continue;

					ModelNodeSkinned* pSkinnedNode = static_cast<ModelNodeSkinned*>(pModelNode);

					const uint32_t nSkinnedBoneCount = pSkinnedNode->GetBoneCount();

					std::vector<string::StringID> vecBoneNames;
					vecBoneNames.resize(nSkinnedBoneCount);

					for (uint32_t j = 0; j < nSkinnedBoneCount; ++j)
					{
						vecBoneNames[j] = pSkinnedNode->GetBoneName(j);
					}

					pSkeleton->SetSkinnedList(pSkinnedNode->GetName(), vecBoneNames.data(), vecBoneNames.size());
				}

				file.Close();

				return true;
			}

			bool LoadMotion(Motion* pMotion, const char* strFilePath)
			{
				file::Stream file;
				if (file.Open(strFilePath, file::eRead | file::eBinary) == false)
					return false;

				pMotion->SetInfo(0.f, 0.f, 0.f);

				while (file.Eof() == false)
				{
					std::string strTemp;
					file.ReadLine(strTemp);

					if (strTemp.empty() == false)
					{
						std::vector<Motion::Keyframe> vecKeyframes;

						string::StringID strName;
						math::Vector3 f3Rotation;

						Motion::Keyframe keyframe;

						std::size_t nFindPos = strTemp.find(": ");
						if (nFindPos == std::string::npos)
						{
							assert(false);
						}

						strName = strTemp.substr(0, nFindPos).c_str();

						std::string strKeyframeData = strTemp.substr(nFindPos + 2, strTemp.size() - (nFindPos + 2));

						sscanf_s(strKeyframeData.c_str(), "%f %f %f %f %f %f %f %f %f",
							&f3Rotation.x, &f3Rotation.y, &f3Rotation.z,
							&keyframe.transform.position.x, &keyframe.transform.position.y, &keyframe.transform.position.z,
							&keyframe.transform.scale.x, &keyframe.transform.scale.y, &keyframe.transform.scale.z);

						keyframe.transform.rotation = math::Quaternion::CreateFromYawPitchRoll(f3Rotation.y, f3Rotation.x, f3Rotation.z);

						vecKeyframes.emplace_back(keyframe);

						pMotion->AddBoneKeyframes(strName, vecKeyframes);
					}
				}

				file.Close();

				return true;
			}
		}
	}
}