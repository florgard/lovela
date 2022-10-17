#pragma once
#include "Node.h"

using INodeGenerator = std::experimental::generator<std::unique_ptr<Node>>;
