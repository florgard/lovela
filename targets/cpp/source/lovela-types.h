#pragma once
#include <string>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <exception>

namespace lovela
{
	namespace detail
	{
		template <typename Item, typename Tuple, size_t index>
		inline constexpr bool is_same_tuple_element = std::is_same_v<Item, std::tuple_element_t<index, Tuple>>;

		template <size_t index>
		inline constexpr size_t rebase_v = index - 1;

		[[nodiscard]] constexpr size_t rebase(size_t index, size_t size)
		{
			if (!index || index > size)
			{
				throw std::out_of_range("index out of range");
			}

			return index - 1;
		}

		template <size_t index, typename Tuple, typename Item = std::tuple_element_t<rebase_v<index>, Tuple>>
		[[nodiscard]] constexpr auto& checked_tuple_get(Tuple& tuple)
		{
			static constexpr size_t rebased = rebase_v<index>;
			static_assert(rebased < std::tuple_size_v<Tuple>, "index out of bounds");
			static_assert(is_same_tuple_element<Item, Tuple, rebased>, "invalid access type");
			return std::get<rebased>(tuple);
		};

		template <typename Item, typename Tuple, size_t visitIndex = 0>
		constexpr void visit(Tuple& tuple, size_t index, auto&& visitor)
		{
			if constexpr (!visitIndex)
			{
				index = rebase(index, std::tuple_size_v<Tuple>);
			}

			if constexpr (visitIndex >= std::tuple_size_v<Tuple>)
			{
				static_cast<void>(visitor);
				throw std::out_of_range("index out of range");
			}
			else if (index == visitIndex)
			{
				if constexpr (is_same_tuple_element<Item, Tuple, visitIndex>)
				{
					visitor(std::get<visitIndex>(tuple));
				}
				else
				{
					throw std::invalid_argument("invalid access type");
				}
			}
			else
			{
				visit<Item, Tuple, visitIndex + 1>(tuple, index, visitor);
			}
		}

		[[nodiscard]] inline size_t to_size(std::u8string_view str)
		{
			return std::stoi(std::string(reinterpret_cast<const char*>(str.data()), str.size()));
		}

		[[nodiscard]] inline size_t to_index(std::u8string_view str, size_t size)
		{
			const size_t index = to_size(str);
			// Check bounds.
			static_cast<void>(rebase(index, size));
			return index;
		}

		template<typename... Arrays>
		[[nodiscard]] constexpr auto array_cat(Arrays&... arrays)
		{
			std::array<std::common_type_t<typename Arrays::value_type...>, (std::tuple_size_v<Arrays> +...)> cat;

			size_t i = 0;
			([&](auto& arr) { for (auto& elem : arr) { cat[i++] = std::move(elem); } }(arrays), ...);

			return cat;
		}
	}

	struct None
	{
	};

	template <typename Item>
	struct variable
	{
		Item _item;

		using item_type = Item;

		[[nodiscard]] constexpr size_t get_size() const { return _size; }
		constexpr void set_size(size_t size) { if (size != _size) { throw std::out_of_range("a variable cannot be resized"); } }
		[[nodiscard]] constexpr size_t get_index(std::u8string_view name) const { return detail::to_index(name, get_size()); }

		template <size_t index> [[nodiscard]] constexpr auto& get_item() { return get_item(index); }
		template <size_t index> constexpr void get_item(Item& item) { get_item(index, item); }
		[[nodiscard]] constexpr auto& get_item(size_t index) { static_cast<void>(rebase(index)); return _item; }
		constexpr void get_item(size_t index, Item& item) { static_cast<void>(rebase(index)); item = _item; }
		[[nodiscard]] constexpr auto& get_item(std::u8string_view name) { return get_item(get_index(name)); }
		constexpr void get_item(std::u8string_view name, Item& item) { get_item(get_index(name), item); }

