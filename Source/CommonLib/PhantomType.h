#pragma once

namespace eastengine
{
	template <typename PHANTOM_TYPE, typename VALUE>
	struct PhantomType
	{
		VALUE value{};

		explicit PhantomType(const VALUE& value)
			: value(value)
		{
		}

		PhantomType(VALUE&& value) noexcept
			: value(std::move(value))
		{
		}

		bool operator == (const PhantomType& other) const { return value == other.value; }
		bool operator != (const PhantomType& other) const { return value != other.value; }
	};
}