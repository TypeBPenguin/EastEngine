#include "stdafx.h"
#include "GeometryModel.h"

#include <DirectXMath.h>

using namespace DirectX;

namespace eastengine
{
	namespace graphics
	{
		namespace geometry
		{
			const float SQRT2 = 1.41421356237309504880f;
			const float SQRT3 = 1.73205080756887729352f;
			const float SQRT6 = 2.44948974278317809820f;

			void CalculateTangentBinormal(const VertexPosTex& vertex1, const VertexPosTex& vertex2, const VertexPosTex& vertex3,
				math::float3& tangent, math::float3& binormal)
			{
				math::float3 vector1, vector2;
				math::float2 tuvVector1, tuvVector2;

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

			void CalculateTangentBinormal(const math::float3& normal, math::float3& tangent, math::float3& binormal)
			{
				math::float3 c1 = normal.Cross(math::float3(0.f, 0.f, 1.f));
				math::float3 c2 = normal.Cross(math::float3(0.f, 1.f, 0.f));

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

			void CalculateNormal(const math::float3& vPos0, const math::float3& vPos1, const math::float3& vPos2, math::float3& vNormal)
			{
				// Calculate the two vectors for this face.
				math::float3 v0(vPos0);
				math::float3 v1(vPos1.x - vPos2.x, vPos1.y - vPos2.y, vPos1.z - vPos2.z);
				math::float3 v2(vPos2.x - vPos1.x, vPos2.y - vPos1.y, vPos2.z - vPos1.z);

				// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
				vNormal = math::float3((v0.y * v1.z) - (v0.z * v1.y),
					(v0.z * v1.x) - (v0.x * v1.z),
					(v0.x * v1.y) - (v0.y * v1.x));

				// Calculate the length.
				vNormal.Normalize();
			}

			void SortBuffer(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, bool rhcoords, bool invertn)
			{
				if (rhcoords == false)
				{
					for (auto it = indices.begin(); it != indices.end(); it += 3)
					{
						std::swap(*it, *(it + 2));
					}

					for (auto it = vertices.begin(); it != vertices.end(); ++it)
					{
						it->uv.x = (1.f - it->uv.x);
					}
				}

				if (invertn == true)
				{
					const size_t nSize = vertices.size();
					for (size_t i = 0; i < nSize; ++i)
					{
						vertices[i].normal.x = -vertices[i].normal.x;
						vertices[i].normal.y = -vertices[i].normal.y;
						vertices[i].normal.z = -vertices[i].normal.z;
					}
				}
			}

			bool CreateCube(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size, bool rhcoords)
			{
				return CreateBox(vertices, indices, math::float3(size, size, size), rhcoords);
			}

			bool CreateBox(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, const math::float3& size, bool rhcoords, bool invertn)
			{
				vertices.clear();
				indices.clear();

				// A box has six faces, each one pointing in a different direction.
				const int FaceCount = 6;

				static const math::float3 faceNormals[FaceCount] =
				{
					{ 0, 0, 1 },
					{ 0, 0, -1 },
					{ 1, 0, 0 },
					{ -1, 0, 0 },
					{ 0, 1, 0 },
					{ 0, -1, 0 },
				};

				static const math::float2 textureCoordinates[4] =
				{
					{ 1, 0 },
					{ 1, 1 },
					{ 0, 1 },
					{ 0, 0 },
				};

				vertices.reserve(FaceCount * 4);
				indices.reserve(FaceCount * 6);

				// Create each face in turn.
				for (int i = 0; i < FaceCount; ++i)
				{
					const math::float3& normal = faceNormals[i];

					// Get two vectors perpendicular both to the face normal and to each other.
					const math::float3 basis = (i >= 4) ? math::float3(0.f, 0.f, 1.f) : math::float3(0.f, 1.f, 0.f);

					const math::float3 side1 = normal.Cross(basis);
					const math::float3 side2 = normal.Cross(side1);

					// Six indices (two triangles) per face.
					const uint32_t vbase = static_cast<uint32_t>(vertices.size());
					indices.emplace_back(vbase + 0);
					indices.emplace_back(vbase + 1);
					indices.emplace_back(vbase + 2);

					indices.emplace_back(vbase + 0);
					indices.emplace_back(vbase + 2);
					indices.emplace_back(vbase + 3);

					// Four vertices per face.
					vertices.emplace_back((normal - side1 - side2) * size, textureCoordinates[0], normal);
					vertices.emplace_back((normal - side1 + side2) * size, textureCoordinates[1], normal);
					vertices.emplace_back((normal + side1 + side2) * size, textureCoordinates[2], normal);
					vertices.emplace_back((normal + side1 - side2) * size, textureCoordinates[3], normal);
				}

				SortBuffer(vertices, indices, rhcoords, invertn);

				return true;
			}

			bool CreateSphere(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float diameter, uint32_t tessellation, bool rhcoords, bool invertn)
			{
				if (tessellation < 3)
					return false;

				vertices.clear();
				indices.clear();

				const uint32_t verticalSegments = tessellation;
				const uint32_t horizontalSegments = tessellation * 2;

				vertices.reserve((verticalSegments + 1) * (horizontalSegments + 1));
				indices.reserve(verticalSegments * (horizontalSegments + 1) * 6);

				const float radius = diameter * 0.5f;

				// Create rings of vertices at progressively higher latitudes.
				for (uint32_t i = 0; i <= verticalSegments; ++i)
				{
					const float v = 1.f - static_cast<float>(i) / verticalSegments;

					const float latitude = (i * math::PI / verticalSegments) - math::PIDIV2;

					float dy = 0.f;
					float dxz = 0.f;

					math::SinCos(&dy, &dxz, latitude);

					// Create a single ring of vertices at this latitude.
					for (uint32_t j = 0; j <= horizontalSegments; ++j)
					{
						const float u = static_cast<float>(j) / horizontalSegments;

						const float longitude = j * math::PI2 / horizontalSegments;

						float dx = 0.f;
						float dz = 0.f;;

						math::SinCos(&dx, &dz, longitude);

						dx *= dxz;
						dz *= dxz;

						const math::float3 normal(dx, dy, dz);
						const math::float2 textureCoordinate(u, v);
						vertices.emplace_back(normal * radius, textureCoordinate, normal);
					}
				}

				// Fill the index buffer with triangles joining each pair of latitude rings.
				const uint32_t stride = horizontalSegments + 1;

				for (uint32_t i = 0; i < verticalSegments; ++i)
				{
					for (uint32_t j = 0; j <= horizontalSegments; ++j)
					{
						const uint32_t nextI = i + 1;
						const uint32_t nextJ = (j + 1) % stride;

						indices.push_back(i * stride + j);
						indices.push_back(nextI * stride + j);
						indices.push_back(i * stride + nextJ);

						indices.push_back(i * stride + nextJ);
						indices.push_back(nextI * stride + j);
						indices.push_back(nextI * stride + nextJ);
					}
				}

				SortBuffer(vertices, indices, rhcoords, invertn);

				return true;
			}

			bool CreateGeoSphere(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float diameter, uint32_t tessellation, bool rhcoords)
			{
				vertices.clear();
				indices.clear();

				// An undirected edge between two vertices, represented by a pair of indexes into a vertex array.
				// Becuse this edge is undirected, (a,b) is the same as (b,a).
				using UndirectedEdge = std::pair<uint32_t, uint32_t>;

				// Makes an undirected edge. Rather than overloading comparison operators to give us the (a,b)==(b,a) property,
				// we'll just ensure that the larger of the two goes first. This'll simplify things greatly.
				auto makeUndirectedEdge = [](uint32_t a, uint32_t b)
				{
					return std::make_pair(std::max(a, b), std::min(a, b));
				};

				// Key: an edge
				// Value: the index of the vertex which lies midway between the two vertices pointed to by the key value
				// This map is used to avoid duplicating vertices when subdividing triangles along edges.
				typedef std::map<UndirectedEdge, uint32_t> EdgeSubdivisionMap;

				const math::float3 OctahedronVertices[] =
				{
					// when looking down the negative z-axis (into the screen)
					{ 0.f, 1.f, 0.f }, // 0 top
					{ 0.f, 0.f, -1.f }, // 1 front
					{ 1.f, 0.f, 0.f }, // 2 right
					{ 0.f, 0.f, 1.f }, // 3 back
					{ -1.f, 0.f, 0.f }, // 4 left
					{ 0.f, -1.f, 0.f }, // 5 bottom
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

				std::vector<math::float3> vertexPositions(std::begin(OctahedronVertices), std::end(OctahedronVertices));

				indices.assign(std::begin(OctahedronIndices), std::end(OctahedronIndices));

				// We know these values by looking at the above index list for the octahedron. Despite the subdivisions that are
				// about to go on, these values aren't ever going to change because the vertices don't move around in the array.
				// We'll need these values later on to fix the singularities that show up at the poles.
				const uint32_t northPoleIndex = 0;
				const uint32_t southPoleIndex = 5;

				for (uint32_t iSubdivision = 0; iSubdivision < tessellation; ++iSubdivision)
				{
					assert(indices.size() % 3 == 0); // sanity

					// We use this to keep track of which edges have already been subdivided.
					EdgeSubdivisionMap subdividedEdges;

					// The new index collection after subdivision.
					std::vector<uint32_t> newIndices;

					const size_t triangleCount = indices.size()  / 3;
					for (size_t nTriangle = 0; nTriangle < triangleCount; ++nTriangle)
					{
						// For each edge on this triangle, create a new vertex in the middle of that edge.
						// The winding order of the triangles we output are the same as the winding order of the inputs.

						// Indices of the vertices making up this triangle
						const uint32_t iv0 = indices[nTriangle * 3 + 0];
						const uint32_t iv1 = indices[nTriangle * 3 + 1];
						const uint32_t iv2 = indices[nTriangle * 3 + 2];

						// Get the new vertices
						math::float3 v01; // vertex on the midpoint of v0 and v1
						math::float3 v12; // ditto v1 and v2
						math::float3 v20; // ditto v2 and v0
						uint32_t iv01; // index of v01
						uint32_t iv12; // index of v12
						uint32_t iv20; // index of v20

						// Function that, when given the index of two vertices, creates a new vertex at the midpoint of those vertices.
						auto divideEdge = [&](uint32_t i0, uint32_t i1, math::float3& outVertex, uint32_t& outIndex)
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

					indices = std::move(newIndices);
				}

				// Now that we've completed subdivision, fill in the final vertex collection
				vertices.reserve(vertexPositions.size());
				for (auto it = vertexPositions.begin(); it != vertexPositions.end(); ++it)
				{
					const math::float3& vertexValue = *it;

					math::float3 normal;
					vertexValue.Normalize(normal);

					const math::float3 pos = normal * radius;

					math::float3 normalFloat3 = normal;

					// calculate texture coordinates for this vertex
					const float longitude = atan2(normalFloat3.x, -normalFloat3.z);
					const float latitude = acos(normalFloat3.y);

					const float u = longitude / math::PI2 + 0.5f;
					const float v = latitude / math::PI;

					const math::float2 texcoord(1.0f - u, v);
					vertices.emplace_back(pos, texcoord, normal);
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
				const size_t preFixupVertexCount = vertices.size();
				for (size_t i = 0; i < preFixupVertexCount; ++i)
				{
					// This vertex is on the prime meridian if position.x and texcoord.u are both zero (allowing for small epsilon).
					const bool isOnPrimeMeridian = math::IsZero(vertices[i].pos.x) && math::IsZero(vertices[i].uv.x);
					if (isOnPrimeMeridian == true)
					{
						const size_t newIndex = vertices.size(); // the index of this vertex that we're about to add

						// copy this vertex, correct the texture coordinate, and add the vertex
						VertexPosTexNor v = vertices[i];
						v.uv.x = 1.0f;
						vertices.push_back(v);

						// Now find all the triangles which contain this vertex and update them if necessary
						const size_t nIndexCount = indices.size();
						for (size_t j = 0; j < nIndexCount; j += 3)
						{
							uint32_t* triIndex0 = &indices[j + 0];
							uint32_t* triIndex1 = &indices[j + 1];
							uint32_t* triIndex2 = &indices[j + 2];

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

							const VertexPosTexNor& v0 = vertices[*triIndex0];
							const VertexPosTexNor& v1 = vertices[*triIndex1];
							const VertexPosTexNor& v2 = vertices[*triIndex2];

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
					auto poleVertex = vertices[poleIndex];
					bool overwrittenPoleVertex = false; // overwriting the original pole vertex saves us one vertex

					const size_t nIndexCount = indices.size();
					for (size_t i = 0; i < nIndexCount; i += 3)
					{
						// These pointers point to the three indices which make up this triangle. pPoleIndex is the pointer to the
						// entry in the index array which represents the pole index, and the other two pointers point to the other
						// two indices making up this triangle.
						uint32_t* pPoleIndex = nullptr;
						uint32_t* pOtherIndex0 = nullptr;
						uint32_t* pOtherIndex1 = nullptr;
						if (indices[i + 0] == poleIndex)
						{
							pPoleIndex = &indices[i + 0];
							pOtherIndex0 = &indices[i + 1];
							pOtherIndex1 = &indices[i + 2];
						}
						else if (indices[i + 1] == poleIndex)
						{
							pPoleIndex = &indices[i + 1];
							pOtherIndex0 = &indices[i + 2];
							pOtherIndex1 = &indices[i + 0];
						}
						else if (indices[i + 2] == poleIndex)
						{
							pPoleIndex = &indices[i + 2];
							pOtherIndex0 = &indices[i + 0];
							pOtherIndex1 = &indices[i + 1];
						}
						else
						{
							continue;
						}

						const auto& otherVertex0 = vertices[*pOtherIndex0];
						const auto& otherVertex1 = vertices[*pOtherIndex1];

						// Calculate the texcoords for the new pole vertex, add it to the vertices and update the index
						VertexPosTexNor newPoleVertex = poleVertex;
						newPoleVertex.uv.x = (otherVertex0.uv.x + otherVertex1.uv.x) * 0.5f;;
						newPoleVertex.uv.y = poleVertex.uv.y;

						if (overwrittenPoleVertex == false)
						{
							vertices[poleIndex] = newPoleVertex;
							overwrittenPoleVertex = true;
						}
						else
						{
							*pPoleIndex = static_cast<uint32_t>(vertices.size());
							vertices.push_back(newPoleVertex);
						}
					}
				};

				fixPole(northPoleIndex);
				fixPole(southPoleIndex);

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			bool CreateCylinder(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float height, float diameter, uint32_t tessellation, bool rhcoords)
			{
				if (tessellation < 3)
					return false;

				auto GetCircleVector = [](uint32_t i, uint32_t tessellation)
				{
					const float angle = i * math::PI2 / tessellation;

					math::float3 dxz;
					math::SinCos(&dxz.x, &dxz.z, angle);

					return dxz;
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

						const uint32_t vbase = static_cast<uint32_t>(vertices.size());
						indices.emplace_back(vbase);
						indices.emplace_back(vbase + i1);
						indices.emplace_back(vbase + i2);
					}

					// Which end of the cylinder is this?
					math::float3 normal(0.f, 1.f, 0.f);
					math::float3 textureScale(-0.5f, -0.5f, -0.5f);

					if (!isTop)
					{
						normal = -normal;
						textureScale *= math::float3(-1.0f, 1.0f, 1.0f);
					}

					// Create cap vertices.
					for (uint32_t i = 0; i < tessellation; ++i)
					{
						const math::float3 circleVector = GetCircleVector(i, tessellation);

						const math::float3 position = (circleVector * radius) + (normal * height);

						const math::float2 textureCoordinate = XMVectorMultiplyAdd(XMVectorSwizzle<0, 2, 3, 3>(circleVector), textureScale, g_XMOneHalf);

						vertices.emplace_back(position, textureCoordinate, normal);
					}
				};

				vertices.clear();
				indices.clear();

				height *= 0.5f;

				const math::float3 topOffset = g_XMIdentityR1 * height;

				const float radius = diameter * 0.5f;
				const uint32_t stride = tessellation + 1;

				vertices.reserve((tessellation + 1) * 2);
				indices.reserve((tessellation + 1) * 6);

				// Create a ring of triangles around the outside of the cylinder.
				for (uint32_t i = 0; i <= tessellation; ++i)
				{
					const math::float3 normal = GetCircleVector(i, tessellation);
					const math::float3 sideOffset = normal * radius;

					const float u = static_cast<float>(i) / tessellation;

					const math::float2 textureCoordinate(u, u);

					vertices.emplace_back(sideOffset + topOffset, textureCoordinate, normal);
					vertices.emplace_back(sideOffset - topOffset, textureCoordinate + g_XMIdentityR1, normal);

					indices.emplace_back(i * 2);
					indices.emplace_back((i * 2 + 2) % (stride * 2));
					indices.emplace_back(i * 2 + 1);

					indices.emplace_back(i * 2 + 1);
					indices.emplace_back((i * 2 + 2) % (stride * 2));
					indices.emplace_back((i * 2 + 3) % (stride * 2));
				}

				// Create flat triangle fan caps to seal the top and bottom.
				CreateCylinderCap(vertices, indices, tessellation, height, radius, true);
				CreateCylinderCap(vertices, indices, tessellation, height, radius, false);

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			bool CreateCone(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float diameter, float height, uint32_t tessellation, bool rhcoords)
			{
				if (tessellation < 3)
					return false;

				auto GetCircleVector = [](uint32_t i, uint32_t tessellation)
				{
					float angle = i * math::PI2 / tessellation;

					math::float3 dxz;
					math::SinCos(&dxz.x, &dxz.z, angle);

					return dxz;
				};

				auto GetCircleTangent = [](uint32_t i, uint32_t tessellation)
				{
					const float angle = (i * math::PI2 / tessellation) + XM_PIDIV2;

					math::float3 dxz;
					math::SinCos(&dxz.x, &dxz.z, angle);

					return dxz;
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

						const uint32_t vbase = static_cast<uint32_t>(vertices.size());
						indices.emplace_back(vbase);
						indices.emplace_back(vbase + i1);
						indices.emplace_back(vbase + i2);
					}

					// Which end of the cylinder is this?
					math::float3 normal = g_XMIdentityR1;
					math::float3 textureScale = g_XMNegativeOneHalf;

					if (!isTop)
					{
						normal = -normal;
						math::float3 vNegateX = g_XMNegateX;
						textureScale *= vNegateX;
					}

					// Create cap vertices.
					for (uint32_t i = 0; i < tessellation; ++i)
					{
						const math::float3 circleVector = GetCircleVector(i, tessellation);

						const math::float3 position = (circleVector * radius) + (normal * height);

						const math::float2 textureCoordinate = XMVectorMultiplyAdd(XMVectorSwizzle<0, 2, 3, 3>(circleVector), textureScale, g_XMOneHalf);

						vertices.emplace_back(position, textureCoordinate, normal);
					}
				};

				vertices.clear();
				indices.clear();

				height /= 2;

				const math::float3 topOffset = g_XMIdentityR1 * height;

				const float radius = diameter * 0.5f;;
				const uint32_t stride = tessellation + 1;

				vertices.reserve((tessellation + 1) * 2);
				indices.reserve((tessellation + 1) * 3);

				// Create a ring of triangles around the outside of the cone.
				for (uint32_t i = 0; i <= tessellation; ++i)
				{
					const math::float3 circlevec = GetCircleVector(i, tessellation);
					const math::float3 sideOffset = circlevec * radius;

					const float u = static_cast<float>(i) / tessellation;

					const math::float2 textureCoordinate(u, u);

					const math::float3 pt = sideOffset - topOffset;

					math::float3 normal = GetCircleTangent(i, tessellation).Cross(topOffset - pt);
					normal.Normalize();

					// Duplicate the top vertex for distinct normals
					vertices.emplace_back(topOffset, math::float2::Zero, normal);
					vertices.emplace_back(pt, textureCoordinate + g_XMIdentityR1, normal);

					indices.emplace_back(i * 2);
					indices.emplace_back((i * 2 + 3) % (stride * 2));
					indices.emplace_back((i * 2 + 1) % (stride * 2));
				}

				// Create flat triangle fan caps to seal the bottom.
				CreateCylinderCap(vertices, indices, tessellation, height, radius, false);

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			bool CreateTorus(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float diameter, float thickness, uint32_t tessellation, bool rhcoords)
			{
				if (tessellation < 3)
					return false;

				vertices.clear();
				indices.clear();

				const uint32_t stride = tessellation + 1;

				vertices.reserve(stride * stride);
				indices.reserve(stride * stride * 6);

				// First we loop around the main ring of the torus.
				for (uint32_t i = 0; i <= tessellation; ++i)
				{
					const float u = static_cast<float>(i) / tessellation;

					const float outerAngle = i * math::PI2 / tessellation - XM_PIDIV2;

					// Create a transform matrix that will align geometry to
					// slice perpendicularly though the current ring position.
					const math::Matrix transform = math::Matrix::CreateTranslation(diameter * 0.5f, 0.f, 0.f) * math::Matrix::CreateRotationY(outerAngle);

					// Now we loop along the other axis, around the side of the tube.
					for (uint32_t j = 0; j <= tessellation; ++j)
					{
						const float v = 1.f - static_cast<float>(j) / tessellation;

						const float innerAngle = j * math::PI2 / tessellation + XM_PI;

						math::float3 normal;
						math::SinCos(&normal.y, &normal.x, innerAngle);

						// Create a vertex.
						math::float3 position = normal * thickness * 0.5f;;
						math::float2 textureCoordinate(u, v);

						position = math::float3::Transform(position, transform);
						normal = math::float3::Transform(normal, transform);

						vertices.emplace_back(position, textureCoordinate, normal);

						// And create indices for two triangles.
						const uint32_t nextI = (i + 1) % stride;
						const uint32_t nextJ = (j + 1) % stride;

						indices.emplace_back(i * stride + j);
						indices.emplace_back(i * stride + nextJ);
						indices.emplace_back(nextI * stride + j);

						indices.emplace_back(i * stride + nextJ);
						indices.emplace_back(nextI * stride + nextJ);
						indices.emplace_back(nextI * stride + j);
					}
				}

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			bool CreateTetrahedron(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size, bool rhcoords)
			{
				vertices.clear();
				indices.clear();

				static const math::float3 verts[4] =
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

				const size_t faceCount = _countof(faces);

				vertices.reserve(faceCount);
				indices.reserve(faceCount);

				for (size_t i = 0; i < faceCount; i += 3)
				{
					const uint32_t& v0 = faces[i];
					const uint32_t& v1 = faces[i + 1];
					const uint32_t& v2 = faces[i + 2];

					math::float3 normal = (verts[v1] - verts[v0]).Cross(verts[v2] - verts[v0]);
					normal.Normalize();

					const uint32_t base = static_cast<uint32_t>(vertices.size());
					indices.emplace_back(base);
					indices.emplace_back(base + 1);
					indices.emplace_back(base + 2);

					// Duplicate vertices to use face normals
					math::float3 position = verts[v0] * size;
					vertices.emplace_back(position, math::float2::Zero, normal);

					position = verts[v1] * size;
					vertices.emplace_back(position, math::float2(1.f, 0.f), normal);

					position = verts[v2] * size;
					vertices.emplace_back(position, math::float2(0.f, 1.f), normal);
				}

				assert(vertices.size() == faceCount);
				assert(indices.size() == faceCount);

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			bool CreateOctahedron(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size, bool rhcoords)
			{
				vertices.clear();
				indices.clear();

				static const math::float3 verts[6] =
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

				const size_t faceCount = _countof(faces);
				for (size_t i = 0; i < faceCount; i += 3)
				{
					const uint32_t& v0 = faces[i];
					const uint32_t& v1 = faces[i + 1];
					const uint32_t& v2 = faces[i + 2];

					math::float3 normal = (verts[v1] - verts[v0]).Cross(verts[v2] - verts[v0]);
					normal.Normalize();

					const uint32_t base = static_cast<uint32_t>(vertices.size());
					indices.emplace_back(base);
					indices.emplace_back(base + 1);
					indices.emplace_back(base + 2);

					// Duplicate vertices to use face normals
					math::float3 position = verts[v0] * size;
					vertices.emplace_back(position, math::float2::Zero, normal);

					position = verts[v1] * size;
					vertices.emplace_back(position, math::float2(1.f, 0.f), normal);

					position = verts[v2] * size;
					vertices.emplace_back(position, math::float2(0.f, 1.f), normal);
				}

				assert(vertices.size() == faceCount);
				assert(indices.size() == faceCount);

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			bool CreateDodecahedron(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size, bool rhcoords)
			{
				vertices.clear();
				indices.clear();

				static const float a = 1.f / SQRT3;
				static const float b = 0.356822089773089931942f; // sqrt( ( 3 - sqrt(5) ) / 6 )
				static const float c = 0.934172358962715696451f; // sqrt( ( 3 + sqrt(5) ) / 6 );

				static const math::float3 verts[20] =
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

				static const math::float2 textureCoordinates[5] =
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

				const size_t faceCount = _countof(faces);

				vertices.reserve(faceCount);
				indices.reserve(12 * 3 * 3);

				size_t t = 0;
				for (size_t j = 0; j < faceCount; j += 5, ++t)
				{
					const uint32_t& v0 = faces[j];
					const uint32_t& v1 = faces[j + 1];
					const uint32_t& v2 = faces[j + 2];
					const uint32_t& v3 = faces[j + 3];
					const uint32_t& v4 = faces[j + 4];

					math::float3 normal = (verts[v1] - verts[v0]).Cross(verts[v2] - verts[v0]);
					normal.Normalize();

					const uint32_t base = static_cast<uint32_t>(vertices.size());

					indices.emplace_back(base);
					indices.emplace_back(base + 2);
					indices.emplace_back(base + 1);

					indices.emplace_back(base);
					indices.emplace_back(base + 3);
					indices.emplace_back(base + 2);

					indices.emplace_back(base);
					indices.emplace_back(base + 4);
					indices.emplace_back(base + 3);

					// Duplicate vertices to use face normals
					math::float3 position = verts[v0] * size;
					vertices.emplace_back(position, textureCoordinates[textureIndex[t][0]], normal);

					position = verts[v1] * size;
					vertices.emplace_back(position, textureCoordinates[textureIndex[t][1]], normal);

					position = verts[v2] * size;
					vertices.emplace_back(position, textureCoordinates[textureIndex[t][2]], normal);

					position = verts[v3] * size;
					vertices.emplace_back(position, textureCoordinates[textureIndex[t][3]], normal);

					position = verts[v4] * size;
					vertices.emplace_back(position, textureCoordinates[textureIndex[t][4]], normal);
				}

				assert(vertices.size() == 12 * 5);
				assert(indices.size() == 12 * 3 * 3);

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			bool CreateIcosahedron(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size, bool rhcoords)
			{
				vertices.clear();
				indices.clear();

				static const float  t = 1.618033988749894848205f; // (1 + sqrt(5)) / 2
				static const float t2 = 1.519544995837552493271f; // sqrt( 1 + sqr( (1 + sqrt(5)) / 2 ) )

				static const math::float3 verts[12] =
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
				
				const size_t faceCount = _countof(faces);

				assert(vertices.size() == faceCount);
				assert(indices.size() == faceCount);

				for (size_t i = 0; i < faceCount; i += 3)
				{
					const uint32_t& v0 = faces[i];
					const uint32_t& v1 = faces[i + 1];
					const uint32_t& v2 = faces[i + 2];

					math::float3 normal = (verts[v1] - verts[v0]).Cross(verts[v2] - verts[v0]);
					normal.Normalize();

					const uint32_t base = static_cast<uint32_t>(vertices.size());
					indices.emplace_back(base);
					indices.emplace_back(base + 2);
					indices.emplace_back(base + 1);

					// Duplicate vertices to use face normals
					math::float3 position = verts[v0] * size;
					vertices.emplace_back(position, math::float2::Zero, normal);

					position = verts[v1] * size;
					vertices.emplace_back(position, math::float2(1.f, 0.f), normal);

					position = verts[v2] * size;
					vertices.emplace_back(position, math::float2(0.f, 1.f), normal);
				}

				assert(vertices.size() == 20 * 3);
				assert(indices.size() == 20 * 3);

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			// Include the teapot control point data.
			namespace
			{
		#include "ExternLib/DirectXTK/Src/TeapotData.inc"
		#include "ExternLib/DirectXTK/Src/Bezier.h"
			}

			bool CreateTeapot(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size, uint32_t tessellation, bool rhcoords)
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
					uint32_t vbase = static_cast<uint32_t>(vertices.size());
					Bezier::CreatePatchIndices(tessellation, isMirrored, [&](size_t index)
					{
						indices.push_back(vbase + static_cast<uint32_t>(index));
					});

					// Create the vertex data.
					Bezier::CreatePatchVertices(controlPoints, tessellation, isMirrored, [&](FXMVECTOR position, FXMVECTOR normal, FXMVECTOR textureCoordinate)
					{
						vertices.push_back(VertexPosTexNor(position, normal, textureCoordinate));
					});
				};

				if (tessellation < 1)
					return false;

				vertices.clear();
				indices.clear();

				XMVECTOR scaleVector = XMVectorReplicate(size);

				XMVECTOR scaleNegateX = scaleVector * g_XMNegateX;
				XMVECTOR scaleNegateZ = scaleVector * g_XMNegateZ;
				XMVECTOR scaleNegateXZ = scaleVector * g_XMNegateX * g_XMNegateZ;

				for (int i = 0; i < sizeof(TeapotPatches) / sizeof(TeapotPatches[0]); i++)
				{
					TeapotPatch const& patch = TeapotPatches[i];

					// Because the teapot is symmetrical from left to right, we only store
					// data for one side, then tessellate each patch twice, mirroring in X.
					TessellatePatch(vertices, indices, patch, tessellation, scaleVector, false);
					TessellatePatch(vertices, indices, patch, tessellation, scaleNegateX, true);

					if (patch.mirrorZ)
					{
						// Some parts of the teapot (the body, lid, and rim, but not the
						// handle or spout) are also symmetrical from front to back, so
						// we tessellate them four times, mirroring in Z as well as X.
						TessellatePatch(vertices, indices, patch, tessellation, scaleNegateZ, true);
						TessellatePatch(vertices, indices, patch, tessellation, scaleNegateXZ, false);
					}
				}

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			bool CreateHexagon(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float radius, bool rhcoords)
			{
				vertices.clear();
				indices.clear();

				vertices.resize(7);
				indices.resize(18);

				const float fHalf = radius * 0.5f;

				vertices[0].pos = math::float3(0.f, 0.f, 0.f);
				vertices[0].normal = math::float3(0.f, 1.f, 0.f);
				vertices[0].uv = math::float2(0.5f, 0.5f);

				vertices[1].pos = math::float3(fHalf, 0.f, radius);
				vertices[1].normal = math::float3(0.f, 1.f, 0.f);
				vertices[1].uv = math::float2(0.75f, 0.f);

				vertices[2].pos = math::float3(radius, 0.f, 0.f);
				vertices[2].normal = math::float3(0.f, 1.f, 0.f);
				vertices[2].uv = math::float2(1.f, 0.5f);

				vertices[3].pos = math::float3(fHalf, 0.f, -radius);
				vertices[3].normal = math::float3(0.f, 1.f, 0.f);
				vertices[3].uv = math::float2(0.75f, 1.f);

				vertices[4].pos = math::float3(-fHalf, 0.f, -radius);
				vertices[4].normal = math::float3(0.f, 1.f, 0.f);
				vertices[4].uv = math::float2(0.25f, 1.f);

				vertices[5].pos = math::float3(-radius, 0.f, 0.f);
				vertices[5].normal = math::float3(0.f, 1.f, 0.f);
				vertices[5].uv = math::float2(0.f, 0.5f);

				vertices[6].pos = math::float3(-fHalf, 0.f, radius);
				vertices[6].normal = math::float3(0.f, 1.f, 0.f);
				vertices[6].uv = math::float2(0.25f, 0.f);

				indices[0] = 2;		indices[1] = 1;		indices[2] = 0;
				indices[3] = 3;		indices[4] = 2;		indices[5] = 0;
				indices[6] = 4;		indices[7] = 3;		indices[8] = 0;
				indices[9] = 5;		indices[10] = 4;		indices[11] = 0;
				indices[12] = 6;		indices[13] = 5;		indices[14] = 0;
				indices[15] = 1;		indices[16] = 6;		indices[17] = 0;

				SortBuffer(vertices, indices, rhcoords, false);

				return true;
			}

			bool CreateCapsule(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float fRadius, float fHeight, int nSubdivisionHeight, int nSegments)
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

				vertices.clear();
				indices.clear();

				auto CalculateRing = [&](int segments, float r, float y, float dy)
				{
					float segIncr = 1.f / static_cast<float>(segments - 1);

					for (int i = 0; i < segments; ++i)
					{
						float x = -cos((math::PI * 2.f) * i * segIncr) * r;
						float z = sin((math::PI * 2.f) * i * segIncr) * r;

						VertexPosTexNor vertex;
						vertex.pos = math::float3(fRadius * x, fRadius * y + fHeight * dy, fRadius * z);
						vertex.normal = math::float3(x, y, z);
						vertex.uv = math::float2(i * segIncr, 0.5f - ((fRadius * y + fHeight * dy) / (2.f * fRadius + fHeight)));

						vertices.emplace_back(vertex);
					}
				};

				const int nRingsBody = nSubdivisionHeight + 1;
				const int nRingsTotal = nSubdivisionHeight + nRingsBody;

				const float fBodyIncr = 1.f / static_cast<float>(nRingsBody - 1);
				float fRingIncr = 1.f / static_cast<float>(nSubdivisionHeight - 1);
				for (int i = 0; i < nSubdivisionHeight / 2; ++i)
				{
					CalculateRing(nSegments, sin(math::PI * i * fRingIncr), sin(math::PI * (i * fRingIncr - 0.5f)), -0.5f);
				}

				for (int i = 0; i < 2; ++i)
				{
					CalculateRing(nSegments, 1.f, 0.f, static_cast<float>(i) * fBodyIncr - 0.5f);
				}

				for (int i = nSubdivisionHeight / 2; i < nSubdivisionHeight; ++i)
				{
					CalculateRing(nSegments, sin(math::PI * i * fRingIncr), sin(math::PI * (i * fRingIncr - 0.5f)), 0.5f);
				}

				for (int i = 0; i < nRingsTotal; ++i)
				{
					for (int j = 0; j < nSegments - 1; ++j)
					{
						indices.push_back(i * nSegments + (j + 1));
						indices.push_back(i * nSegments + (j + 0));
						indices.push_back((i + 1) * nSegments + (j + 1));
						
						indices.push_back((i + 1) * nSegments + (j + 0));
						indices.push_back((i + 1) * nSegments + (j + 1));
						indices.push_back(i * nSegments + j);
					}
				}

				SortBuffer(vertices, indices, false, false);

				return true;
			}

			bool CreateGrid(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float fGridSizeX, float fGridSizeZ, uint32_t nBlockCountWidth, uint32_t nBlockCountLength)
			{
				vertices.clear();
				indices.clear();

				vertices.reserve((nBlockCountWidth + 1) * (nBlockCountLength + 1));
				indices.reserve(3 * 2 * nBlockCountWidth * nBlockCountLength);

				const float fStartX = fGridSizeX * -0.5f;
				const float fStartZ = fGridSizeZ * -0.5f;
				const float fEachSizeX = fGridSizeX / nBlockCountWidth;
				const float fEachSizeZ = fGridSizeZ / nBlockCountLength;

				for (uint32_t i = 0; i <= nBlockCountLength; ++i)
				{
					for (uint32_t j = 0; j <= nBlockCountWidth; ++j)
					{
						VertexPosTexNor vertex;
						vertex.pos = math::float3(fStartX + j * fEachSizeX, 0.f, fStartZ + i * fEachSizeZ);
						vertex.normal = math::float3(0.f, 1.f, 0.f);
						vertex.uv = math::float2((float)j / nBlockCountWidth, (float)i / nBlockCountLength);

						vertices.emplace_back(vertex);
					}
				}

				const uint32_t dwWidth = nBlockCountWidth + 1;
				for (uint32_t i = 0; i < nBlockCountLength; ++i)
				{
					for (uint32_t j = 0; j < nBlockCountWidth; ++j)
					{
						indices.emplace_back((dwWidth * i) + 0 + j);
						indices.emplace_back((dwWidth * i) + dwWidth + j);
						indices.emplace_back((dwWidth * i) + (dwWidth + 1) + j);

						indices.emplace_back((dwWidth * i) + 0 + j);
						indices.emplace_back((dwWidth * i) + (dwWidth + 1) + j);
						indices.emplace_back((dwWidth * i) + 1 + j);
					}
				}

				SortBuffer(vertices, indices, false, false);

				return true;
			}

			bool CreatePlane(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float fEachLengthX, float fEachLengthZ, int nTotalCountX, int nTotalCountZ)
			{
				vertices.clear();
				indices.clear();

				vertices.reserve(4 * nTotalCountX * nTotalCountZ);
				indices.reserve(6 * nTotalCountX * nTotalCountZ);

				int nCnt = 0;
				const float fStartX = nTotalCountX * fEachLengthX * -0.5f;
				const float fStartZ = nTotalCountZ * fEachLengthZ * -0.5f;
				for (int i = 0; i < nTotalCountX; ++i)
				{
					for (int j = 0; j < nTotalCountZ; ++j)
					{
						vertices.emplace_back(math::float3(fStartX + i * fEachLengthX, 0.f, fStartZ + j * fEachLengthZ),
							math::float2(0.f, 1.f), math::float3(0.f, 1.f, 0.f));
						vertices.emplace_back(math::float3(fStartX + i * fEachLengthX, 0.f, fStartZ + j * fEachLengthZ + fEachLengthZ),
							math::float2(0.f, 0.f), math::float3(0.f, 1.f, 0.f));
						vertices.emplace_back(math::float3(fStartX + i * fEachLengthX + fEachLengthX, 0.f, fStartZ + j * fEachLengthZ),
							math::float2(1.f, 1.f), math::float3(0.f, 1.f, 0.f));
						vertices.emplace_back(math::float3(fStartX + i * fEachLengthX + fEachLengthX, 0.f, fStartZ + j * fEachLengthZ + fEachLengthZ),
							math::float2(1.f, 0.f), math::float3(0.f, 1.f, 0.f));
						nCnt += 4;

						indices.emplace_back(nCnt - 1);
						indices.emplace_back(nCnt - 3);
						indices.emplace_back(nCnt - 4);
						indices.emplace_back(nCnt - 2);
						indices.emplace_back(nCnt - 1);
						indices.emplace_back(nCnt - 4);
					}
				}

				SortBuffer(vertices, indices, false, false);

				return true;
			}

			// Mesh Simplification
			// code by https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
			namespace Simplify
			{
				struct vec3f
				{
					float x, y, z;

					vec3f(void) {}

					vec3f(const math::float3& a)
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

					inline vec3f operator = (const math::float3 a)
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
				float calculate_error(int id_v1, int id_v2, vec3f& p_result, math::float2* pOut_f2UV, math::float3* pOut_f3Normal);
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
						LOG_WARNING("Object will not survive such extreme decimation, Triangle : %lld, Target : %d, Reduce Fraction : %f", triangles.size(), target_count, fReduceFraction);
						return false;
					}

					triangles.clear();
					triangles.resize(in_vecIndices.size() / 3);
					vertices.clear();
					vertices.resize(in_vecVertices.size());

					{
						size_t nSize = in_vecVertices.size();
						for (size_t i = 0; i < nSize; ++i)
						{
							vertices[i].p = vec3f(in_vecVertices[i].pos.x, in_vecVertices[i].pos.y, in_vecVertices[i].pos.z);
							vertices[i].vertex.SetVertex(in_vecVertices[i].pos, in_vecVertices[i].uv, in_vecVertices[i].normal);
						}

						uint32_t nIdx = 0;
						nSize = in_vecIndices.size();
						for (size_t i = 0; i < nSize; i += 3)
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
					size_t nSize = triangles.size();
					for (size_t i = 0; i < nSize; ++i)
					{
						triangles[i].deleted = 0;
					}

					// main iteration loop 

					int deleted_triangles = 0;
					std::vector<int> deleted0, deleted1;
					int triangle_count = static_cast<int>(triangles.size());

					for (int iteration = 0; iteration < 100; ++iteration)
					{
						// target number of triangles reached ? Then break
						//LOG_WARNING("iteration %d - triangles %d\n", iteration, triangle_count - deleted_triangles);
						if (triangle_count - deleted_triangles <= target_count)
							break;

						// update mesh once in a while
						if (iteration % 5 == 0)
						{
							update_mesh(iteration);
						}

						// clear dirty flag
						for (size_t i = 0; i < triangles.size(); ++i)
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
						for (size_t i = 0; i < triangles.size(); ++i)
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
									math::float2 uv;
									math::float3 normal;
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
									v0.vertex.pos = math::float3(p.x, p.y, p.z);
									v0.vertex.uv = uv;
									v0.vertex.normal = normal;
									v0.q = v1.q + v0.q;
									int tstart = static_cast<int>(refs.size());

									update_triangles(i0, v0, deleted0, deleted_triangles);
									update_triangles(i0, v1, deleted1, deleted_triangles);

									int tcount = static_cast<int>(refs.size()) - tstart;

									if (tcount <= v0.tcount)
									{
										// save ram
										if (tcount)memory::Copy(&refs[v0.tstart], tcount * sizeof(Ref), &refs[tstart], tcount * sizeof(Ref));
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

					for (size_t i = 0; i < vertices.size(); ++i)
					{
						out_vecVertices.emplace_back(math::float3(vertices[i].p.x, vertices[i].p.y, vertices[i].p.z),
							vertices[i].vertex.uv, vertices[i].vertex.normal);
					}

					for (size_t i = 0; i < triangles.size(); ++i)
					{
						if (triangles[i].deleted == false)
						{
							out_vecVertices[triangles[i].v[0]].normal = math::float3(triangles[i].n.x, triangles[i].n.y, triangles[i].n.z);
							out_vecVertices[triangles[i].v[1]].normal = math::float3(triangles[i].n.x, triangles[i].n.y, triangles[i].n.z);
							out_vecVertices[triangles[i].v[2]].normal = math::float3(triangles[i].n.x, triangles[i].n.y, triangles[i].n.z);

							out_vecIndices.emplace_back(triangles[i].v[0]);
							out_vecIndices.emplace_back(triangles[i].v[1]);
							out_vecIndices.emplace_back(triangles[i].v[2]);
						}
					}

					triangles.shrink_to_fit();
					vertices.shrink_to_fit();

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
						t.err[3] = std::min(t.err[0], std::min(t.err[1], t.err[2]));
						refs.push_back(r);
					}
				}

				// compact triangles, compute edge error and build reference list

				void update_mesh(int iteration)
				{
					if (iteration > 0) // compact triangles
					{
						int dst = 0;
						for (size_t i = 0; i < triangles.size(); ++i)
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
						for (size_t i = 0; i < vertices.size(); ++i)
						{
							vertices[i].q = SymetricMatrix(0.0);
						}

						for (size_t i = 0; i < triangles.size(); ++i)
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

						for (size_t i = 0; i < triangles.size(); ++i)
						{
							// Calc Edge Error
							Triangle& t = triangles[i];
							vec3f p;

							for (int j = 0; j < 3; ++j)
							{
								t.err[j] = calculate_error(t.v[j], t.v[(j + 1) % 3], p, nullptr, nullptr);
							}
							t.err[3] = std::min(t.err[0], std::min(t.err[1], t.err[2]));
						}
					}

					// Init Reference ID list	
					for (size_t i = 0; i < vertices.size(); ++i)
					{
						vertices[i].tstart = 0;
						vertices[i].tcount = 0;
					}

					for (size_t i = 0; i < triangles.size(); ++i)
					{
						Triangle& t = triangles[i];
						for (int j = 0; j < 3; ++j)
						{
							vertices[t.v[j]].tcount++;
						}
					}

					int tstart = 0;
					for (size_t i = 0; i < vertices.size(); ++i)
					{
						Vertex& v = vertices[i];
						v.tstart = tstart;
						tstart += v.tcount;
						v.tcount = 0;
					}

					// Write References
					refs.resize(triangles.size() * 3);
					for (size_t i = 0; i < triangles.size(); ++i)
					{
						Triangle& t = triangles[i];
						for (int j = 0; j < 3; ++j)
						{
							Vertex& v = vertices[t.v[j]];
							refs[v.tstart + v.tcount].tid = static_cast<int>(i);
							refs[v.tstart + v.tcount].tvertex = j;
							v.tcount++;
						}
					}

					// Identify boundary : vertices[].border=0,1 
					if (iteration == 0)
					{
						std::vector<int> vcount, vids;

						for (size_t i = 0; i < vertices.size(); ++i)
						{
							vertices[i].border = 0;
						}

						for (size_t i = 0; i < vertices.size(); ++i)
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
									size_t ofs = 0;
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

							for (size_t j = 0; j < vcount.size(); ++j)
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
					for (size_t i = 0; i < vertices.size(); ++i)
					{
						vertices[i].tcount = 0;
					}

					for (size_t i = 0; i < triangles.size(); ++i)
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

					for (size_t i = 0; i < vertices.size(); ++i)
					{
						if (vertices[i].tcount)
						{
							vertices[i].tstart = dst;
							vertices[dst].p = vertices[i].p;
							vertices[dst].vertex = vertices[i].vertex;
							dst++;
						}
					}

					for (size_t i = 0; i < triangles.size(); ++i)
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

				float calculate_error(int id_v1, int id_v2, vec3f& p_result, math::float2* pOut_f2UV, math::float3* pOut_f3Normal)
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

						error = std::min(error1, std::min(error2, error3));

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

			class DebugModel
			{
			public:
				DebugModel()
				{
					m_pVertexBuffer.fill(nullptr);
					m_pIndexBuffer.fill(nullptr);
				}

				~DebugModel()
				{
					Release();
				}

			public:
				bool Initialize()
				{
					bool isSuccess = false;

					isSuccess = CreateBox();
					assert(isSuccess);

					isSuccess = CreateSphere();
					assert(isSuccess);

					if (isSuccess == false)
					{
						Release();
						return false;
					}

					return true;
				}

				void Release()
				{
					for (auto& pVertexBuffer : m_pVertexBuffer)
					{
						ReleaseResource(&pVertexBuffer);
					}
					m_pVertexBuffer.fill(nullptr);

					for (auto& pIndexBuffer : m_pIndexBuffer)
					{
						ReleaseResource(&pIndexBuffer);
					}
					m_pIndexBuffer.fill(nullptr);
				}

				void Get(DebugModelType emType, IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer)
				{
					*ppVertexBuffer = m_pVertexBuffer[emType];
					*ppIndexBuffer = m_pIndexBuffer[emType];
				}

			private:
				bool CreateBox()
				{
					std::vector<VertexPosTexNor> vertices;
					std::vector<uint32_t> indices;
					
					geometry::CreateCube(vertices, indices, 0.5f, false);

					std::vector<VertexPos> posVertices;
					posVertices.reserve(vertices.size());
					for (auto& vertex : vertices)
					{
						posVertices.emplace_back(vertex.pos);
					}

					m_pVertexBuffer[DebugModelType::eBox] = CreateVertexBuffer(reinterpret_cast<uint8_t*>(posVertices.data()), static_cast<uint32_t>(posVertices.size()), sizeof(VertexPos));
					m_pIndexBuffer[DebugModelType::eBox] = CreateIndexBuffer(reinterpret_cast<uint8_t*>(indices.data()), static_cast<uint32_t>(indices.size()), sizeof(uint32_t));

					return true;
				}

				bool CreateSphere()
				{
					std::vector<VertexPosTexNor> vertices;
					std::vector<uint32_t> indices;

					geometry::CreateSphere(vertices, indices, 1.f, 6);

					std::vector<VertexPos> posVertices;
					posVertices.reserve(vertices.size());
					for (auto& vertex : vertices)
					{
						posVertices.emplace_back(vertex.pos);
					}

					m_pVertexBuffer[DebugModelType::eSphere] = CreateVertexBuffer(reinterpret_cast<uint8_t*>(posVertices.data()), static_cast<uint32_t>(posVertices.size()), sizeof(VertexPos));
					m_pIndexBuffer[DebugModelType::eSphere] = CreateIndexBuffer(reinterpret_cast<uint8_t*>(indices.data()), static_cast<uint32_t>(indices.size()), sizeof(uint32_t));

					return true;
				}

			private:
				std::array<IVertexBuffer*, DebugModelType::eCount> m_pVertexBuffer;
				std::array<IIndexBuffer*, DebugModelType::eCount> m_pIndexBuffer;
			};

			std::unique_ptr<DebugModel> s_pDebugModel;

			bool Initialize()
			{
				if (s_pDebugModel != nullptr)
					return true;

				s_pDebugModel = std::make_unique<DebugModel>();
				if (s_pDebugModel->Initialize() == false)
				{
					Release();
					return false;
				}

				return true;
			}

			void Release()
			{
				if (s_pDebugModel == nullptr)
					return;

				s_pDebugModel.reset();
			}

			void GetDebugModel(DebugModelType emType, IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer)
			{
				s_pDebugModel->Get(emType, ppVertexBuffer, ppIndexBuffer);
			}
		}
	}
}