#pragma once

namespace eastengine
{
	namespace graphics
	{
		namespace EmVertexFormat
		{
			enum Type
			{
				eUnknown = 0,
				ePos,					// Position(float3)
				ePos4,					// Position(float4)
				ePosCol,				// Position(float3), Color(float4)
				ePosTex,				// Position(float3), UV(float2)
				ePosTexCol,				// Position(float3), UV(float2), Color(float4)
				ePosTexNor,				// Position(float3), UV(float2), Normal(float3)
				ePosTexNorCol,			// Position(float3), UV(float2), Normal(float3), Color(float4)
				ePosTexNorTanBin,		// Position(float3), UV(float2), Normal(float3), Tangent(float3), Binormal(float3)
				ePosTexNorWeiIdx,		// Position(float3), UV(float2), Normal(float3), BoneWeight(float3), BoneIndices(uint)

				eCount,
			};
		};
		size_t GetVertexFormatSize(EmVertexFormat::Type emFormat);

		struct VertexPosTexNor;

		struct VertexPos
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePos; }
			static constexpr size_t Size() { return sizeof(VertexPos); }

			math::float3 pos;

			VertexPos();
			VertexPos(const math::float3& f3Pos);

			void SetVertex(const math::float3& f3Pos) { pos = f3Pos; }
		};

		struct VertexPos4
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePos4; }
			static constexpr size_t Size() { return sizeof(VertexPos4); }

			math::float4 pos;

			VertexPos4();
			VertexPos4(const math::float4& f4Pos);

			void SetVertex(const math::float4& f4Pos) { pos = f4Pos; }
		};

		struct VertexPosCol
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosCol; }
			static constexpr size_t Size() { return sizeof(VertexPosCol); }

			math::float3 pos;
			math::Color color;

			VertexPosCol();
			VertexPosCol(const math::float3& f3Pos, const math::Color& color);

			void SetVertex(const math::float3& f3Pos) { pos = f3Pos; }
			void SetColor(const math::Color& setColor) { color = setColor; }
		};

		struct VertexPosTex
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTex; }
			static constexpr size_t Size() { return sizeof(VertexPosTex); }

			math::float3 pos;
			math::float2 uv;

			VertexPosTex();
			VertexPosTex(const math::float3& f3Pos, const math::float2& f2UV);

			void SetVertex(const math::float3& f3Pos, const math::float2& f2UV) { pos = f3Pos;	uv = f2UV; }
		};

		struct VertexPosTexCol
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexCol; }
			static constexpr size_t Size() { return sizeof(VertexPosTexCol); }

			math::float3 pos;
			math::float2 uv;
			math::Color color;

			VertexPosTexCol();
			VertexPosTexCol(const math::float3& f3Pos, const math::float2& f2UV);

			void SetVertex(const math::float3& f3Pos, const math::float2& f2UV) { pos = f3Pos;	uv = f2UV; }
		};

		struct VertexPosTexNor
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNor; }
			static constexpr size_t Size() { return sizeof(VertexPosTexNor); }

			math::float3 pos;
			math::float2 uv;
			math::float3 normal;

			VertexPosTexNor();
			VertexPosTexNor(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal);

			void SetVertex(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal) { pos = f3Pos; uv = f2UV; normal = f3Normal; }
		};

		struct VertexPosTexNorCol
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNorCol; }
			static constexpr size_t Size() { return sizeof(VertexPosTexNorCol); }

			math::float3 pos;
			math::float2 uv;
			math::float3 normal;
			math::Color color;

			VertexPosTexNorCol();
			VertexPosTexNorCol(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal, const math::Color& color);

			void SetVertex(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal) { pos = f3Pos; uv = f2UV; normal = f3Normal; }
			void SetColor(const math::Color& setColor) { color = setColor; }
		};

		struct VertexPosTexNorTanBin
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNorTanBin; }
			static constexpr size_t Size() noexcept { return sizeof(VertexPosTexNorTanBin); }

			math::float3 pos;
			math::float2 uv;
			math::float3 normal;
			math::float3 tangent;
			math::float3 binormal;

			VertexPosTexNorTanBin();
			VertexPosTexNorTanBin(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal);
			VertexPosTexNorTanBin(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal, const math::float3& f3Tangent, const math::float3& f3Binormal);

			void SetVertex(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal) { pos = f3Pos; uv = f2UV; normal = f3Normal; }
			void SetVertex(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal, const math::float3& f3Tangent, const math::float3& f3Binormal) { pos = f3Pos; uv = f2UV; normal = f3Normal; tangent = f3Tangent; binormal = f3Binormal; }
		};

		struct VertexPosTexNorWeiIdx
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNorWeiIdx; }
			static constexpr size_t Size() { return sizeof(VertexPosTexNorWeiIdx); }

			math::float3 pos;
			math::float2 uv;
			math::float3 normal;
			math::float3 boneWeight;
			uint16_t boneIndices[4]{};

			VertexPosTexNorWeiIdx();
			VertexPosTexNorWeiIdx(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal, const math::float3& f3Weight, const uint16_t boneIndices[4]);

			void operator = (const VertexPosTexNor& vertex)
			{
				pos = vertex.pos;
				uv = vertex.uv;
				normal = vertex.normal;
			}

			void SetVertex(const math::float3& f3Pos, const math::float2& f2UV, const math::float3& f3Normal, const math::float3& f3Weight, const uint16_t _boneIndices[4]) { pos = f3Pos; uv = f2UV; normal = f3Normal; boneWeight = f3Weight; Memory::Copy(boneIndices, _boneIndices); }
		};

		// For OcclusionCulling
		struct VertexClipSpace
		{
			math::float4 pos;

			VertexClipSpace();
			VertexClipSpace(const math::float4& f4Pos);
			VertexClipSpace(const math::float3& f3Pos);

			VertexClipSpace& operator = (const math::float3& f3Pos)
			{
				pos = math::float4(f3Pos.x, f3Pos.y, 0.f, f3Pos.z);
				return *this;
			}

			VertexClipSpace& operator = (const VertexPos& vertexPos)
			{
				pos = math::float4(vertexPos.pos.x, vertexPos.pos.y, 0.f, vertexPos.pos.z);
				return *this;
			}
		};
	}
}