#pragma once
#include "NameSpace.h"

struct TypeSpec
{
	enum class Kind
	{
		Any,
		None,
		Tagged,
		Named,
		Primitive,
		Invalid,
	} kind{};

	std::string name;
	NameSpace nameSpace{};
	std::vector<size_t> arrayDims;

	struct Primitive
	{
		unsigned char bits{};
		bool signedType{};
		bool floatType{};

		[[nodiscard]] constexpr auto operator<=>(const Primitive& rhs) const noexcept = default;

		[[nodiscard]] void Print(std::ostream& stream) const
		{
			stream << '[' << static_cast<unsigned int>(bits) << ',' << signedType << ',' << floatType << ']';
		}
	} primitive{};

	[[nodiscard]] constexpr bool Is(Kind k) const noexcept { return kind == k; }

	[[nodiscard]] constexpr auto operator<=>(const TypeSpec& rhs) const noexcept = default;

	[[nodiscard]] void PrintPrimitiveName(std::ostream& stream) const
	{
		stream << (primitive.floatType ? 'f' : (primitive.signedType ? 'i' : 'u')) << static_cast<unsigned int>(primitive.bits);
	}

	[[nodiscard]] std::string GetQualifiedName() const
	{
		std::ostringstream s;

		s << '[';

		nameSpace.Print(s);

		if (Is(Kind::Primitive))
		{
			PrintPrimitiveName(s);
		}
		else
		{
			s << name;
		}

		s << ']';

		for (auto& length : arrayDims)
		{
			s << '#';

			if (length)
			{
				s << length;
			}
		}

		return s.str();
	}

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << to_string(kind) << ',' << name << ',';
		nameSpace.Print(stream);
		stream << ',' << '[';
		for (auto& length : arrayDims)
		{
			stream << length << ',';
		}
		stream << ']' << ',';
		primitive.Print(stream);
		stream << ']';
	}

private:
	static constexpr const char* noneTypeName = "()";
};
