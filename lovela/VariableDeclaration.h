#pragma once
#include "TypeSpec.h"

struct VariableDeclaration
{
	std::string name;
	TypeSpec type{};

	[[nodiscard]] auto operator<=>(const VariableDeclaration& rhs) const noexcept = default;

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << name << ',' << type << ']';
	}
};

inline std::ostream& operator<<(std::ostream& stream, const VariableDeclaration& varDecl)
{
	varDecl.Print(stream);
	return stream;
}
