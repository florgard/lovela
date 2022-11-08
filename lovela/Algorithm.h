#pragma once

constexpr bool not_empty(const auto& x)
{
	return !!x;
};

template <typename NodeT>
struct Traverse
{
	static constexpr void DepthFirstPreorder(NodeT& tree, std::function<void(NodeT& node)> visitor) noexcept
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

	static constexpr void DepthFirstPostorder(NodeT& tree, std::function<void(NodeT& node)> visitor) noexcept
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

	static constexpr void DepthFirstPreorder(std::ranges::range auto& range, std::function<void(NodeT& node)> visitor) noexcept
	{
		auto end = range.end();
		for (auto it = range.begin(); it != end; it++)
		{
			DepthFirstPreorder(**it, visitor);
		}
	}

	static constexpr void DepthFirstPostorder(std::ranges::range auto& range, std::function<void(NodeT& node)> visitor) noexcept
	{
		auto end = range.end();
		for (auto it = range.begin(); it != end; it++)
		{
			DepthFirstPostorder(*it, visitor);
		}
	}
};
