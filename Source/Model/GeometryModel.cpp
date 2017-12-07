#include "stdafx.h"
#include "GeometryModel.h"

#include <DirectXMath.h>

using namespace DirectX;

namespace EastEngine
{
	namespace Graphics
	{
		namespace GeometryModel
		{
			const float SQRT2 = 1.41421356237309504880f;
			const float SQRT3 = 1.73205080756887729352f;
			const float SQRT6 = 2.44948974278317809820f;

			void CalculateTangentBinormal(const VertexPosTex& vertex1, const VertexPosTex& vertex2, const VertexPosTex& vertex3,
				Math::Vector3& tangent, Math::Vector3& binormal)
			{
				Math::Vector3 vector1, vector2;
				Math::Vector2 tuvVector1, tuvVector2;

				// Calculate the two vectors for this face
				vector1.x = vertex2.pos.x - vertex1.pos.x;
				vector1.y = vertex2.pos.y - vertex1.pos.y;
				vector1.z = vertex2.pos.z - vertex1.pos.z;

				vector2.x = vertex3.pos.x - vertex1.pos.x;
				vector2.y = vertex3.pos.y - vertex1.pos.y;
				vector2.z = vertex3.pos.z - vertex1.pos.z;

				// Calculate the tu and tv texture space vectors.
				tuvVector1.x = vertex2.uv.x - vertex1.uv.x;
				tuvVector1.y = vertex2.uv.y - vertex1.uv.y;

				tuvVector2.x = vertex3.uv.x - vertex1.uv.x;
				tuvVector2.y = vertex3.uv.y - vertex1.uv.y;

				// Calculate the denominator of the tangent/binormal equation.
				float den = 1.f / (tuvVector1.x * tuvVector2.y - tuvVector2.x * tuvVector1.y);

				// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
				tangent.x = (tuvVector2.y * vector1.x - tuvVector1.y * vector2.x) * den;
				tangent.y = (tuvVector2.y * vector1.y - tuvVector1.y * vector2.y) * den;
				tangent.z = (tuvVector2.y * vector1.z - tuvVector1.y * vector2.z) * den;

				binormal.x = (tuvVector1.x * vector2.x - tuvVector2.x * vector1.x) * den;
				binormal.y = (tuvVector1.x * vector2.y - tuvVector2.x * vector1.y) * den;
				binormal.z = (tuvVector1.x * vector2.z - tuvVector2.x * vector1.z) * den;

				// Normalize the normal and then store it
				tangent.Normalize();

				// Calculate the length of this normal.
				binormal.Normalize();
			}

			void CalculateTangentBinormal(const Math::Vector3& normal, Math::Vector3& tangent, Math::Vector3& binormal)
			{
				Math::Vector3 c1 = normal.Cross(Math::Vector3(0.f, 0.f, 1.f));
				Math::Vector3 c2 = normal.Cross(Math::Vector3(0.f, 1.f, 0.f));

				if (c1.LengthSquared() > c2.LengthSquared())
				{
					tangent = c1;
				}
				else
				{
					tangent = c2;
				}

				tangent.Normalize();

				binormal = normal.Cross(tangent);
				binormal.Normalize();
			}

			void CalculateNormal(const Math::Vector3& vPos0, const Math::Vector3& vPos1, const Math::Vector3& vPos2, Math::Vector3& vNormal)
			{
				// Calculate the two vectors for this face.
				Math::Vector3 v0(vPos0);
				Math::Vector3 v1(vPos1.x - vPos2.x, vPos1.y - vPos2.y, vPos1.z - vPos2.z);
				Math::Vector3 v2(vPos2.x - vPos1.x, vPos2.y - vPos1.y, vPos2.z - vPos1.z);

				// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
				vNormal = Math::Vector3((v0.y * v1.z) - (v0.z * v1.y),
					(v0.z * v1.x) - (v0.x * v1.z),
					(v0.x * v1.y) - (v0.y * v1.x));

				// Calculate the length.
				vNormal.Normalize();
			}

			bool InitBuffer(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, 
				std::vector<VertexPosTexNor>& vecVertices, std::vector<uint32_t>& vecIndices,
				bool rhcoords, bool invertn)
			{
				if (rhcoords == false)
				{
					for (auto it = vecIndices.begin(); it != vecIndices.end(); it += 3)
					{
						std::swap(*it, *(it + 2));
					}

					for (auto it = vecVertices.begin(); it != vecVertices.end(); ++it)
					{
						it->uv.x = (1.f - it->uv.x);
					}
				}

				if (invertn)
				{
					uint32_t nSize = vecVertices.size();
					for (uint32_t i = 0; i < nSize; ++i)
					{
						vecVertices[i].normal.x = -vecVertices[i].normal.x;
						vecVertices[i].normal.y = -vecVertices[i].normal.y;
						vecVertices[i].normal.z = -vecVertices[i].normal.z;
					}
				}

				SafeDelete(*ppVertexBuffer);
				*ppVertexBuffer = IVertexBuffer::Create(VertexPosTexNor::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_DYNAMIC, IVertexBuffer::eSaveVertexPos/* | IVertexBuffer::eSaveVertexClipSpace*/);
		
				SafeDelete(*ppIndexBuffer);
				*ppIndexBuffer =  IIndexBuffer::Create(vecIndices.size(), &vecIndices.front(), D3D11_USAGE_DYNAMIC, IIndexBuffer::eSaveRawValue);
		
				vecVertices.clear();
				vecIndices.clear();

				if (*ppVertexBuffer == nullptr || *ppIndexBuffer == nullptr)
				{
					SafeDelete(*ppVertexBuffer);
					SafeDelete(*ppIndexBuffer);
					return false;
				}

				return true;
			}

			bool CreateCube(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size, bool rhcoords)
			{
				return CreateBox(ppVertexBuffer, ppIndexBuffer, Math::Vector3(size, size, size), rhcoords);
			}

			bool CreateBox(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, const Math::Vector3& size, bool rhcoords, bool invertn)
			{
				if (*ppVertexBuffer != nullptr)
					return false;

				if (*ppIndexBuffer != nullptr)
					return false;

				// A box has six faces, each one pointing in a different direction.
				const int FaceCount = 6;

				static const Math::Vector3 faceNormals[FaceCount] =
				{
					{ 0, 0, 1 },
					{ 0, 0, -1 },
					{ 1, 0, 0 },
					{ -1, 0, 0 },
					{ 0, 1, 0 },
					{ 0, -1, 0 },
				};

				static const Math::Vector2 textureCoordinates[4] =
				{
					{ 1, 0 },
					{ 1, 1 },
					{ 0, 1 },
					{ 0, 0 },
				};

				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				const Math::Vector3 tsize(0.5f, 0.5f, 0.5f);

				// Create each face in turn.
				for (int i = 0; i < FaceCount; ++i)
				{
					Math::Vector3 normal = faceNormals[i];

					// Get two vectors perpendicular both to the face normal and to each other.
					Math::Vector3 basis = (i >= 4) ? Math::Vector3(0.f, 0.f, 1.f) : Math::Vector3(0.f, 1.f, 0.f);

					Math::Vector3 side1 = normal.Cross(basis);
					Math::Vector3 side2 = normal.Cross(side1);

					// Six indices (two triangles) per face.
					uint32_t vbase = vecVertices.size();
					vecIndices.push_back(vbase + 0);
					vecIndices.push_back(vbase + 1);
					vecIndices.push_back(vbase + 2);

					vecIndices.push_back(vbase + 0);
					vecIndices.push_back(vbase + 2);
					vecIndices.push_back(vbase + 3);

					// Four vertices per face.
					vecVertices.push_back(VertexPosTexNor((normal - side1 - side2) * tsize, textureCoordinates[0], normal));
					vecVertices.push_back(VertexPosTexNor((normal - side1 + side2) * tsize, textureCoordinates[1], normal));
					vecVertices.push_back(VertexPosTexNor((normal + side1 + side2) * tsize, textureCoordinates[2], normal));
					vecVertices.push_back(VertexPosTexNor((normal + side1 - side2) * tsize, textureCoordinates[3], normal));
				}

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, invertn);
			}

			bool CreateSphere(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float diameter, uint32_t tessellation, bool rhcoords, bool invertn)
			{
				if (*ppVertexBuffer != nullptr)
					return false;

				if (*ppIndexBuffer != nullptr)
					return false;

				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				if (tessellation < 3)
					return false;

				uint32_t verticalSegments = tessellation;
				uint32_t horizontalSegments = tessellation * 2;

				float radius = diameter * 0.5f;

				// Create rings of vertices at progressively higher latitudes.
				for (uint32_t i = 0; i <= verticalSegments; ++i)
				{
					float v = 1 - (float)i / verticalSegments;

					float latitude = (i * Math::PI / verticalSegments) - Math::PIDIV2;
					float dy, dxz;

					Math::SinCos(&dy, &dxz, latitude);

					// Create a single ring of vertices at this latitude.
					for (uint32_t j = 0; j <= horizontalSegments; ++j)
					{
						float u = (float)j / horizontalSegments;

						float longitude = j * Math::PI2 / horizontalSegments;
						float dx, dz;

						Math::SinCos(&dx, &dz, longitude);

						dx *= dxz;
						dz *= dxz;

						Math::Vector3 normal(dx, dy, dz);
						Math::Vector2 textureCoordinate(u, v);

						vecVertices.push_back(VertexPosTexNor(normal * radius, textureCoordinate, normal));
					}
				}

				// Fill the index buffer with triangles joining each pair of latitude rings.
				uint32_t stride = horizontalSegments + 1;

				for (uint32_t i = 0; i < verticalSegments; ++i)
				{
					for (uint32_t j = 0; j <= horizontalSegments; ++j)
					{
						uint32_t nextI = i + 1;
						uint32_t nextJ = (j + 1) % stride;

						vecIndices.push_back(i * stride + j);
						vecIndices.push_back(nextI * stride + j);
						vecIndices.push_back(i * stride + nextJ);

						vecIndices.push_back(i * stride + nextJ);
						vecIndices.push_back(nextI * stride + j);
						vecIndices.push_back(nextI * stride + nextJ);
					}
				}

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, invertn);
			}

