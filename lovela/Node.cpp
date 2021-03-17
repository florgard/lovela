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
		&& rhs.parameters == parameters
		&& rhs.imported == imported
		&& rhs.exported == exported;
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
		.imported = imported,
		.exported = exported
	};
}
