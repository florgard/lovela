#pragma once
#include "NameSpace.h"
#include "TypeSpec.h"
#include "ApiSpec.h"
#include "VariableDeclaration.h"

using ParameterList = std::vector<std::shared_ptr<VariableDeclaration>>;

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
		stream << '[' << name << ',';
		nameSpace.Print(stream);
		stream << ',';
		outType.Print(stream);
		stream << ',';
		inType.Print(stream);
		stream << ',' << '[';
		for (auto& param : parameters)
		{
			param->Print(stream);
			stream << ',';
		}
		stream << ']' << ',';
		api.Print(stream);
		stream << ']';
	}
};
