#include "stdafx.h"
#include "ObjImporter.h"
#include "GeometryModel.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

#include "ModelInterface.h"

#include "Model.h"
#include "ModelNodeStatic.h"

#include "MtlImporter.h"

#include <boost/functional/hash.hpp>

namespace EastEngine
{
	namespace Graphics
	{
		SObjImporter::SObjImporter()
			: m_pMtlImporter(new CMtlImporter)
		{
		}

		SObjImporter::~SObjImporter()
		{
			SafeReleaseDelete(m_pMtlImporter);
		}

		bool SObjImporter::LoadModel(IModel* pModel, const char* strFileName, const float fScaleFactor, uint32_t nLodMax, const LODReductionRate* pLodReductionRate)
		{
			if (pModel == nullptr)
				return false;

			if (m_pMtlImporter != nullptr)
			{
				m_pMtlImporter->Release();
			}

			File::FileStream file;
			if (file.Open(strFileName, File::EmState::eNone) == false)
				return false;

			if (loadModelData(file, fScaleFactor) == false)
			{
				ClearData();
				return false;
			}

			file.Close();

			return buildModel(pModel, nLodMax, pLodReductionRate);
		}

		void SObjImporter::ClearData()
		{
			m_objData.Clear();

			if (m_pMtlImporter != nullptr)
			{
				m_pMtlImporter->Release();
			}
		}

		IMaterial* SObjImporter::GetMaterial(const String::StringID& strName)
		{
			if (m_pMtlImporter == nullptr)
				return nullptr;

			return m_pMtlImporter->GetMaterial(strName);
		}