		template <size_t index> constexpr void set_item(const Item& item) { set_item(index, item); }
		template <size_t index> constexpr void set_item(Item&& item) { set_item(index, std::move(item)); }
		constexpr void set_item(size_t index, const Item& item) { static_cast<void>(rebase(index)); _item = item; }
		constexpr void set_item(size_t index, Item&& item) { static_cast<void>(rebase(index)); _item = std::move(item); }
		constexpr void set_item(std::u8string_view name, const Item& item) { set_item(get_index(name), item); }
		constexpr void set_item(std::u8string_view name, Item&& item) { set_item(get_index(name), std::move(item)); }

		constexpr void add_item(const Item&) { throw std::out_of_range("a variable cannot be appended to"); }
		constexpr void add_item(Item&&) { throw std::out_of_range("a variable cannot be appended to"); }

	private:
		static constexpr size_t _size = 1;

		[[nodiscard]] static constexpr size_t rebase(size_t index) { return detail::rebase(index, _size); }
	};

	template <typename Item, size_t _size>
	class fixed_array
	{
		std::array<Item, _size> _items;

		[[nodiscard]] static constexpr size_t rebase(size_t index) { return detail::rebase(index, _size); }

	public:
		using item_type = Item;

		[[nodiscard]] constexpr size_t get_size() const { return _size; }
		constexpr void set_size(size_t size) { if (size != _size) { throw std::out_of_range("a fixed array cannot be resized"); } }
		[[nodiscard]] constexpr size_t get_index(std::u8string_view name) const { return detail::to_index(name, get_size()); }

		template <size_t index> [[nodiscard]] constexpr auto& get_item() { return get_item(index); }
		template <size_t index> constexpr void get_item(Item& item) { get_item(index, item); }
		[[nodiscard]] constexpr auto& get_item(size_t index) { return _items.at(rebase(index)); }
		constexpr void get_item(size_t index, Item& item) { item = _items.at(rebase(index)); }
		[[nodiscard]] constexpr auto& get_item(std::u8string_view name) { return get_item(get_index(name)); }
		constexpr void get_item(std::u8string_view name, Item& item) { get_item(get_index(name), item); }

		template <size_t index> constexpr void set_item(const Item& item) { set_item(index, item); }
		template <size_t index> constexpr void set_item(Item&& item) { set_item(index, std::move(item)); }
		constexpr void set_item(size_t index, const Item& item) { _items[rebase(index)] = item; }
		constexpr void set_item(size_t index, Item&& item) { _items[rebase(index)] = std::move(item); }
		constexpr void set_item(std::u8string_view name, const Item& item) { set_item(get_index(name), item); }
		constexpr void set_item(std::u8string_view name, Item&& item) { set_item(get_index(name), std::move(item)); }

		constexpr void add_item(const Item&) { throw std::out_of_range("a fixed array cannot be appended to"); }
		constexpr void add_item(Item&&) { throw std::out_of_range("a fixed array cannot be appended to"); }
	};

	template <typename Item>
	class dynamic_array
	{
		std::vector<Item> _items;

		[[nodiscard]] constexpr size_t rebase(size_t index) const  { return detail::rebase(index, _items.size()); }

	public:
		using item_type = Item;

		[[nodiscard]] constexpr size_t get_size() const { return _items.size(); }
		constexpr void set_size(size_t size) { _items.resize(size); }
		[[nodiscard]] constexpr size_t get_index(std::u8string_view name) const { return detail::to_index(name, get_size()); }

		template <size_t index> [[nodiscard]] constexpr auto& get_item() { return get_item(index); }
		template <size_t index> constexpr void get_item(Item& item) { get_item(index, item); }
		[[nodiscard]] constexpr auto& get_item(size_t index) { return _items.at(rebase(index)); }
		constexpr void get_item(size_t index, Item& item) { item = _items.at(rebase(index)); }
		[[nodiscard]] constexpr auto& get_item(std::u8string_view name) { return get_item(get_index(name)); }
		constexpr void get_item(std::u8string_view name, Item& item) { get_item(get_index(name), item); }

