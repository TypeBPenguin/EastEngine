#include "stdafx.h"
#include "Vertex.h"

namespace EastEngine
{
	namespace Graphics
	{
		size_t GetVertexFormatSize(EmVertexFormat::Type emFormat)
		{
			switch (emFormat)
			{
			case EmVertexFormat::eUnknown:
				return 0;
			case EmVertexFormat::ePos:
				return sizeof(VertexPos);
			case EmVertexFormat::ePos4:
				return sizeof(VertexPos4);
			case EmVertexFormat::ePosCol:
				return sizeof(VertexPosCol);
			case EmVertexFormat::ePosTex:
				return sizeof(VertexPosTex);
			case EmVertexFormat::ePosTexCol:
				return sizeof(VertexPosTexCol);
			case EmVertexFormat::ePosTexNor:
				return sizeof(VertexPosTexNor);
			case EmVertexFormat::ePosTexNorCol:
				return sizeof(VertexPosTexNorCol);
			case EmVertexFormat::ePosTexNorTanBin:
				return sizeof(VertexPosTexNorTanBin);
			case EmVertexFormat::ePosTexNorWeiIdx:
				return sizeof(VertexPosTexNorWeiIdx);
			case EmVertexFormat::eUI:
				return sizeof(VertexUI);
			}

			return 0;
		}

		VertexPos::VertexPos()
		{
		}

		VertexPos::VertexPos(const Math::Vector3& f3Pos)
			: pos(f3Pos)
		{
		}

		VertexPos4::VertexPos4()
		{
		}

		VertexPos4::VertexPos4(const Math::Vector4& f4Pos)
			: pos(f4Pos)
		{
		}

		VertexPosCol::VertexPosCol()
		{
		}

		VertexPosCol::VertexPosCol(const Math::Vector3& f3Pos, const Math::Color& color)
			: pos(f3Pos)
			, color(color)
		{
		}

		VertexPosTex::VertexPosTex()
		{
		}

		VertexPosTex::VertexPosTex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV)
			: pos(f3Pos)
			, uv(f2UV)
		{
		}

		VertexPosTexCol::VertexPosTexCol()
		{
		}

		VertexPosTexCol::VertexPosTexCol(const Math::Vector3& f3Pos, const Math::Vector2& f2UV)
			: pos(f3Pos)
			, uv(f2UV)
		{
		}

		VertexPosTexNor::VertexPosTexNor()
		{
		}

		VertexPosTexNor::VertexPosTexNor(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal)
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
		{
		}

		VertexPosTexNorCol::VertexPosTexNorCol()
		{
		}

		VertexPosTexNorCol::VertexPosTexNorCol(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Color& color)
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
			, color(color)
		{
		}

		VertexPosTexNorTanBin::VertexPosTexNorTanBin()
		{
		}

		VertexPosTexNorTanBin::VertexPosTexNorTanBin(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal)
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
		{
		}

		VertexPosTexNorTanBin::VertexPosTexNorTanBin(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Vector3& f3Tangent, const Math::Vector3& f3Binormal)
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
			, tangent(f3Tangent)
			, binormal(f3Binormal)
		{
		}

		VertexPosTexNorWeiIdx::VertexPosTexNorWeiIdx()
			: boneIndices(0u)
		{
		}

		VertexPosTexNorWeiIdx::VertexPosTexNorWeiIdx(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Vector3& f3Weight, const Math::UByte4& indices)
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
			, boneWeight(f3Weight)
			, boneIndices(boneIndices)
		{
		}

		VertexUI::VertexUI()
			: vertexIdx(0)
		{
		}

		VertexUI::VertexUI(const Math::Vector2& f3Pos, int nVertexIdx)
			: pos(f3Pos)
			, vertexIdx(nVertexIdx)
		{
		}

		VertexClipSpace::VertexClipSpace()
		{
		}

		VertexClipSpace::VertexClipSpace(const Math::Vector4& f4Pos)
			: pos(f4Pos)
		{
		}

		VertexClipSpace::VertexClipSpace(const Math::Vector3& f3Pos)
			: pos(f3Pos.x, f3Pos.y, 0.f, f3Pos.z)
		{
		}
	}
}