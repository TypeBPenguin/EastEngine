#include "stdafx.h"
#include "Terrain.h"

#ifndef NEW_TERRAIN
#include "CommonLib/FileStream.h"

#include "DirectX/Device.h"
#include "DirectX/CameraManager.h"

namespace StrID
{
	RegisterStringID(Terrain_Physics);
}

namespace EastEngine
{
	namespace GameObject
	{
		Terrain::Terrain()
			: m_nTerrainWidth(0)
			, m_nTerrainHeight(0)
			, m_nCellWidth(0)
			, m_nCellHeight(0)
			, m_nCellCount(0)
			, m_pPhysics(nullptr)
			, m_nLastCell(0)
			, m_isShowCellLine(false)
			, m_isLoadComplete(false)
		{
		}

		Terrain::~Terrain()
		{
			SafeDelete(m_pPhysics);

			uint32_t nSize = m_veTerrainCells.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				m_veTerrainCells[i].Release();
			}
			m_veTerrainCells.clear();

			m_vecHeight.clear();
			m_vecHeightMap.clear();

			m_vecVertices.clear();
			m_vecIndices.clear();

			std::for_each(m_vecMaterial.begin(), m_vecMaterial.end(), DeleteSTLObject());
			m_vecMaterial.clear();
		}

		bool Terrain::Init(const char* strSetupFile)
		{
			std::string strFile = strSetupFile;
			auto task = Concurrency::create_task(std::bind(&Terrain::initByThread, this, strFile));

			/*Physics::RigidBodyProperty prop;
			if (loadSetup(strSetupFile, prop) == false)
			{
				Release();
				return false;
			}

			if (initTerrain() == false)
			{
				Release();
				return false;
			}

			prop.fMass = 0.f;
			prop.strName = "Terrain";
			prop.emPhysicsShapeType = Physics::EmPhysicsShapeType::TriangleMesh;
			prop.nCollisionFlag = Physics::CF_STATIC_OBJECT;

			Math::Vector2 vSize((float)(m_nTerrainWidth), (float)(m_nTerrainHeight));

			m_pPhysics = PhysicsInst->CreateRigidBodyTriangleMesh(Vector3::Zero, &m_vecVertices, &m_vecIndices, prop);
			m_pPhysics->SetVisible(false);*/

			return true;
		}

		bool Terrain::initByThread(std::string strSetupFile)
		{
			Physics::RigidBodyProperty prop;
			if (loadSetup(strSetupFile.c_str(), prop) == false)
				return false;

			if (initTerrain() == false)
				return false;

			prop.fMass = 0.f;
			prop.strName = StrID::Terrain_Physics;
			prop.nCollisionFlag = Physics::EmCollision::eStaticObject;
			prop.shapeInfo.SetTriangleMesh(&m_vecVertices[0].pos, m_vecVertices.size(), &m_vecIndices[0], m_vecIndices.size());

			m_pPhysics = Physics::RigidBody::Create(prop);

			m_vecVertices.clear();
			m_vecIndices.clear();

			m_vecHeight.clear();
			m_vecHeightMap.clear();

			m_isLoadComplete = true;

			return true;
		}

		void Terrain::Update(float fElapsedTime, const Math::Vector3* pPlayerPosition)
		{
			if (m_isLoadComplete == false)
				return;

			Graphics::Camera* pMainCamera = Graphics::CameraManager::GetInstance()->GetMainCamera();
			if (pMainCamera == nullptr)
				return;

			const Math::Vector3& vPos = pPlayerPosition != nullptr ? *pPlayerPosition : pMainCamera->GetPosition();

			int nCellIdx = -1;
			m_nLastCell = std::min(m_nLastCell, m_veTerrainCells.size());

			if (m_veTerrainCells[m_nLastCell].IsInsideCell(vPos))
			{
				nCellIdx = m_nLastCell;
			}
			else
			{
				uint32_t nSize = m_veTerrainCells.size();
				for (uint32_t i = 0; i < nSize; ++i)
				{
					if (m_veTerrainCells[i].IsInsideCell(vPos))
					{
						m_nLastCell = nCellIdx = i;
						break;
					}
				}
			}

			if (nCellIdx == -1)
			{
				float fMinDist = D3D11_FLOAT32_MAX;
				uint32_t nSize = m_veTerrainCells.size();
				for (uint32_t i = 0; i < nSize; ++i)
				{
					if (pMainCamera->IsFrustumContains(m_veTerrainCells[i].GetBoundingBox()) == Collision::EmContainment::eDisjoint)
						continue;

					Math::Vector3 vDist = vPos - m_veTerrainCells[i].GetBoundingBox().Center;
					float fDist = vDist.LengthSquared();
					if (fDist < fMinDist)
					{
						nCellIdx = i;
						fMinDist = fDist;
					}
				}
			}

			if (nCellIdx >= 0)
			{
				const Math::UInt2& n2NodeIdx = m_veTerrainCells[nCellIdx].GetIndex();
				uint32_t nCellRowCount = (m_nTerrainWidth - 1) / (m_nCellWidth - 1);

				for (int i = 0; i < 9; ++i)
				{
					int x = (i % 3) - 1;
					int y = (i / 3) - 1;

					Math::UInt2 n2ArroundNodeIdx(n2NodeIdx.x + x, n2NodeIdx.y + y);

					if (n2ArroundNodeIdx.x < 0 || n2ArroundNodeIdx.x >= nCellRowCount ||
						n2ArroundNodeIdx.y < 0 || n2ArroundNodeIdx.y >= nCellRowCount)
						continue;

					int nArroundCellIdx = nCellIdx + (x + (y * nCellRowCount));

					if (pMainCamera->IsFrustumContains(m_veTerrainCells[nArroundCellIdx].GetBoundingBox()) == Collision::EmContainment::eDisjoint)
						continue;

					m_veTerrainCells[nArroundCellIdx].Update(fElapsedTime, m_vecMaterial, m_isShowCellLine);
				}
			}
			else
			{
				uint32_t nSize = m_veTerrainCells.size();
				for (uint32_t i = 0; i < nSize; ++i)
				{
					if (pMainCamera->IsFrustumContains(m_veTerrainCells[i].GetBoundingBox()) == Collision::EmContainment::eDisjoint)
						continue;

					if (m_veTerrainCells[i].IsInsideCell(vPos) == false)
						continue;

					m_veTerrainCells[i].Update(fElapsedTime, m_vecMaterial, m_isShowCellLine);
				}
			}

			m_pPhysics->Update(fElapsedTime);
		}

		float Terrain::GetHeightAtPosition(float fPosX, float fPosZ)
		{
			int nCellIdx = -1;
			uint32_t nSize = m_veTerrainCells.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				Math::Vector3 vMin;
				Math::Vector3 vMax;
				m_veTerrainCells[i].GetCellDimensions(vMin, vMax);

				// Check to see if the positions are in this cell.
				if ((fPosX < vMax.x) && (fPosX > vMin.x) && (fPosZ < vMax.z) && (fPosZ > vMin.z))
				{
					nCellIdx = i;
					i = nSize;
				}
			}

			// If we didn't find a cell then the input position is off the terrain grid.
			if (nCellIdx == -1)
				return 0.f;

			// If this is the right cell then check all the triangles in this cell to see what the height of the triangle at this position is.
			//uint32_t nCount = m_veTerrainCells[nCellIdx].GetVertexCount() / 3;
			//for (uint32_t i = 0; i < nCount; ++i)
			//{
			//	uint32_t nIdx = i * 3;

			//	Vector3 v0 = m_veTerrainCells[nCellIdx].GetVertex(nIdx);
			//	nIdx++;

			//	Vector3 v1 = m_veTerrainCells[nCellIdx].GetVertex(nIdx);
			//	nIdx++;

			//	Vector3 v2 = m_veTerrainCells[nCellIdx].GetVertex(nIdx);

			//	float fHeight = 0.f;
			//	// Check to see if this is the polygon we are looking for.
			//	if (checkHeightOfTriangle(fPosX, fPosZ, fHeight, v0, v1, v2))
			//		return fHeight;
			//}

