#include "stdafx.h"
#include "Terrain.h"

#ifndef NEW_TERRAIN
#include "CommonLib/FileStream.h"

#include "DirectX/D3DInterface.h"
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
			: m_f3Scale(Math::Vector3::One)
			, m_f3PrevScale(Math::Vector3::One)
			, m_isDestroy(false)
			, m_isVisible(true)
			, m_isDirtyWorldMatrix(true)
			, m_nCellCount(0)
			, m_pPhysics(nullptr)
			, m_nLastCell(std::numeric_limits<uint32_t>::max())
			, m_isShowCellLine(false)
			, m_isLoadComplete(false)
		{
		}

		Terrain::~Terrain()
		{
			if (m_optLoadTask.has_value() == true)
			{
				Concurrency::task<bool>& task = m_optLoadTask.value();
				if (task.is_done() == false)
				{
					task.wait();
				}
			}

			SafeDelete(m_pPhysics);

			uint32_t nSize = m_veTerrainCells.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				m_veTerrainCells[i].Release();
			}
			m_veTerrainCells.clear();

			m_vecHeight.clear();
			m_vecHeightMap.clear();

			if (m_optVertices.has_value() == true)
			{
				std::vector<Math::Vector3>& vecVertices = m_optVertices.value();
				vecVertices.clear();

				m_optVertices.reset();
			}

			std::for_each(m_vecMaterial.begin(), m_vecMaterial.end(), [](Graphics::IMaterial* pMaterial)
			{
				Graphics::IMaterial::Destroy(&pMaterial);
			});
			m_vecMaterial.clear();
		}

		const Math::Matrix& Terrain::CalcWorldMatrix()
		{
			m_isDirtyWorldMatrix = false;
/*
			Math::Vector3 f3Pos(m_f3Pos);
			f3Pos.x -= m_property.nGridPoints * m_property.fGeometryScale * 0.5f;
			f3Pos.z -= m_property.nGridPoints * m_property.fGeometryScale * 0.5f;

			Math::Matrix::Compose(m_f3Scale, m_quatPrevRotation, f3Pos, m_matWorld);*/

			return m_matWorld;
		}

		bool Terrain::Init(const TerrainProperty* pTerrainProperty, bool isEnableThreadLoad)
		{
			m_property = *pTerrainProperty;

			if (isEnableThreadLoad == true)
			{
				m_optLoadTask = Concurrency::create_task([&]() -> bool
				{
					return init();
				});
			}
			else
			{
				return init();
			}

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

		bool Terrain::init()
		{
			if (loadRawHeightmap(m_property.strTexHeightMap.c_str()) == false)
				return false;

			if (loadColorMap(m_property.strTexColorMap.c_str()) == false)
				return false;

			if (initTerrain() == false)
				return false;

			m_property.materialInfo.f4DisRoughMetEmi.y = 1.f;
			m_property.materialInfo.f4DisRoughMetEmi.z = 0.f;
			m_vecMaterial.emplace_back(Graphics::IMaterial::Create(&m_property.materialInfo));

			//m_property.rigidBodyProperty.fMass = 0.f;
			//m_property.rigidBodyProperty.strName = StrID::Terrain_Physics;
			//m_property.rigidBodyProperty.nCollisionFlag = Physics::EmCollision::eStaticObject;
			//
			//if (m_optVertices.has_value() == true)
			//{
			//	std::vector<Math::Vector3>& vecVertices = m_optVertices.value();
			//	m_property.rigidBodyProperty.shapeInfo.SetTriangleMesh(vecVertices.data(), vecVertices.size());
			//}
			//
			//m_pPhysics = Physics::RigidBody::Create(m_property.rigidBodyProperty);

			m_optVertices.reset();

			m_vecHeight.clear();
			m_vecHeight.resize(0);
			m_vecHeightMap.clear();
			m_vecHeightMap.resize(0);

			m_isLoadComplete = true;

			return true;
		}

		void Terrain::Update(float fElapsedTime)
		{
			if (m_isLoadComplete == false)
				return;

			Graphics::Camera* pMainCamera = Graphics::CameraManager::GetInstance()->GetMainCamera();
			if (pMainCamera == nullptr)
				return;

			const Math::Vector3& f3Pos = pMainCamera->GetPosition();

			int nCellIdx = -1;
			m_nLastCell = std::min(m_nLastCell, m_veTerrainCells.size() - 1);

			if (m_veTerrainCells[m_nLastCell].IsInsideCell(f3Pos) == true)
			{
				nCellIdx = m_nLastCell;
			}
			else
			{
				uint32_t nSize = m_veTerrainCells.size();
				for (uint32_t i = 0; i < nSize; ++i)
				{
					if (m_veTerrainCells[i].IsInsideCell(f3Pos) == true)
					{
						m_nLastCell = nCellIdx = i;
						break;
					}
				}
			}

			if (nCellIdx == -1)
			{
				float fMinDist = std::numeric_limits<float>::max();
				uint32_t nSize = m_veTerrainCells.size();
				for (uint32_t i = 0; i < nSize; ++i)
				{
					if (pMainCamera->IsFrustumContains(m_veTerrainCells[i].GetBoundingBox()) == Collision::EmContainment::eDisjoint)
						continue;

					Math::Vector3 f3Dist = f3Pos - m_veTerrainCells[i].GetBoundingBox().Center;
					float fDist = f3Dist.LengthSquared();
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
				uint32_t nCellRowCount = (m_property.nWidth - 1) / (m_property.nCellWidth - 1);

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

					if (m_veTerrainCells[i].IsInsideCell(f3Pos) == false)
						continue;

					m_veTerrainCells[i].Update(fElapsedTime, m_vecMaterial, m_isShowCellLine);
				}
			}

			if (m_pPhysics != nullptr)
			{
				m_pPhysics->Update(fElapsedTime);
			}
		}

		float Terrain::GetHeight(float fPosX, float fPosZ) const
		{
			//int nCellIdx = -1;
			//uint32_t nSize = m_veTerrainCells.size();
			//for (uint32_t i = 0; i < nSize; ++i)
			//{
			//	if (m_veTerrainCells[i].IsInsideDimensions(fPosX, fPosZ) == true)
			//	{
			//		nCellIdx = i;
			//		break;
			//	}
			//}
			//
			//if (nCellIdx == -1)
			//	return 0.f;

			// If this is the right cell then check all the triangles in this cell to see what the height of the triangle at this position is.
			//uint32_t nCount = m_veTerrainCells[nCellIdx].GetVertexCount() / 3;
			//for (uint32_t i = 0; i < nCount; ++i)
			//{
			//	uint32_t nIdx = i * 3;
			//
			//	Vector3 v0 = m_veTerrainCells[nCellIdx].GetVertex(nIdx);
			//	nIdx++;
			//
			//	Vector3 v1 = m_veTerrainCells[nCellIdx].GetVertex(nIdx);
			//	nIdx++;
			//
			//	Vector3 v2 = m_veTerrainCells[nCellIdx].GetVertex(nIdx);
			//
			//	float fHeight = 0.f;
			//	// Check to see if this is the polygon we are looking for.
			//	if (checkHeightOfTriangle(fPosX, fPosZ, fHeight, v0, v1, v2))
			//		return fHeight;
			//}

			return 0.f;
		}

		bool Terrain::loadHeightMap(const char* strFilePath)
		{
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eBinary | File::EmState::eRead) == false)
				return false;

			BITMAPFILEHEADER bitmapFileHeader;
			file.Read(reinterpret_cast<char*>(&bitmapFileHeader), sizeof(BITMAPFILEHEADER));

			BITMAPINFOHEADER bitmapInfoHeader;
			file.Read(reinterpret_cast<char*>(&bitmapInfoHeader), sizeof(BITMAPINFOHEADER));

			// Calculate the size of the bitmap image data.  
			// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
			uint32_t imageSize = m_property.nHeight * ((m_property.nWidth * 3) + 1);

			// Allocate memory for the bitmap image data.
			std::vector<unsigned char> vecBitmapImage;
			vecBitmapImage.resize(imageSize);

			file.Read(reinterpret_cast<char*>(vecBitmapImage.data()), imageSize);

			file.Close();

			m_vecHeightMap.clear();
			m_vecHeightMap.resize(m_property.nWidth * m_property.nHeight);

			int k = 0;
			int nIdx = 0;
			float fZ = static_cast<float>(m_property.nHeight - 1);
			for (int i = 0; i < m_property.nHeight; ++i)
			{
				for (int j = 0; j < m_property.nWidth; ++j)
				{
					// Set the X and Z coordinates.
					m_vecHeightMap[nIdx].pos.x = static_cast<float>(j) * m_property.f3Scaling.x + m_f3DefaultPos.x;
					m_vecHeightMap[nIdx].pos.z = (-static_cast<float>(i) + fZ) * m_property.f3Scaling.z + m_f3DefaultPos.z;

					int nIdx_y = (m_property.nWidth * (m_property.nHeight - 1 - i)) + j;
					m_vecHeightMap[nIdx_y].pos.y = static_cast<float>(vecBitmapImage[k]) / m_property.f3Scaling.y + m_f3DefaultPos.y;

					++nIdx;

					k += 3;
				}

				++k;
			}

			vecBitmapImage.clear();

			//// Start by creating the array structure to hold the height map data.
			//// Open the bitmap map file in binary.
			//FILE* filePtr = nullptr;
			//if (fopen_s(&filePtr, strFilePath, "rb") != 0)
			//	return false;
			//
			//BITMAPFILEHEADER bitmapFileHeader;
			//// Read in the bitmap file header.
			//if (fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1)
			//	return false;
			//
			//BITMAPINFOHEADER bitmapInfoHeader;
			//// Read in the bitmap info header.
			//if (fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1)
			//	return false;
			//
			//// Calculate the size of the bitmap image data.  
			//// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
			//uint32_t imageSize = m_property.nHeight * ((m_property.nWidth * 3) + 1);
			//
			//// Allocate memory for the bitmap image data.
			//std::vector<unsigned char> vecBitmapImage;
			//vecBitmapImage.resize(imageSize);
			//
			//// Read in the bitmap image data.
			//if (fread(&vecBitmapImage[0], 1, imageSize, filePtr) != imageSize)
			//	return false;
			//
			//// Close the file.
			//if (fclose(filePtr) != 0)
			//	return false;
			//
			//m_vecHeightMap.clear();
			//m_vecHeightMap.resize(m_property.nWidth * m_property.nHeight);
			//
			//int k = 0;
			//int nIdx = 0;
			//float fZ = static_cast<float>(m_property.nHeight - 1);
			//for (uint32_t i = 0; i < m_property.nHeight; ++i)
			//{
			//	for (uint32_t j = 0; j < m_property.nWidth; ++j)
			//	{
			//		// Set the X and Z coordinates.
			//		m_vecHeightMap[nIdx].pos.x = static_cast<float>(j) * m_f3Scaling.x + m_f3DefaultPos.x;
			//		m_vecHeightMap[nIdx].pos.z = (-static_cast<float>(i) + fZ) * m_f3Scaling.z + m_f3DefaultPos.z;
			//
			//		int nIdx_y = (m_property.nWidth * (m_property.nHeight - 1 - i)) + j;
			//		m_vecHeightMap[nIdx_y].pos.y = static_cast<float>(vecBitmapImage[k]) / m_f3Scaling.y + m_f3DefaultPos.y;
			//
			//		++nIdx;
			//
			//		k += 3;
			//	}
			//
			//	++k;
			//}
			//
			//vecBitmapImage.clear();
			
			return true;
		}

		bool Terrain::loadColorMap(const char* strFilePath)
		{
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eBinary | File::EmState::eRead) == false)
				return false;

			BITMAPFILEHEADER bitmapFileHeader;
			file.Read(reinterpret_cast<char*>(&bitmapFileHeader), sizeof(BITMAPFILEHEADER));

			BITMAPINFOHEADER bitmapInfoHeader;
			file.Read(reinterpret_cast<char*>(&bitmapInfoHeader), sizeof(BITMAPINFOHEADER));

			// Calculate the size of the bitmap image data.  
			// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
			uint32_t imageSize = m_property.nHeight * ((m_property.nWidth * 3) + 1);

			// Allocate memory for the bitmap image data.
			std::vector<unsigned char> vecBitmapImage;
			vecBitmapImage.resize(imageSize);

			file.Read(reinterpret_cast<char*>(vecBitmapImage.data()), imageSize);

			file.Close();

			// Initialize the position in the image data buffer.
			int k = 0;
			// Read the image data into the color map portion of the height map structure.
			for (int j = 0; j < m_property.nHeight; ++j)
			{
				for (int i = 0; i < m_property.nWidth; ++i)
				{
					// Bitmaps are upside down so load bottom to top into the array.
					int nIdx = (m_property.nWidth * (m_property.nHeight - 1 - j)) + i;

					m_vecHeightMap[nIdx].color.a = 1.f;
					m_vecHeightMap[nIdx].color.b = static_cast<float>(vecBitmapImage[k] / 255.f);
					m_vecHeightMap[nIdx].color.g = static_cast<float>(vecBitmapImage[k + 1] / 255.f);
					m_vecHeightMap[nIdx].color.r = static_cast<float>(vecBitmapImage[k + 2] / 255.f);

					k += 3;
				}

				// Compensate for extra byte at end of each line in non-divide by 2 bitmaps (eg. 257x257).
				++k;
			}


			//// Open the color map file in binary.
			//FILE* filePtr = nullptr;
			//if (fopen_s(&filePtr, strFilePath, "rb") != 0)
			//	return false;
			//
			//// Read in the file header.
			//BITMAPFILEHEADER bitmapFileHeader;
			//if (fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1)
			//	return false;
			//
			//// Read in the bitmap info header.
			//BITMAPINFOHEADER bitmapInfoHeader;
			//if (fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1)
			//	return false;
			//
			//// Calculate the size of the bitmap image data.  Since this is non-divide by 2 dimensions (eg. 257x257) need to add extra byte to each line.
			//uint32_t nImageSize = m_property.nHeight * ((m_property.nWidth * 3) + 1);
			//
			//// Allocate memory for the bitmap image data.
			//std::vector<unsigned char> vecBitmapImage;
			//vecBitmapImage.resize(nImageSize);
			//
			//// Read in the bitmap image data.
			//if (fread(&vecBitmapImage[0], 1, nImageSize, filePtr) != nImageSize)
			//	return false;
			//
			//// Close the file.
			//if (fclose(filePtr) != 0)
			//	return false;
			//
			//// Initialize the position in the image data buffer.
			//int k = 0;
			//// Read the image data into the color map portion of the height map structure.
			//for (uint32_t j = 0; j < m_property.nHeight; ++j)
			//{
			//	for (uint32_t i = 0; i < m_property.nWidth; ++i)
			//	{
			//		// Bitmaps are upside down so load bottom to top into the array.
			//		int nIdx = (m_property.nWidth * (m_property.nHeight - 1 - j)) + i;
			//
			//		m_vecHeightMap[nIdx].color.w = 1.f;
			//		m_vecHeightMap[nIdx].color.z = static_cast<float>(vecBitmapImage[k] / 255.f);
			//		m_vecHeightMap[nIdx].color.y = static_cast<float>(vecBitmapImage[k + 1] / 255.f);
			//		m_vecHeightMap[nIdx].color.x = static_cast<float>(vecBitmapImage[k + 2] / 255.f);
			//
			//		k += 3;
			//	}
			//
			//	// Compensate for extra byte at end of each line in non-divide by 2 bitmaps (eg. 257x257).
			//	++k;
			//}

			return true;
		}

		bool Terrain::loadRawHeightmap(const char* strFilePath)
		{
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eBinary | File::EmState::eRead) == false)
				return false;

			// Start by creating the array structure to hold the height map data.
			// Open the bitmap map file in binary.
			//FILE* filePtr = nullptr;
			//if (fopen_s(&filePtr, strFilePath, "rb") != 0)
			//	return false;

			// Calculate the size of the bitmap image data.  
			// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
			uint32_t imageSize = m_property.nHeight * m_property.nWidth;

			m_vecHeightMap.clear();
			m_vecHeightMap.reserve(imageSize);

			m_vecHeight.clear();
			m_vecHeight.reserve(imageSize);

			std::vector<uint16_t> vecRawData;
			vecRawData.resize(imageSize);

			file.Read(reinterpret_cast<char*>(vecRawData.data()), imageSize * 2);

			//if (fread(&vecRawData[0], sizeof(uint16_t), imageSize, filePtr) != imageSize)
			//	return false;
			//
			//if (fclose(filePtr) != 0)
			//	return false;

			float fZ = static_cast<float>(m_property.nHeight - 1);

			std::deque<float> dequeHeight;

			uint32_t nIdx = 0;
			for (int i = 0; i < m_property.nHeight; ++i)
			{
				for (int j = 0; j < m_property.nWidth; ++j)
				{
					HeightMapVertex vertex;

					// Set the X and Z coordinates.
					vertex.pos.x = static_cast<float>(j) * m_property.f3Scaling.x + m_f3DefaultPos.x;
					vertex.pos.z = (-static_cast<float>(i) + fZ) * m_property.f3Scaling.z + m_f3DefaultPos.z;

					// Scale the height.
					vertex.pos.y = static_cast<float>(vecRawData[nIdx]) / m_property.f3Scaling.y + m_f3DefaultPos.y;

					if (m_property.fHeightMax > vertex.pos.y)
					{
						m_property.fHeightMax = vertex.pos.y;
					}

					if (m_property.fHeightMin < vertex.pos.y)
					{
						m_property.fHeightMin = vertex.pos.y;
					}

					m_vecHeight.emplace_back(vertex.pos.y);
					m_vecHeightMap.emplace_back(vertex);

					++nIdx;
				}
			}

			vecRawData.clear();

			return true;
		}

		bool Terrain::initTerrain()
		{
			uint32_t nVertexCount = (m_property.nHeight - 1) * (m_property.nWidth - 1) * 6;

			std::vector<Graphics::VertexPosTexNorCol> vecVertex;
			vecVertex.reserve(nVertexCount);

			m_optVertices.reset();
			std::vector<Math::Vector3>& vecVertices = m_optVertices.emplace();
			vecVertices.reserve(nVertexCount);

			std::vector<Math::Vector3> vecNormal;
			vecNormal.reserve((m_property.nHeight - 1) * (m_property.nWidth - 1));

			int nHeight = m_property.nHeight - 1;
			int nWidth = m_property.nWidth - 1;
			for (int j = 0; j < nHeight; ++j)
			{
				for (int i = 0; i < nWidth; ++i)
				{
					int nIdx1 = ((j + 1) * m_property.nWidth) + i;		// Bottom left vertex.
					int nIdx2 = ((j + 1) * m_property.nWidth) + (i + 1);	// Bottom right vertex.
					int nIdx3 = (j * m_property.nWidth) + i;				// Upper left vertex.

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
			nHeight = m_property.nHeight;
			nWidth = m_property.nWidth;
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

						sum += vecNormal[nIdx];
					}

					// Bottom right face.
					if ((i < (nWidth - 1)) && ((j - 1) >= 0))
					{
						nIdx = ((j - 1) * (nWidth - 1)) + i;

						sum += vecNormal[nIdx];
					}

					// Upper left face.
					if (((i - 1) >= 0) && (j < (nHeight - 1)))
					{
						nIdx = (j * (nWidth - 1)) + (i - 1);

						sum += vecNormal[nIdx];
					}

					// Upper right face.
					if ((i < (nWidth - 1)) && (j < (nHeight - 1)))
					{
						nIdx = (j * (nWidth - 1)) + i;

						sum += vecNormal[nIdx];
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

			// Load the 3D terrain model with the height map terrain data.
			// We will be creating 2 triangles for each of the four points in a quad.
			nHeight = m_property.nHeight - 1;
			nWidth = m_property.nWidth - 1;
			for (int i = 0; i < nHeight; ++i)
			{
				for (int j = 0; j < nWidth; ++j)
				{
					// Get the indexes to the four points of the quad.
					uint32_t nIdx1 = (m_property.nWidth * i) + j;				// Upper left.
					uint32_t nIdx2 = (m_property.nWidth * i) + (j + 1);			// Upper right.
					uint32_t nIdx3 = (m_property.nWidth * (i + 1)) + j;			// Bottom left.
					uint32_t nIdx4 = (m_property.nWidth * (i + 1)) + (j + 1);	// Bottom right.

					// Now create two triangles for that quad.
					// Triangle 1 - Upper left.
					vecVertex.push_back({
						m_vecHeightMap[nIdx1].pos,
						Math::Vector2::Zero,
						m_vecHeightMap[nIdx1].normal,
						m_vecHeightMap[nIdx1].color
					});
					vecVertices.push_back(m_vecHeightMap[nIdx1].pos);

					// Triangle 1 - Upper right.
					vecVertex.push_back({
						m_vecHeightMap[nIdx2].pos,
						Math::Vector2(1.f, 0.f),
						m_vecHeightMap[nIdx2].normal,
						m_vecHeightMap[nIdx2].color
					});
					vecVertices.push_back(m_vecHeightMap[nIdx2].pos);

					// Triangle 1 - Bottom left.
					vecVertex.push_back({
						m_vecHeightMap[nIdx3].pos,
						Math::Vector2(0.f, 1.f),
						m_vecHeightMap[nIdx3].normal,
						m_vecHeightMap[nIdx3].color
					});
					vecVertices.push_back(m_vecHeightMap[nIdx3].pos);

					// Triangle 2 - Bottom left.
					vecVertex.push_back({
						m_vecHeightMap[nIdx3].pos,
						Math::Vector2(0.f, 1.f),
						m_vecHeightMap[nIdx3].normal,
						m_vecHeightMap[nIdx3].color
					});
					vecVertices.push_back(m_vecHeightMap[nIdx3].pos);

					// Triangle 2 - Upper right.
					vecVertex.push_back({
						m_vecHeightMap[nIdx2].pos,
						Math::Vector2(1.f, 0.f),
						m_vecHeightMap[nIdx2].normal,
						m_vecHeightMap[nIdx2].color
					});
					vecVertices.push_back(m_vecHeightMap[nIdx2].pos);

					// Triangle 2 - Bottom right.
					vecVertex.push_back({
						m_vecHeightMap[nIdx4].pos,
						Math::Vector2(1.f, 1.f),
						m_vecHeightMap[nIdx4].normal,
						m_vecHeightMap[nIdx4].color
					});
					vecVertices.push_back(m_vecHeightMap[nIdx4].pos);
				}
			}

			return initTerrainCell(vecVertex);
		}

		bool Terrain::initTerrainCell(const std::vector<Graphics::VertexPosTexNorCol>& vecModel)
		{
			int nCellRowCount = (m_property.nWidth - 1) / (m_property.nCellWidth - 1);
			m_nCellCount = nCellRowCount * nCellRowCount;

			m_veTerrainCells.resize(m_nCellCount);

			Concurrency::parallel_for(0, nCellRowCount * nCellRowCount, [&](int n)
			{
				int i = n / nCellRowCount;
				int j = n % nCellRowCount;

				m_veTerrainCells[n].Init(vecModel, j, i, m_property.nCellWidth, m_property.nCellHeight, m_property.nWidth);
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

		std::optional<float> Terrain::checkHeightOfTriangle(float x, float z, const Math::Vector3& v0, const Math::Vector3& v1, const Math::Vector3& v2)
		{
			// Starting position of the ray that is being cast.
			Math::Vector3 f3StartVector(x, 0.f, z);

			// The direction the ray is being cast.
			const Math::Vector3 f3DrectionVector(0.f, -1.f, 0.f);

			// Calculate the two edges from the three points given.
			const Math::Vector3 vEdge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);

			const Math::Vector3 vEdge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

			// Calculate the normal of the triangle from the two edges.
			Math::Vector3 vNormal((vEdge1.y * vEdge2.z) - (vEdge1.z * vEdge2.y),
				(vEdge1.z * vEdge2.x) - (vEdge1.x * vEdge2.z),
				(vEdge1.x * vEdge2.y) - (vEdge1.y * vEdge2.x));

			float fMagnitude = std::sqrt((vNormal.x * vNormal.x) + (vNormal.y * vNormal.y) + (vNormal.z * vNormal.z));
			vNormal.x = vNormal.x / fMagnitude;
			vNormal.y = vNormal.y / fMagnitude;
			vNormal.z = vNormal.z / fMagnitude;

			// Find the distance from the origin to the plane.
			float D = ((-vNormal.x * v0.x) + (-vNormal.y * v0.y) + (-vNormal.z * v0.z));

			// Get the denominator of the equation.
			float fDenominator = ((vNormal.x * f3DrectionVector.x) + (vNormal.y * f3DrectionVector.y) + (vNormal.z * f3DrectionVector.z));

			// Make sure the result doesn't get too close to zero to prevent divide by zero.
			if (std::abs(fDenominator) < 0.0001f)
				return {};

			// Get the numerator of the equation.
			float fNumerator = -1.f * (((vNormal.x * f3StartVector.x) + (vNormal.y * f3StartVector.y) + (vNormal.z * f3StartVector.z)) + D);

			// Calculate where we intersect the triangle.
			float t = fNumerator / fDenominator;

			// Find the intersection vector.
			Math::Vector3 Q(f3StartVector.x + (f3DrectionVector.x * t),
				f3StartVector.y + (f3DrectionVector.y * t),
				f3StartVector.z + (f3DrectionVector.z * t));

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
				return {};

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
				return {};

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
				return {};

			return Q.y;
		}

		//bool Terrain::checkHeightOfTriangle(float x, float z, float& height, Vector3& v0, Vector3& v1, Vector3& v2)
		//{
		//	//float f3StartVector[3], f3DrectionVector[3], vEdge1[3], vEdge2[3], m_vecNormals[3];
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

#include "Physics/RigidBody.h"

namespace EastEngine
{
	namespace GameObject
	{
		static uint32_t s_nTerrainIndex = 0;

		Terrain::Terrain()
			: m_f3Scale(Math::Vector3::One)
			, m_f3PrevScale(Math::Vector3::One)
			, m_isDestroy(false)
			, m_isVisible(true)
			, m_isDirtyWorldMatrix(true)
			, m_pHeightField(nullptr)
			, m_pSky(nullptr)
			, m_pRigidBody(nullptr)
			, m_pDebugTriangles(nullptr)
			, m_pDebugTrianglesIB(nullptr)
		{
		}

		Terrain::~Terrain()
		{
			SafeDelete(m_pRigidBody);

			SafeDelete(m_pHeightField);
			SafeDelete(m_pSky);

			SafeDelete(m_pDebugTriangles);
			SafeDelete(m_pDebugTrianglesIB);
		}

		void Terrain::Init(const TerrainProperty* pTerrainProperty)
		{
			++s_nTerrainIndex;

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

			int nGridPointSize = m_property.nGridPoints + 1;
			m_vecHeights = std::vector<std::vector<float>>(nGridPointSize, std::vector<float>(nGridPointSize));
			m_vecNormals = std::vector<std::vector<Math::Vector3>>(nGridPointSize, std::vector<Math::Vector3>(nGridPointSize));

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
					//float mv = static_cast<float>((i - m_property.nGridPoints / 2.f) * (i - m_property.nGridPoints / 2.f) + (j - m_property.nGridPoints / 2.f) * (j - m_property.nGridPoints / 2.f));
					//rm = static_cast<float>((m_property.nGridPoints * 0.8f) * (m_property.nGridPoints * 0.8f) / 4.f);
					//if (mv > rm)
					//{
					//	m_vecHeights[i][j] -= ((mv - rm) / 1000.f) * m_property.fGeometryScale;
					//}

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

			String::StringID strName;
			strName.Format("TerrainLayerDefMap_%d", s_nTerrainIndex);
			m_pTexLayerdef = Graphics::ITexture::Create(strName, tex_desc, &subresource_data);

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

			strName.Format("TerrainHeightMap_%d", s_nTerrainIndex);
			m_pTexHeightMap = Graphics::ITexture::Create(strName, tex_desc, &subresource_data);

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

			strName.Format("TerrainDepthMap_%d", s_nTerrainIndex);
			m_pTexDepthMap = Graphics::ITexture::Create(strName, tex_desc, &subresource_data);

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

			Physics::RigidBodyProperty physicsProp;
			physicsProp.fMass = 0.f;
			physicsProp.fRestitution = 0.5f;
			physicsProp.fFriction = 0.5f;
			physicsProp.fLinearDamping = 0.5f;
			physicsProp.fAngularDamping = 0.5f;
			
			physicsProp.strName = "Terrain_Physics";
			physicsProp.nCollisionFlag = Physics::EmCollision::eStaticObject;

			physicsProp.shapeInfo.SetTerrain({ nGridPointSize ,nGridPointSize }, m_property.fGeometryScale, m_property.fMaxHeight, m_property.fMinHeight, &m_vecHeights[0][0], nGridPointSize * nGridPointSize);
			physicsProp.funcTriangleDrawCallback = [&](const Math::Vector3* pTriangles, const uint32_t nCount)
			{
				PhysicsDebugDrawCallback(pTriangles, nCount);
			};
			
			m_pRigidBody = Physics::RigidBody::Create(physicsProp);
			m_pRigidBody->SetEnableTriangleDrawCallback(true);
		}

		void Terrain::Update(float fElapsedTime)
		{
			if (m_isDestroy == true)
				return;

			if (m_isDirtyWorldMatrix == true)
			{
				Math::Vector3 f3Pos(m_f3Pos);
				f3Pos.x -= m_property.nGridPoints * m_property.fGeometryScale * 0.5f;
				f3Pos.z -= m_property.nGridPoints * m_property.fGeometryScale * 0.5f;

				Math::Matrix::Compose(m_f3Scale, m_quatPrevRotation, f3Pos, m_matWorld);

				m_isDirtyWorldMatrix = false;
			}

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
			terrain.matWorld = m_matWorld;
			terrain.fHalfSpaceCullSign = m_property.isHalfSpaceCullSign == true ? 1.f : 0.f;
			terrain.fHalfSpaceCullPosition = m_property.fHalfSpaceCullHeight;

			Graphics::RendererManager::GetInstance()->AddRender(terrain);

			PhysicsUpdate(fElapsedTime);
		}

		void Terrain::PhysicsUpdate(float fElapsedTime)
		{
			//if (m_pRigidBody != nullptr)
			//{
			//	m_pRigidBody->Update(fElapsedTime);
			//
			//	if (m_pDebugTriangles != nullptr || m_pDebugTrianglesIB != nullptr)
			//	{
			//		Graphics::RendererManager::GetInstance()->AddRender({ m_pDebugTriangles, m_pDebugTrianglesIB, &m_matWorld, Math::Color::Red });
			//	}
			//}
		}

		const Math::Matrix& Terrain::CalcWorldMatrix()
		{
			m_isDirtyWorldMatrix = false;

			Math::Vector3 f3Pos(m_f3Pos);
			f3Pos.x -= m_property.nGridPoints * m_property.fGeometryScale * 0.5f;
			f3Pos.z -= m_property.nGridPoints * m_property.fGeometryScale * 0.5f;

			Math::Matrix::Compose(m_f3Scale, m_quatPrevRotation, f3Pos, m_matWorld);

			return m_matWorld;
		}

		void Terrain::PhysicsDebugDrawCallback(const Math::Vector3* pTriangles, const uint32_t nCount)
		{
			SafeDelete(m_pDebugTriangles);
			SafeDelete(m_pDebugTrianglesIB);

			m_pDebugTriangles = Graphics::IVertexBuffer::Create(Graphics::VertexPos::Format(), nCount, pTriangles, D3D11_USAGE_IMMUTABLE);

			std::vector<uint32_t> vecIndices(nCount);
			for (uint32_t i = 0; i < vecIndices.size(); ++i)
			{
				vecIndices[i] = i;
			}

			m_pDebugTrianglesIB = Graphics::IIndexBuffer::Create(nCount, vecIndices.data(), D3D11_USAGE_IMMUTABLE);

			if (m_pDebugTriangles != nullptr || m_pDebugTrianglesIB != nullptr)
			{
				m_pRigidBody->SetEnableTriangleDrawCallback(false);
			}
		}
	}
}
#endif