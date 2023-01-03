#pragma once

template <typename ItemT>
class IEnumerator
{
protected:
	[[nodiscard]] virtual ItemT& GetNext() noexcept = 0;
	[[nodiscard]] virtual bool IsDone() noexcept = 0;
	virtual void Advance() noexcept = 0;
};

/// <summary>
/// Range enumerator that takes ownership of the range object.
/// Provides access to the encapsulated range object to the base enumerator class.
/// </summary>
/// <typeparam name="BaseT">The base enumerator class. Must inherit IEnumerator.</typeparam>
/// <typeparam name="RangeT">The range type to encapsulate. The range object will be moved into the range enumerator.</typeparam>
template <class BaseT, std::ranges::range RangeT>
class RangeEnumerator : public BaseT
{
	RangeT _range;

	using IteratorT = decltype(std::ranges::begin(_range));
	IteratorT _iterator;

	using ItemT = std::decay_t<decltype(*std::ranges::begin(_range))>;
	static_assert(std::is_base_of_v<IEnumerator<ItemT>, BaseT>, "The base class must inherit IEnumerator for the item type.");

	[[nodiscard]] ItemT& GetNext() noexcept override
	{
		return *_iterator;
	}

	[[nodiscard]] bool IsDone() noexcept override
	{
		return _iterator == _range.end();
	}

	[[nodiscard]] void Advance() noexcept override
	{
		_iterator++;
	}

public:
	RangeEnumerator() noexcept = default;

	RangeEnumerator(RangeT&& range) noexcept
		: _range(std::move(range))
		, _iterator(_range.begin())
	{
	}

	void Initialize(RangeT&& range) noexcept
	{
		_range = std::move(range);
		_iterator = _range.begin();
	}
};

/// <summary>
/// Range enumerator that doesn't take ownership of the range object.
/// Provides access to the encapsulated range object to the base enumerator class.
/// Use this range enumerator variant when the referenced range must retain its items.
/// </summary>
/// <typeparam name="BaseT">The base enumerator class. Must inherit IEnumerator.</typeparam>
/// <typeparam name="RangeT">The range type to encapsulate. The range object will only be referenced, not moved into the range enumerator.</typeparam>
template <class BaseT, std::ranges::range RangeT>
class RangeRefEnumerator : public BaseT
{
	RangeT* _rangePtr;

	using IteratorT = decltype(std::ranges::begin(*_rangePtr));
	IteratorT _iterator;

	using ItemT = std::decay_t<decltype(*std::ranges::begin(*_rangePtr))>;
	static_assert(std::is_base_of_v<IEnumerator<ItemT>, BaseT>, "The base class must inherit IEnumerator for the item type.");

	[[nodiscard]] ItemT& GetNext() noexcept override
	{
		return *_iterator;
	}

	[[nodiscard]] bool IsDone() noexcept override
	{
		return _iterator == _rangePtr->end();
	}

	[[nodiscard]] void Advance() noexcept override
	{
		_iterator++;
	}

public:
	RangeRefEnumerator() noexcept = default;

	RangeRefEnumerator(RangeT& range) noexcept
		: _rangePtr(&range)
		, _iterator(range.begin())
	{
	}

	void Initialize(RangeT& range) noexcept
	{
		_rangePtr = &range;
		_iterator = range.begin();
	}
};

/// <summary>
/// Range enumerator that takes ownership of the range object.
/// Provides access to the encapsulated range object to the base enumerator class.
/// Use this range enumerator variant for ranges that only gives const access to the items.
/// </summary>
/// <typeparam name="BaseT">The base enumerator class. Must inherit IEnumerator.</typeparam>
/// <typeparam name="RangeT">The range type to encapsulate. The range object will be moved into the range enumerator.</typeparam>
template <class BaseT, std::ranges::range RangeT>
class ConstRangeEnumerator : public BaseT
{
	RangeT _range;

	using IteratorT = decltype(std::ranges::begin(_range));
	IteratorT _iterator;

	using ItemT = std::decay_t<decltype(*std::ranges::begin(_range))>;
	static_assert(std::is_base_of_v<IEnumerator<ItemT>, BaseT>, "The base class must inherit IEnumerator for the item type.");

	ItemT _item{};

	[[nodiscard]] ItemT& GetNext() noexcept override
	{
		_item = *_iterator;
		return _item;
	}

	[[nodiscard]] bool IsDone() noexcept override
	{
		return _iterator == _range.end();
	}

	[[nodiscard]] void Advance() noexcept override
	{
		_iterator++;
	}

public:
	ConstRangeEnumerator() noexcept = default;

	ConstRangeEnumerator(RangeT&& range) noexcept
		: _range(std::move(range))
		, _iterator(_range.begin())
	{
	}

	void Initialize(RangeT&& range) noexcept
	{
		_range = std::move(range);
		_iterator = _range.begin();
	}
};

template <class BaseT, class StreamT>
class StreamEnumerator : public BaseT
{
	StreamT* _stream;

	using ItemT = StreamT::char_type;
	static_assert(std::is_base_of_v<IEnumerator<ItemT>, BaseT>, "The base class must inherit IEnumerator for the item type.");

	ItemT _item{};
	bool _done{};

	[[nodiscard]] ItemT& GetNext() noexcept override
	{
		return _item;
	}

	[[nodiscard]] bool IsDone() noexcept override
	{
		return _done;
	}

	[[nodiscard]] void Advance() noexcept override
	{
		(*_stream) >> _item;
		_done = _stream->eof() || _stream->fail();
	}

public:
	StreamEnumerator() noexcept = default;

	StreamEnumerator(StreamT& stream) noexcept
		: _stream(&stream)
		, _item((*_stream) >> _item)
	{
	}

	void Initialize(StreamT& stream) noexcept
	{
		_stream = &stream;
		(*_stream) >> _item;
	}
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

		for (auto& child : tree.children)
		{
			DepthFirstPreorder(child, visitor);
		}
	}

	static constexpr void DepthFirstPostorder(NodeT& tree, std::function<void(NodeT& node)> visitor) noexcept
	{
		for (auto& child : tree.children)
		{
			DepthFirstPostorder(child, visitor);
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
