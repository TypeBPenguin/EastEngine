#pragma once

namespace eastengine
{
	template <typename PHANTOM_TYPE, typename VALUE>
	struct PhantomType
	{
	private:
		VALUE value{};

	public:
		explicit PhantomType(const VALUE& value)
			: value(value)
		{
		}

		PhantomType(VALUE&& value) noexcept
			: value(std::move(value))
		{
		}

		PhantomType& operator = (const VALUE& source) noexcept
		{
			value = source;
			return *this;
		}

		PhantomType& operator = (VALUE&& source) noexcept
		{
			value = std::move(source);
			return *this;
		}

		bool operator == (const PhantomType& other) const { return value == other.value; }
		bool operator != (const PhantomType& other) const { return value != other.value; }

		operator VALUE() const { return value; }
		const VALUE& Value() const { return value; }
	};
}