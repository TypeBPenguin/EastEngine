#pragma once

namespace EastEngine
{
	namespace Graphics
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
				eUI,

				eCount,
			};
		};

		struct VertexPosTexNor;

		size_t GetVertexFormatSize(EmVertexFormat::Type emFormat);

		struct VertexPos
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePos; }
			static constexpr size_t Size() { return sizeof(VertexPos); }

			Math::Vector3 pos;

			VertexPos();
			VertexPos(const Math::Vector3& f3Pos);

			void SetVertex(const Math::Vector3& f3Pos) { pos = f3Pos; }
		};

		struct VertexPos4
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePos4; }
			static constexpr size_t Size() { return sizeof(VertexPos4); }

			Math::Vector4 pos;

			VertexPos4();
			VertexPos4(const Math::Vector4& f4Pos);

			void SetVertex(const Math::Vector4& f4Pos) { pos = f4Pos; }
		};

		struct VertexPosCol
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosCol; }
			static constexpr size_t Size() { return sizeof(VertexPosCol); }

			Math::Vector3 pos;
			Math::Color color;

			VertexPosCol();
			VertexPosCol(const Math::Vector3& f3Pos, const Math::Color& color);

			void SetVertex(const Math::Vector3& f3Pos) { pos = f3Pos; }
			void SetColor(const Math::Color& setColor) { color = setColor; }
		};

		struct VertexPosTex
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTex; }
			static constexpr size_t Size() { return sizeof(VertexPosTex); }

			Math::Vector3 pos;
			Math::Vector2 uv;

			VertexPosTex();
			VertexPosTex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV);

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV) { pos = f3Pos;	uv = f2UV; }
		};

		struct VertexPosTexCol
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexCol; }
			static constexpr size_t Size() { return sizeof(VertexPosTexCol); }

			Math::Vector3 pos;
			Math::Vector2 uv;
			Math::Color color;

			VertexPosTexCol();
			VertexPosTexCol(const Math::Vector3& f3Pos, const Math::Vector2& f2UV);

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV) { pos = f3Pos;	uv = f2UV; }
		};

		struct VertexPosTexNor
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNor; }
			static constexpr size_t Size() { return sizeof(VertexPosTexNor); }

			Math::Vector3 pos;
			Math::Vector2 uv;
			Math::Vector3 normal;

			VertexPosTexNor();
			VertexPosTexNor(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal);

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal) { pos = f3Pos; uv = f2UV; normal = f3Normal; }
		};

		struct VertexPosTexNorCol
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNorCol; }
			static constexpr size_t Size() { return sizeof(VertexPosTexNorCol); }

			Math::Vector3 pos;
			Math::Vector2 uv;
			Math::Vector3 normal;
			Math::Color color;

			VertexPosTexNorCol();
			VertexPosTexNorCol(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Color& color);

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal) { pos = f3Pos; uv = f2UV; normal = f3Normal; }
			void SetColor(const Math::Color& setColor) { color = setColor; }
		};

		struct VertexPosTexNorTanBin
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNorTanBin; }
			static constexpr size_t Size() { return sizeof(VertexPosTexNorTanBin); }

			Math::Vector3 pos;
			Math::Vector2 uv;
			Math::Vector3 normal;
			Math::Vector3 tangent;
			Math::Vector3 binormal;

			VertexPosTexNorTanBin();
			VertexPosTexNorTanBin(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal);
			VertexPosTexNorTanBin(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Vector3& f3Tangent, const Math::Vector3& f3Binormal);

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal) { pos = f3Pos; uv = f2UV; normal = f3Normal; }
			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Vector3& f3Tangent, const Math::Vector3& f3Binormal) { pos = f3Pos; uv = f2UV; normal = f3Normal; tangent = f3Tangent; binormal = f3Binormal; }
		};

		struct VertexPosTexNorWeiIdx
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNorWeiIdx; }
			static constexpr size_t Size() { return sizeof(VertexPosTexNorWeiIdx); }

			Math::Vector3 pos;
			Math::Vector2 uv;
			Math::Vector3 normal;
			Math::Vector3 boneWeight;
			uint16_t boneIndices[4] = {};

			VertexPosTexNorWeiIdx();
			VertexPosTexNorWeiIdx(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Vector3& f3Weight, const uint16_t boneIndices[4]);

			void operator = (const VertexPosTexNor& vertex)
			{
				pos = vertex.pos;
				uv = vertex.uv;
				normal = vertex.normal;
			}

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Vector3& f3Weight, const uint16_t _boneIndices[4]) { pos = f3Pos; uv = f2UV; normal = f3Normal; boneWeight = f3Weight; Memory::Copy(boneIndices, _boneIndices); }
		};

		struct VertexUI
		{
			static constexpr EmVertexFormat::Type Format() { return EmVertexFormat::eUI; }
			static constexpr size_t Size() { return sizeof(VertexUI); }

			Math::Vector2 pos;
			int vertexIdx;

			VertexUI();
			VertexUI(const Math::Vector2& f3Pos, int nVertexIdx);

			void SetVertex(const Math::Vector2& f3Pos, int nVertexIdx) { pos = f3Pos; vertexIdx = nVertexIdx; }
		};

		// For OcclusionCulling
		struct VertexClipSpace
		{
			Math::Vector4 pos;

			VertexClipSpace();
			VertexClipSpace(const Math::Vector4& f4Pos);
			VertexClipSpace(const Math::Vector3& f3Pos);
			
			VertexClipSpace& operator = (const Math::Vector3& f3Pos)
			{
				pos = Math::Vector4(f3Pos.x, f3Pos.y, 0.f, f3Pos.z);
				return *this;
			}

			VertexClipSpace& operator = (const VertexPos& vertexPos)
			{
				pos = Math::Vector4(vertexPos.pos.x, vertexPos.pos.y, 0.f, vertexPos.pos.z);
				return *this;
			}
		};
	}
}