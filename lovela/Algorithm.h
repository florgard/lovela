#pragma once
#include "Node.h"

inline bool not_empty(const auto& x)
{
	return !!x;
};

struct Traverse
{
	static void DepthFirstPreorder(Node& tree, std::function<void(Node& node)> visitor) noexcept;
	static void DepthFirstPostorder(Node& tree, std::function<void(Node& node)> visitor) noexcept;

	static void DepthFirstPreorder(std::ranges::range auto& range, std::function<void(Node& node)> visitor) noexcept
	{
		auto end = range.end();
		for (auto it = range.begin(); it != end; it++)
		{
			DepthFirstPreorder(**it, visitor);
		}
	}

	static void DepthFirstPostorder(std::ranges::range auto& range, std::function<void(Node& node)> visitor) noexcept
	{
		auto end = range.end();
		for (auto it = range.begin(); it != end; it++)
		{
			DepthFirstPostorder(*it, visitor);
		}
	}
};