			return 0.f;
		}

		bool Terrain::loadSetup(const char* strFile, Physics::RigidBodyProperty& physicsProp)
		{
			XML::CXmlDoc doc;
			if (doc.LoadFile(strFile) == false)
				return false;

			XML::CXmlNode node = doc.GetFirstChild("Class");
			if (node.IsVaild() == false)
				return false;

			XML::CXmlElement element = node.GetFirstChildElement("Terrain");
			if (element.IsVaild() == false)
				return false;

			m_nTerrainWidth = element.AttributeInt("Width");
			m_nTerrainHeight = element.AttributeInt("Height");

			m_nCellWidth = element.AttributeInt("CellWidth");
			m_nCellHeight = element.AttributeInt("CellHeight");

			for (XML::CXmlElement childElement = element.FirstChildElement();
				childElement.IsVaild() == true; childElement = childElement.NextSibling())
			{
				if (String::IsEquals(childElement.GetName(), "HeightMap"))
				{
					const char* strHeightMapFile = childElement.Attribute("File");
					if (strHeightMapFile == nullptr)
						return false;

					const char* strPath = childElement.Attribute("Path");
					if (strPath == nullptr)
						return false;

					std::string strScaling = childElement.Attribute("Scaling");
					auto slice = String::Tokenizer(strScaling, " ");

					float* fPos = &m_f3Scaling.x;
					uint32_t nSize = slice.size();
					for (uint32_t i = 0; i < nSize; ++i)
					{
						fPos[i] = (float)(atof(slice[i].c_str()));
					}

					if (loadHeightMap(strHeightMapFile, strPath) == false)
					{
						PRINT_LOG("Can't Load HeightMap : %s", strHeightMapFile);
						return false;
					}
				}
				else if (String::IsEquals(childElement.GetName(), "ColorMap"))
				{
					const char* strColorMapFile = childElement.Attribute("File");
					if (strColorMapFile == nullptr)
						return false;

					const char* strPath = childElement.Attribute("Path");
					if (strPath == nullptr)
						return false;

					if (loadColorMap(strColorMapFile, strPath) == false)
					{
						PRINT_LOG("Can't Load ColorMap : %s", strColorMapFile);
						return false;
					}
				}
				else if (String::IsEquals(childElement.GetName(), "RawHeightMap"))
				{
					const char* strRawHeightMapFile = childElement.Attribute("File");
					if (strRawHeightMapFile == nullptr)
						return false;

					const char* strPath = childElement.Attribute("Path");
					if (strPath == nullptr)
						return false;

					std::string strScaling = childElement.Attribute("Scaling");
					auto slice = String::Tokenizer(strScaling, " ");

					float* fPos = &m_f3Scaling.x;
					uint32_t nSize = slice.size();
					for (uint32_t i = 0; i < nSize; ++i)
					{
						fPos[i] = (float)(atof(slice[i].c_str()));
					}

					std::string strDefaultPos = childElement.Attribute("DefaultPos");
					slice = String::Tokenizer(strDefaultPos, " ");

					fPos = &m_f3DefaultPos.x;
					nSize = slice.size();
					for (uint32_t i = 0; i < nSize; ++i)
					{
						fPos[i] = (float)(atof(slice[i].c_str()));
					}

					if (loadRawHeightmap(strRawHeightMapFile, strPath) == false)
					{
						PRINT_LOG("Can't Load RawHeightMap : %s", strRawHeightMapFile);
						return false;
					}
				}
				else if (String::IsEquals(childElement.GetName(), "Material"))
				{
					std::string strName = childElement.Attribute("Name");
					//std::string strName = childElement.Attribute("Group");

					Graphics::MaterialInfo info;

					auto vecSlice = String::Tokenizer(childElement.Attribute("AlbedoColor"), " ");
					if (vecSlice.empty())
						continue;

					float* pfColor = &info.colorAlbedo.x;

					uint32_t nSize = vecSlice.size();
					for (uint32_t i = 0; i < nSize; ++i)
					{
						if (vecSlice[i].empty())
							continue;

						pfColor[i] = static_cast<float>(atof(vecSlice[i].c_str()));
					}

					vecSlice.clear();

					vecSlice = String::Tokenizer(childElement.Attribute("EmissiveColor"), " ");
					if (vecSlice.empty())
						continue;

					pfColor = &info.colorEmissive.x;

					nSize = vecSlice.size();
					for (uint32_t i = 0; i < nSize; ++i)
					{
						if (vecSlice[i].empty())
							continue;

						pfColor[i] = static_cast<float>(atof(vecSlice[i].c_str()));
					}

					vecSlice.clear();

					info.f4DisRoughMetEmi.x = childElement.AttributeFloat("Displacement");
					info.f4DisRoughMetEmi.y = childElement.AttributeFloat("Roughness");
					info.f4DisRoughMetEmi.z = childElement.AttributeFloat("Metalic");
					info.f4DisRoughMetEmi.w = childElement.AttributeFloat("Emissive");

					info.f4SurSpecTintAniso.x = childElement.AttributeFloat("Surface");
					info.f4SurSpecTintAniso.y = childElement.AttributeFloat("Specular");
					info.f4SurSpecTintAniso.z = childElement.AttributeFloat("SpecularTint");
					info.f4SurSpecTintAniso.w = childElement.AttributeFloat("Anisotropic");

					info.f4SheenTintClearcoatGloss.x = childElement.AttributeFloat("Sheen");
					info.f4SheenTintClearcoatGloss.y = childElement.AttributeFloat("SheenTint");
					info.f4SheenTintClearcoatGloss.z = childElement.AttributeFloat("Clearcoat");
					info.f4SheenTintClearcoatGloss.w = childElement.AttributeFloat("ClearcoatGloss");

					std::string strPath(File::GetDataPath());
					strPath.append(std::string(childElement.Attribute("Path")));

					info.strPath = strPath;

					info.strTextureNameArray[Graphics::EmMaterial::eAlbedo] = childElement.Attribute("TexAlbedo");
					info.strTextureNameArray[Graphics::EmMaterial::eNormal] = childElement.Attribute("TexNormal");
					info.strTextureNameArray[Graphics::EmMaterial::eDisplacement] = childElement.Attribute("TexDisplacement");
					info.strTextureNameArray[Graphics::EmMaterial::eRoughness] = childElement.Attribute("TexRoughness");
					info.strTextureNameArray[Graphics::EmMaterial::eMetallic] = childElement.Attribute("TexMetalic");
					info.strTextureNameArray[Graphics::EmMaterial::eSpecularColor] = childElement.Attribute("TexSpecularColor");
					info.strTextureNameArray[Graphics::EmMaterial::eEmissive] = childElement.Attribute("TexEmissive");
					info.strTextureNameArray[Graphics::EmMaterial::eSurface] = childElement.Attribute("TexSurface");
					info.strTextureNameArray[Graphics::EmMaterial::eSpecular] = childElement.Attribute("TexSpecular");
					info.strTextureNameArray[Graphics::EmMaterial::eSpecularTint] = childElement.Attribute("TexSpecularTint");
					info.strTextureNameArray[Graphics::EmMaterial::eAnisotropic] = childElement.Attribute("TexAnisotropic");
					info.strTextureNameArray[Graphics::EmMaterial::eSheen] = childElement.Attribute("TexSheen");
					info.strTextureNameArray[Graphics::EmMaterial::eSheenTint] = childElement.Attribute("TexSheenTint");
					info.strTextureNameArray[Graphics::EmMaterial::eClearcoat] = childElement.Attribute("TexClearcoat");
					info.strTextureNameArray[Graphics::EmMaterial::eClearcoatGloss] = childElement.Attribute("TexClearcoatGloss");

					// 아래는 나중에 전용 툴을 만들면 그때 쓰자
					//info.strTexNorDispMap = childElement.Attribute("TexNormalDis");
					//info.strTexRoughMetEmiMap = childElement.Attribute("TexRoughMetEmiMap");
					//info.strTexSurSpecTintAnisoMap = childElement.Attribute("TexSurSpecTintAnisoMap");
					//info.strTexSheenTintClearGlossMap = childElement.Attribute("TexSheenTintClearGlossMap");

					Graphics::IMaterial* pMaterial = Graphics::IMaterial::Create(&info);

					m_vecMaterial.push_back(pMaterial);
				}
				else if (String::IsEquals(childElement.GetName(), "Physics"))
				{
					physicsProp.fMass = childElement.AttributeFloat("Mass");
					physicsProp.fRestitution = childElement.AttributeFloat("Restitution");
					physicsProp.fFriction = childElement.AttributeFloat("Friction");
					physicsProp.fLinearDamping = childElement.AttributeFloat("LinearDamping");
					physicsProp.fAngularDamping = childElement.AttributeFloat("AngularDamping");
				}
			}

			return true;
		}

		bool Terrain::loadHeightMap(const char* strHeightMapFile, const char* strPath)
		{
			std::string strFilePath = File::GetDataPath();
			strFilePath.append(strPath);
			strFilePath.append(strHeightMapFile);

			// Start by creating the array structure to hold the height map data.
			// Open the bitmap map file in binary.

			FILE* filePtr = nullptr;
			if (fopen_s(&filePtr, strFilePath.c_str(), "rb") != 0)
				return false;

			BITMAPFILEHEADER bitmapFileHeader;
			// Read in the bitmap file header.
			if (fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1)
				return false;

			BITMAPINFOHEADER bitmapInfoHeader;
			// Read in the bitmap info header.
			if (fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1)
				return false;

			// Calculate the size of the bitmap image data.  
			// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
			uint32_t imageSize = m_nTerrainHeight * ((m_nTerrainWidth * 3) + 1);

			// Allocate memory for the bitmap image data.
			std::vector<unsigned char> vecBitmapImage;
			vecBitmapImage.resize(imageSize);

			// Read in the bitmap image data.
			if (fread(&vecBitmapImage[0], 1, imageSize, filePtr) != imageSize)
				return false;

			// Close the file.
			if (fclose(filePtr) != 0)
				return false;

			m_vecHeightMap.clear();
			m_vecHeightMap.resize(m_nTerrainWidth * m_nTerrainHeight);

			int k = 0;
			int nIdx = 0;
			float fZ = static_cast<float>(m_nTerrainHeight - 1);
			for (uint32_t i = 0; i < m_nTerrainHeight; ++i)
			{
				for (uint32_t j = 0; j < m_nTerrainWidth; ++j)
				{
					// Set the X and Z coordinates.
					m_vecHeightMap[nIdx].pos.x = static_cast<float>(j) * m_f3Scaling.x + m_f3DefaultPos.x;
					m_vecHeightMap[nIdx].pos.z = (-static_cast<float>(i) + fZ) * m_f3Scaling.z + m_f3DefaultPos.z;

					int nIdx_y = (m_nTerrainWidth * (m_nTerrainHeight - 1 - i)) + j;
					m_vecHeightMap[nIdx_y].pos.y = static_cast<float>(vecBitmapImage[k]) / m_f3Scaling.y + m_f3DefaultPos.y;

					++nIdx;

					k += 3;
				}

				++k;
			}

			vecBitmapImage.clear();

			return true;
		}

		bool Terrain::loadColorMap(const char* strColorMapFile, const char* strPath)
		{
			std::string strFilePath = File::GetDataPath();
			strFilePath.append(strPath);
			strFilePath.append(strColorMapFile);

			// Open the color map file in binary.
			FILE* filePtr = nullptr;
			if (fopen_s(&filePtr, strFilePath.c_str(), "rb") != 0)
				return false;

			// Read in the file header.
			BITMAPFILEHEADER bitmapFileHeader;
			if (fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1)
				return false;

			// Read in the bitmap info header.
			BITMAPINFOHEADER bitmapInfoHeader;
			if (fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1)
				return false;

			// Calculate the size of the bitmap image data.  Since this is non-divide by 2 dimensions (eg. 257x257) need to add extra byte to each line.
			uint32_t nImageSize = m_nTerrainHeight * ((m_nTerrainWidth * 3) + 1);

			// Allocate memory for the bitmap image data.
			std::vector<unsigned char> vecBitmapImage;
			vecBitmapImage.resize(nImageSize);

			// Read in the bitmap image data.
			if (fread(&vecBitmapImage[0], 1, nImageSize, filePtr) != nImageSize)
				return false;

			// Close the file.
			if (fclose(filePtr) != 0)
				return false;

			// Initialize the position in the image data buffer.
			int k = 0;
			// Read the image data into the color map portion of the height map structure.
			for (uint32_t j = 0; j < m_nTerrainHeight; ++j)
			{
				for (uint32_t i = 0; i < m_nTerrainWidth; ++i)
				{
					// Bitmaps are upside down so load bottom to top into the array.
					int nIdx = (m_nTerrainWidth * (m_nTerrainHeight - 1 - j)) + i;

					m_vecHeightMap[nIdx].color.w = 1.f;
					m_vecHeightMap[nIdx].color.z = static_cast<float>(vecBitmapImage[k] / 255.f);
					m_vecHeightMap[nIdx].color.y = static_cast<float>(vecBitmapImage[k + 1] / 255.f);
					m_vecHeightMap[nIdx].color.x = static_cast<float>(vecBitmapImage[k + 2] / 255.f);

					k += 3;
				}

				// Compensate for extra byte at end of each line in non-divide by 2 bitmaps (eg. 257x257).
				++k;
			}

			return true;
		}

		bool Terrain::loadRawHeightmap(const char* strRawHeightMapFile, const char* strPath)
		{
			std::string strFilePath = File::GetDataPath();
			strFilePath.append(strPath);
			strFilePath.append(strRawHeightMapFile);

			// Start by creating the array structure to hold the height map data.
			// Open the bitmap map file in binary.

			FILE* filePtr = nullptr;
			if (fopen_s(&filePtr, strFilePath.c_str(), "rb") != 0)
				return false;

			// Calculate the size of the bitmap image data.  
			// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
			uint32_t imageSize = m_nTerrainHeight * m_nTerrainWidth;

			m_vecHeightMap.clear();
			m_vecHeightMap.reserve(imageSize);

			m_vecHeight.clear();
			m_vecHeight.reserve(imageSize);

			std::vector<uint16_t> vecRawData;
			vecRawData.resize(imageSize);

			if (fread(&vecRawData[0], sizeof(uint16_t), imageSize, filePtr) != imageSize)
				return false;

			if (fclose(filePtr) != 0)
				return false;

			float fZ = static_cast<float>(m_nTerrainHeight - 1);

			std::deque<float> dequeHeight;

			uint32_t nIdx = 0;
			for (uint32_t i = 0; i < m_nTerrainHeight; ++i)
			{
				std::vector<float> vecHeight;
				vecHeight.reserve(m_nTerrainWidth);

				for (uint32_t j = 0; j < m_nTerrainWidth; ++j)
				{
					HeightMapVertex vertex;

					// Set the X and Z coordinates.
					vertex.pos.x = static_cast<float>(j) * m_f3Scaling.x + m_f3DefaultPos.x;
					vertex.pos.z = (-static_cast<float>(i) + fZ) * m_f3Scaling.z + m_f3DefaultPos.z;

					// Scale the height.
					vertex.pos.y = static_cast<float>(vecRawData[nIdx]) / m_f3Scaling.y + m_f3DefaultPos.y;

					if (m_f2MinMaxHeight.x > vertex.pos.y)
					{
						m_f2MinMaxHeight.x = vertex.pos.y;
					}

					if (m_f2MinMaxHeight.y < vertex.pos.y)
					{
						m_f2MinMaxHeight.y = vertex.pos.y;
					}

					vecHeight.emplace_back(vertex.pos.y);
					//m_vecHeight.emplace_back(vertex.pos.y);
					m_vecHeightMap.emplace_back(vertex);

					++nIdx;
				}

				std::copy(vecHeight.begin(), vecHeight.end(), std::front_inserter(dequeHeight));
			}

			std::copy(dequeHeight.begin(), dequeHeight.end(), std::back_inserter(m_vecHeight));

			vecRawData.clear();

			return true;
		}

		bool Terrain::initTerrain()
		{
			uint32_t nVertexCount = (m_nTerrainHeight - 1) * (m_nTerrainWidth - 1) * 6;
			std::vector<Graphics::VertexPosTexNorCol> vecVertex;
			vecVertex.reserve(nVertexCount);

			m_vecVertices.reserve(nVertexCount);
			m_vecIndices.reserve(nVertexCount);

			std::vector<Math::Vector3> vecNormal;
			vecNormal.reserve((m_nTerrainHeight - 1) * (m_nTerrainWidth - 1));

			int nHeight = (int)(m_nTerrainHeight - 1);
			int nWidth = (int)(m_nTerrainWidth - 1);
			for (int j = 0; j < nHeight; ++j)
			{
				for (int i = 0; i < nWidth; ++i)
				{
					int nIdx1 = ((j + 1) * m_nTerrainWidth) + i;		// Bottom left vertex.
					int nIdx2 = ((j + 1) * m_nTerrainWidth) + (i + 1);	// Bottom right vertex.
					int nIdx3 = (j * m_nTerrainWidth) + i;				// Upper left vertex.

					// Get three vertices from the face.
					Math::Vector3 v1 = m_vecHeightMap[nIdx1].pos;
					Math::Vector3 v2 = m_vecHeightMap[nIdx2].pos;
					Math::Vector3 v3 = m_vecHeightMap[nIdx3].pos;

					// Calculate the two vectors for this face.
					v1.x = v1.x - v3.x;
					v1.y = v1.y - v3.y;
					v1.z = v1.z - v3.z;
					v2.x = v3.x - v2.x;
					v2.y = v3.y - v2.y;
					v2.z = v3.z - v2.z;

					// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
					Math::Vector3 vNormal(
						(v1.y * v2.z) - (v1.z * v2.y),
						(v1.z * v2.x) - (v1.x * v2.z),
						(v1.x * v2.y) - (v1.y * v2.x)
					);

					vNormal.Normalize();

					//// Calculate the length.
					//float fLength = sqrt((vNormal.x * vNormal.x) + (vNormal.y * vNormal.y) + (vNormal.z * vNormal.z));
					//
					//// Normalize the final value for this face using the length.
					//vNormal /= fLength;

					vecNormal.emplace_back(vNormal);
				}
			}

			// Now go through all the vertices and take a sum of the face normals that touch this vertex.
			nHeight = m_nTerrainHeight;
			nWidth = m_nTerrainWidth;
			for (int j = 0; j < nHeight; ++j)
			{
				for (int i = 0; i < nWidth; ++i)
				{
					// Initialize the sum.
					Math::Vector3 sum;
					int nIdx = 0;

					// Bottom left face.
					if (((i - 1) >= 0) && ((j - 1) >= 0))
					{
						nIdx = ((j - 1) * (nWidth - 1)) + (i - 1);

						sum += vecm_vecNormals[nIdx];
					}

					// Bottom right face.
					if ((i < (nWidth - 1)) && ((j - 1) >= 0))
					{
						nIdx = ((j - 1) * (nWidth - 1)) + i;

						sum += vecm_vecNormals[nIdx];
					}

					// Upper left face.
					if (((i - 1) >= 0) && (j < (nHeight - 1)))
					{
						nIdx = (j * (nWidth - 1)) + (i - 1);

						sum += vecm_vecNormals[nIdx];
					}

					// Upper right face.
					if ((i < (nWidth - 1)) && (j < (nHeight - 1)))
					{
						nIdx = (j * (nWidth - 1)) + i;

						sum += vecm_vecNormals[nIdx];
					}

					// Calculate the length of this normal.
					float fLength = sqrt((sum.x * sum.x) + (sum.y * sum.y) + (sum.z * sum.z));

					// Get an index to the vertex location in the height map array.
					nIdx = (j * nWidth) + i;

					// Normalize the final shared normal for this vertex and store it in the height map array.
					m_vecHeightMap[nIdx].normal = (sum / fLength);
				}
			}

			vecNormal.clear();

			int nIdx = 0;
			// Load the 3D terrain model with the height map terrain data.
			// We will be creating 2 triangles for each of the four points in a quad.
			nHeight = (int)(m_nTerrainHeight - 1);
			nWidth = (int)(m_nTerrainWidth - 1);
			for (int i = 0; i < nHeight; ++i)
			{
				for (int j = 0; j < nWidth; ++j)
				{
					// Get the indexes to the four points of the quad.
					int nIdx1 = (m_nTerrainWidth * i) + j;				// Upper left.
					int nIdx2 = (m_nTerrainWidth * i) + (j + 1);		// Upper right.
					int nIdx3 = (m_nTerrainWidth * (i + 1)) + j;		// Bottom left.
					int nIdx4 = (m_nTerrainWidth * (i + 1)) + (j + 1);	// Bottom right.

					// Now create two triangles for that quad.
					// Triangle 1 - Upper left.
					vecVertex.push_back({
						m_vecHeightMap[nIdx1].pos,
						Math::Vector2::Zero,
						m_vecHeightMap[nIdx1].normal,
						m_vecHeightMap[nIdx1].color
					});
					m_vecVertices.push_back(m_vecHeightMap[nIdx1].pos);
					m_vecIndices.push_back(nIdx);
					nIdx++;

					// Triangle 1 - Upper right.
					vecVertex.push_back({
						m_vecHeightMap[nIdx2].pos,
						Math::Vector2(1.f, 0.f),
						m_vecHeightMap[nIdx2].normal,
						m_vecHeightMap[nIdx2].color
					});
					m_vecVertices.push_back(m_vecHeightMap[nIdx2].pos);
					m_vecIndices.push_back(nIdx);
					nIdx++;

					// Triangle 1 - Bottom left.
					vecVertex.push_back({
						m_vecHeightMap[nIdx3].pos,
						Math::Vector2(0.f, 1.f),
						m_vecHeightMap[nIdx3].normal,
						m_vecHeightMap[nIdx3].color
					});
					m_vecVertices.push_back(m_vecHeightMap[nIdx3].pos);
					m_vecIndices.push_back(nIdx);
					nIdx++;

					// Triangle 2 - Bottom left.
					vecVertex.push_back({
						m_vecHeightMap[nIdx3].pos,
						Math::Vector2(0.f, 1.f),
						m_vecHeightMap[nIdx3].normal,
						m_vecHeightMap[nIdx3].color
					});
					m_vecVertices.push_back(m_vecHeightMap[nIdx3].pos);
					m_vecIndices.push_back(nIdx);
					nIdx++;

					// Triangle 2 - Upper right.
					vecVertex.push_back({
						m_vecHeightMap[nIdx2].pos,
						Math::Vector2(1.f, 0.f),
						m_vecHeightMap[nIdx2].normal,
						m_vecHeightMap[nIdx2].color
					});
					m_vecVertices.push_back(m_vecHeightMap[nIdx2].pos);
					m_vecIndices.push_back(nIdx);
					nIdx++;

					// Triangle 2 - Bottom right.
					vecVertex.push_back({
						m_vecHeightMap[nIdx4].pos,
						Math::Vector2(1.f, 1.f),
						m_vecHeightMap[nIdx4].normal,
						m_vecHeightMap[nIdx4].color
					});
					m_vecVertices.push_back(m_vecHeightMap[nIdx4].pos);
					m_vecIndices.push_back(nIdx);
					nIdx++;
				}
			}

			return initTerrainCell(vecVertex);
		}

		bool Terrain::initTerrainCell(std::vector<Graphics::VertexPosTexNorCol>& vecModel)
		{
			int nCellRowCount = (m_nTerrainWidth - 1) / (m_nCellWidth - 1);
			m_nCellCount = nCellRowCount * nCellRowCount;

			m_veTerrainCells.resize(m_nCellCount);

			Concurrency::parallel_for(0, nCellRowCount * nCellRowCount, [&](int n)
			{
				int i = n / nCellRowCount;
				int j = n % nCellRowCount;

				m_veTerrainCells[n].Init(vecModel, j, i, m_nCellWidth, m_nCellHeight, m_nTerrainWidth);
			});

			//int nIdx = 0;
			//for (int i = 0; i < nCellRowCount; ++i)
			//{
			//	for (int j = 0; j < nCellRowCount; ++j)
			//	{
			//		if (m_veTerrainCells[nIdx].Init(vecModel, j, i, m_nCellWidth, m_nCellHeight, m_nTerrainWidth) == false)
			//		{
			//			Release();
			//			return false;
			//		}
			//
			//		++nIdx;
			//	}
			//}

			return true;
		}

		bool Terrain::checkHeightOfTriangle(float x, float z, float& height, Math::Vector3& v0, Math::Vector3& v1, Math::Vector3& v2)
		{
			// Starting position of the ray that is being cast.
			Math::Vector3 vStartVector(x, 0.f, z);

			// The direction the ray is being cast.
			Math::Vector3 vDrectionVector(0.f, -1.f, 0.f);

			// Calculate the two edges from the three points given.
			Math::Vector3 vEdge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);

			Math::Vector3 vEdge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

			// Calculate the normal of the triangle from the two edges.
			Math::Vector3 vNormal((vEdge1.y * vEdge2.z) - (vEdge1.z * vEdge2.y),
				(vEdge1.z * vEdge2.x) - (vEdge1.x * vEdge2.z),
				(vEdge1.x * vEdge2.y) - (vEdge1.y * vEdge2.x));

			float fMagnitude = sqrtf((vNormal.x * vNormal.x) + (vNormal.y * vNormal.y) + (vNormal.z * vNormal.z));
			vNormal.x = vNormal.x / fMagnitude;
			vNormal.y = vNormal.y / fMagnitude;
			vNormal.z = vNormal.z / fMagnitude;

			// Find the distance from the origin to the plane.
			float D = ((-vNormal.x * v0.x) + (-vNormal.y * v0.y) + (-vNormal.z * v0.z));

			// Get the denominator of the equation.
			float fDenominator = ((vNormal.x * vDrectionVector.x) + (vNormal.y * vDrectionVector.y) + (vNormal.z * vDrectionVector.z));

			// Make sure the result doesn't get too close to zero to prevent divide by zero.
			if (fabs(fDenominator) < 0.0001f)
				return false;

			// Get the numerator of the equation.
			float fNumerator = -1.f * (((vNormal.x * vStartVector.x) + (vNormal.y * vStartVector.y) + (vNormal.z * vStartVector.z)) + D);

			// Calculate where we intersect the triangle.
			float t = fNumerator / fDenominator;

			// Find the intersection vector.
			Math::Vector3 Q(vStartVector.x + (vDrectionVector.x * t),
				vStartVector.y + (vDrectionVector.y * t),
				vStartVector.z + (vDrectionVector.z * t));

			// Find the three edges of the triangle.
			Math::Vector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
			Math::Vector3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
			Math::Vector3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

			// Calculate the normal for the first edge.
			Math::Vector3 edgeNormal((e1.y * vNormal.z) - (e1.z * vNormal.y),
				(e1.z * vNormal.x) - (e1.x * vNormal.z),
				(e1.x * vNormal.y) - (e1.y * vNormal.x));

			// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
			Math::Vector3 temp(Q.x - v0.x, Q.y - v0.y, Q.z - v0.z);

			float fDeterminant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

			// Check if it is outside.
			if (fDeterminant > 0.001f)
				return false;

			// Calculate the normal for the second edge.
			edgeNormal.x = (e2.y * vNormal.z) - (e2.z * vNormal.y);
			edgeNormal.y = (e2.z * vNormal.x) - (e2.x * vNormal.z);
			edgeNormal.z = (e2.x * vNormal.y) - (e2.y * vNormal.x);

			// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
			temp.x = Q.x - v1.x;
			temp.y = Q.y - v1.y;
			temp.z = Q.z - v1.z;

			fDeterminant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

			// Check if it is outside.
			if (fDeterminant > 0.001f)
				return false;

			// Calculate the normal for the third edge.
			edgeNormal.x = (e3.y * vNormal.z) - (e3.z * vNormal.y);
			edgeNormal.y = (e3.z * vNormal.x) - (e3.x * vNormal.z);
			edgeNormal.z = (e3.x * vNormal.y) - (e3.y * vNormal.x);

			// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
			temp.x = Q.x - v2.x;
			temp.y = Q.y - v2.y;
			temp.z = Q.z - v2.z;

			fDeterminant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

			// Check if it is outside.
			if (fDeterminant > 0.001f)
				return false;

			// Now we have our height.
			height = Q.y;

			return true;
		}

		//bool Terrain::checkHeightOfTriangle(float x, float z, float& height, Vector3& v0, Vector3& v1, Vector3& v2)
		//{
		//	//float vStartVector[3], vDrectionVector[3], vEdge1[3], vEdge2[3], m_vecNormals[3];
		//	//float Q[3], e1[3], e2[3], e3[3], edgem_vecNormals[3], temp[3];
		//	//float magnitude, D, denominator, numerator, t, determinant;

		//	// Starting position of the ray that is being cast.
		//	Vector3 vvStartVector(x, 0.f, z);

		//	// The direction the ray is being cast.
		//	Vector3 vvDrectionVector(0.f, -1.f, 0.f);

		//	// Calculate the two edges from the three points given.
		//	Vector3 vvEdge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
		//	Vector3 vvEdge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

		//	// Calculate the normal of the triangle from the two edges.
		//	Vector3 vNormal((vvEdge1.y * vvEdge2.z) - (vvEdge1.z * vvEdge2.y),
		//		(vvEdge1.z * vvEdge2.x) - (vvEdge1.x * vvEdge2.z),
		//		(vvEdge1.x * vvEdge2.y) - (vvEdge1.y * vvEdge2.x));

		//	float fMagnitude = sqrtf((vNormal.x * vNormal.x) + (vNormal.y * vNormal.y) + (vNormal.z * vNormal.z));
		//	vNormal.x /= fMagnitude;
		//	vNormal.y /= fMagnitude;
		//	vNormal.z /= fMagnitude;

		//	// Find the distance from the origin to the plane.
		//	float D((-vNormal.x * v0.x) + (-vNormal.y * v0.y) + (-vNormal.z * v0.z));

		//	// Get the denominator of the equation.
		//	float fDenominator = ((vNormal.x * vvDrectionVector.x) + (vNormal.y * vvDrectionVector.y) + (vNormal.z * vvDrectionVector.z));

		//	// Make sure the result doesn't get too close to zero to prevent divide by zero.
		//	if (AloMath::IsZero(fDenominator))
		//		return false;

		//	// Get the numerator of the equation.
		//	float fNumerator = -1.f * (((vNormal.x * vvStartVector.x) + (vNormal.y * vvStartVector.y) + (vNormal.z * vvStartVector.z)) + D);

		//	// Calculate where we intersect the triangle.
		//	float t = fNumerator / fDenominator;

		//	// Find the intersection vector.
		//	Vector3 Q(vvStartVector.x + (vvDrectionVector.x * t),
		//		vvStartVector.y + (vvDrectionVector.y * t),
		//		vvStartVector.z + (vvDrectionVector.z * t));

		//	// Find the three edges of the triangle.
		//	Vector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
		//	Vector3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
		//	Vector3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

		//	// Calculate the normal for the first edge.
		//	Vector3 vEdgeNormal((e1.y * vNormal.z) - (e1.z * vNormal.y),
		//		(e1.z * vNormal.x) - (e1.x * vNormal.z),
		//		(e1.x * vNormal.y) - (e1.y * vNormal.x));

		//	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
		//	Vector3 temp(Q.x - v0.x, Q.y - v0.y, Q.z - v0.z);

		//	float fDeterminant = ((vEdgeNormal.x * temp.x) + (vEdgeNormal.y * temp.y) + (vEdgeNormal.z * temp.z));

		//	// Check if it is outside.
		//	if (AloMath::IsZero(fDeterminant))
		//		return false;

		//	// Calculate the normal for the second edge.
		//	vEdgeNormal.x = (e2.y * vNormal.z) - (e2.z * vNormal.y);
		//	vEdgeNormal.y = (e2.z * vNormal.x) - (e2.x * vNormal.z);
		//	vEdgeNormal.z = (e2.x * vNormal.y) - (e2.y * vNormal.x);

		//	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
		//	temp.x = Q.x - v1.x;
		//	temp.y = Q.y - v1.y;
		//	temp.z = Q.z - v1.z;

		//	fDeterminant = ((vEdgeNormal.x * temp.x) + (vEdgeNormal.y * temp.y) + (vEdgeNormal.z * temp.z));

		//	// Check if it is outside.
		//	if (AloMath::IsZero(fDeterminant))
		//	{
		//		return false;
		//	}

		//	// Calculate the normal for the third edge.
		//	vEdgeNormal.x = (e3.y * vNormal.z) - (e3.z * vNormal.y);
		//	vEdgeNormal.y = (e3.z * vNormal.x) - (e3.x * vNormal.z);
		//	vEdgeNormal.z = (e3.x * vNormal.y) - (e3.y * vNormal.x);

		//	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
		//	temp.x = Q.x - v2.x;
		//	temp.y = Q.y - v2.y;
		//	temp.z = Q.z - v2.z;

		//	fDeterminant = ((vEdgeNormal.x * temp.x) + (vEdgeNormal.y * temp.y) + (vEdgeNormal.z * temp.z));

		//	// Check if it is outside.
		//	if (AloMath::IsZero(fDeterminant))
		//		return false;

		//	// Now we have our height.
		//	height = Q.y;

		//	return true;
		//}
	}
}
#else