		template <size_t index> constexpr void set_item(const Item& item) { set_item(index, item); }
		template <size_t index> constexpr void set_item(Item&& item) { set_item(index, std::move(item)); }
		constexpr void set_item(size_t index, const Item& item) { _items[rebase(index)] = item; }
		constexpr void set_item(size_t index, Item&& item) { _items[rebase(index)] = std::move(item); }
		constexpr void set_item(std::u8string_view name, const Item& item) { set_item(get_index(name), item); }
		constexpr void set_item(std::u8string_view name, Item&& item) { set_item(get_index(name), std::move(item)); }

		constexpr void add_item(const Item& item) { _items.push_back(item); }
		constexpr void add_item(Item&& item) { _items.emplace_back(std::move(item)); }
	};

	template <size_t size>
	struct default_tuple_names_t
	{
		static constexpr auto values = std::array<std::u8string_view, size>{};
	};

	template <typename tuple_t>
	struct fixed_tuple
	{
		tuple_t _items;

		constexpr auto& as_tuple() { return _items; }
		constexpr auto& as_fixed_tuple() { return *this; }
		//named_tuple<default_tuple_names_t<_size>, tuple_t> as_named_tuple() { return { {{_items}} }; }

	private:
		static constexpr size_t _size = std::tuple_size_v<tuple_t>;

		[[nodiscard]] static constexpr size_t rebase(size_t index) { return detail::rebase(index, _size); }

	public:
		template <size_t index>
		using item_type = std::tuple_element_t<detail::rebase_v<index>, tuple_t>;

		[[nodiscard]] constexpr size_t get_size() const { return _size; }
		constexpr void set_size(size_t size) { if (size != _size) { throw std::out_of_range("a fixed tuple cannot be resized"); } }
		[[nodiscard]] constexpr size_t get_index(std::u8string_view name) const { return detail::to_index(name, get_size()); }