		bool SObjImporter::loadModelData(File::FileStream& file, const float fScaleFactor)
		{
			file.Seekg(0, std::ios::beg);

			m_objData.Clear();

			std::string temp;
			while (file.Eof() == false)
			{
				file >> temp;

				if (temp == "#")
				{
					file.GetLine(temp);
					continue;
				}
				else if (temp == "mtllib")
				{
					std::string strFileName;
					file.GetLine(strFileName);

					if (strFileName.empty())
						continue;

					strFileName = strFileName.substr(1, strFileName.length());

					if (m_pMtlImporter->Init(strFileName.c_str(), File::GetFilePath(file.GetPath().c_str()).c_str()) == false)
					{
						LOG_WARNING("Cant Load Mtl File : %s", strFileName.c_str());
					}
				}
				else if (temp == "g")
				{
					std::string strFileName;
					file.GetLine(strFileName);

					if (strFileName.empty())
						continue;

					strFileName = strFileName.substr(1, strFileName.length());

					if (m_objData.GetCurGroupData().strGroupName.empty() == true)
					{
						m_objData.GetCurGroupData().strGroupName = strFileName;
					}
					else
					{
						ObjGroupData groupData;
						groupData.strGroupName = strFileName;

						m_objData.vecGroupData.push_back(groupData);
					}
				}
				else if (temp == "usemtl")
				{
					std::string strFileName;
					file.GetLine(strFileName);

					if (strFileName.empty())
						continue;

					strFileName = strFileName.substr(1, strFileName.length());

					if (m_objData.GetCurGroupData().GetCurSubModel().strMtlName.empty() == true)
					{
						m_objData.GetCurGroupData().GetCurSubModel().strMtlName = strFileName;
					}
					else
					{
						ObjSubModel subModel;
						subModel.strMtlName = strFileName;

						m_objData.GetCurGroupData().vecSubModel.push_back(subModel);
					}
				}
				else if (temp == "v")
				{
					Math::Vector3 f3Vertex;
					file.Read(&f3Vertex.x, 3);

					// Invert the Z vertex to change to left hand system.
					f3Vertex.z = f3Vertex.z * -1.f;
					f3Vertex *= fScaleFactor;

					m_objData.vecVertex.push_back(f3Vertex);
				}
				else if (temp == "vt")
				{
					Math::Vector2 f2Texcoord;
					file.Read(&f2Texcoord.x, 2);

					f2Texcoord.y = 1.f - f2Texcoord.y;

					m_objData.vecTexcoord.push_back(f2Texcoord);
				}
				else if (temp == "vn")
				{
					Math::Vector3 f3Normal;
					file.Read(&f3Normal.x, 3);

					f3Normal.z = f3Normal.z * -1.f;

					m_objData.vecNormal.push_back(f3Normal);
				}
				else if (temp == "f")
				{
					std::string strFace;
					file.GetLine(strFace);

					std::vector<std::string> vecFace = String::Tokenizer(strFace, " ");
					if (vecFace.empty() == false)
					{
						// 귀찮아서 급하게 만든거 나중에 여유되면 고치삼
						auto GetFace = [](const std::string& str) -> std::array<int, 3>
						{
							std::array<int, 3> indexArray;
							indexArray.fill(0);
							int nPrevPos = 0;
							int nIndex = 0;
							for (size_t i = 0; i < str.size(); ++i)
							{
								if (str[i] == '/')
								{
									std::string t = str.substr(nPrevPos, i - nPrevPos).c_str();
									if (t == "/")
									{
										indexArray[nIndex] = 0;
									}
									else if (t.empty() == false)
									{
										indexArray[nIndex] = std::stoi(t.c_str());
									}
									else
									{
										indexArray[nIndex] = 0;
									}
									nPrevPos = i + 1;
									++nIndex;
								}

								if (i == str.size() - 1)
								{
									indexArray[nIndex] = String::ToValue<int>(str.substr(nPrevPos).c_str());
								}
							}

							return indexArray;
						};

						{
							FaceType faceType;
							uint32_t* pV = &faceType.vIdx.x;
							uint32_t* pT = &faceType.tIdx.x;
							uint32_t* pN = &faceType.nIdx.x;

							for (int i = 0; i < 3; ++i)
							{
								std::array<int, 3> indexArray = GetFace(vecFace[i]);
								pV[2 - i] = indexArray[0];
								pT[2 - i] = indexArray[1];
								pN[2 - i] = indexArray[2];

								//std::vector<std::string> vecValue = String::Tokenizer(vecFace[i], "/");

								//pV[2 - i] = vecValue[0].empty() == false ? String::ToValue<int>(vecValue[0].c_str()) : 0;
								//pT[2 - i] = vecValue[1].empty() == false ? String::ToValue<int>(vecValue[1].c_str()) : 0;
								//pN[2 - i] = vecValue[2].empty() == false ? String::ToValue<int>(vecValue[2].c_str()) : 0;
							}

							m_objData.GetCurGroupData().GetCurSubModel().vecFaceType.push_back(faceType);
						}

						if (vecFace.size() == 4)
						{
							FaceType faceType;
							uint32_t* pV = &faceType.vIdx.x;
							uint32_t* pT = &faceType.tIdx.x;
							uint32_t* pN = &faceType.nIdx.x;

							std::array<int, 3> indexArray = GetFace(vecFace[0]);
							pV[2] = indexArray[0];
							pT[2] = indexArray[1];
							pN[2] = indexArray[2];

							indexArray = GetFace(vecFace[2]);
							pV[1] = indexArray[0];
							pT[1] = indexArray[1];
							pN[1] = indexArray[2];

							indexArray = GetFace(vecFace[3]);
							pV[0] = indexArray[0];
							pT[0] = indexArray[1];
							pN[0] = indexArray[2];

							//std::vector<std::string> vecValue = String::Tokenizer(vecFace[0], "/");
							//pV[2] = vecValue[0].empty() == false ? String::ToValue<int>(vecValue[0].c_str()) : 0;
							//pT[2] = vecValue[1].empty() == false ? String::ToValue<int>(vecValue[1].c_str()) : 0;
							//pN[2] = vecValue[2].empty() == false ? String::ToValue<int>(vecValue[2].c_str()) : 0;
							//
							//vecValue = String::Tokenizer(vecFace[2], "/");
							//pV[1] = vecValue[0].empty() == false ? String::ToValue<int>(vecValue[0].c_str()) : 0;
							//pT[1] = vecValue[1].empty() == false ? String::ToValue<int>(vecValue[1].c_str()) : 0;
							//pN[1] = vecValue[2].empty() == false ? String::ToValue<int>(vecValue[2].c_str()) : 0;
							//
							//vecValue = String::Tokenizer(vecFace[3], "/");
							//pV[0] = vecValue[0].empty() == false ? String::ToValue<int>(vecValue[0].c_str()) : 0;
							//pT[0] = vecValue[1].empty() == false ? String::ToValue<int>(vecValue[1].c_str()) : 0;
							//pN[0] = vecValue[2].empty() == false ? String::ToValue<int>(vecValue[2].c_str()) : 0;

							m_objData.GetCurGroupData().GetCurSubModel().vecFaceType.push_back(faceType);
						}
					}
				}
			}

			m_objData.strObjName = File::GetFileNameWithoutExtension(file.GetPath().c_str()).c_str();

			if (m_objData.Empty() == true)
				return false;

			m_objData.emObjImportType |= m_objData.vecVertex.empty() == false ? EmObjVertex::eVertex : 0;
			m_objData.emObjImportType |= m_objData.vecTexcoord.empty() == false ? EmObjVertex::eTexcoord : 0;
			m_objData.emObjImportType |= m_objData.vecNormal.empty() == false ? EmObjVertex::eNormal : 0;

			return true;
		}

