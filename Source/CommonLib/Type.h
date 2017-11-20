#pragma once

namespace EastEngine
{
	template <typename T1, typename T2>
	struct Type
	{
		T2 value;

		explicit Type(const T2& value)
			: value(value)
		{
		}

		bool operator == (const Type& other) const { return value == other.value; }
		bool operator != (const Type& other) const { return value != other.value; }
	};
}