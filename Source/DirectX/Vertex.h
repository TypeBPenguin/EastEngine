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
				ePos,
				ePos4,
				ePosCol,
				ePosTex,
				ePosTexCol,
				ePosTexNor,
				ePosTexNorCol,
				ePosTexNorTanBin,
				ePosTexNorBleIdx,
				eUI,

				eCount,
			};
		};

		struct VertexPosTexNor;

		uint32_t GetVertexFormatSize(EmVertexFormat::Type emFormat);

		struct VertexPos
		{
			static EmVertexFormat::Type Format() { return EmVertexFormat::ePos; }

			Math::Vector3 pos;

			VertexPos();
			VertexPos(const Math::Vector3& f3Pos);

			void SetVertex(const Math::Vector3& f3Pos) { pos = f3Pos; }
		};

		struct VertexPos4
		{
			static EmVertexFormat::Type Format() { return EmVertexFormat::ePos4; }

			Math::Vector4 pos;

			VertexPos4();
			VertexPos4(const Math::Vector4& f4Pos);

			void SetVertex(const Math::Vector4& f4Pos) { pos = f4Pos; }
		};

		struct VertexPosCol
		{
			static EmVertexFormat::Type Format() { return EmVertexFormat::ePosCol; }

			Math::Vector3 pos;
			Math::Color color;

			VertexPosCol();
			VertexPosCol(const Math::Vector3& f3Pos, const Math::Color& color);

			void SetVertex(const Math::Vector3& f3Pos) { pos = f3Pos; }
			void SetColor(const Math::Color& setColor) { color = setColor; }
		};

		struct VertexPosTex
		{
			static EmVertexFormat::Type Format() { return EmVertexFormat::ePosTex; }

			Math::Vector3 pos;
			Math::Vector2 uv;

			VertexPosTex();
			VertexPosTex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV);

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV) { pos = f3Pos;	uv = f2UV; }
		};

		struct VertexPosTexCol
		{
			static EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexCol; }

			Math::Vector3 pos;
			Math::Vector2 uv;
			Math::Color color;

			VertexPosTexCol();
			VertexPosTexCol(const Math::Vector3& f3Pos, const Math::Vector2& f2UV);

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV) { pos = f3Pos;	uv = f2UV; }
		};

		struct VertexPosTexNor
		{
			static EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNor; }

			Math::Vector3 pos;
			Math::Vector2 uv;
			Math::Vector3 normal;

			VertexPosTexNor();
			VertexPosTexNor(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal);

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal) { pos = f3Pos; uv = f2UV; normal = f3Normal; }
		};

		struct VertexPosTexNorCol
		{
			static EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNorCol; }

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
			static EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNorTanBin; }

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

		struct VertexPosTexNorBleIdx
		{
			static EmVertexFormat::Type Format() { return EmVertexFormat::ePosTexNorBleIdx; }

			Math::Vector3 pos;
			Math::Vector2 uv;
			Math::Vector3 normal;
			Math::Vector3 blend;
			uint32_t idx;

			VertexPosTexNorBleIdx();
			VertexPosTexNorBleIdx(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Vector3& vBlend, uint32_t nIdx);

			void operator = (const VertexPosTexNor& vertex)
			{
				pos = vertex.pos;
				uv = vertex.uv;
				normal = vertex.normal;
			}

			void SetVertex(const Math::Vector3& f3Pos, const Math::Vector2& f2UV, const Math::Vector3& f3Normal, const Math::Vector3& vBlend, uint32_t nIdx) { pos = f3Pos; uv = f2UV; normal = f3Normal; blend = vBlend; idx = nIdx; }
		};

		struct VertexUI
		{
			static EmVertexFormat::Type Format() { return EmVertexFormat::eUI; }

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