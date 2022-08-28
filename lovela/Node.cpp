#include "pch.h"
#include "Node.h"

bool Node::operator==(const Node& rhs) const noexcept
{
	// Compare owned data (not the child nodes, token or callee)
	return rhs.type == type
		&& rhs.value == value
		&& rhs.outType == outType
		&& rhs.nameSpace == nameSpace
		&& rhs.inType == inType
		&& rhs.api == api
		&& std::equal(rhs.parameters.begin(), rhs.parameters.end(), parameters.begin(), [](const auto& v1, const auto& v2) { return *v1 == *v2; });
}

FunctionDeclaration Node::ToFunctionDeclaration() const
{
	return FunctionDeclaration
	{
		.name = value,
		.nameSpace = nameSpace,
		.outType = outType,
		.inType = inType,
		.parameters = parameters,
		.api = api,
	};
}

std::wstring Node::GetQualifiedName() const
{
	std::wostringstream name;
	for (auto& part : nameSpace)
	{
		name << part << '|';
	}
	name << value;
	return name.str();
}
