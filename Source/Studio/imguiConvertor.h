#pragma once

struct ImVec2;

namespace eastengine
{
	namespace math
	{
		inline const ImVec2& Convert(const Vector2& vec2)
		{
			return *reinterpret_cast<const ImVec2*>(&vec2);
		}

		inline const Vector2& Convert(const ImVec2& vec2)
		{
			return *reinterpret_cast<const Vector2*>(&vec2);
		}
	}
}
