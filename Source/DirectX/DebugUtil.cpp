#include "stdafx.h"
#include "DebugUtil.h"

#include "D3DInterface.h"
#include "Vertex.h"

namespace EastEngine
{
	namespace Graphics
	{
		namespace Debug
		{
			IVertexBuffer* s_pLineBoxVB = nullptr;
			IIndexBuffer* s_pLineBoxIB = nullptr;

			void Init()
			{
				// LineBox
				{
					uint32_t nVertexCount = 24;
					uint32_t nIndexCount = nVertexCount;

					Math::Color color = Math::Color::DarkRed;

					std::vector<VertexPosCol> vecVertices;
					vecVertices.reserve(nVertexCount);

					std::vector<uint32_t> vecIndices;
					vecIndices.reserve(nIndexCount);

					Math::Vector3 f3MinSize(-1.f, -1.f, -1.f);
					Math::Vector3 f3MaxSize(1.f, 1.f, 1.f);

					uint32_t nIdx = 0;
					// 8 Horizontal lines.
					vecVertices.push_back(VertexPosCol(f3MinSize, color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MinSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MinSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MinSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MinSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MinSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MinSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MinSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MaxSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MaxSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MaxSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MaxSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MaxSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MaxSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MaxSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MaxSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					// 4 Verticle lines.
					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MaxSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MinSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MaxSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MinSize.y, f3MaxSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MaxSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MaxSize.x, f3MinSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MaxSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					vecVertices.push_back(VertexPosCol(Math::Vector3(f3MinSize.x, f3MinSize.y, f3MinSize.z), color));
					vecIndices.push_back(nIdx);
					++nIdx;

					s_pLineBoxVB = IVertexBuffer::Create(VertexPosCol::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_IMMUTABLE);
					s_pLineBoxIB = IIndexBuffer::Create(vecIndices.size(), &vecIndices.front(),D3D11_USAGE_IMMUTABLE);

					if (s_pLineBoxVB == nullptr || s_pLineBoxIB == nullptr)
					{
						Release();
						return;
					}
				}
			}

			void Release()
			{
				SafeDelete(s_pLineBoxVB);
				SafeDelete(s_pLineBoxIB);
			}

			IVertexBuffer* GetLineBoxVertexBuffer() { return s_pLineBoxVB; }
			IIndexBuffer* GetLineBoxIndexBuffer() { return s_pLineBoxIB; }
		}
	}
}