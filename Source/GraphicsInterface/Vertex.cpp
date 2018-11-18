#include "stdafx.h"
#include "Vertex.h"

namespace eastengine
{
	namespace graphics
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
			}

			return 0;
		}

		VertexPos::VertexPos()
		{
		}

		VertexPos::VertexPos(const math::float3& f3Pos)
			: pos(f3Pos)
		{
		}

		VertexPos4::VertexPos4()
		{
		}

		VertexPos4::VertexPos4(const math::float4& f4Pos)
			: pos(f4Pos)
		{
		}

		VertexPosCol::VertexPosCol()
		{
		}

		VertexPosCol::VertexPosCol(const math::float3& f3Pos, const math::Color& color)
			: pos(f3Pos)
			, color(color)
		{
		}

		VertexPosTex::VertexPosTex()
		{
		}

		VertexPosTex::VertexPosTex(const math::float3& f3Pos, const math::float2& f2UV)
			: pos(f3Pos)
			, uv(f2UV)
		{
		}

		VertexPosTexCol::VertexPosTexCol()
		{
		}

		VertexPosTexCol::VertexPosTexCol(const math::float3& f3Pos, const math::float2& f2UV)
			: pos(f3Pos)
			, uv(f2UV)
		{
		}

		VertexPosTexNor::VertexPosTexNor()
		{
		}

		VertexPosTexNor::VertexPosTexNor(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal)
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
		{
		}

		VertexPosTexNorCol::VertexPosTexNorCol()
		{
		}

		VertexPosTexNorCol::VertexPosTexNorCol(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal, const math::Color& color)
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
			, color(color)
		{
		}

		VertexPosTexNorTanBin::VertexPosTexNorTanBin()
		{
		}

		VertexPosTexNorTanBin::VertexPosTexNorTanBin(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal)
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
		{
		}

		VertexPosTexNorTanBin::VertexPosTexNorTanBin(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal, const math::float3& f3Tangent, const math::float3& f3Binormal)
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
			, tangent(f3Tangent)
			, binormal(f3Binormal)
		{
		}

		VertexPosTexNorWeiIdx::VertexPosTexNorWeiIdx()
		{
		}

		VertexPosTexNorWeiIdx::VertexPosTexNorWeiIdx(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal, const math::float3& f3Weight, const uint16_t indices[4])
			: pos(f3Pos)
			, uv(f2UV)
			, normal(f3Normal)
			, boneWeight(f3Weight)
		{
			memory::Copy(boneIndices, indices);
		}

		VertexClipSpace::VertexClipSpace()
		{
		}

		VertexClipSpace::VertexClipSpace(const math::float4& f4Pos)
			: pos(f4Pos)
		{
		}

		VertexClipSpace::VertexClipSpace(const math::float3& f3Pos)
			: pos(f3Pos.x, f3Pos.y, 0.f, f3Pos.z)
		{
		}
	}
}