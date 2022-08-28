#pragma once
#include "Node.h"

struct Traverse
{
	static void DepthFirstPreorder(Node& tree, std::function<void(Node& node)> visitor) noexcept;
	static void DepthFirstPostorder(Node& tree, std::function<void(Node& node)> visitor) noexcept;
};