#include "CommonLib/FileUtil.h"

namespace EastEngine
{
	namespace GameObject
	{
		Terrain::Terrain()
		{
		}

		Terrain::~Terrain()
		{
			SafeDelete(m_pHeightField);
			SafeDelete(m_pSky);
		}

		void Terrain::Init(TerrainProperty* pTerrainProperty)
		{
			if (pTerrainProperty != nullptr)
			{
				m_property = *pTerrainProperty;
			}

			auto gp_wrap = [&](int a) -> int
			{
				if (a < 0)
					return (a + m_property.nGridPoints);

				if (a >= m_property.nGridPoints)
					return (a - m_property.nGridPoints);

				return a;
			};

			/*int i, j, k, l;
			float x, z;
			int ix, iz;
			float* backterrain = nullptr;
			Math::Vector3 vec1, vec2, vec3;
			int currentstep = m_property.nGridPoints;
			float mv, rm;
			float offset = 0, yscale = 0, maxheight = 0, minheight = 0;

			float* height_linear_array = nullptr;
			D3D11_SUBRESOURCE_DATA subresource_data;*/

			int nGridPointSize = m_property.nGridPoints + 1;
			m_vecHeights.resize(nGridPointSize * nGridPointSize);
			m_vecNormals.resize(nGridPointSize * nGridPointSize);

			std::vector<float> backterrain(nGridPointSize * nGridPointSize);
			float rm = m_property.fFractalInitialValue;
			backterrain[0] = 0.f;
			backterrain[0 + m_property.nGridPoints * m_property.nGridPoints] = 0;
			backterrain[m_property.nGridPoints] = 0.f;
			backterrain[m_property.nGridPoints + m_property.nGridPoints * m_property.nGridPoints] = 0.f;
			int currentstep = m_property.nGridPoints;

			// generating fractal terrain using square-diamond method
			while (currentstep > 1)
			{
				//square step;
				int i = 0;
				while (i < m_property.nGridPoints)
				{
					int j = 0;
					while (j < m_property.nGridPoints)
					{
						float mv = backterrain[i + m_property.nGridPoints*j];
						mv += backterrain[(i + currentstep) + m_property.nGridPoints * j];
						mv += backterrain[(i + currentstep) + m_property.nGridPoints * (j + currentstep)];
						mv += backterrain[i + m_property.nGridPoints * (j + currentstep)];
						mv /= 4.f;

						backterrain[i + currentstep / 2 + m_property.nGridPoints * (j + currentstep / 2)] = mv + rm * (Math::Random(0.f, 1000.f) / 1000.f - 0.5f);
						j += currentstep;
					}
					i += currentstep;
				}

				//diamond step;
				i = 0;
				while (i < m_property.nGridPoints)
				{
					int j = 0;
					while (j < m_property.nGridPoints)
					{
						float mv = 0.f;
						mv = backterrain[i + m_property.nGridPoints * j];
						mv += backterrain[(i + currentstep) + m_property.nGridPoints * j];
						mv += backterrain[(i + currentstep / 2) + m_property.nGridPoints * (j + currentstep / 2)];
						mv += backterrain[i + currentstep / 2 + m_property.nGridPoints * gp_wrap(j - currentstep / 2)];
						mv /= 4.f;
						backterrain[i + currentstep / 2 + m_property.nGridPoints * j] = mv + rm * (Math::Random(0.f, 1000.f) / 1000.f - 0.5f);

						mv = 0.f;
						mv = backterrain[i + m_property.nGridPoints * j];
						mv += backterrain[i + m_property.nGridPoints * (j + currentstep)];
						mv += backterrain[(i + currentstep / 2) + m_property.nGridPoints * (j + currentstep / 2)];
						mv += backterrain[gp_wrap(i - currentstep / 2) + m_property.nGridPoints * (j + currentstep / 2)];
						mv /= 4.f;
						backterrain[i + m_property.nGridPoints * (j + currentstep / 2)] = mv + rm * (Math::Random(0.f, 1000.f) / 1000.f - 0.5f);

						mv = 0.f;
						mv = backterrain[i + currentstep + m_property.nGridPoints * j];
						mv += backterrain[i + currentstep + m_property.nGridPoints * (j + currentstep)];
						mv += backterrain[(i + currentstep / 2) + m_property.nGridPoints * (j + currentstep / 2)];
						mv += backterrain[gp_wrap(i + currentstep / 2 + currentstep) + m_property.nGridPoints * (j + currentstep / 2)];
						mv /= 4.f;
						backterrain[i + currentstep + m_property.nGridPoints * (j + currentstep / 2)] = mv + rm * (Math::Random(0.f, 1000.f) / 1000.f - 0.5f);

						mv = 0.f;
						mv = backterrain[i + currentstep + m_property.nGridPoints * (j + currentstep)];
						mv += backterrain[i + m_property.nGridPoints * (j + currentstep)];
						mv += backterrain[(i + currentstep / 2) + m_property.nGridPoints * (j + currentstep / 2)];
						mv += backterrain[i + currentstep / 2 + m_property.nGridPoints * gp_wrap(j + currentstep / 2 + currentstep)];
						mv /= 4.f;
						backterrain[i + currentstep / 2 + m_property.nGridPoints * (j + currentstep)] = mv + rm * (Math::Random(0.f, 1000.f) / 1000.f - 0.5f);
						j += currentstep;
					}
					i += currentstep;
				}
				//changing current step;
				currentstep /= 2;
				rm *= m_property.fFractalFactor;
			}

			// scaling to minheight..maxheight range
			for (int i = 0; i < nGridPointSize; ++i)
			{
				for (int j = 0; j < nGridPointSize; ++j)
				{
					m_vecHeights[i][j] = backterrain[i + m_property.nGridPoints*j];
				}
			}

			float maxHeight = m_vecHeights[0][0];
			float minHeight = m_vecHeights[0][0];
			for (int i = 0; i < nGridPointSize; ++i)
			{
				for (int j = 0; j < nGridPointSize; ++j)
				{
					if (m_vecHeights[i][j] > maxHeight)
					{
						maxHeight = m_vecHeights[i][j];
					}

					if (m_vecHeights[i][j] < minHeight)
					{
						minHeight = m_vecHeights[i][j];
					}
				}
			}

			float yscale = (m_property.fMaxHeight - m_property.fMinHeight) / (maxHeight - minHeight);

			for (int i = 0; i < nGridPointSize; ++i)
			{
				for (int j = 0; j < nGridPointSize; ++j)
				{
					m_vecHeights[i][j] -= minHeight;
					m_vecHeights[i][j] *= yscale;
					m_vecHeights[i][j] += m_property.fMinHeight;
				}
			}

			// moving down edges of heightmap	
			for (int i = 0; i < nGridPointSize; ++i)
			{
				for (int j = 0; j < nGridPointSize; ++j)
				{
					float mv = static_cast<float>((i - m_property.nGridPoints / 2.f) * (i - m_property.nGridPoints / 2.f) + (j - m_property.nGridPoints / 2.f) * (j - m_property.nGridPoints / 2.f));
					rm = static_cast<float>((m_property.nGridPoints * 0.8f) * (m_property.nGridPoints * 0.8f) / 4.f);
					if (mv > rm)
					{
						m_vecHeights[i][j] -= ((mv - rm) / 1000.f) * m_property.fGeometryScale;
					}

					if (m_vecHeights[i][j] < m_property.fMinHeight)
					{
						m_vecHeights[i][j] = m_property.fMinHeight;
					}
				}
			}

			// terrain banks
			for (int k = 0; k < 10; ++k)
			{
				for (int i = 0; i < nGridPointSize; ++i)
				{
					for (int j = 0; j < nGridPointSize; ++j)
					{
						float mv = m_vecHeights[i][j];
						if ((mv) > 0.02f)
						{
							mv -= 0.02f;
						}

						if (mv < -0.02f)
						{
							mv += 0.02f;
						}
						m_vecHeights[i][j] = mv;
					}
				}
			}

			// smoothing 
			for (int k = 0; k < m_property.nSmoothSteps; ++k)
			{
				for (int i = 0; i < nGridPointSize; ++i)
				{
					for (int j = 0; j < nGridPointSize; ++j)
					{
						Math::Vector3 vec1, vec2, vec3;

						vec1.x = 2.f * m_property.fGeometryScale;
						vec1.y = m_property.fGeometryScale * (m_vecHeights[gp_wrap(i + 1)][j] - m_vecHeights[gp_wrap(i - 1)][j]);
						vec1.z = 0.f;

						vec2.x = 0.f;
						vec2.y = -m_property.fGeometryScale*(m_vecHeights[i][gp_wrap(j + 1)] - m_vecHeights[i][gp_wrap(j - 1)]);
						vec2.z = -2.f * m_property.fGeometryScale;

						vec1.Cross(vec2, vec3);
						vec3.Normalize();

						if (((vec3.y > m_property.fRockfactor) || (m_vecHeights[i][j] < 1.2f)))
						{
							rm = m_property.fSmoothFactor1;
							float mv = m_vecHeights[i][j] * (1.f - rm) + rm * 0.25f * (m_vecHeights[gp_wrap(i - 1)][j] + m_vecHeights[i][gp_wrap(j - 1)] + m_vecHeights[gp_wrap(i + 1)][j] + m_vecHeights[i][gp_wrap(j + 1)]);
							backterrain[i + m_property.nGridPoints*j] = mv;
						}
						else
						{
							rm = m_property.fSmoothFactor2;
							float mv = m_vecHeights[i][j] * (1.f - rm) + rm * 0.25f * (m_vecHeights[gp_wrap(i - 1)][j] + m_vecHeights[i][gp_wrap(j - 1)] + m_vecHeights[gp_wrap(i + 1)][j] + m_vecHeights[i][gp_wrap(j + 1)]);
							backterrain[i + m_property.nGridPoints*j] = mv;
						}

					}
				}

				for (int i = 0; i < nGridPointSize; ++i)
				{
					for (int j = 0; j < nGridPointSize; ++j)
					{
						m_vecHeights[i][j] = (backterrain[i + m_property.nGridPoints * j]);
					}
				}
			}

			for (int i = 0; i < nGridPointSize; ++i)
			{
				for (int j = 0; j < nGridPointSize; ++j)
				{
					rm = 0.5f;
					float mv = m_vecHeights[i][j] * (1.f - rm) + rm * 0.25f * (m_vecHeights[gp_wrap(i - 1)][j] + m_vecHeights[i][gp_wrap(j - 1)] + m_vecHeights[gp_wrap(i + 1)][j] + m_vecHeights[i][gp_wrap(j + 1)]);
					backterrain[i + m_property.nGridPoints * j] = mv;
				}
			}

			for (int i = 0; i < nGridPointSize; ++i)
			{
				for (int j = 0; j < nGridPointSize; ++j)
				{
					m_vecHeights[i][j] = (backterrain[i + m_property.nGridPoints*j]);
				}
			}
			backterrain.resize(0);

			//calculating normals
			for (int i = 0; i < nGridPointSize; ++i)
			{
				for (int j = 0; j < nGridPointSize; ++j)
				{
					Math::Vector3 vec1, vec2, vec3;

					vec1.x = 2.f * m_property.fGeometryScale;
					vec1.y = m_property.fGeometryScale * (m_vecHeights[gp_wrap(i + 1)][j] - m_vecHeights[gp_wrap(i - 1)][j]);
					vec1.z = 0.f;
					vec2.x = 0.f;
					vec2.y = -m_property.fGeometryScale * (m_vecHeights[i][gp_wrap(j + 1)] - m_vecHeights[i][gp_wrap(j - 1)]);
					vec2.z = -2.f * m_property.fGeometryScale;

					vec1.Cross(vec2, m_vecNormals[i][j]);
					m_vecNormals[i][j].Normalize();
				}
			}

			auto bilinear_interpolation = [](float fx, float fy, float a, float b, float c, float d) -> float
			{
				float s1, s2, s3, s4;
				s1 = fx*fy;
				s2 = (1 - fx)*fy;
				s3 = (1 - fx)*(1 - fy);
				s4 = fx*(1 - fy);
				return((a*s3 + b*s4 + c*s1 + d*s2));
			};

			// buiding layerdef 
			std::vector<byte> temp_layerdef_map_texture_pixels(m_property.nLayerDefMapSize * m_property.nLayerDefMapSize * 4);
			std::vector<byte> layerdef_map_texture_pixels(m_property.nLayerDefMapSize * m_property.nLayerDefMapSize * 4);
			for (int i = 0; i < m_property.nLayerDefMapSize; ++i)
			{
				for (int j = 0; j < m_property.nLayerDefMapSize; ++j)
				{
					float x = static_cast<float>(m_property.nGridPoints) * (static_cast<float>(i) / static_cast<float>(m_property.nLayerDefMapSize));
					float z = static_cast<float>(m_property.nGridPoints) * (static_cast<float>(j) / static_cast<float>(m_property.nLayerDefMapSize));
					int ix = static_cast<int>(std::floor(x));
					int iz = static_cast<int>(std::floor(z));
					rm = bilinear_interpolation(x - ix, z - iz, 
						m_vecHeights[ix][iz], 
						m_vecHeights[ix + 1][iz], 
						m_vecHeights[ix + 1][iz + 1], 
						m_vecHeights[ix][iz + 1]) * m_property.fGeometryScale;

					temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 0] = 0;
					temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 1] = 0;
					temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 2] = 0;
					temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 3] = 0;

