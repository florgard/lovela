#include "pch.h"
#include "Algorithm.h"

void Traverse::DepthFirstPreorder(Node& tree, std::function<void(Node& node)> visitor) noexcept
{
	visitor(tree);

	if (tree.left)
	{
		DepthFirstPreorder(*tree.left, visitor);
	}
	if (tree.right)
	{
		DepthFirstPreorder(*tree.right, visitor);
	}
}

void Traverse::DepthFirstPostorder(Node& tree, std::function<void(Node& node)> visitor) noexcept
{
	if (tree.left)
	{
		DepthFirstPostorder(*tree.left, visitor);
	}
	if (tree.right)
	{
		DepthFirstPostorder(*tree.right, visitor);
	}

	visitor(tree);
}
