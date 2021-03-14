#include "pch.h"
#include "Node.h"

bool Node::operator==(const Node& rhs) const noexcept
{
	// Compare owned data (not the child nodes, token or callee)
	return rhs.type == type
		&& rhs.value == value
		&& rhs.dataType == dataType
		&& rhs.nameSpace == nameSpace
		&& rhs.objectType == objectType
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
		.dataType = dataType,
		.objectType = objectType,
		.parameters = parameters,
		.imported = imported,
		.exported = exported
	};
}