					if ((rm > m_property.fHeightUnderWaterStart) && (rm <= m_property.fHeightUnderWaterEnd))
					{
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 0] = 255;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 1] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 2] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 3] = 0;
					}

					if ((rm > m_property.fHeightSandStart) && (rm <= m_property.fHeightSandEnd))
					{
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 0] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 1] = 255;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 2] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 3] = 0;
					}

					if ((rm > m_property.fHeightGrassStart) && (rm <= m_property.fHeightGrassEnd))
					{
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 0] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 1] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 2] = 255;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 3] = 0;
					}

					float mv = bilinear_interpolation(x - ix, z - iz, m_vecNormals[ix][iz].y, m_vecNormals[ix + 1][iz].y, m_vecNormals[ix + 1][iz + 1].y, m_vecNormals[ix][iz + 1].y);

					if ((mv < m_property.fSlopeGrassStart) && (rm > m_property.fHeightSandEnd))
					{
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 0] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 1] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 2] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 3] = 0;
					}

					if ((mv < m_property.fSlopeRocksStart) && (rm > m_property.fHeightRocksStart))
					{
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 0] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 1] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 2] = 0;
						temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 3] = 255;
					}
				}
			}

			for (int i = 0; i < m_property.nLayerDefMapSize; ++i)
			{
				for (int j = 0; j < m_property.nLayerDefMapSize; ++j)
				{
					layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 0] = temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 0];
					layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 1] = temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 1];
					layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 2] = temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 2];
					layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 3] = temp_layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 3];
				}
			}

			for (int i = 2; i < m_property.nLayerDefMapSize - 2; ++i)
			{
				for (int j = 2; j < m_property.nLayerDefMapSize - 2; ++j)
				{
					int n1 = 0;
					int n2 = 0;
					int n3 = 0;
					int n4 = 0;
					for (int k = -2; k < 3; ++k)
					{
						for (int l = -2; l < 3; ++l)
						{
							n1 += temp_layerdef_map_texture_pixels[((j + k) * m_property.nLayerDefMapSize + i + l) * 4 + 0];
							n2 += temp_layerdef_map_texture_pixels[((j + k) * m_property.nLayerDefMapSize + i + l) * 4 + 1];
							n3 += temp_layerdef_map_texture_pixels[((j + k) * m_property.nLayerDefMapSize + i + l) * 4 + 2];
							n4 += temp_layerdef_map_texture_pixels[((j + k) * m_property.nLayerDefMapSize + i + l) * 4 + 3];
						}
					}
					layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 0] = static_cast<byte>(n1 / 25);
					layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 1] = static_cast<byte>(n2 / 25);
					layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 2] = static_cast<byte>(n3 / 25);
					layerdef_map_texture_pixels[(j * m_property.nLayerDefMapSize + i) * 4 + 3] = static_cast<byte>(n4 / 25);
				}
			}

			// putting the generated data to textures
			D3D11_SUBRESOURCE_DATA subresource_data;
			subresource_data.pSysMem = temp_layerdef_map_texture_pixels.data();
			subresource_data.SysMemPitch = m_property.nLayerDefMapSize * 4;
			subresource_data.SysMemSlicePitch = 0;

			Graphics::TextureDesc2D tex_desc;
			tex_desc.Width = m_property.nLayerDefMapSize;
			tex_desc.Height = m_property.nLayerDefMapSize;
			tex_desc.MipLevels = 1;
			tex_desc.ArraySize = 1;
			tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			tex_desc.SampleDesc.Count = 1;
			tex_desc.SampleDesc.Quality = 0;
			tex_desc.Usage = D3D11_USAGE_DEFAULT;
			tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			tex_desc.CPUAccessFlags = 0;
			tex_desc.MiscFlags = 0;
			tex_desc.Build();
			m_pTexLayerdef = Graphics::ITexture::Create("TerrainLayerDefMap", tex_desc, &subresource_data);

			temp_layerdef_map_texture_pixels.resize(0);
			layerdef_map_texture_pixels.resize(0);

			std::vector<float> height_linear_array(m_property.nGridPoints * m_property.nGridPoints * 4);

			for (int i = 0; i < m_property.nGridPoints; ++i)
			{
				for (int j = 0; j < m_property.nGridPoints; ++j)
				{
					height_linear_array[(i + j * m_property.nGridPoints) * 4 + 0] = m_vecNormals[i][j].x;
					height_linear_array[(i + j * m_property.nGridPoints) * 4 + 1] = m_vecNormals[i][j].y;
					height_linear_array[(i + j * m_property.nGridPoints) * 4 + 2] = m_vecNormals[i][j].z;
					height_linear_array[(i + j * m_property.nGridPoints) * 4 + 3] = m_vecHeights[i][j];
				}
			}

			subresource_data.pSysMem = height_linear_array.data();
			subresource_data.SysMemPitch = m_property.nGridPoints * 4 * sizeof(float);
			subresource_data.SysMemSlicePitch = 0;

			tex_desc.Width = m_property.nGridPoints;
			tex_desc.Height = m_property.nGridPoints;
			tex_desc.MipLevels = 1;
			tex_desc.ArraySize = 1;
			tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			tex_desc.SampleDesc.Count = 1;
			tex_desc.SampleDesc.Quality = 0;
			tex_desc.Usage = D3D11_USAGE_DEFAULT;
			tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			tex_desc.CPUAccessFlags = 0;
			tex_desc.MiscFlags = 0;
			tex_desc.Build();
			m_pTexHeightMap = Graphics::ITexture::Create("TerrainHeightMap", tex_desc, &subresource_data);

			height_linear_array.resize(0);

			//building depthmap
			std::vector<byte> depth_shadow_map_texture_pixels(m_property.nDepthShadowMapSize * m_property.nDepthShadowMapSize * 4);
			for (int i = 0; i < m_property.nDepthShadowMapSize; ++i)
			{
				for (int j = 0; j < m_property.nDepthShadowMapSize; ++j)
				{
					float x = static_cast<float>(m_property.nGridPoints) * (static_cast<float>(i) / static_cast<float>(m_property.nDepthShadowMapSize));
					float z = static_cast<float>(m_property.nGridPoints) * (static_cast<float>(j) / static_cast<float>(m_property.nDepthShadowMapSize));
					int ix = static_cast<int>(std::floor(x));
					int iz = static_cast<int>(std::floor(z));
					rm = bilinear_interpolation(x - ix, z - iz, 
						m_vecHeights[ix][iz], 
						m_vecHeights[ix + 1][iz], 
						m_vecHeights[ix + 1][iz + 1], 
						m_vecHeights[ix][iz + 1]) * m_property.fGeometryScale;

					if (rm > 0)
					{
						depth_shadow_map_texture_pixels[(j * m_property.nDepthShadowMapSize + i) * 4 + 0] = 0;
						depth_shadow_map_texture_pixels[(j * m_property.nDepthShadowMapSize + i) * 4 + 1] = 0;
						depth_shadow_map_texture_pixels[(j * m_property.nDepthShadowMapSize + i) * 4 + 2] = 0;
					}
					else
					{
						float no = (1.f * 255.f * (rm / (m_property.fMinHeight * m_property.fGeometryScale))) - 1.f;
						no = Math::Clamp(no, 0.f, 255.f);

						depth_shadow_map_texture_pixels[(j * m_property.nDepthShadowMapSize + i) * 4 + 0] = static_cast<byte>(no);

						no = (10.f * 255.f * (rm / (m_property.fMinHeight*m_property.fGeometryScale))) - 40.f;
						no = Math::Clamp(no, 0.f, 255.f);

						depth_shadow_map_texture_pixels[(j*m_property.nDepthShadowMapSize + i) * 4 + 1] = static_cast<byte>(no);

						no = (100.f * 255.f * (rm / (m_property.fMinHeight*m_property.fGeometryScale))) - 300.f;
						no = Math::Clamp(no, 0.f, 255.f);

						depth_shadow_map_texture_pixels[(j*m_property.nDepthShadowMapSize + i) * 4 + 2] = static_cast<byte>(no);
					}
					depth_shadow_map_texture_pixels[(j*m_property.nDepthShadowMapSize + i) * 4 + 3] = 0;
				}
			}

			subresource_data.pSysMem = depth_shadow_map_texture_pixels.data();
			subresource_data.SysMemPitch = m_property.nDepthShadowMapSize * 4;
			subresource_data.SysMemSlicePitch = 0;

			tex_desc.Width = m_property.nDepthShadowMapSize;
			tex_desc.Height = m_property.nDepthShadowMapSize;
			tex_desc.MipLevels = 1;
			tex_desc.ArraySize = 1;
			tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			tex_desc.SampleDesc.Count = 1;
			tex_desc.SampleDesc.Quality = 0;
			tex_desc.Usage = D3D11_USAGE_DEFAULT;
			tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			tex_desc.CPUAccessFlags = 0;
			tex_desc.MiscFlags = 0;
			tex_desc.Build();
			m_pTexDepthMap = Graphics::ITexture::Create("TerrainDepthMap", tex_desc, &subresource_data);

			depth_shadow_map_texture_pixels.resize(0);

			std::vector<Graphics::VertexPos4> vecPatches_rawdata;
			vecPatches_rawdata.resize(m_property.nPatches * m_property.nPatches);

			// creating terrain vertex buffer
			for (int i = 0; i < m_property.nPatches; ++i)
			{
				for (int j = 0; j < m_property.nPatches; ++j)
				{
					Graphics::VertexPos4& vertex = vecPatches_rawdata[i + j * m_property.nPatches];
					vertex.pos.x = i * m_property.fGeometryScale * m_property.nGridPoints / m_property.nPatches;
					vertex.pos.y = j * m_property.fGeometryScale * m_property.nGridPoints / m_property.nPatches;
					vertex.pos.z = m_property.fGeometryScale * m_property.nGridPoints / m_property.nPatches;
					vertex.pos.w = m_property.fGeometryScale * m_property.nGridPoints / m_property.nPatches;
				}
			}

			m_pHeightField = Graphics::IVertexBuffer::Create(Graphics::VertexPos4::Format(), vecPatches_rawdata.size(), &vecPatches_rawdata.front(), D3D11_USAGE_IMMUTABLE);

			// creating sky vertex buffer
			std::vector<Graphics::VertexPosTex> vecSky_rawData;
			vecSky_rawData.resize(m_property.nSkyGridPoints * (m_property.nSkyGridPoints + 2) * 2);

			int floatnum = 0;
			for (int j = 0; j < m_property.nSkyGridPoints; ++j)
			{
				int index = 0;
				floatnum = (j * (m_property.nSkyGridPoints + 2) * 2);
				vecSky_rawData[floatnum].pos.x = m_property.nGridPoints * m_property.fGeometryScale * 0.5f + 4000.f * Math::Cos(2.f * Math::PI * static_cast<float>(index) / static_cast<float>(m_property.nSkyGridPoints)) * Math::Cos(-0.5f * Math::PI + Math::PI * static_cast<float>(j) / static_cast<float>(m_property.nSkyGridPoints));
				vecSky_rawData[floatnum].pos.y = 4000.f * Math::Sin(-0.5f * Math::PI + Math::PI * static_cast<float>(j) / static_cast<float>(m_property.nSkyGridPoints));
				vecSky_rawData[floatnum].pos.z = m_property.nGridPoints * m_property.fGeometryScale * 0.5f + 4000.f * Math::Sin(2.f * Math::PI * static_cast<float>(index) / static_cast<float>(m_property.nSkyGridPoints)) * Math::Cos(-0.5f * Math::PI + Math::PI * static_cast<float>(j) / static_cast<float>(m_property.nSkyGridPoints));
				vecSky_rawData[floatnum].uv.x = (m_property.fSkyTextureAngle + static_cast<float>(index) / static_cast<float>(m_property.nSkyGridPoints));
				vecSky_rawData[floatnum].uv.y = 2.f - 2.f * static_cast<float>(j) / static_cast<float>(m_property.nSkyGridPoints);
				++floatnum;

				for (int i = 0; i < m_property.nSkyGridPoints + 1; ++i)
				{
					vecSky_rawData[floatnum].pos.x = m_property.nGridPoints * m_property.fGeometryScale * 0.5f + 4000.f * Math::Cos(2.f * Math::PI * static_cast<float>(i) / static_cast<float>(m_property.nSkyGridPoints)) * Math::Cos(-0.5f * Math::PI + Math::PI * static_cast<float>(j) / static_cast<float>(m_property.nSkyGridPoints));
					vecSky_rawData[floatnum].pos.y = 4000.f * Math::Sin(-0.5f * Math::PI + Math::PI * static_cast<float>(j) / static_cast<float>(m_property.nSkyGridPoints));
					vecSky_rawData[floatnum].pos.z = m_property.nGridPoints * m_property.fGeometryScale * 0.5f + 4000.f * Math::Sin(2.f * Math::PI * static_cast<float>(i) / static_cast<float>(m_property.nSkyGridPoints)) * Math::Cos(-0.5f * Math::PI + Math::PI * static_cast<float>(j) / static_cast<float>(m_property.nSkyGridPoints));
					vecSky_rawData[floatnum].uv.x = (m_property.fSkyTextureAngle + static_cast<float>(i) / static_cast<float>(m_property.nSkyGridPoints));
					vecSky_rawData[floatnum].uv.y = 2.f - 2.f * static_cast<float>(j) / static_cast<float>(m_property.nSkyGridPoints);
					++floatnum;

					vecSky_rawData[floatnum].pos.x = m_property.nGridPoints * m_property.fGeometryScale * 0.5f + 4000.f * Math::Cos(2.f * Math::PI * static_cast<float>(i) / static_cast<float>(m_property.nSkyGridPoints)) * Math::Cos(-0.5f * Math::PI + Math::PI * static_cast<float>(j + 1) / static_cast<float>(m_property.nSkyGridPoints));
					vecSky_rawData[floatnum].pos.y = 4000.f * Math::Sin(-0.5f * Math::PI + Math::PI * static_cast<float>(j + 1) / static_cast<float>(m_property.nSkyGridPoints));
					vecSky_rawData[floatnum].pos.z = m_property.nGridPoints * m_property.fGeometryScale * 0.5f + 4000.f * Math::Sin(2.f * Math::PI * static_cast<float>(i) / static_cast<float>(m_property.nSkyGridPoints)) * Math::Cos(-0.5f * Math::PI + Math::PI * static_cast<float>(j + 1) / static_cast<float>(m_property.nSkyGridPoints));
					vecSky_rawData[floatnum].uv.x = (m_property.fSkyTextureAngle + static_cast<float>(i) / static_cast<float>(m_property.nSkyGridPoints));
					vecSky_rawData[floatnum].uv.y = 2.f - 2.f * static_cast<float>(j + 1) / static_cast<float>(m_property.nSkyGridPoints);
					++floatnum;
				}
				index = m_property.nSkyGridPoints;
				vecSky_rawData[floatnum].pos.x = m_property.nGridPoints * m_property.fGeometryScale * 0.5f + 4000.f * Math::Cos(2.f * Math::PI * static_cast<float>(index) / static_cast<float>(m_property.nSkyGridPoints)) * Math::Cos(-0.5f * Math::PI + Math::PI * static_cast<float>(j + 1) / static_cast<float>(m_property.nSkyGridPoints));
				vecSky_rawData[floatnum].pos.y = 4000.f * Math::Sin(-0.5f * Math::PI + Math::PI * static_cast<float>(j + 1) / static_cast<float>(m_property.nSkyGridPoints));
				vecSky_rawData[floatnum].pos.z = m_property.nGridPoints * m_property.fGeometryScale * 0.5f + 4000.f * Math::Sin(2.f * Math::PI * static_cast<float>(index) / static_cast<float>(m_property.nSkyGridPoints)) * Math::Cos(-0.5f * Math::PI + Math::PI * static_cast<float>(j + 1) / static_cast<float>(m_property.nSkyGridPoints));
				vecSky_rawData[floatnum].uv.x = (m_property.fSkyTextureAngle + static_cast<float>(index) / static_cast<float>(m_property.nSkyGridPoints));
				vecSky_rawData[floatnum].uv.y = 2.f - 2.f * static_cast<float>(j + 1) / static_cast<float>(m_property.nSkyGridPoints);
				++floatnum;
			}

			m_pSky = Graphics::IVertexBuffer::Create(Graphics::VertexPosTex::Format(), vecSky_rawData.size(), &vecSky_rawData.front(), D3D11_USAGE_DEFAULT);

			m_pTexRockBump = Graphics::ITexture::Create(File::GetFileName(m_property.strTexRockBumpFile).c_str(), m_property.strTexRockBumpFile.c_str());
			m_pTexRockMicroBump = Graphics::ITexture::Create(File::GetFileName(m_property.strTexRockMicroFile).c_str(), m_property.strTexRockMicroFile.c_str());
			m_pTexRockDiffuse = Graphics::ITexture::Create(File::GetFileName(m_property.strTexRockDiffuseFile).c_str(), m_property.strTexRockDiffuseFile.c_str());

			m_pTexSandBump = Graphics::ITexture::Create(File::GetFileName(m_property.strTexSandBumpFile).c_str(), m_property.strTexSandBumpFile.c_str());
			m_pTexSandMicroBump = Graphics::ITexture::Create(File::GetFileName(m_property.strTexSandMicroFile).c_str(), m_property.strTexSandMicroFile.c_str());
			m_pTexSandDIffuse = Graphics::ITexture::Create(File::GetFileName(m_property.strTexSandDiffuseFile).c_str(), m_property.strTexSandDiffuseFile.c_str());
			
			m_pTexGrassDiffuse = Graphics::ITexture::Create(File::GetFileName(m_property.strTexGrassDiffuse).c_str(), m_property.strTexGrassDiffuse.c_str());
			m_pTexSlopeDiffuse = Graphics::ITexture::Create(File::GetFileName(m_property.strTexSlopeDiffuse).c_str(), m_property.strTexSlopeDiffuse.c_str());

			/*std::string strPath = File::GetPath(File::eTexture);
			strPath.append("Terrain\\rock_bump6.dds");
			m_pTexRockBump = Graphics::ITexture::Create(File::GetFileName(strPath).c_str(), strPath.c_str());

			strPath = File::GetPath(File::eTexture);
			strPath.append("Terrain\\rock_bump4.dds");
			m_pTexRockMicroBump = Graphics::ITexture::Create(File::GetFileName(strPath).c_str(), strPath.c_str());

			strPath = File::GetPath(File::eTexture);
			strPath.append("Terrain\\terrain_rock4.dds");
			m_pTexRockDiffuse = Graphics::ITexture::Create(File::GetFileName(strPath).c_str(), strPath.c_str());

			strPath = File::GetPath(File::eTexture);
			strPath.append("Terrain\\rock_bump4.dds");
			m_pTexSandBump = Graphics::ITexture::Create(File::GetFileName(strPath).c_str(), strPath.c_str());

			strPath = File::GetPath(File::eTexture);
			strPath.append("Terrain\\lichen1_normal.dds");
			m_pTexSandMicroBump = Graphics::ITexture::Create(File::GetFileName(strPath).c_str(), strPath.c_str());

			strPath = File::GetPath(File::eTexture);
			strPath.append("Terrain\\sand_diffuse.dds");
			m_pTexSandDIffuse = Graphics::ITexture::Create(File::GetFileName(strPath).c_str(), strPath.c_str());

			strPath = File::GetPath(File::eTexture);
			strPath.append("Terrain\\terrain_grass.dds");
			m_pTexGrassDiffuse = Graphics::ITexture::Create(File::GetFileName(strPath).c_str(), strPath.c_str());

			strPath = File::GetPath(File::eTexture);
			strPath.append("Terrain\\terrain_slope.dds");
			m_pTexSlopeDiffuse = Graphics::ITexture::Create(File::GetFileName(strPath).c_str(), strPath.c_str());*/
		}

		void Terrain::Update(float fElapsedTime)
		{
			Graphics::RenderSubsetTerrain terrain;
			terrain.pVertexBuffer = m_pHeightField;
			terrain.pTexHeightField = m_pTexHeightMap;
			terrain.pTexLayerdef = m_pTexLayerdef;
			terrain.pTexRockBump = m_pTexRockBump;
			terrain.pTexRockMicroBump = m_pTexRockMicroBump;
			terrain.pTexRockDiffuse = m_pTexRockDiffuse;
			terrain.pTexSandBump = m_pTexSandBump;
			terrain.pTexSandMicroBump = m_pTexSandMicroBump;
			terrain.pTexSandDiffuse = m_pTexSandDIffuse;
			terrain.pTexGrassDiffuse = m_pTexGrassDiffuse;
			terrain.pTexSlopeDiffuse = m_pTexSlopeDiffuse;
			terrain.fHeightFieldSize = m_property.nGridPoints * m_property.fGeometryScale;
			terrain.matWorld = Math::Matrix::CreateTranslation(-m_property.nGridPoints * m_property.fGeometryScale * 0.5f, 0.f, -m_property.nGridPoints * m_property.fGeometryScale * 0.5f);
			terrain.fHalfSpaceCullSign = m_property.isHalfSpaceCullSign == true ? 1.f : 0.f;
			terrain.fHalfSpaceCullPosition = m_property.fHalfSpaceCullHeight;

			Graphics::RendererManager::GetInstance()->AddRender(terrain);
		}
	}
}
#endif