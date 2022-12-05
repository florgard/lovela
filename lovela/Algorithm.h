#pragma once

template <typename ItemT>
class IEnumerator
{
protected:
	[[nodiscard]] virtual ItemT& GetNext() noexcept = 0;
	[[nodiscard]] virtual bool IsDone() noexcept = 0;
	virtual void Advance() noexcept = 0;
};

constexpr bool not_empty(const auto& x)
{
	return !!x;
};

constexpr auto to_vector(std::ranges::range auto&& range)
{
	std::vector<std::decay_t<decltype(*range.begin())>> v;

	for (auto&& item : range)
	{
		v.emplace_back(std::move(item));
	}

	return v;
}

constexpr auto to_vector(std::ranges::range auto& range)
{
	return std::vector<std::decay_t<decltype(*range.begin())>>(range.begin(), range.end());
}

template <typename T>
constexpr auto to_vector(std::vector<T>&& v)
{
	return v;
}

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
