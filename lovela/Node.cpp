#include "pch.h"
#include "Node.h"

bool Node::operator==(const Node& rhs) const noexcept
{
	// Compare everything but the children
	return rhs.type == type
		&& rhs.value == value
		&& rhs.dataType == dataType
		&& rhs.nameSpace == nameSpace
		&& rhs.objectType == objectType
		&& rhs.parameters == parameters
		&& rhs.imported == imported
		&& rhs.exported == exported;
}
