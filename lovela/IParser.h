#pragma once
#include "Node.h"

using NodeGenerator = tl::generator<Node>;

class IParser
{
public:
	virtual ~IParser() = default;

	[[nodiscard]] virtual NodeGenerator Parse() noexcept = 0;
};
