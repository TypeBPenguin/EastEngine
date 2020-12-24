#include "stdafx.h"
#include "Terrain.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"
#include "CommonLib/ThreadPool.h"

namespace sid
{
	RegisterStringID(Terrain_Physics);
}

namespace est
{
	namespace gameobject
	{
		static uint32_t s_nTerrainIndex = 0;

		Terrain::Terrain(const Handle& handle)
			: ITerrain(handle)
		{
		}

		Terrain::~Terrain()
		{
			graphics::ReleaseResource(m_pTexHeightMap);
			graphics::ReleaseResource(m_pTexColorMap);
			graphics::ReleaseResource(m_pTexDetailMap);
			graphics::ReleaseResource(m_pTexDetailNormalMap);

			graphics::ReleaseResource(m_pHeightField);
			physics::scene::RemoveActor(std::move(m_pPhysics));
		}

		bool Terrain::Initialize(const TerrainProperty& terrainProperty, bool isEnableThreadLoad)
		{
			m_property = terrainProperty;

			if (isEnableThreadLoad == true)
			{
				thread::CreateTask([&]()
				{
					init();
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
			if (loadRawHeightmap(m_property.texHeightMap.c_str()) == false)
				return false;

			if (loadColorMap(m_property.texColorMap.c_str()) == false)
				return false;

			if (initTerrain() == false)
				return false;

			m_pTexDetailMap = graphics::CreateTextureAsync(m_property.texDetailMap.c_str());
			m_pTexDetailNormalMap = graphics::CreateTextureAsync(m_property.texDetailNormalMap.c_str());

			m_matWorld = math::Matrix::Compose(m_property.transform.scale, m_property.transform.rotation, m_property.transform.position);

			if (m_rigidBodyData.vertices.empty() == false && m_rigidBodyData.indices.empty() == false)
			{
				m_property.rigidActorProperty.rigidAcotr.name = sid::Terrain_Physics;
				m_property.rigidActorProperty.rigidAcotr.type = physics::IActor::eRigidStatic;
				m_property.rigidActorProperty.rigidAcotr.globalTransform.position = m_property.transform.position;
				m_property.rigidActorProperty.rigidAcotr.globalTransform.rotation = m_property.transform.rotation;

				m_property.rigidActorProperty.shape.flags = physics::IShape::eSimulationShape;
				m_property.rigidActorProperty.shape.SetTriangleMeshGeometry(math::float3::One, math::Quaternion::Identity,
					m_rigidBodyData.vertices.data(), static_cast<uint32_t>(m_rigidBodyData.vertices.size()), m_rigidBodyData.indices.data(), static_cast<uint32_t>(m_rigidBodyData.indices.size()), physics::IGeometry::MeshFlag::eNone);

				m_pPhysics = physics::CreateRigidActor(m_property.rigidActorProperty);
				m_pPhysics->SetUserData(this);

				physics::scene::AddActor(m_pPhysics.get());
			}

			m_isBuildComplete = true;

			return true;
		}

		void Terrain::Update(float elapsedTime)
		{
			if (m_isBuildComplete == false)
				return;

			if (IsVisible() == true)
			{
				if (m_pTexHeightMap->GetState() == graphics::IResource::eComplete &&
					m_pTexColorMap->GetState() == graphics::IResource::eComplete &&
					m_pTexDetailMap->GetState() == graphics::IResource::eComplete &&
					m_pTexDetailNormalMap->GetState() == graphics::IResource::eComplete)
				{
					graphics::RenderJobTerrain job;
					job.pVertexBuffer = m_pHeightField;

					job.f2PatchSize.x = static_cast<float>(m_property.size.x) / static_cast<float>(m_property.patches.x);
					job.f2PatchSize.y = static_cast<float>(m_property.size.y) / static_cast<float>(m_property.patches.y);

					job.f2HeightFieldSize.x = static_cast<float>(m_property.size.x);
					job.f2HeightFieldSize.y = static_cast<float>(m_property.size.y);

					job.pTexHeightField = m_pTexHeightMap;
					job.pTexColorMap = m_pTexColorMap;

					job.pTexDetailMap = m_pTexDetailMap;
					job.pTexDetailNormalMap = m_pTexDetailNormalMap;

					job.worldMatrix = m_matWorld;
					job.prevWorldMatrix = m_matPrevWorld;

					// 컬링 방식, 그리려고하는 패치의 중점을 계산한뒤, World * View * Proj 를 곱해서 프로젝션 영역안에 있는지 판별하는 방식
					// 근데, 패치의 중점만 있는지 없는지 판단하기 때문에 패치의 꼭지점이 프로젝션 영역 안에 있어도 컬링되고맘
					// 모든 꼭지점을 컬링할까 했지만 성능 문제로 패스
					// 얼마나 느려지는지 확인해보려했으나 전부 안나옴 흐흐
					// 나중에 심심할때 다시 테스트 해보자
					// 컬링 하냐마냐의 차이는 1025 x 1025 사이즈 터레인에서 0.03 ms 정도
					job.isEnableFrustumCullInHS = false;

					graphics::PushRenderJob(job);
				}
			}

			m_matPrevWorld = m_matWorld;
		}

		float Terrain::GetHeight(float posX, float posZ) const
		{
			if (m_isBuildComplete == false)
				return 0.f;

			if (m_pPhysics != nullptr)
			{
				const math::float3 origin(posX, m_property.transform.position.y + m_heightMax + 100.f, posZ);
				const math::float3 direction(0.f, -1.f, 0.f);
				const float distance = m_heightMax - m_heightMin + 100.f;

				const physics::Transform& pose = m_pPhysics->GetGlobalTransform();

				physics::HitLocation hitLocation;
				if (physics::scene::Raycast(origin, direction, distance, m_pPhysics->GetShape(0).get(), pose, &hitLocation) == true)
					return hitLocation.position.y;

				return 0.f;
			}
			else
			{
				const math::Matrix matInvWorld = m_matWorld.Invert();

				math::float3 f3Pos(posX, 0.f, posZ);
				f3Pos = math::float3::Transform(f3Pos, matInvWorld);

				int i = static_cast<int>(f3Pos.x);
				int j = static_cast<int>(f3Pos.z);

				if (i < 0 || i >= m_property.size.x ||
					j < 0 || j >= m_property.size.y)
					return 0.f;

				// Get the indexes to the four points of the quad.
				const uint32_t index1 = (m_property.size.x * i) + j;				// Upper left.
				const uint32_t index2 = (m_property.size.x * i) + (j + 1);			// Upper right.
				const uint32_t index3 = (m_property.size.x * (i + 1)) + j;			// Bottom left.
				const uint32_t index4 = (m_property.size.x * (i + 1)) + (j + 1);	// Bottom right.

				// Triangle 1 - Upper left.
				const math::float3& p0 = m_vecHeightMap[index1].pos;

				// Triangle 1 - Upper right.
				const math::float3& p1 = m_vecHeightMap[index2].pos;

				// Triangle 1 - Bottom left.
				const math::float3& p2 = m_vecHeightMap[index3].pos;

				// Triangle 2 - Bottom right.
				const math::float3& p3 = m_vecHeightMap[index4].pos;

				std::optional<float> optHeight = CheckHeightOfTriangle(posX, posZ, p0, p1, p2);
				if (optHeight.has_value())
					return optHeight.value();

				optHeight = CheckHeightOfTriangle(posX, posZ, p2, p1, p3);
				if (optHeight.has_value())
					return optHeight.value();

				return 0.f;
			}
		}

		bool Terrain::loadHeightMap(const wchar_t* strFilePath)
		{
			file::Stream file;
			if (file.Open(strFilePath, file::eReadBinary) == false)
				return false;

			BITMAPFILEHEADER bitmapFileHeader;
			file.Read(reinterpret_cast<char*>(&bitmapFileHeader), sizeof(BITMAPFILEHEADER));

			BITMAPINFOHEADER bitmapInfoHeader;
			file.Read(reinterpret_cast<char*>(&bitmapInfoHeader), sizeof(BITMAPINFOHEADER));

			// Calculate the size of the bitmap image data.  
			// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
			uint32_t imageSize = m_property.size.y * ((m_property.size.x * 3) + 1);

			// Allocate memory for the bitmap image data.
			std::vector<unsigned char> vecBitmapImage;
			vecBitmapImage.resize(imageSize);

			file.Read(reinterpret_cast<char*>(vecBitmapImage.data()), imageSize);

			file.Close();

			m_vecHeightMap.clear();
			m_vecHeightMap.resize(m_property.size.x * m_property.size.y);

			int k = 0;
			int nIdx = 0;
			float fZ = static_cast<float>(m_property.size.y - 1);
			for (int i = 0; i < m_property.size.y; ++i)
			{
				for (int j = 0; j < m_property.size.x; ++j)
				{
					// Set the X and Z coordinates.
					m_vecHeightMap[nIdx].pos.x = static_cast<float>(j);
					m_vecHeightMap[nIdx].pos.z = (-static_cast<float>(i) + fZ);

					int nIdx_y = (m_property.size.x * (m_property.size.y - 1 - i)) + j;
					m_vecHeightMap[nIdx_y].pos.y = static_cast<float>(vecBitmapImage[k]) / m_property.heightScale;

					++nIdx;

					k += 3;
				}

				++k;
			}

			vecBitmapImage.clear();

			return true;
		}

		bool Terrain::loadColorMap(const wchar_t* strFilePath)
		{
			m_pTexColorMap = graphics::CreateTexture(strFilePath);

			return true;
		}

		bool Terrain::loadRawHeightmap(const wchar_t* strFilePath)
		{
			// Start by creating the array structure to hold the height map data.
			// Open the bitmap map file in binary.
			file::Stream file;
			if (file.Open(strFilePath, file::eReadBinary) == false)
				return false;

			// Calculate the size of the bitmap image data.  
			// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
			uint32_t imageSize = m_property.size.y * m_property.size.x;

			m_vecHeightMap.clear();
			m_vecHeightMap.reserve(imageSize);

			std::vector<uint16_t> vecRawData;
			vecRawData.resize(imageSize);

			file.Read(reinterpret_cast<char*>(vecRawData.data()), imageSize * 2);

			float offsetZ = static_cast<float>(m_property.size.y - 1);

			uint32_t nIdx = 0;
			for (int i = 0; i < m_property.size.y; ++i)
			{
				for (int j = 0; j < m_property.size.x; ++j)
				{
					HeightMapVertex vertex;

					// Set the X and Z coordinates.
					vertex.pos.x = static_cast<float>(j);
					vertex.pos.z = (-static_cast<float>(i) + offsetZ);

					// Scale the height.
					vertex.pos.y = static_cast<float>(vecRawData[nIdx]) / m_property.heightScale;

					m_vecHeightMap.emplace_back(std::move(vertex));

					++nIdx;
				}
			}

			vecRawData.clear();

			return true;
		}

		bool Terrain::initTerrain()
		{
			uint32_t nVertexCount = (m_property.size.y - 1) * (m_property.size.x - 1) * 4;
			uint32_t indexCount = (m_property.size.y - 1) * (m_property.size.x - 1) * 6;

			std::vector<graphics::VertexPosTexNorCol> vecVertex;
			vecVertex.reserve(nVertexCount);

			m_rigidBodyData.vertices.reserve(nVertexCount);
			m_rigidBodyData.indices.reserve(indexCount);

			std::vector<math::float3> vecNormal;
			vecNormal.reserve((m_property.size.y - 1) * (m_property.size.x - 1));

			int index = 0;

			int nHeight = m_property.size.y - 1;
			int nWidth = m_property.size.x - 1;
			for (int j = 0; j < nHeight; ++j)
			{
				for (int i = 0; i < nWidth; ++i)
				{
					{
						const int nIdx1 = ((j + 1) * m_property.size.x) + i;		// Bottom left vertex.
						const int nIdx2 = ((j + 1) * m_property.size.x) + (i + 1);	// Bottom right vertex.
						const int nIdx3 = (j * m_property.size.x) + i;				// Upper left vertex.

						// Get three vertices from the face.
						math::float3 v1 = m_vecHeightMap[nIdx1].pos;
						math::float3 v2 = m_vecHeightMap[nIdx2].pos;
						math::float3 v3 = m_vecHeightMap[nIdx3].pos;

						// Calculate the two vectors for this face.
						v1.x = v1.x - v3.x;
						v1.y = v1.y - v3.y;
						v1.z = v1.z - v3.z;
						v2.x = v3.x - v2.x;
						v2.y = v3.y - v2.y;
						v2.z = v3.z - v2.z;

						// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
						math::float3 vNormal(
							(v1.y * v2.z) - (v1.z * v2.y),
							(v1.z * v2.x) - (v1.x * v2.z),
							(v1.x * v2.y) - (v1.y * v2.x)
						);

						vNormal.Normalize();

						vecNormal.emplace_back(vNormal);
					}

					{
						// Get the indexes to the four points of the quad.
						const uint32_t nIdx1 = (m_property.size.x * i) + j;				// Upper left.
						const uint32_t nIdx2 = (m_property.size.x * i) + (j + 1);		// Upper right.
						const uint32_t nIdx3 = (m_property.size.x * (i + 1)) + j;		// Bottom left.
						const uint32_t nIdx4 = (m_property.size.x * (i + 1)) + (j + 1);	// Bottom right.

						// Now create two triangles for that quad.
						// Triangle 1 - Upper left.
						m_rigidBodyData.vertices.push_back(m_vecHeightMap[nIdx1].pos);

						// Triangle 1 - Upper right.
						m_rigidBodyData.vertices.push_back(m_vecHeightMap[nIdx2].pos);

						// Triangle 1 - Bottom left.
						m_rigidBodyData.vertices.push_back(m_vecHeightMap[nIdx3].pos);

						// Triangle 2 - Bottom right.
						m_rigidBodyData.vertices.push_back(m_vecHeightMap[nIdx4].pos);

						m_rigidBodyData.indices.emplace_back(index + 0);
						m_rigidBodyData.indices.emplace_back(index + 1);
						m_rigidBodyData.indices.emplace_back(index + 2);
						m_rigidBodyData.indices.emplace_back(index + 2);
						m_rigidBodyData.indices.emplace_back(index + 1);
						m_rigidBodyData.indices.emplace_back(index + 3);

						index += 4;
					}
				}
			}

			// Now go through all the vertices and take a sum of the face normals that touch this vertex.
			nHeight = m_property.size.y;
			nWidth = m_property.size.x;
			for (int j = 0; j < nHeight; ++j)
			{
				for (int i = 0; i < nWidth; ++i)
				{
					// Initialize the sum.
					math::float3 sum;
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

			std::vector<math::float4> vecHeightLinear(m_property.size.x * m_property.size.y);

			m_heightMax = std::numeric_limits<float>::min();
			m_heightMin = std::numeric_limits<float>::max();

			for (int i = 0; i < m_property.size.x; ++i)
			{
				for (int j = 0; j < m_property.size.y; ++j)
				{
					const HeightMapVertex& vertex = m_vecHeightMap[i + j * m_property.size.y];
					vecHeightLinear[i + j * m_property.size.y] = math::float4(vertex.normal.x, vertex.normal.y, vertex.normal.z, vertex.pos.y);

					m_heightMax = std::max(m_heightMax, vertex.pos.y);
					m_heightMin = std::min(m_heightMin, vertex.pos.y);
				}
			}

			graphics::TextureDesc desc;
			desc.name = string::Format(L"TerrainHeightMap_%d", s_nTerrainIndex);
			desc.Width = m_property.size.x;
			desc.Height = m_property.size.y;

			desc.resourceFormat = graphics::eRF_R32G32B32A32_FLOAT;
			desc.isDynamic = false;

			desc.subResourceData.pSysMem = vecHeightLinear.data();
			desc.subResourceData.SysMemPitch = m_property.size.x * sizeof(math::float4);
			desc.subResourceData.SysMemSlicePitch = 0;

			desc.subResourceData.MemSize = vecHeightLinear.size() * sizeof(math::float4);

			m_pTexHeightMap = graphics::CreateTexture(desc);

			/*D3D11_SUBRESOURCE_DATA subresource_data;
			subresource_data.pSysMem = vecHeightLinear.data();
			subresource_data.SysMemPitch = m_property.size.x * sizeof(math::float4);
			subresource_data.SysMemSlicePitch = 0;

			graphics::TextureDesc2D tex_desc;
			tex_desc.Width = m_property.size.x;
			tex_desc.Height = m_property.size.y;
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

			string::StringID strName;
			strName.Format("TerrainHeightMap_%d", s_nTerrainIndex);

			m_pTexHeightMap = graphics::ITexture::Create(strName, tex_desc, &subresource_data);*/

			std::vector<graphics::VertexPos4> vecPatches_rawdata;
			vecPatches_rawdata.resize(m_property.patches.x * m_property.patches.y);

			// creating terrain vertex buffer
			for (int i = 0; i < m_property.patches.x; ++i)
			{
				for (int j = 0; j < m_property.patches.y; ++j)
				{
					graphics::VertexPos4& vertex = vecPatches_rawdata[i + j * m_property.patches.y];
					vertex.pos.x = i * static_cast<float>(m_property.size.x) / static_cast<float>(m_property.patches.x);
					vertex.pos.y = j * static_cast<float>(m_property.size.y) / static_cast<float>(m_property.patches.y);
				}
			}

			m_pHeightField = graphics::CreateVertexBuffer(reinterpret_cast<const uint8_t*>(vecPatches_rawdata.data()), static_cast<uint32_t>(vecPatches_rawdata.size()), sizeof(vecPatches_rawdata[0]), false);

			return true;
		}

		std::optional<float> Terrain::CheckHeightOfTriangle(float x, float z, const math::float3& v0, const math::float3& v1, const math::float3& v2) const
		{
			// Starting position of the ray that is being cast.
			math::float3 f3StartVector(x, 0.f, z);

			// The direction the ray is being cast.
			const math::float3 f3DrectionVector(0.f, -1.f, 0.f);

			// Calculate the two edges from the three points given.
			const math::float3 vEdge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);

			const math::float3 vEdge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

			// Calculate the normal of the triangle from the two edges.
			math::float3 vNormal((vEdge1.y * vEdge2.z) - (vEdge1.z * vEdge2.y),
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
			math::float3 Q(f3StartVector.x + (f3DrectionVector.x * t),
				f3StartVector.y + (f3DrectionVector.y * t),
				f3StartVector.z + (f3DrectionVector.z * t));

			// Find the three edges of the triangle.
			math::float3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
			math::float3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
			math::float3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

			// Calculate the normal for the first edge.
			math::float3 edgeNormal((e1.y * vNormal.z) - (e1.z * vNormal.y),
				(e1.z * vNormal.x) - (e1.x * vNormal.z),
				(e1.x * vNormal.y) - (e1.y * vNormal.x));

			// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
			math::float3 temp(Q.x - v0.x, Q.y - v0.y, Q.z - v0.z);

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