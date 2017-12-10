#include "stdafx.h"
#include "Terrain.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

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
		static uint32_t s_nTerrainIndex = 0;

		Terrain::Terrain()
			: m_f3Scale(Math::Vector3::One)
			, m_f3PrevScale(Math::Vector3::One)
			, m_isDestroy(false)
			, m_isVisible(true)
			, m_isDirtyWorldMatrix(true)
			, m_isBuildComplete(false)
			, m_pPhysics(nullptr)
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

			m_vecHeightMap.clear();

			if (m_optVertices.has_value() == true)
			{
				std::vector<Math::Vector3>& vecVertices = m_optVertices.value();
				vecVertices.clear();

				m_optVertices.reset();
			}
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

			m_pTexDetailMap = Graphics::ITexture::Create(File::GetFileName(m_property.strTexDetailMap).c_str(), m_property.strTexDetailMap, true);
			m_pTexDetailNormalMap = Graphics::ITexture::Create(File::GetFileName(m_property.strTexDetailNormalMap).c_str(), m_property.strTexDetailNormalMap, true);

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

			m_vecHeightMap.resize(0);

			m_isBuildComplete = true;

			return true;
		}

		void Terrain::Update(float fElapsedTime)
		{
			if (m_isBuildComplete == false)
				return;

			if (IsVisible() == true)
			{
				if (m_pTexHeightMap->GetLoadState() == Graphics::EmLoadState::eComplete &&
					m_pTexColorMap->GetLoadState() == Graphics::EmLoadState::eComplete &&
					m_pTexDetailMap->GetLoadState() == Graphics::EmLoadState::eComplete &&
					m_pTexDetailNormalMap->GetLoadState() == Graphics::EmLoadState::eComplete)
				{
					Graphics::RenderSubsetTerrain subset;
					subset.pVertexBuffer = m_pHeightField;

					subset.f2PatchSize.x = m_property.n2Size.x / m_property.n2Patches.x;
					subset.f2PatchSize.y = m_property.n2Size.y / m_property.n2Patches.y;

					subset.f2HeightFieldSize.x = m_property.n2Size.x;
					subset.f2HeightFieldSize.y = m_property.n2Size.y;

					subset.pTexHeightField = m_pTexHeightMap;
					subset.pTexColorMap = m_pTexColorMap;

					subset.pTexDetailMap = m_pTexDetailMap;
					subset.pTexDetailNormalMap = m_pTexDetailNormalMap;

					Graphics::RendererManager::GetInstance()->AddRender(subset);
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
			uint32_t imageSize = m_property.n2Size.y * ((m_property.n2Size.x * 3) + 1);

			// Allocate memory for the bitmap image data.
			std::vector<unsigned char> vecBitmapImage;
			vecBitmapImage.resize(imageSize);

			file.Read(reinterpret_cast<char*>(vecBitmapImage.data()), imageSize);

			file.Close();

			m_vecHeightMap.clear();
			m_vecHeightMap.resize(m_property.n2Size.x * m_property.n2Size.y);

			int k = 0;
			int nIdx = 0;
			float fZ = static_cast<float>(m_property.n2Size.y - 1);
			for (int i = 0; i < m_property.n2Size.y; ++i)
			{
				for (int j = 0; j < m_property.n2Size.x; ++j)
				{
					// Set the X and Z coordinates.
					m_vecHeightMap[nIdx].pos.x = static_cast<float>(j);
					m_vecHeightMap[nIdx].pos.z = (-static_cast<float>(i) + fZ);

					int nIdx_y = (m_property.n2Size.x * (m_property.n2Size.y - 1 - i)) + j;
					m_vecHeightMap[nIdx_y].pos.y = static_cast<float>(vecBitmapImage[k]) / m_property.fHeightScale;

					++nIdx;

					k += 3;
				}

				++k;
			}

			vecBitmapImage.clear();

			return true;
		}

		bool Terrain::loadColorMap(const char* strFilePath)
		{
			m_pTexColorMap = Graphics::ITexture::Create(File::GetFileName(strFilePath).c_str(), strFilePath, false);

			return true;
		}

		bool Terrain::loadRawHeightmap(const char* strFilePath)
		{
			// Start by creating the array structure to hold the height map data.
			// Open the bitmap map file in binary.
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eBinary | File::EmState::eRead) == false)
				return false;

			// Calculate the size of the bitmap image data.  
			// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
			uint32_t imageSize = m_property.n2Size.y * m_property.n2Size.x;

			m_vecHeightMap.clear();
			m_vecHeightMap.reserve(imageSize);

			std::vector<uint16_t> vecRawData;
			vecRawData.resize(imageSize);

			file.Read(reinterpret_cast<char*>(vecRawData.data()), imageSize * 2);

			float fOffsetZ = static_cast<float>(m_property.n2Size.y - 1);

			uint32_t nIdx = 0;
			for (int i = 0; i < m_property.n2Size.y; ++i)
			{
				for (int j = 0; j < m_property.n2Size.x; ++j)
				{
					HeightMapVertex vertex;

					// Set the X and Z coordinates.
					vertex.pos.x = static_cast<float>(j);
					vertex.pos.z = (-static_cast<float>(i) + fOffsetZ);

					// Scale the height.
					vertex.pos.y = static_cast<float>(vecRawData[nIdx]) / m_property.fHeightScale;

					m_vecHeightMap.emplace_back(vertex);

					++nIdx;
				}
			}

			vecRawData.clear();

			return true;
		}

		bool Terrain::initTerrain()
		{
			uint32_t nVertexCount = (m_property.n2Size.y - 1) * (m_property.n2Size.x - 1) * 6;

			std::vector<Graphics::VertexPosTexNorCol> vecVertex;
			vecVertex.reserve(nVertexCount);

			m_optVertices.reset();
			std::vector<Math::Vector3>& vecVertices = m_optVertices.emplace();
			vecVertices.reserve(nVertexCount);

			std::vector<Math::Vector3> vecNormal;
			vecNormal.reserve((m_property.n2Size.y - 1) * (m_property.n2Size.x - 1));

			int nHeight = m_property.n2Size.y - 1;
			int nWidth = m_property.n2Size.x - 1;
			for (int j = 0; j < nHeight; ++j)
			{
				for (int i = 0; i < nWidth; ++i)
				{
					int nIdx1 = ((j + 1) * m_property.n2Size.x) + i;		// Bottom left vertex.
					int nIdx2 = ((j + 1) * m_property.n2Size.x) + (i + 1);	// Bottom right vertex.
					int nIdx3 = (j * m_property.n2Size.x) + i;				// Upper left vertex.

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

					vecNormal.emplace_back(vNormal);
				}
			}

			// Now go through all the vertices and take a sum of the face normals that touch this vertex.
			nHeight = m_property.n2Size.y;
			nWidth = m_property.n2Size.x;
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
			nHeight = m_property.n2Size.y - 1;
			nWidth = m_property.n2Size.x - 1;
			for (int i = 0; i < nHeight; ++i)
			{
				for (int j = 0; j < nWidth; ++j)
				{
					// Get the indexes to the four points of the quad.
					uint32_t nIdx1 = (m_property.n2Size.x * i) + j;				// Upper left.
					uint32_t nIdx2 = (m_property.n2Size.x * i) + (j + 1);			// Upper right.
					uint32_t nIdx3 = (m_property.n2Size.x * (i + 1)) + j;			// Bottom left.
					uint32_t nIdx4 = (m_property.n2Size.x * (i + 1)) + (j + 1);	// Bottom right.

					// Now create two triangles for that quad.
					// Triangle 1 - Upper left.
					vecVertices.push_back(m_vecHeightMap[nIdx1].pos);

					// Triangle 1 - Upper right.
					vecVertices.push_back(m_vecHeightMap[nIdx2].pos);

					// Triangle 1 - Bottom left.
					vecVertices.push_back(m_vecHeightMap[nIdx3].pos);

					// Triangle 2 - Bottom left.
					vecVertices.push_back(m_vecHeightMap[nIdx3].pos);

					// Triangle 2 - Upper right.
					vecVertices.push_back(m_vecHeightMap[nIdx2].pos);

					// Triangle 2 - Bottom right.
					vecVertices.push_back(m_vecHeightMap[nIdx4].pos);
				}
			}

			std::vector<Math::Vector4> vecHeightLinear(m_property.n2Size.x * m_property.n2Size.y);

			for (int i = 0; i < m_property.n2Size.x; ++i)
			{
				for (int j = 0; j < m_property.n2Size.y; ++j)
				{
					const HeightMapVertex& vertex = m_vecHeightMap[i + j * m_property.n2Size.y];
					vecHeightLinear[i + j * m_property.n2Size.y] = Math::Vector4(vertex.normal.x, vertex.normal.y, vertex.normal.z, vertex.pos.y);
				}
			}

			D3D11_SUBRESOURCE_DATA subresource_data;
			subresource_data.pSysMem = vecHeightLinear.data();
			subresource_data.SysMemPitch = m_property.n2Size.x * sizeof(Math::Vector4);
			subresource_data.SysMemSlicePitch = 0;

			Graphics::TextureDesc2D tex_desc;
			tex_desc.Width = m_property.n2Size.x;
			tex_desc.Height = m_property.n2Size.y;
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

			String::StringID strName;
			strName.Format("TerrainHeightMap_%d", s_nTerrainIndex);

			m_pTexHeightMap = Graphics::ITexture::Create(strName, tex_desc, &subresource_data);

			std::vector<Graphics::VertexPos4> vecPatches_rawdata;
			vecPatches_rawdata.resize(m_property.n2Patches.x * m_property.n2Patches.y);

			// creating terrain vertex buffer
			for (int i = 0; i < m_property.n2Patches.x; ++i)
			{
				for (int j = 0; j < m_property.n2Patches.y; ++j)
				{
					Graphics::VertexPos4& vertex = vecPatches_rawdata[i + j * m_property.n2Patches.y];
					vertex.pos.x = i * m_property.n2Size.x / m_property.n2Patches.x;
					vertex.pos.y = j * m_property.n2Size.y / m_property.n2Patches.y;
				}
			}

			m_pHeightField = Graphics::IVertexBuffer::Create(Graphics::VertexPos4::Format(), vecPatches_rawdata.size(), &vecPatches_rawdata.front(), D3D11_USAGE_IMMUTABLE);

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
	}
}