		template <size_t index> [[nodiscard]] constexpr auto& get_item() { return detail::checked_tuple_get<index, tuple_t>(_items); };
		template <size_t index, typename Item> constexpr void get_item(Item& item) { item = detail::checked_tuple_get<index, tuple_t, Item>(_items); };
		template <typename Item> [[nodiscard]] constexpr auto get_item(size_t index) { Item item; detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { item = elem; }); return item; }
		template <typename Item> constexpr void get_item(size_t index, Item& item) { detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { item = elem; }); }
		template <typename Item> [[nodiscard]] constexpr auto get_item(std::u8string_view name) { return get_item<Item>(get_index(name)); }
		template <typename Item> constexpr void get_item(std::u8string_view name, Item& item) { get_item(get_index(name), item); }

		template <size_t index, typename Item> constexpr void set_item(const Item& item) { detail::checked_tuple_get<index, tuple_t, Item>(_items) = item; };
		template <size_t index, typename Item> constexpr void set_item(Item&& item) { detail::checked_tuple_get<index, tuple_t, Item>(_items) = std::move(item); };
		template <typename Item> constexpr void set_item(size_t index, const Item& item) { detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { elem = item; }); }
		template <typename Item> constexpr void set_item(size_t index, Item&& item) { detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { elem = std::move(item); }); }
		template <typename Item> constexpr void set_item(std::u8string_view name, const Item& item) { set_item(get_index(name), item); }
		template <typename Item> constexpr void set_item(std::u8string_view name, Item&& item) { set_item(get_index(name), std::move(item)); }

		template <typename Item> constexpr void add_item(const Item&) { throw std::out_of_range("a fixed tuple cannot be appended to"); }
		template <typename Item> constexpr void add_item(Item&&) { throw std::out_of_range("a fixed tuple cannot be appended to"); }
	};

	template <typename tuple_t, typename names_t>
	struct named_tuple
	{
		tuple_t _items;

		[[nodiscard]] constexpr auto& as_tuple() { return _items; }
		[[nodiscard]] constexpr auto as_fixed_tuple() { return fixed_tuple<tuple_t>{ _items }; }
		[[nodiscard]] constexpr auto& as_named_tuple() { return *this; }

	private:
		static constexpr size_t _size = std::tuple_size_v<tuple_t>;

		[[nodiscard]] static constexpr size_t rebase(size_t index) { return detail::rebase(index, _size); }

	public:
		template <size_t index>
		using item_type = std::tuple_element_t<detail::rebase_v<index>, tuple_t>;

		[[nodiscard]] constexpr size_t get_size() const { return _size; }
		constexpr void set_size(size_t size) { if (size != _size) { throw std::out_of_range("a named tuple cannot be resized"); } }
		[[nodiscard]] constexpr size_t get_index(std::u8string_view name) const
		{
			static constexpr auto names = names_t::values;

			auto iter = std::find(names.begin(), names.end(), name);
			if (iter != names.end())
			{
				return std::distance(names.begin(), iter) + 1;
			}
			else
			{
				return detail::to_index(name, get_size());
			}
		}

		template <size_t index> [[nodiscard]] constexpr auto& get_item() { return detail::checked_tuple_get<index, tuple_t>(_items); };
		template <size_t index, typename Item> constexpr void get_item(Item& item) { item = detail::checked_tuple_get<index, tuple_t, Item>(_items); };
		template <typename Item> [[nodiscard]] constexpr auto get_item(size_t index) { Item item; detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { item = elem; }); return item; }
		template <typename Item> constexpr void get_item(size_t index, Item& item) { detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { item = elem; }); }
		template <typename Item> [[nodiscard]] constexpr auto get_item(std::u8string_view name) { return get_item<Item>(get_index(name)); }
		template <typename Item> constexpr void get_item(std::u8string_view name, Item& item) { get_item(get_index(name), item); }

		template <size_t index, typename Item> constexpr void set_item(const Item& item) { detail::checked_tuple_get<index, tuple_t, Item>(_items) = item; };
		template <size_t index, typename Item> constexpr void set_item(Item&& item) { detail::checked_tuple_get<index, tuple_t, Item>(_items) = std::move(item); };
		template <typename Item> constexpr void set_item(size_t index, const Item& item) { detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { elem = item; }); }
		template <typename Item> constexpr void set_item(size_t index, Item&& item) { detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { elem = std::move(item); }); }
		template <typename Item> constexpr void set_item(std::u8string_view name, const Item& item) { set_item(get_index(name), item); }
		template <typename Item> constexpr void set_item(std::u8string_view name, Item&& item) { set_item(get_index(name), std::move(item)); }

		template <typename Item> constexpr void add_item(const Item&) { throw std::out_of_range("a named tuple cannot be appended to"); }
		template <typename Item> constexpr void add_item(Item&&) { throw std::out_of_range("a named tuple cannot be appended to"); }
	};

	// Type casts

	namespace detail
	{
		template <class T, size_t N, size_t... Is>
		[[nodiscard]] constexpr auto as_tuple(std::array<T, N>&& arr, std::index_sequence<Is...>)
		{
			return std::make_tuple(T{ std::move(arr[Is]) }...);
		}

		template <class T, size_t N>
		[[nodiscard]] constexpr auto as_tuple(std::array<T, N>&& arr)
		{
			return as_tuple(std::move(arr), std::make_index_sequence<N>{});
		}
	}

	template <typename... Items>
	[[nodiscard]] constexpr auto to_fixed_tuple(Items&&... items)
	{
		return fixed_tuple<std::tuple<Items...>>{ {std::move(items)...} };
	}

	template <typename... Items>
	[[nodiscard]] constexpr auto to_fixed_tuple(std::tuple<Items...>&& src)
	{
		return fixed_tuple<std::tuple<Items...>>{ std::move(src) };
	}

	template <typename Tuple>
	[[nodiscard]] constexpr auto to_fixed_tuple(fixed_tuple<Tuple>&& src)
	{
		return std::move(src);
	}

	template <typename Tuple, typename Names>
	[[nodiscard]] constexpr auto to_fixed_tuple(named_tuple<Names, Tuple>&& src)
	{
		return std::move(src.as_fixed_tuple());
	}
}