		bool SObjImporter::buildModel(IModel* pIModel, uint32_t nLodMax, const LODReductionRate* pLodReductionRate)
		{
			Model* pModel = static_cast<Model*>(pIModel);

			LODReductionRate lodReductionRate;
			if (pLodReductionRate != nullptr)
			{
				lodReductionRate = *pLodReductionRate;
			}

			for (auto& iter : m_objData.vecGroupData)
			{
				uint32_t nFaceCount = 0;
				for (auto& iter_sub : iter.vecSubModel)
				{
					nFaceCount += iter_sub.vecFaceType.size();
				}

				std::vector<VertexPosTexNor> vecVertices[eMaxLod];
				std::vector<uint32_t> vecIndices[eMaxLod];

				IVertexBuffer* pVertexBuffer[eMaxLod] = { nullptr, };
				IIndexBuffer* pIndexBuffer[eMaxLod] = { nullptr, };

				std::vector<IMaterial*> vecMaterial;
				std::vector<ModelSubset> vecModelSubsets[eMaxLod];

				uint32_t nStartIdx[eMaxLod] = { 0, };
				uint32_t nStartVertex[eMaxLod] = { 0, };

				std::string strName;
				for (auto& iter_sub : iter.vecSubModel)
				{
					int nMtrlIdx = -1;
					if (iter_sub.strMtlName.empty() == false)
					{
						IMaterial* pMaterial = GetMaterial(iter_sub.strMtlName.c_str());
						if (pMaterial != nullptr)
						{
							bool bFind = false;
							for (auto& iter_mtrl : vecMaterial)
							{
								++nMtrlIdx;

								if (iter_mtrl->GetName() == pMaterial->GetName())
								{
									bFind = true;
									break;
								}
							}

							if (bFind == false)
							{
								vecMaterial.push_back(pMaterial);
								nMtrlIdx = vecMaterial.size() - 1;
							}
						}
					}

					strName.append(iter.strGroupName.c_str());
					strName.append("_");

					std::vector<VertexPosTexNor> vecVertexIn;
					std::vector<uint32_t> vecIndexIn;

					auto GetKey = [](uint32_t vIdx, uint32_t tIdx, uint32_t nIdx)
					{
						std::tuple<uint32_t, uint32_t, uint32_t> key(vIdx, tIdx, nIdx);

						return boost::hash<std::tuple<uint32_t, uint32_t, uint32_t>>{}(key);
					};

					std::unordered_map<std::size_t, uint32_t> umap;

					auto func = [&](uint32_t vIdx, uint32_t tIdx, uint32_t nIdx) -> size_t
					{
						size_t ret = ((size_t) - 1);
						auto key = GetKey(vIdx, tIdx, nIdx);
						auto iter = umap.find(key);
						if (iter == umap.end())
						{
							VertexPosTexNor v;
							v.pos = m_objData.vecVertex[vIdx];
							if (UINT32_MAX != tIdx)
							{
								v.uv = m_objData.vecTexcoord[tIdx];
							}
							if (UINT32_MAX != nIdx)
							{
								v.normal = m_objData.vecNormal[nIdx];
							}

							umap.emplace(key, vecVertexIn.size());

							ret = vecVertexIn.size();
							vecVertexIn.push_back(v);
						}
						else
						{
							ret = iter->second;
						}

						return ret;
					};

					size_t nSize = iter_sub.vecFaceType.size();
					for (size_t i = 0; i < nSize; ++i)
					{
						uint32_t vIdx = iter_sub.vecFaceType[i].vIdx.x - 1;
						uint32_t tIdx = iter_sub.vecFaceType[i].tIdx.x - 1;
						uint32_t nIdx = iter_sub.vecFaceType[i].nIdx.x - 1;

						vecIndexIn.emplace_back(func(vIdx, tIdx, nIdx));

						vIdx = iter_sub.vecFaceType[i].vIdx.y - 1;
						tIdx = iter_sub.vecFaceType[i].tIdx.y - 1;
						nIdx = iter_sub.vecFaceType[i].nIdx.y - 1;

						vecIndexIn.emplace_back(func(vIdx, tIdx, nIdx));

						vIdx = iter_sub.vecFaceType[i].vIdx.z - 1;
						tIdx = iter_sub.vecFaceType[i].tIdx.z - 1;
						nIdx = iter_sub.vecFaceType[i].nIdx.z - 1;

						vecIndexIn.emplace_back(func(vIdx, tIdx, nIdx));
					}

					uint32_t nLod = nLodMax;
					if (vecIndexIn.size() < 50)
					{
						nLod = 0;
					}

					nSize = nLod + 1;
					for (size_t i = 0; i < nSize; ++i)
					{
						std::vector<VertexPosTexNor> vecVertexOut;
						std::vector<uint32_t> vecIndexOut;

						if (GeometryModel::Simplify::GenerateSimplificationMesh(vecVertexIn, vecIndexIn,
							vecVertexOut, vecIndexOut, lodReductionRate.fLv[i]) == false)
							continue;

						std::copy(vecVertexOut.begin(), vecVertexOut.end(), std::back_inserter(vecVertices[i]));
						for (size_t j = 0; j < vecIndexOut.size(); ++j)
						{
							vecIndices[i].push_back(vecIndexOut[j] + nStartVertex[i]);
						}

						ModelSubset modelSubset;
						modelSubset.nStartIndex = nStartIdx[i];
						modelSubset.nMaterialID = nMtrlIdx;
						modelSubset.nIndexCount = vecIndexOut.size();
						vecModelSubsets[i].push_back(modelSubset);

						nStartIdx[i] += vecIndexOut.size();
						nStartVertex[i] += vecVertexOut.size();
					}
				}

				auto FailFunc = [&]()
				{
					for (uint32_t i = 0; i < eMaxLod; ++i)
					{
						SafeDelete(pVertexBuffer[i]);
						SafeDelete(pIndexBuffer[i]);
					}
				};

				auto SuccessFunc = [&]()
				{
					for (uint32_t i = 0; i < eMaxLod; ++i)
					{
						vecVertices[i].clear();
						vecIndices[i].clear();
					}
				};

				uint32_t nLodMax_Calc = 0;
				uint32_t nSize = nLodMax + 1;
				for (uint32_t i = 0; i < nSize; ++i)
				{
					if (vecVertices[i].empty() || vecIndices[i].empty())
						break;

					SafeDelete(pVertexBuffer[i]);
					pVertexBuffer[i] = IVertexBuffer::Create(VertexPosTexNor::Format(), vecVertices[i].size(), &vecVertices[i].front(), D3D11_USAGE_DYNAMIC, IVertexBuffer::eSaveVertexPos/* | IVertexBuffer::eSaveVertexClipSpace*/);
					if (pVertexBuffer[i] == nullptr)
					{
						FailFunc();
						return false;
					}

					SafeDelete(pIndexBuffer[i]);
					pIndexBuffer[i] = IIndexBuffer::Create(vecIndices[i].size(), &vecIndices[i].front(), D3D11_USAGE_DYNAMIC, IIndexBuffer::eSaveRawValue);
					if (pIndexBuffer[i] == nullptr)
					{
						FailFunc();
						return false;
					}

					nLodMax_Calc = i;
				}

				ModelNodeStatic* pModelStatic = new ModelNodeStatic(nLodMax_Calc);
				pModelStatic->SetNodeName(iter.strGroupName.c_str());

				nSize = nLodMax_Calc + 1;
				for (uint32_t i = 0; i < nSize; ++i)
				{
					pModelStatic->SetVertexBuffer(pVertexBuffer[i], i);
					pModelStatic->SetIndexBuffer(pIndexBuffer[i], i);
					pModelStatic->AddModelSubsets(vecModelSubsets[i], i);
				}

				pModelStatic->AddMaterialArray(&vecMaterial.front(), vecMaterial.size());
				pModel->AddNode(pModelStatic, iter.strGroupName.c_str(), true);

				SuccessFunc();
			}

			ClearData();

			return true;
		}
	}
}