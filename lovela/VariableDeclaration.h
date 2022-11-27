#pragma once
#include "TypeSpec.h"

struct VariableDeclaration
{
	std::string name;
	TypeSpec type{};

	[[nodiscard]] auto operator<=>(const VariableDeclaration& rhs) const noexcept = default;

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << name << ',';
		type.Print(stream);
		stream << ']';
	}
};