			bool CreateGeoSphere(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float diameter, uint32_t tessellation, bool rhcoords)
			{
				// An undirected edge between two vertices, represented by a pair of indexes into a vertex array.
				// Becuse this edge is undirected, (a,b) is the same as (b,a).
				typedef std::pair<uint32_t, uint32_t> UndirectedEdge;

				// Makes an undirected edge. Rather than overloading comparison operators to give us the (a,b)==(b,a) property,
				// we'll just ensure that the larger of the two goes first. This'll simplify things greatly.
				auto makeUndirectedEdge = [](uint32_t a, uint32_t b)
				{
					return std::make_pair(Math::Max(a, b), Math::Min(a, b));
				};

				// Key: an edge
				// Value: the index of the vertex which lies midway between the two vertices pointed to by the key value
				// This map is used to avoid duplicating vertices when subdividing triangles along edges.
				typedef std::map<UndirectedEdge, uint32_t> EdgeSubdivisionMap;


				const Math::Vector3 OctahedronVertices[] =
				{
					// when looking down the negative z-axis (into the screen)
					Math::Vector3(0, 1, 0), // 0 top
					Math::Vector3(0, 0, -1), // 1 front
					Math::Vector3(1, 0, 0), // 2 right
					Math::Vector3(0, 0, 1), // 3 back
					Math::Vector3(-1, 0, 0), // 4 left
					Math::Vector3(0, -1, 0), // 5 bottom
				};

				const uint32_t OctahedronIndices[] =
				{
					0, 1, 2, // top front-right face
					0, 2, 3, // top back-right face
					0, 3, 4, // top back-left face
					0, 4, 1, // top front-left face
					5, 1, 4, // bottom front-left face
					5, 4, 3, // bottom back-left face
					5, 3, 2, // bottom back-right face
					5, 2, 1, // bottom front-right face
				};

				const float radius = diameter * 0.5f;

				// Start with an octahedron; copy the data into the vertex/index collection.

				std::vector<Math::Vector3> vertexPositions(std::begin(OctahedronVertices), std::end(OctahedronVertices));

				std::vector<uint32_t> vecIndices;
				vecIndices.insert(vecIndices.begin(), std::begin(OctahedronIndices), std::end(OctahedronIndices));

				// We know these values by looking at the above index list for the octahedron. Despite the subdivisions that are
				// about to go on, these values aren't ever going to change because the vertices don't move around in the array.
				// We'll need these values later on to fix the singularities that show up at the poles.
				const uint32_t northPoleIndex = 0;
				const uint32_t southPoleIndex = 5;

				for (uint32_t iSubdivision = 0; iSubdivision < tessellation; ++iSubdivision)
				{
					assert(vecIndices.size() % 3 == 0); // sanity

					// We use this to keep track of which edges have already been subdivided.
					EdgeSubdivisionMap subdividedEdges;

					// The new index collection after subdivision.
					std::vector<uint32_t> newIndices;

					const uint32_t triangleCount = vecIndices.size()  / 3;
					for (uint32_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle)
					{
						// For each edge on this triangle, create a new vertex in the middle of that edge.
						// The winding order of the triangles we output are the same as the winding order of the inputs.

						// Indices of the vertices making up this triangle
						uint32_t iv0 = vecIndices[iTriangle * 3 + 0];
						uint32_t iv1 = vecIndices[iTriangle * 3 + 1];
						uint32_t iv2 = vecIndices[iTriangle * 3 + 2];

						// Get the new vertices
						Math::Vector3 v01; // vertex on the midpoint of v0 and v1
						Math::Vector3 v12; // ditto v1 and v2
						Math::Vector3 v20; // ditto v2 and v0
						uint32_t iv01; // index of v01
						uint32_t iv12; // index of v12
						uint32_t iv20; // index of v20

						// Function that, when given the index of two vertices, creates a new vertex at the midpoint of those vertices.
						auto divideEdge = [&](uint32_t i0, uint32_t i1, Math::Vector3& outVertex, uint32_t& outIndex)
						{
							const UndirectedEdge edge = makeUndirectedEdge(i0, i1);

							// Check to see if we've already generated this vertex
							auto it = subdividedEdges.find(edge);
							if (it != subdividedEdges.end())
							{
								// We've already generated this vertex before
								outIndex = it->second; // the index of this vertex
								outVertex = vertexPositions[outIndex]; // and the vertex itself
							}
							else
							{
								// Haven't generated this vertex before: so add it now

								outVertex = (vertexPositions[i0] + vertexPositions[i1]) * 0.5;

								outIndex = static_cast<uint32_t>(vertexPositions.size());
								vertexPositions.push_back(outVertex);

								// Now add it to the map.
								subdividedEdges.insert(std::make_pair(edge, outIndex));
							}
						};

						// Add/get new vertices and their indices
						divideEdge(iv0, iv1, v01, iv01);
						divideEdge(iv1, iv2, v12, iv12);
						divideEdge(iv0, iv2, v20, iv20);

						// Add the new indices. We have four new triangles from our original one:
						//        v0
						//        o
						//       /a\
												//  v20 o---o v01
						//     /b\c/d\
												// v2 o---o---o v1
						//       v12
						const uint32_t indicesToAdd[] =
						{
							iv0, iv01, iv20, // a
							iv20, iv12, iv2, // b
							iv20, iv01, iv12, // c
							iv01, iv1, iv12, // d
						};
						newIndices.insert(newIndices.end(), std::begin(indicesToAdd), std::end(indicesToAdd));
					}

					vecIndices = std::move(newIndices);
				}

				// Now that we've completed subdivision, fill in the final vertex collection
				std::vector<VertexPosTexNor> vecVertices;
				vecVertices.reserve(vertexPositions.size());
				for (auto it = vertexPositions.begin(); it != vertexPositions.end(); ++it)
				{
					Math::Vector3 vertexValue = *it;

					vertexValue.Normalize();
					Math::Vector3 normal = vertexValue;
					Math::Vector3 pos = normal * radius;

					Math::Vector3 normalFloat3 = normal;

					// calculate texture coordinates for this vertex
					float longitude = atan2(normalFloat3.x, -normalFloat3.z);
					float latitude = acos(normalFloat3.y);

					float u = longitude / Math::PI2 + 0.5f;
					float v = latitude / Math::PI;

					Math::Vector2 texcoord(1.0f - u, v);
					vecVertices.push_back(VertexPosTexNor(pos, texcoord, normal));
				}

				// There are a couple of fixes to do. One is a texture coordinate wraparound fixup. At some point, there will be
				// a set of triangles somewhere in the mesh with texture coordinates such that the wraparound across 0.0/1.0
				// occurs across that triangle. Eg. when the left hand side of the triangle has a U coordinate of 0.98 and the
				// right hand side has a U coordinate of 0.0. The intent is that such a triangle should render with a U of 0.98 to
				// 1.0, not 0.98 to 0.0. If we don't do this fixup, there will be a visible seam across one side of the sphere.
				// 
				// Luckily this is relatively easy to fix. There is a straight edge which runs down the prime meridian of the
				// completed sphere. If you imagine the vertices along that edge, they circumscribe a semicircular arc starting at
				// y=1 and ending at y=-1, and sweeping across the range of z=0 to z=1. x stays zero. It's along this edge that we
				// need to duplicate our vertices - and provide the correct texture coordinates.
				uint32_t preFixupVertexCount = vecVertices.size();
				for (uint32_t i = 0; i < preFixupVertexCount; ++i)
				{
					// This vertex is on the prime meridian if position.x and texcoord.u are both zero (allowing for small epsilon).
					bool isOnPrimeMeridian = Math::IsZero(vecVertices[i].pos.x) && Math::IsZero(vecVertices[i].uv.x);

					if (isOnPrimeMeridian)
					{
						uint32_t newIndex = vecVertices.size(); // the index of this vertex that we're about to add

						// copy this vertex, correct the texture coordinate, and add the vertex
						VertexPosTexNor v = vecVertices[i];
						v.uv.x = 1.0f;
						vecVertices.push_back(v);

						// Now find all the triangles which contain this vertex and update them if necessary
						uint32_t nIndexCount = vecIndices.size();
						for (uint32_t j = 0; j < nIndexCount; j += 3)
						{
							uint32_t* triIndex0 = &vecIndices[j + 0];
							uint32_t* triIndex1 = &vecIndices[j + 1];
							uint32_t* triIndex2 = &vecIndices[j + 2];

							if (*triIndex0 == i)
							{
								// nothing; just keep going
							}
							else if (*triIndex1 == i)
							{
								std::swap(triIndex0, triIndex1); // swap the pointers (not the values)
							}
							else if (*triIndex2 == i)
							{
								std::swap(triIndex0, triIndex2); // swap the pointers (not the values)
							}
							else
							{
								// this triangle doesn't use the vertex we're interested in
								continue;
							}

							// If we got to this point then triIndex0 is the pointer to the index to the vertex we're looking at
							assert(*triIndex0 == i);
							assert(*triIndex1 != i && *triIndex2 != i); // assume no degenerate triangles

							const VertexPosTexNor& v0 = vecVertices[*triIndex0];
							const VertexPosTexNor& v1 = vecVertices[*triIndex1];
							const VertexPosTexNor& v2 = vecVertices[*triIndex2];

							// check the other two vertices to see if we might need to fix this triangle

							if (abs(v0.uv.x - v1.uv.x) > 0.5f ||
								abs(v0.uv.x - v2.uv.x) > 0.5f)
							{
								// yep; replace the specified index to point to the new, corrected vertex
								*triIndex0 = static_cast<uint32_t>(newIndex);
							}
						}
					}
				}

				// And one last fix we need to do: the poles. A common use-case of a sphere mesh is to map a rectangular texture onto
				// it. If that happens, then the poles become singularities which map the entire top and bottom rows of the texture
				// onto a single point. In general there's no real way to do that right. But to match the behavior of non-geodesic
				// spheres, we need to duplicate the pole vertex for every triangle that uses it. This will introduce seams near the
				// poles, but reduce stretching.
				auto fixPole = [&](uint32_t poleIndex)
				{
					auto poleVertex = vecVertices[poleIndex];
					bool overwrittenPoleVertex = false; // overwriting the original pole vertex saves us one vertex

					uint32_t nIndexCount = vecIndices.size();
					for (uint32_t i = 0; i < nIndexCount; i += 3)
					{
						// These pointers point to the three indices which make up this triangle. pPoleIndex is the pointer to the
						// entry in the index array which represents the pole index, and the other two pointers point to the other
						// two indices making up this triangle.
						uint32_t* pPoleIndex;
						uint32_t* pOtherIndex0;
						uint32_t* pOtherIndex1;
						if (vecIndices[i + 0] == poleIndex)
						{
							pPoleIndex = &vecIndices[i + 0];
							pOtherIndex0 = &vecIndices[i + 1];
							pOtherIndex1 = &vecIndices[i + 2];
						}
						else if (vecIndices[i + 1] == poleIndex)
						{
							pPoleIndex = &vecIndices[i + 1];
							pOtherIndex0 = &vecIndices[i + 2];
							pOtherIndex1 = &vecIndices[i + 0];
						}
						else if (vecIndices[i + 2] == poleIndex)
						{
							pPoleIndex = &vecIndices[i + 2];
							pOtherIndex0 = &vecIndices[i + 0];
							pOtherIndex1 = &vecIndices[i + 1];
						}
						else
						{
							continue;
						}

						const auto& otherVertex0 = vecVertices[*pOtherIndex0];
						const auto& otherVertex1 = vecVertices[*pOtherIndex1];

						// Calculate the texcoords for the new pole vertex, add it to the vertices and update the index
						VertexPosTexNor newPoleVertex = poleVertex;
						newPoleVertex.uv.x = (otherVertex0.uv.x + otherVertex1.uv.x) * 0.5f;;
						newPoleVertex.uv.y = poleVertex.uv.y;

						if (!overwrittenPoleVertex)
						{
							vecVertices[poleIndex] = newPoleVertex;
							overwrittenPoleVertex = true;
						}
						else
						{
							*pPoleIndex = static_cast<uint32_t>(vecVertices.size());
							vecVertices.push_back(newPoleVertex);
						}
					}
				};

				fixPole(northPoleIndex);
				fixPole(southPoleIndex);

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			bool CreateCylinder(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float height, float diameter, uint32_t tessellation, bool rhcoords)
			{
				auto GetCircleVector = [](uint32_t i, uint32_t tessellation)
				{
					float angle = i * Math::PI2 / tessellation;
					float dx, dz;

					Math::SinCos(&dx, &dz, angle);

					return Math::Vector3(dx, 0, dz);
				};

				auto CreateCylinderCap = [&GetCircleVector](std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, uint32_t tessellation, float height, float radius, bool isTop)
				{
					// Create cap indices.
					for (uint32_t i = 0; i < tessellation - 2; ++i)
					{
						uint32_t i1 = (i + 1) % tessellation;
						uint32_t i2 = (i + 2) % tessellation;

						if (isTop)
						{
							std::swap(i1, i2);
						}

						uint32_t vbase = vertices.size();
						indices.push_back(vbase);
						indices.push_back(vbase + i1);
						indices.push_back(vbase + i2);
					}

					// Which end of the cylinder is this?
					Math::Vector3 normal(0.f, 1.f, 0.f);
					Math::Vector3 textureScale(-0.5f, -0.5f, -0.5f);

					if (!isTop)
					{
						normal = -normal;
						textureScale *= Math::Vector3(-1.0f, 1.0f, 1.0f);
					}

					// Create cap vertices.
					for (uint32_t i = 0; i < tessellation; ++i)
					{
						Math::Vector3 circleVector = GetCircleVector(i, tessellation);

						Math::Vector3 position = (circleVector * radius) + (normal * height);

						Math::Vector2 textureCoordinate = XMVectorMultiplyAdd(XMVectorSwizzle<0, 2, 3, 3>(circleVector), textureScale, g_XMOneHalf);

						vertices.push_back(VertexPosTexNor(position, textureCoordinate, normal));
					}
				};

				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				if (tessellation < 3)
					return false;

				height *= 0.5f;

				Math::Vector3 topOffset = g_XMIdentityR1 * height;

				float radius = diameter * 0.5f;
				uint32_t stride = tessellation + 1;

				// Create a ring of triangles around the outside of the cylinder.
				for (uint32_t i = 0; i <= tessellation; ++i)
				{
					Math::Vector3 normal = GetCircleVector(i, tessellation);

					Math::Vector3 sideOffset = normal * radius;

					float u = (float)i / tessellation;

					Math::Vector2 textureCoordinate = XMLoadFloat(&u);

					vecVertices.push_back(VertexPosTexNor(sideOffset + topOffset, textureCoordinate, normal));
					vecVertices.push_back(VertexPosTexNor(sideOffset - topOffset, textureCoordinate + g_XMIdentityR1, normal));

					vecIndices.push_back(i * 2);
					vecIndices.push_back((i * 2 + 2) % (stride * 2));
					vecIndices.push_back(i * 2 + 1);

					vecIndices.push_back(i * 2 + 1);
					vecIndices.push_back((i * 2 + 2) % (stride * 2));
					vecIndices.push_back((i * 2 + 3) % (stride * 2));
				}

				// Create flat triangle fan caps to seal the top and bottom.
				CreateCylinderCap(vecVertices, vecIndices, tessellation, height, radius, true);
				CreateCylinderCap(vecVertices, vecIndices, tessellation, height, radius, false);

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			bool CreateCone(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float diameter, float height, uint32_t tessellation, bool rhcoords)
			{
				auto GetCircleVector = [](uint32_t i, uint32_t tessellation)
				{
					float angle = i * Math::PI2 / tessellation;
					float dx, dz;

					XMScalarSinCos(&dx, &dz, angle);

					return Math::Vector3(dx, 0, dz);
				};

				auto GetCircleTangent = [](uint32_t i, uint32_t tessellation)
				{
					float angle = (i * Math::PI2 / tessellation) + XM_PIDIV2;
					float dx, dz;

					XMScalarSinCos(&dx, &dz, angle);

					return Math::Vector3(dx, 0, dz);
				};

				auto CreateCylinderCap = [&GetCircleVector](std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, uint32_t tessellation, float height, float radius, bool isTop)
				{
					// Create cap indices.
					for (uint32_t i = 0; i < tessellation - 2; ++i)
					{
						uint32_t i1 = (i + 1) % tessellation;
						uint32_t i2 = (i + 2) % tessellation;

						if (isTop)
						{
							std::swap(i1, i2);
						}

						uint32_t vbase = vertices.size();
						indices.push_back(vbase);
						indices.push_back(vbase + i1);
						indices.push_back(vbase + i2);
					}

					// Which end of the cylinder is this?
					Math::Vector3 normal = g_XMIdentityR1;
					Math::Vector3 textureScale = g_XMNegativeOneHalf;

					if (!isTop)
					{
						normal = -normal;
						Math::Vector3 vNegateX = g_XMNegateX;
						textureScale *= vNegateX;
					}

					// Create cap vertices.
					for (uint32_t i = 0; i < tessellation; ++i)
					{
						Math::Vector3 circleVector = GetCircleVector(i, tessellation);

						Math::Vector3 position = (circleVector * radius) + (normal * height);

						Math::Vector2 textureCoordinate = XMVectorMultiplyAdd(XMVectorSwizzle<0, 2, 3, 3>(circleVector), textureScale, g_XMOneHalf);

						vertices.push_back(VertexPosTexNor(position, textureCoordinate, normal));
					}
				};

				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				if (tessellation < 3)
					return false;

				height /= 2;

				Math::Vector3 topOffset = g_XMIdentityR1 * height;

				float radius = diameter * 0.5f;;
				uint32_t stride = tessellation + 1;

				// Create a ring of triangles around the outside of the cone.
				for (uint32_t i = 0; i <= tessellation; ++i)
				{
					Math::Vector3 circlevec = GetCircleVector(i, tessellation);

					Math::Vector3 sideOffset = circlevec * radius;

					float u = (float)i / tessellation;

					Math::Vector2 textureCoordinate(u, u);

					Math::Vector3 pt = sideOffset - topOffset;

					Math::Vector3 normal = GetCircleTangent(i, tessellation).Cross(topOffset - pt);
					normal.Normalize();

					// Duplicate the top vertex for distinct normals
					vecVertices.push_back(VertexPosTexNor(topOffset, Math::Vector2(0.f, 0.f), normal));
					vecVertices.push_back(VertexPosTexNor(pt, textureCoordinate + g_XMIdentityR1, normal));

					vecIndices.push_back(i * 2);
					vecIndices.push_back((i * 2 + 3) % (stride * 2));
					vecIndices.push_back((i * 2 + 1) % (stride * 2));
				}

				// Create flat triangle fan caps to seal the bottom.
				CreateCylinderCap(vecVertices, vecIndices, tessellation, height, radius, false);

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			bool CreateTorus(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float diameter, float thickness, uint32_t tessellation, bool rhcoords)
			{
				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				if (tessellation < 3)
					return false;

				uint32_t stride = tessellation + 1;

				// First we loop around the main ring of the torus.
				for (uint32_t i = 0; i <= tessellation; ++i)
				{
					float u = (float)i / tessellation;

					float outerAngle = i * Math::PI2 / tessellation - XM_PIDIV2;

					// Create a transform matrix that will align geometry to
					// slice perpendicularly though the current ring position.
					Math::Matrix transform = Math::Matrix::CreateTranslation(diameter * 0.5f, 0.f, 0.f) * Math::Matrix::CreateRotationY(outerAngle);

					// Now we loop along the other axis, around the side of the tube.
					for (uint32_t j = 0; j <= tessellation; ++j)
					{
						float v = 1 - (float)j / tessellation;

						float innerAngle = j * Math::PI2 / tessellation + XM_PI;
						float dx, dy;

						XMScalarSinCos(&dy, &dx, innerAngle);

						// Create a vertex.
						Math::Vector3 normal(dx, dy, 0);
						Math::Vector3 position = normal * thickness * 0.5f;;
						Math::Vector2 textureCoordinate(u, v);

						position = Math::Vector3::Transform(position, transform);
						normal = Math::Vector3::Transform(normal, transform);

						vecVertices.push_back(VertexPosTexNor(position, textureCoordinate, normal));

						// And create indices for two triangles.
						uint32_t nextI = (i + 1) % stride;
						uint32_t nextJ = (j + 1) % stride;

						vecIndices.push_back(i * stride + j);
						vecIndices.push_back(i * stride + nextJ);
						vecIndices.push_back(nextI * stride + j);

						vecIndices.push_back(i * stride + nextJ);
						vecIndices.push_back(nextI * stride + nextJ);
						vecIndices.push_back(nextI * stride + j);
					}
				}

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			bool CreateTetrahedron(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size, bool rhcoords)
			{
				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				static const Math::Vector3 verts[4] =
				{
					{ 0.f, 0.f, 1.f },
					{ 2.f*SQRT2 / 3.f, 0.f, -1.f / 3.f },
					{ -SQRT2 / 3.f, SQRT6 / 3.f, -1.f / 3.f },
					{ -SQRT2 / 3.f, -SQRT6 / 3.f, -1.f / 3.f }
				};

				static const uint32_t faces[4 * 3] =
				{
					0, 1, 2,
					0, 2, 3,
					0, 3, 1,
					1, 3, 2,
				};

				for (size_t j = 0; j < _countof(faces); j += 3)
				{
					uint32_t v0 = faces[j];
					uint32_t v1 = faces[j + 1];
					uint32_t v2 = faces[j + 2];

					Math::Vector3 normal = (verts[v1] - verts[v0]).Cross(verts[v2] - verts[v0]);
					normal.Normalize();

					uint32_t base = vecVertices.size();
					vecIndices.push_back(base);
					vecIndices.push_back(base + 1);
					vecIndices.push_back(base + 2);

					// Duplicate vertices to use face normals
					Math::Vector3 position = verts[v0] * size;
					vecVertices.push_back(VertexPosTexNor(position, Math::Vector2(0.f, 0.f), normal));

					position = verts[v1] * size;
					vecVertices.push_back(VertexPosTexNor(position, Math::Vector2(1.f, 0.f), normal));

					position = verts[v2] * size;
					vecVertices.push_back(VertexPosTexNor(position, Math::Vector2(0.f, 1.f), normal));
				}

				assert(vecVertices.size() == 4 * 3);
				assert(vecIndices.size() == 4 * 3);

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			bool CreateOctahedron(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size, bool rhcoords)
			{
				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				static const Math::Vector3 verts[6] =
				{
					{ 1, 0, 0 },
					{ -1, 0, 0 },
					{ 0, 1, 0 },
					{ 0, -1, 0 },
					{ 0, 0, 1 },
					{ 0, 0, -1 }
				};

				static const uint32_t faces[8 * 3] =
				{
					4, 0, 2,
					4, 2, 1,
					4, 1, 3,
					4, 3, 0,
					5, 2, 0,
					5, 1, 2,
					5, 3, 1,
					5, 0, 3
				};

				for (size_t j = 0; j < _countof(faces); j += 3)
				{
					uint32_t v0 = faces[j];
					uint32_t v1 = faces[j + 1];
					uint32_t v2 = faces[j + 2];

					Math::Vector3 normal = (verts[v1] - verts[v0]).Cross(verts[v2] - verts[v0]);
					normal.Normalize();

					size_t base = vecVertices.size();
					vecIndices.push_back(base);
					vecIndices.push_back(base + 1);
					vecIndices.push_back(base + 2);

					// Duplicate vertices to use face normals
					Math::Vector3 position = verts[v0] * size;
					vecVertices.push_back(VertexPosTexNor(position, Math::Vector2(0.f, 0.f), normal));

					position = verts[v1] * size;
					vecVertices.push_back(VertexPosTexNor(position, Math::Vector2(1.f, 0.f), normal));

					position = verts[v2] * size;
					vecVertices.push_back(VertexPosTexNor(position, Math::Vector2(0.f, 1.f), normal));
				}

				assert(vecVertices.size() == 8 * 3);
				assert(vecIndices.size() == 8 * 3);

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			bool CreateDodecahedron(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size, bool rhcoords)
			{
				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				static const float a = 1.f / SQRT3;
				static const float b = 0.356822089773089931942f; // sqrt( ( 3 - sqrt(5) ) / 6 )
				static const float c = 0.934172358962715696451f; // sqrt( ( 3 + sqrt(5) ) / 6 );

				static const Math::Vector3 verts[20] =
				{
					{ a, a, a },
					{ a, a, -a },
					{ a, -a, a },
					{ a, -a, -a },
					{ -a, a, a },
					{ -a, a, -a },
					{ -a, -a, a },
					{ -a, -a, -a },
					{ b, c, 0 },
					{ -b, c, 0 },
					{ b, -c, 0 },
					{ -b, -c, 0 },
					{ c, 0, b },
					{ c, 0, -b },
					{ -c, 0, b },
					{ -c, 0, -b },
					{ 0, b, c },
					{ 0, -b, c },
					{ 0, b, -c },
					{ 0, -b, -c }
				};

				static const uint32_t faces[12 * 5] =
				{
					0, 8, 9, 4, 16,
					0, 16, 17, 2, 12,
					12, 2, 10, 3, 13,
					9, 5, 15, 14, 4,
					3, 19, 18, 1, 13,
					7, 11, 6, 14, 15,
					0, 12, 13, 1, 8,
					8, 1, 18, 5, 9,
					16, 4, 14, 6, 17,
					6, 11, 10, 2, 17,
					7, 15, 5, 18, 19,
					7, 19, 3, 10, 11,
				};

				static const Math::Vector2 textureCoordinates[5] =
				{
					{ 0.654508f, 0.0244717f },
					{ 0.0954915f, 0.206107f },
					{ 0.0954915f, 0.793893f },
					{ 0.654508f, 0.975528f },
					{ 1.f, 0.5f }
				};

				static const uint32_t textureIndex[12][5] =
				{
					{ 0, 1, 2, 3, 4 },
					{ 2, 3, 4, 0, 1 },
					{ 4, 0, 1, 2, 3 },
					{ 1, 2, 3, 4, 0 },
					{ 2, 3, 4, 0, 1 },
					{ 0, 1, 2, 3, 4 },
					{ 1, 2, 3, 4, 0 },
					{ 4, 0, 1, 2, 3 },
					{ 4, 0, 1, 2, 3 },
					{ 1, 2, 3, 4, 0 },
					{ 0, 1, 2, 3, 4 },
					{ 2, 3, 4, 0, 1 },
				};

				size_t t = 0;
				uint32_t nCount = _countof(faces);
				for (size_t j = 0; j < nCount; j += 5, ++t)
				{
					uint32_t v0 = faces[j];
					uint32_t v1 = faces[j + 1];
					uint32_t v2 = faces[j + 2];
					uint32_t v3 = faces[j + 3];
					uint32_t v4 = faces[j + 4];

					Math::Vector3 normal = (verts[v1] - verts[v0]).Cross(verts[v2] - verts[v0]);
					normal.Normalize();

					size_t base = vecVertices.size();

					vecIndices.push_back(base);
					vecIndices.push_back(base + 2);
					vecIndices.push_back(base + 1);

					vecIndices.push_back(base);
					vecIndices.push_back(base + 3);
					vecIndices.push_back(base + 2);

					vecIndices.push_back(base);
					vecIndices.push_back(base + 4);
					vecIndices.push_back(base + 3);

					// Duplicate vertices to use face normals
					Math::Vector3 position = verts[v0] * size;
					vecVertices.push_back(VertexPosTexNor(position, textureCoordinates[textureIndex[t][0]], normal));

					position = verts[v1] * size;
					vecVertices.push_back(VertexPosTexNor(position, textureCoordinates[textureIndex[t][1]], normal));

					position = verts[v2] * size;
					vecVertices.push_back(VertexPosTexNor(position, textureCoordinates[textureIndex[t][2]], normal));

					position = verts[v3] * size;
					vecVertices.push_back(VertexPosTexNor(position, textureCoordinates[textureIndex[t][3]], normal));

					position = verts[v4] * size;
					vecVertices.push_back(VertexPosTexNor(position, textureCoordinates[textureIndex[t][4]], normal));
				}

				assert(vecVertices.size() == 12 * 5);
				assert(vecIndices.size() == 12 * 3 * 3);

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			bool CreateIcosahedron(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size, bool rhcoords)
			{
				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				static const float  t = 1.618033988749894848205f; // (1 + sqrt(5)) / 2
				static const float t2 = 1.519544995837552493271f; // sqrt( 1 + sqr( (1 + sqrt(5)) / 2 ) )

				static const Math::Vector3 verts[12] =
				{
					{ t / t2, 1.f / t2, 0 },
					{ -t / t2, 1.f / t2, 0 },
					{ t / t2, -1.f / t2, 0 },
					{ -t / t2, -1.f / t2, 0 },
					{ 1.f / t2, 0, t / t2 },
					{ 1.f / t2, 0, -t / t2 },
					{ -1.f / t2, 0, t / t2 },
					{ -1.f / t2, 0, -t / t2 },
					{ 0, t / t2, 1.f / t2 },
					{ 0, -t / t2, 1.f / t2 },
					{ 0, t / t2, -1.f / t2 },
					{ 0, -t / t2, -1.f / t2 }
				};

				static const uint32_t faces[20 * 3] =
				{
					0, 8, 4,
					0, 5, 10,
					2, 4, 9,
					2, 11, 5,
					1, 6, 8,
					1, 10, 7,
					3, 9, 6,
					3, 7, 11,
					0, 10, 8,
					1, 8, 10,
					2, 9, 11,
					3, 11, 9,
					4, 2, 0,
					5, 0, 2,
					6, 1, 3,
					7, 3, 1,
					8, 6, 4,
					9, 4, 6,
					10, 5, 7,
					11, 7, 5
				};

				uint32_t nCount = _countof(faces);
				for (size_t j = 0; j < nCount; j += 3)
				{
					uint32_t v0 = faces[j];
					uint32_t v1 = faces[j + 1];
					uint32_t v2 = faces[j + 2];

					Math::Vector3 normal = (verts[v1] - verts[v0]).Cross(verts[v2] - verts[v0]);
					normal.Normalize();

					size_t base = vecVertices.size();
					vecIndices.push_back(base);
					vecIndices.push_back(base + 2);
					vecIndices.push_back(base + 1);

					// Duplicate vertices to use face normals
					Math::Vector3 position = verts[v0] * size;
					vecVertices.push_back(VertexPosTexNor(position, Math::Vector2(0.f, 0.f), normal));

					position = verts[v1] * size;
					vecVertices.push_back(VertexPosTexNor(position, Math::Vector2(1.f, 0.f), normal));

					position = verts[v2] * size;
					vecVertices.push_back(VertexPosTexNor(position, Math::Vector2(0.f, 1.f), normal));
				}

				assert(vecVertices.size() == 20 * 3);
				assert(vecIndices.size() == 20 * 3);

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			// Include the teapot control point data.
			namespace
			{
		#include "ExternLib/DirectXTK/Src/TeapotData.inc"
		#include "ExternLib/DirectXTK/Src/Bezier.h"
			}

			bool CreateTeapot(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size, uint32_t tessellation, bool rhcoords)
			{
				auto TessellatePatch = [](std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, TeapotPatch const& patch, size_t tessellation, FXMVECTOR scale, bool isMirrored)
				{
					// Look up the 16 control points for this patch.
					XMVECTOR controlPoints[16];

					for (int i = 0; i < 16; i++)
					{
						controlPoints[i] = TeapotControlPoints[patch.indices[i]] * scale;
					}

					// Create the index data.
					size_t vbase = vertices.size();
					Bezier::CreatePatchIndices(tessellation, isMirrored, [&](size_t index)
					{
						indices.push_back(vbase + index);
					});

					// Create the vertex data.
					Bezier::CreatePatchVertices(controlPoints, tessellation, isMirrored, [&](FXMVECTOR position, FXMVECTOR normal, FXMVECTOR textureCoordinate)
					{
						vertices.push_back(VertexPosTexNor(position, normal, textureCoordinate));
					});
				};

				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				if (tessellation < 1)
					throw std::out_of_range("tesselation parameter out of range");

				XMVECTOR scaleVector = XMVectorReplicate(size);

				XMVECTOR scaleNegateX = scaleVector * g_XMNegateX;
				XMVECTOR scaleNegateZ = scaleVector * g_XMNegateZ;
				XMVECTOR scaleNegateXZ = scaleVector * g_XMNegateX * g_XMNegateZ;

				for (int i = 0; i < sizeof(TeapotPatches) / sizeof(TeapotPatches[0]); i++)
				{
					TeapotPatch const& patch = TeapotPatches[i];

					// Because the teapot is symmetrical from left to right, we only store
					// data for one side, then tessellate each patch twice, mirroring in X.
					TessellatePatch(vecVertices, vecIndices, patch, tessellation, scaleVector, false);
					TessellatePatch(vecVertices, vecIndices, patch, tessellation, scaleNegateX, true);

					if (patch.mirrorZ)
					{
						// Some parts of the teapot (the body, lid, and rim, but not the
						// handle or spout) are also symmetrical from front to back, so
						// we tessellate them four times, mirroring in Z as well as X.
						TessellatePatch(vecVertices, vecIndices, patch, tessellation, scaleNegateZ, true);
						TessellatePatch(vecVertices, vecIndices, patch, tessellation, scaleNegateXZ, false);
					}
				}

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			bool CreateHexagon(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float radius, bool rhcoords)
			{
				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				vecVertices.resize(7);
				vecIndices.resize(18);

				float fHalf = radius * 0.5f;

				vecVertices[0].pos = Math::Vector3(0.f, 0.f, 0.f);
				vecVertices[0].normal = Math::Vector3(0.f, 1.f, 0.f);
				vecVertices[0].uv = Math::Vector2(0.5f, 0.5f);

				vecVertices[1].pos = Math::Vector3(fHalf, 0.f, radius);
				vecVertices[1].normal = Math::Vector3(0.f, 1.f, 0.f);
				vecVertices[1].uv = Math::Vector2(0.75f, 0.f);

				vecVertices[2].pos = Math::Vector3(radius, 0.f, 0.f);
				vecVertices[2].normal = Math::Vector3(0.f, 1.f, 0.f);
				vecVertices[2].uv = Math::Vector2(1.f, 0.5f);

				vecVertices[3].pos = Math::Vector3(fHalf, 0.f, -radius);
				vecVertices[3].normal = Math::Vector3(0.f, 1.f, 0.f);
				vecVertices[3].uv = Math::Vector2(0.75f, 1.f);

				vecVertices[4].pos = Math::Vector3(-fHalf, 0.f, -radius);
				vecVertices[4].normal = Math::Vector3(0.f, 1.f, 0.f);
				vecVertices[4].uv = Math::Vector2(0.25f, 1.f);

				vecVertices[5].pos = Math::Vector3(-radius, 0.f, 0.f);
				vecVertices[5].normal = Math::Vector3(0.f, 1.f, 0.f);
				vecVertices[5].uv = Math::Vector2(0.f, 0.5f);

				vecVertices[6].pos = Math::Vector3(-fHalf, 0.f, radius);
				vecVertices[6].normal = Math::Vector3(0.f, 1.f, 0.f);
				vecVertices[6].uv = Math::Vector2(0.25f, 0.f);

				vecIndices[0] = 2;		vecIndices[1] = 1;		vecIndices[2] = 0;
				vecIndices[3] = 3;		vecIndices[4] = 2;		vecIndices[5] = 0;
				vecIndices[6] = 4;		vecIndices[7] = 3;		vecIndices[8] = 0;
				vecIndices[9] = 5;		vecIndices[10] = 4;		vecIndices[11] = 0;
				vecIndices[12] = 6;		vecIndices[13] = 5;		vecIndices[14] = 0;
				vecIndices[15] = 1;		vecIndices[16] = 6;		vecIndices[17] = 0;

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, rhcoords, false);
			}

			bool CreateCapsule(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float fRadius, float fHeight, int nSubdivisionHeight, int nSegments)
			{
				if (fRadius <= 0.f)
				{
					fRadius = 0.f;
				}

				if (fHeight <= 0.f)
				{
					fHeight = fRadius * 2.f;
				}

				if (nSubdivisionHeight <= 0)
				{
					nSubdivisionHeight = 12;
				}

				if (nSegments <= 0)
				{
					nSegments = 12;
				}

				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				auto CalculateRing = [&](int segments, float r, float y, float dy)
				{
					float segIncr = 1.f / static_cast<float>(segments - 1);

					for (int i = 0; i < segments; ++i)
					{
						float x = -cos((Math::PI * 2.f) * i * segIncr) * r;
						float z = sin((Math::PI * 2.f) * i * segIncr) * r;

						VertexPosTexNor vertex;
						vertex.pos = Math::Vector3(fRadius * x, fRadius * y + fHeight * dy, fRadius * z);
						vertex.normal = Math::Vector3(x, y, z);
						vertex.uv = Math::Vector2(i * segIncr, 0.5f - ((fRadius * y + fHeight * dy) / (2.f * fRadius + fHeight)));

						vecVertices.push_back(vertex);
					}
				};

				int nRingsBody = nSubdivisionHeight + 1;
				int nRingsTotal = nSubdivisionHeight + nRingsBody;

				float fBodyIncr = 1.f / static_cast<float>(nRingsBody - 1);
				float fRingIncr = 1.f / static_cast<float>(nSubdivisionHeight - 1);
				for (int i = 0; i < nSubdivisionHeight / 2; ++i)
				{
					CalculateRing(nSegments, sin(Math::PI * i * fRingIncr), sin(Math::PI * (i * fRingIncr - 0.5f)), -0.5f);
				}

				for (int i = 0; i < 2; ++i)
				{
					CalculateRing(nSegments, 1.f, 0.f, static_cast<float>(i) * fBodyIncr - 0.5f);
				}

				for (int i = nSubdivisionHeight / 2; i < nSubdivisionHeight; ++i)
				{
					CalculateRing(nSegments, sin(Math::PI * i * fRingIncr), sin(Math::PI * (i * fRingIncr - 0.5f)), 0.5f);
				}

				for (int i = 0; i < nRingsTotal; ++i)
				{
					for (int j = 0; j < nSegments - 1; ++j)
					{
						vecIndices.push_back(i * nSegments + (j + 1));
						vecIndices.push_back(i * nSegments + (j + 0));
						vecIndices.push_back((i + 1) * nSegments + (j + 1));
						
						vecIndices.push_back((i + 1) * nSegments + (j + 0));
						vecIndices.push_back((i + 1) * nSegments + (j + 1));
						vecIndices.push_back(i * nSegments + j);
					}
				}

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, false, false);
			}

			bool CreateGrid(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float fGridSizeX, float fGridSizeZ, uint32_t nBlockCountWidth, uint32_t nBlockCountLength)
			{
				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				vecVertices.reserve((nBlockCountWidth + 1) * (nBlockCountLength + 1));
				vecIndices.reserve(3 * 2 * nBlockCountWidth * nBlockCountLength);

				float fStartX = fGridSizeX * -0.5f;
				float fStartZ = fGridSizeZ * -0.5f;
				float fEachSizeX = fGridSizeX / nBlockCountWidth;
				float fEachSizeZ = fGridSizeZ / nBlockCountLength;

				for (uint32_t i = 0; i <= nBlockCountLength; ++i)
				{
					for (uint32_t j = 0; j <= nBlockCountWidth; ++j)
					{
						VertexPosTexNor vertex;
						vertex.pos = Math::Vector3(fStartX + j * fEachSizeX, 0.f, fStartZ + i * fEachSizeZ);
						vertex.normal = Math::Vector3(0.f, 1.f, 0.f);
						vertex.uv = Math::Vector2((float)j / nBlockCountWidth, (float)i / nBlockCountLength);

						vecVertices.push_back(vertex);
					}
				}

				uint32_t dwWidth = nBlockCountWidth + 1;
				for (uint32_t i = 0; i < nBlockCountLength; ++i)
				{
					for (uint32_t j = 0; j < nBlockCountWidth; ++j)
					{
						vecIndices.push_back((dwWidth * i) + 0 + j);
						vecIndices.push_back((dwWidth * i) + dwWidth + j);
						vecIndices.push_back((dwWidth * i) + (dwWidth + 1) + j);

						vecIndices.push_back((dwWidth * i) + 0 + j);
						vecIndices.push_back((dwWidth * i) + (dwWidth + 1) + j);
						vecIndices.push_back((dwWidth * i) + 1 + j);
					}
				}

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, false, false);
			}

			bool CreatePlane(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float fEachLengthX, float fEachLengthZ, int nTotalCountX, int nTotalCountZ)
			{
				std::vector<VertexPosTexNor> vecVertices;
				std::vector<uint32_t> vecIndices;

				vecVertices.reserve(4 * nTotalCountX * nTotalCountZ);
				vecIndices.reserve(6 * nTotalCountX * nTotalCountZ);

				int nCnt = 0;
				float fStartX = nTotalCountX * fEachLengthX * -0.5f;
				float fStartZ = nTotalCountZ * fEachLengthZ * -0.5f;
				for (int i = 0; i < nTotalCountX; ++i)
				{
					for (int j = 0; j < nTotalCountZ; ++j)
					{
						vecVertices.push_back(VertexPosTexNor(Math::Vector3(fStartX + i * fEachLengthX, 0.f, fStartZ + j * fEachLengthZ),
							Math::Vector2(0.f, 1.f), Math::Vector3(0.f, 1.f, 0.f)));
						vecVertices.push_back(VertexPosTexNor(Math::Vector3(fStartX + i * fEachLengthX, 0.f, fStartZ + j * fEachLengthZ + fEachLengthZ),
							Math::Vector2(0.f, 0.f), Math::Vector3(0.f, 1.f, 0.f)));
						vecVertices.push_back(VertexPosTexNor(Math::Vector3(fStartX + i * fEachLengthX + fEachLengthX, 0.f, fStartZ + j * fEachLengthZ),
							Math::Vector2(1.f, 1.f), Math::Vector3(0.f, 1.f, 0.f)));
						vecVertices.push_back(VertexPosTexNor(Math::Vector3(fStartX + i * fEachLengthX + fEachLengthX, 0.f, fStartZ + j * fEachLengthZ + fEachLengthZ),
							Math::Vector2(1.f, 0.f), Math::Vector3(0.f, 1.f, 0.f)));
						nCnt += 4;

						vecIndices.push_back(nCnt - 1);
						vecIndices.push_back(nCnt - 3);
						vecIndices.push_back(nCnt - 4);
						vecIndices.push_back(nCnt - 2);
						vecIndices.push_back(nCnt - 1);
						vecIndices.push_back(nCnt - 4);
					}
				}

				return InitBuffer(ppVertexBuffer, ppIndexBuffer, vecVertices, vecIndices, false, false);
			}

			// Mesh Simplification
			// code by https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
			namespace Simplify
			{
				struct Vector3
				{
					float x, y, z;
				};

				struct vec3f
				{
					float x, y, z;

					inline vec3f(void) {}

					//inline vec3f operator =( Math::Vector3 a )
					// { vec3f b ; b.x = a.x; b.y = a.y; b.z = a.z; return b;}

					inline vec3f(Math::Vector3 a)
					{
						x = a.x; y = a.y; z = a.z;
					}

					inline vec3f(const float X, const float Y, const float Z)
					{
						x = X; y = Y; z = Z;
					}

					inline vec3f operator + (const vec3f& a) const
					{
						return vec3f(x + a.x, y + a.y, z + a.z);
					}

					inline vec3f operator += (const vec3f& a) const
					{
						return vec3f(x + a.x, y + a.y, z + a.z);
					}

					inline vec3f operator * (const float a) const
					{
						return vec3f(x * a, y * a, z * a);
					}

					inline vec3f operator * (const vec3f a) const
					{
						return vec3f(x * a.x, y * a.y, z * a.z);
					}

					inline vec3f operator = (const Math::Vector3 a)
					{
						x = a.x; y = a.y; z = a.z; return *this;
					}

					inline vec3f operator = (const vec3f a)
					{
						x = a.x; y = a.y; z = a.z; return *this;
					}

					inline vec3f operator / (const vec3f a) const
					{
						return vec3f(x / a.x, y / a.y, z / a.z);
					}

					inline vec3f operator - (const vec3f& a) const
					{
						return vec3f(x - a.x, y - a.y, z - a.z);
					}

					inline vec3f operator / (const float a) const
					{
						return vec3f(x / a, y / a, z / a);
					}

					inline float dot(const vec3f& a) const
					{
						return a.x*x + a.y*y + a.z*z;
					}

					inline vec3f cross(const vec3f& a, const vec3f& b)
					{
						x = a.y * b.z - a.z * b.y;
						y = a.z * b.x - a.x * b.z;
						z = a.x * b.y - a.y * b.x;
						return *this;
					}

					inline float angle(const vec3f& v)
					{
						vec3f a = v, b = *this;
						float dot = v.x*x + v.y*y + v.z*z;
						float len = a.length() * b.length();
						if (len == 0)len = 0.00001f;
						float input = dot / len;
						if (input<-1) input = -1;
						if (input>1) input = 1;
						return acos(input);
					}

					inline float angle2(const vec3f& v, const vec3f& w)
					{
						vec3f a = v, b = *this;
						float dot = a.x*b.x + a.y*b.y + a.z*b.z;
						float len = a.length() * b.length();
						if (len == 0)len = 1;

						vec3f plane; plane.cross(b, w);

						if (plane.x * a.x + plane.y * a.y + plane.z * a.z > 0)
							return -acos(dot / len);

						return acos(dot / len);
					}

					inline vec3f rot_x(float a)
					{
						float yy = cos(a) * y + sin(a) * z;
						float zz = cos(a) * z - sin(a) * y;
						y = yy; z = zz;
						return *this;
					}
					inline vec3f rot_y(float a)
					{
						float xx = cos(-a) * x + sin(-a) * z;
						float zz = cos(-a) * z - sin(-a) * x;
						x = xx; z = zz;
						return *this;
					}
					inline void clamp(float min, float max)
					{
						if (x<min) x = min;
						if (y<min) y = min;
						if (z<min) z = min;
						if (x>max) x = max;
						if (y>max) y = max;
						if (z>max) z = max;
					}
					inline vec3f rot_z(float a)
					{
						float yy = cos(a) * y + sin(a) * x;
						float xx = cos(a) * x - sin(a) * y;
						y = yy; x = xx;
						return *this;
					}
					inline vec3f invert()
					{
						x = -x; y = -y; z = -z; return *this;
					}
					inline vec3f frac()
					{
						return vec3f(
							x - float(int(x)),
							y - float(int(y)),
							z - float(int(z))
						);
					}

					inline vec3f integer()
					{
						return vec3f(
							float(int(x)),
							float(int(y)),
							float(int(z))
						);
					}

					inline float length() const
					{
						return sqrt(x*x + y*y + z*z);
					}

					inline vec3f normalize(float desired_length = 1)
					{
						float square = sqrt(x*x + y*y + z*z);
						/*
						if (square <= 0.00001f )
						{
						x=1;y=0;z=0;
						return *this;
						}*/
						//float len = desired_length / square; 
						x /= square; y /= square; z /= square;

						return *this;
					}
				};

				class SymetricMatrix
				{
				public:
					// Constructor
					SymetricMatrix(float c = 0)
					{
						for (int i = 0; i < 10; ++i)
						{
							m[i] = c;
						}
					}

					SymetricMatrix(float m11, float m12, float m13, float m14,
						float m22, float m23, float m24,
						float m33, float m34,
						float m44) {
						m[0] = m11;  m[1] = m12;  m[2] = m13;  m[3] = m14;
						m[4] = m22;  m[5] = m23;  m[6] = m24;
						m[7] = m33;  m[8] = m34;
						m[9] = m44;
					}

					// Make plane

					SymetricMatrix(float a, float b, float c, float d)
					{
						m[0] = a*a;  m[1] = a*b;  m[2] = a*c;  m[3] = a*d;
						m[4] = b*b;  m[5] = b*c;  m[6] = b*d;
						m[7] = c*c; m[8] = c*d;
						m[9] = d*d;
					}

					float operator[](int c) const { return m[c]; }

					// Determinant

					float det(int a11, int a12, int a13,
						int a21, int a22, int a23,
						int a31, int a32, int a33)
					{
						float det = m[a11] * m[a22] * m[a33] + m[a13] * m[a21] * m[a32] + m[a12] * m[a23] * m[a31]
							- m[a13] * m[a22] * m[a31] - m[a11] * m[a23] * m[a32] - m[a12] * m[a21] * m[a33];
						return det;
					}

					const SymetricMatrix operator+(const SymetricMatrix& n) const
					{
						return SymetricMatrix(m[0] + n[0], m[1] + n[1], m[2] + n[2], m[3] + n[3],
							m[4] + n[4], m[5] + n[5], m[6] + n[6],
							m[7] + n[7], m[8] + n[8],
							m[9] + n[9]);
					}

					SymetricMatrix& operator+=(const SymetricMatrix& n)
					{
						m[0] += n[0];   m[1] += n[1];   m[2] += n[2];   m[3] += n[3];
						m[4] += n[4];   m[5] += n[5];   m[6] += n[6];   m[7] += n[7];
						m[8] += n[8];   m[9] += n[9];
						return *this;
					}

					float m[10];
				};
				/////////////////////////////////////////

				// Global Variables & Strctures

				struct Triangle { int v[3]; float err[4]; int deleted, dirty; vec3f n; };
				struct Vertex { vec3f p; int tstart, tcount; SymetricMatrix q; int border; VertexPosTexNor vertex; };
				struct Ref { int tid, tvertex; };
				std::vector<Triangle> triangles;
				std::vector<Vertex> vertices;
				std::vector<Ref> refs;

				// Helper functions

				float vertex_error(SymetricMatrix q, float x, float y, float z);
				float calculate_error(int id_v1, int id_v2, vec3f& p_result, Math::Vector2* pOut_f2UV, Math::Vector3* pOut_f3Normal);
				bool flipped(vec3f p, int i0, int i1, Vertex& v0, Vertex& v1, std::vector<int>& deleted);
				void update_triangles(int i0, Vertex& v, std::vector<int>& deleted, int& deleted_triangles);
				void update_mesh(int iteration);
				void compact_mesh();

				//
				// Main simplification function 
				//
				// target_count  : target nr. of triangles
				// agressiveness : sharpness to increase the threashold.
				//                 5..8 are good numbers
				//                 more iterations yield higher quality
				//
				bool GenerateSimplificationMesh(const std::vector<VertexPosTexNor>& in_vecVertices, const std::vector<uint32_t>& in_vecIndices, std::vector<VertexPosTexNor>& out_vecVertices, std::vector<uint32_t>& out_vecIndices, float fReduceFraction, float dAgressiveness)
				{
					if (fReduceFraction <= 0.f)
						return false;

					if (fReduceFraction >= 1.f)
					{
						std::copy(in_vecVertices.begin(), in_vecVertices.end(), std::back_inserter(out_vecVertices));
						std::copy(in_vecIndices.begin(), in_vecIndices.end(), std::back_inserter(out_vecIndices));

						return true;
					}

					int target_count = static_cast<int>(std::round((float)(in_vecIndices.size() / 3) * fReduceFraction));
					if (target_count < 4)
					{
						PRINT_LOG("Object will not survive such extreme decimation, Triangle : %d, Target : %d, Reduce Fraction : %f", triangles.size(), target_count, fReduceFraction);
						return false;
					}

					triangles.clear();
					triangles.resize(in_vecIndices.size() / 3);
					vertices.clear();
					vertices.resize(in_vecVertices.size());

					{
						uint32_t nSize = in_vecVertices.size();
						for (uint32_t i = 0; i < nSize; ++i)
						{
							vertices[i].p = vec3f(in_vecVertices[i].pos.x, in_vecVertices[i].pos.y, in_vecVertices[i].pos.z);
							vertices[i].vertex.SetVertex(in_vecVertices[i].pos, in_vecVertices[i].uv, in_vecVertices[i].normal);
						}

						uint32_t nIdx = 0;
						nSize = in_vecIndices.size();
						for (uint32_t i = 0; i < nSize; i += 3)
						{
							uint32_t v0 = in_vecIndices[i + 0];
							uint32_t v1 = in_vecIndices[i + 1];
							uint32_t v2 = in_vecIndices[i + 2];

							triangles[nIdx].v[0] = v0;
							triangles[nIdx].v[1] = v1;
							triangles[nIdx].v[2] = v2;
							++nIdx;
						}
					}

					// init
					for (std::size_t i = 0; i < triangles.size(); ++i)
					{
						triangles[i].deleted = 0;
					}

					// main iteration loop 

					int deleted_triangles = 0;
					std::vector<int> deleted0, deleted1;
					int triangle_count = triangles.size();

					for (int iteration = 0; iteration < 100; ++iteration)
					{
						// target number of triangles reached ? Then break
						//PRINT_LOG("iteration %d - triangles %d\n", iteration, triangle_count - deleted_triangles);
						if (triangle_count - deleted_triangles <= target_count)
							break;

						// update mesh once in a while
						if (iteration % 5 == 0)
						{
							update_mesh(iteration);
						}

						// clear dirty flag
						for (std::size_t i = 0; i < triangles.size(); ++i)
						{
							triangles[i].dirty = 0;
						}

						//
						// All triangles with edges below the threshold will be removed
						//
						// The following numbers works well for most models.
						// If it does not, try to adjust the 3 parameters
						//
						float threshold = 0.000000001f * pow(float(iteration + 3), dAgressiveness);

						// remove vertices&  mark deleted triangles		
						for (std::size_t i = 0; i < triangles.size(); ++i)
						{
							Triangle& t = triangles[i];
							if (t.err[3]>threshold ||
								t.deleted ||
								t.dirty)
								continue;

							for (int j = 0; j < 3; ++j)
							{
								if (t.err[j]<threshold)
								{
									int i0 = t.v[j];
									Vertex& v0 = vertices[i0];
									int i1 = t.v[(j + 1) % 3];
									Vertex& v1 = vertices[i1];

									// Border check
									if (v0.border != v1.border)
										continue;

									// Compute vertex to collapse to
									vec3f p;
									Math::Vector2 uv;
									Math::Vector3 normal;
									calculate_error(i0, i1, p, &uv, &normal);

									deleted0.resize(v0.tcount); // normals temporarily
									deleted1.resize(v1.tcount); // normals temporarily

																// dont remove if flipped
									if (flipped(p, i0, i1, v0, v1, deleted0))
										continue;

									if (flipped(p, i1, i0, v1, v0, deleted1))
										continue;

									// not flipped, so remove edge												
									v0.p = p;
									v0.vertex.pos = Math::Vector3(p.x, p.y, p.z);
									v0.vertex.uv = uv;
									v0.vertex.normal = normal;
									v0.q = v1.q + v0.q;
									int tstart = refs.size();

									update_triangles(i0, v0, deleted0, deleted_triangles);
									update_triangles(i0, v1, deleted1, deleted_triangles);

									int tcount = refs.size() - tstart;

									if (tcount <= v0.tcount)
									{
										// save ram
										if (tcount)Memory::Copy(&refs[v0.tstart], tcount * sizeof(Ref), &refs[tstart], tcount * sizeof(Ref));
									}
									else
										// append
										v0.tstart = tstart;

									v0.tcount = tcount;
									break;
								}
							}

							// done?
							if (triangle_count - deleted_triangles <= target_count)
								break;
						}
					}

					// clean up mesh
					compact_mesh();

					for (std::size_t i = 0; i < vertices.size(); ++i)
					{
						out_vecVertices.emplace_back(Math::Vector3(vertices[i].p.x, vertices[i].p.y, vertices[i].p.z),
							vertices[i].vertex.uv, vertices[i].vertex.normal);
					}

					for (std::size_t i = 0; i < triangles.size(); ++i)
					{
						if (triangles[i].deleted == false)
						{
							out_vecVertices[triangles[i].v[0]].normal = Math::Vector3(triangles[i].n.x, triangles[i].n.y, triangles[i].n.z);
							out_vecVertices[triangles[i].v[1]].normal = Math::Vector3(triangles[i].n.x, triangles[i].n.y, triangles[i].n.z);
							out_vecVertices[triangles[i].v[2]].normal = Math::Vector3(triangles[i].n.x, triangles[i].n.y, triangles[i].n.z);

							out_vecIndices.emplace_back(triangles[i].v[0]);
							out_vecIndices.emplace_back(triangles[i].v[1]);
							out_vecIndices.emplace_back(triangles[i].v[2]);
						}
					}

					triangles.resize(0);
					vertices.resize(0);

					return true;
				}

				// Check if a triangle flips when this edge is removed

				bool flipped(vec3f p, int i0, int i1, Vertex& v0, Vertex& v1, std::vector<int>& deleted)
				{
					int bordercount = 0;
					for (int k = 0; k < v0.tcount; ++k)
					{
						Triangle& t = triangles[refs[v0.tstart + k].tid];
						if (t.deleted)
							continue;

						int s = refs[v0.tstart + k].tvertex;
						int id1 = t.v[(s + 1) % 3];
						int id2 = t.v[(s + 2) % 3];

						if (id1 == i1 || id2 == i1) // delete ?
						{
							bordercount++;
							deleted[k] = 1;
							continue;
						}
						vec3f d1 = vertices[id1].p - p;
						d1.normalize();
						vec3f d2 = vertices[id2].p - p;
						d2.normalize();

						if (fabs(d1.dot(d2)) > 0.999)
							return true;

						vec3f n;
						n.cross(d1, d2);
						n.normalize();
						deleted[k] = 0;

						if (n.dot(t.n) < 0.2)
							return true;
					}
					return false;
				}

				// Update triangle connections and edge error after a edge is collapsed

				void update_triangles(int i0, Vertex& v, std::vector<int>& deleted, int& deleted_triangles)
				{
					vec3f p;
					for (int k = 0; k < v.tcount; ++k)
					{
						Ref& r = refs[v.tstart + k];
						Triangle& t = triangles[r.tid];
						if (t.deleted)continue;
						if (deleted[k])
						{
							t.deleted = 1;
							deleted_triangles++;
							continue;
						}
						t.v[r.tvertex] = i0;
						t.dirty = 1;
						t.err[0] = calculate_error(t.v[0], t.v[1], p, nullptr, nullptr);
						t.err[1] = calculate_error(t.v[1], t.v[2], p, nullptr, nullptr);
						t.err[2] = calculate_error(t.v[2], t.v[0], p, nullptr, nullptr);
						t.err[3] = Math::Min(t.err[0], Math::Min(t.err[1], t.err[2]));
						refs.push_back(r);
					}
				}

				// compact triangles, compute edge error and build reference list

				void update_mesh(int iteration)
				{
					if (iteration > 0) // compact triangles
					{
						int dst = 0;
						for (std::size_t i = 0; i < triangles.size(); ++i)
						{
							if (triangles[i].deleted == false)
							{
								triangles[dst++] = triangles[i];
							}
						}
						triangles.resize(dst);
					}
					//
					// Init Quadrics by Plane & Edge Errors
					//
					// required at the beginning ( iteration == 0 )
					// recomputing during the simplification is not required,
					// but mostly improves the result for closed meshes
					//
					if (iteration == 0)
					{
						for (std::size_t i = 0; i < vertices.size(); ++i)
						{
							vertices[i].q = SymetricMatrix(0.0);
						}

						for (std::size_t i = 0; i < triangles.size(); ++i)
						{
							Triangle& t = triangles[i];
							vec3f n, p[3];

							for (int j = 0; j < 3; ++j)
							{
								p[j] = vertices[t.v[j]].p;
							}

							n.cross(p[1] - p[0], p[2] - p[0]);
							n.normalize();
							t.n = n;

							for (int j = 0; j < 3; ++j)
							{
								vertices[t.v[j]].q =
									vertices[t.v[j]].q + SymetricMatrix(n.x, n.y, n.z, -n.dot(p[0]));
							}
						}

						for (std::size_t i = 0; i < triangles.size(); ++i)
						{
							// Calc Edge Error
							Triangle& t = triangles[i];
							vec3f p;

							for (int j = 0; j < 3; ++j)
							{
								t.err[j] = calculate_error(t.v[j], t.v[(j + 1) % 3], p, nullptr, nullptr);
							}
							t.err[3] = Math::Min(t.err[0], Math::Min(t.err[1], t.err[2]));
						}
					}

					// Init Reference ID list	
					for (std::size_t i = 0; i < vertices.size(); ++i)
					{
						vertices[i].tstart = 0;
						vertices[i].tcount = 0;
					}

					for (std::size_t i = 0; i < triangles.size(); ++i)
					{
						Triangle& t = triangles[i];
						for (int j = 0; j < 3; ++j)
						{
							vertices[t.v[j]].tcount++;
						}
					}

					int tstart = 0;
					for (std::size_t i = 0; i < vertices.size(); ++i)
					{
						Vertex& v = vertices[i];
						v.tstart = tstart;
						tstart += v.tcount;
						v.tcount = 0;
					}

					// Write References
					refs.resize(triangles.size() * 3);
					for (std::size_t i = 0; i < triangles.size(); ++i)
					{
						Triangle& t = triangles[i];
						for (int j = 0; j < 3; ++j)
						{
							Vertex& v = vertices[t.v[j]];
							refs[v.tstart + v.tcount].tid = i;
							refs[v.tstart + v.tcount].tvertex = j;
							v.tcount++;
						}
					}

					// Identify boundary : vertices[].border=0,1 
					if (iteration == 0)
					{
						std::vector<int> vcount, vids;

						for (std::size_t i = 0; i < vertices.size(); ++i)
						{
							vertices[i].border = 0;
						}

						for (std::size_t i = 0; i < vertices.size(); ++i)
						{
							Vertex& v = vertices[i];
							vcount.clear();
							vids.clear();
							for (int j = 0; j < v.tcount; ++j)
							{
								int index = refs[v.tstart + j].tid;
								Triangle& t = triangles[index];

								for (int k = 0; k < 3; ++k)
								{
									std::size_t ofs = 0;
									int id = t.v[k];
									while (ofs<vcount.size())
									{
										if (vids[ofs] == id)
											break;

										ofs++;
									}

									if (ofs == vcount.size())
									{
										vcount.push_back(1);
										vids.push_back(id);
									}
									else
									{
										vcount[ofs]++;
									}
								}
							}

							for (std::size_t j = 0; j < vcount.size(); ++j)
							{
								if (vcount[j] == 1)
								{
									vertices[vids[j]].border = 1;
								}
							}
						}
					}
				}

				// Finally compact mesh before exiting

				void compact_mesh()
				{
					int dst = 0;
					for (std::size_t i = 0; i < vertices.size(); ++i)
					{
						vertices[i].tcount = 0;
					}

					for (std::size_t i = 0; i < triangles.size(); ++i)
					{
						if (!triangles[i].deleted == false)
						{
							Triangle& t = triangles[i];
							triangles[dst++] = t;

							for (int j = 0; j < 3; ++j)
							{
								vertices[t.v[j]].tcount = 1;
							}
						}
					}
					triangles.resize(dst);
					dst = 0;

					for (std::size_t i = 0; i < vertices.size(); ++i)
					{
						if (vertices[i].tcount)
						{
							vertices[i].tstart = dst;
							vertices[dst].p = vertices[i].p;
							vertices[dst].vertex = vertices[i].vertex;
							dst++;
						}
					}

					for (std::size_t i = 0; i < triangles.size(); ++i)
					{
						Triangle& t = triangles[i];
						for (int j = 0; j < 3; ++j)
						{
							t.v[j] = vertices[t.v[j]].tstart;
						}
					}
					vertices.resize(dst);
				}

				// Error between vertex and Quadric

				float vertex_error(SymetricMatrix q, float x, float y, float z)
				{
					return   q[0] * x*x + 2 * q[1] * x*y + 2 * q[2] * x*z + 2 * q[3] * x + q[4] * y*y
						+ 2 * q[5] * y*z + 2 * q[6] * y + q[7] * z*z + 2 * q[8] * z + q[9];
				}

				// Error for one edge

				float calculate_error(int id_v1, int id_v2, vec3f& p_result, Math::Vector2* pOut_f2UV, Math::Vector3* pOut_f3Normal)
				{
					// compute interpolated vertex 
					SymetricMatrix q = vertices[id_v1].q + vertices[id_v2].q;
					bool border = vertices[id_v1].border & vertices[id_v2].border;
					float error = 0.0;
					float det = q.det(0, 1, 2, 1, 4, 5, 2, 5, 7);

					if (det != 0 && !border)
					{
						// q_delta is invertible
						p_result.x = -1 / det*(q.det(1, 2, 3, 4, 5, 6, 5, 7, 8));	// vx = A41/det(q_delta) 
						p_result.y =  1 / det*(q.det(0, 2, 3, 1, 5, 6, 2, 7, 8));	// vy = A42/det(q_delta) 
						p_result.z = -1 / det*(q.det(0, 1, 3, 1, 4, 6, 2, 5, 8));	// vz = A43/det(q_delta) 
						error = vertex_error(q, p_result.x, p_result.y, p_result.z);
					}
					else
					{
						// det = 0 -> try to find best result
						vec3f p1 = vertices[id_v1].p;
						vec3f p2 = vertices[id_v2].p;
						vec3f p3 = (p1 + p2) / 2;

						float error1 = vertex_error(q, p1.x, p1.y, p1.z);
						float error2 = vertex_error(q, p2.x, p2.y, p2.z);
						float error3 = vertex_error(q, p3.x, p3.y, p3.z);

						error = Math::Min(error1, Math::Min(error2, error3));

						if (error1 == error)
						{
							if (pOut_f2UV != nullptr) *pOut_f2UV = vertices[id_v1].vertex.uv;
							if (pOut_f3Normal != nullptr) *pOut_f3Normal = vertices[id_v1].vertex.normal;
							p_result = p1;
						}

						if (error2 == error)
						{
							if (pOut_f2UV != nullptr) *pOut_f2UV = vertices[id_v2].vertex.uv;
							if (pOut_f3Normal != nullptr)*pOut_f3Normal = vertices[id_v2].vertex.normal;
							p_result = p2;
						}

						if (error3 == error)
						{
							if (pOut_f2UV != nullptr) *pOut_f2UV = (vertices[id_v1].vertex.uv + vertices[id_v2].vertex.uv) / 2;
							if (pOut_f3Normal != nullptr)
							{
								*pOut_f3Normal = (vertices[id_v1].vertex.normal + vertices[id_v2].vertex.normal) / 2;
								pOut_f3Normal->Normalize();
							}
							p_result = p3;
						}
					}
					return error;
				}
			};
		}
	}
}