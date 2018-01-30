#pragma once

namespace EastEngine
{
	template <typename PHANTOM_TYPE, typename VALUE>
	struct PhantomType
	{
		VALUE value{};

		explicit PhantomType(const VALUE& value)
			: value(value)
		{
		}

		bool operator == (const PhantomType& other) const { return value == other.value; }
		bool operator != (const PhantomType& other) const { return value != other.value; }
	};
}