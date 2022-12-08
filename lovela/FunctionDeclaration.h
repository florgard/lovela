#pragma once
#include "NameSpace.h"
#include "TypeSpec.h"
#include "ApiSpec.h"
#include "VariableDeclaration.h"

using ParameterList = std::vector<std::shared_ptr<VariableDeclaration>>;

inline std::ostream& operator<<(std::ostream& stream, const ParameterList& parameters)
{
	for (auto& param : parameters)
	{
		stream << *param << ',';
	}
	return stream;
}

struct FunctionDeclaration
{
	std::string name;
	NameSpace nameSpace{};
	TypeSpec outType{};
	TypeSpec inType{};
	ParameterList parameters{};
	ApiSpec api{};

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << name << ','
			<< nameSpace << ','
			<< outType << ','
			<< inType << ','
			<< '[' << parameters << ']' << ','
			<< api << ']';
	}
};
