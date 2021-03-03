#include "Node.h"

bool Node::operator==(const Node& rhs) const noexcept
{
	// Compare everything but the children
	return rhs.type == type
		&& rhs.name == name
		&& rhs.dataType == dataType
		&& rhs.objectType == objectType
		&& rhs.parameters == parameters;
}
