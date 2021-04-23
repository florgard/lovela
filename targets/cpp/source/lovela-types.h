#pragma once
#define LOVELA
#include <string>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <exception>
#include "utfcpp/utf8.h"

namespace lovela
{
	namespace detail
	{
		template <typename Item, typename Tuple, size_t index>
		static constexpr bool is_same_tuple_element = std::is_same_v<Item, std::tuple_element<index, Tuple>::type>;

		template <size_t index>
		static constexpr size_t rebase_v = index - 1;

		[[nodiscard]] constexpr size_t rebase(size_t index, size_t size)
		{
			if (!index || index > size)
			{
				throw std::out_of_range("index out of range");
			}

			return index - 1;
		}

		template <size_t index, typename Tuple, typename Item = std::tuple_element<rebase_v<index>, Tuple>::type>
		[[nodiscard]] constexpr auto& checked_tuple_get(Tuple& tuple)
		{
			static constexpr size_t rebased = rebase_v<index>;
			static_assert(rebased < std::tuple_size_v<Tuple>, "index out of bounds");
			static_assert(is_same_tuple_element<Item, Tuple, rebased>, "invalid access type");
			return std::get<rebased>(tuple);
		};

		template <typename Item, typename Tuple, size_t visitIndex = 0>
		void visit(Tuple& tuple, size_t index, auto&& visitor)
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
		constexpr auto array_cat(Arrays&... arrays)
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

	template <typename Item, size_t _size>
	class fixed_array
	{
		std::array<Item, _size> _items;

		static constexpr size_t rebase(size_t index) { return detail::rebase(index, _size); }

	public:
		constexpr size_t get_size() const { return _size; }
		constexpr void set_size(size_t size) { if (size != _size) { throw std::out_of_range("a fixed array cannot be resized"); } }
		constexpr size_t get_index(std::u8string_view name) const { return detail::to_index(name, get_size()); }

		template <size_t index> auto& get_item() { return get_item(index); }
		template <size_t index> void get_item(Item& item) { get_item(index, item); }
		auto& get_item(size_t index) { return _items.at(rebase(index)); }
		void get_item(size_t index, Item& item) { item = _items.at(rebase(index)); }
		auto& get_item(std::u8string_view name) { return get_item(get_index(name)); }
		void get_item(std::u8string_view name, Item& item) { get_item(get_index(name), item); }

		template <size_t index> void set_item(const Item& item) { set_item(index, item); }
		template <size_t index> void set_item(Item&& item) { set_item(index, std::move(item)); }
		void set_item(size_t index, const Item& item) { _items[rebase(index)] = item; }
		void set_item(size_t index, Item&& item) { _items[rebase(index)] = std::move(item); }
		void set_item(std::u8string_view name, const Item& item) { set_item(get_index(name), item); }
		void set_item(std::u8string_view name, Item&& item) { set_item(get_index(name), std::move(item)); }

		void add_item(const Item&) { throw std::out_of_range("a fixed array cannot be appended to"); }
		void add_item(Item&&) { throw std::out_of_range("a fixed array cannot be appended to"); }
	};

	template <typename Item>
	class dynamic_array
	{
		std::vector<Item> _items;

		constexpr size_t rebase(size_t index) const  { return detail::rebase(index, _items.size()); }

	public:
		size_t get_size() const { return _items.size(); }
		void set_size(size_t size) { _items.resize(size); }
		size_t get_index(std::u8string_view name) const { return detail::to_index(name, get_size()); }

		template <size_t index> auto& get_item() { return get_item(index); }
		template <size_t index> void get_item(Item& item) { get_item(index, item); }
		auto& get_item(size_t index) { return _items.at(rebase(index)); }
		void get_item(size_t index, Item& item) { item = _items.at(rebase(index)); }
		auto& get_item(std::u8string_view name) { return get_item(get_index(name)); }
		void get_item(std::u8string_view name, Item& item) { get_item(get_index(name), item); }

		template <size_t index> void set_item(const Item& item) { set_item(index, item); }
		template <size_t index> void set_item(Item&& item) { set_item(index, std::move(item)); }
		void set_item(size_t index, const Item& item) { _items[rebase(index)] = item; }
		void set_item(size_t index, Item&& item) { _items[rebase(index)] = std::move(item); }
		void set_item(std::u8string_view name, const Item& item) { set_item(get_index(name), item); }
		void set_item(std::u8string_view name, Item&& item) { set_item(get_index(name), std::move(item)); }

		void add_item(const Item& item) { _items.push_back(item); }
		void add_item(Item&& item) { _items.emplace_back(std::move(item)); }
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

		static constexpr size_t rebase(size_t index) { return detail::rebase(index, _size); }

	public:
		template <size_t index>
		using item_type = std::remove_reference_t<decltype(std::get<detail::rebase_v<index>>(_items))>;

		constexpr size_t get_size() const { return _size; }
		constexpr void set_size(size_t size) { if (size != _size) { throw std::out_of_range("a fixed tuple cannot be resized"); } }
		constexpr size_t get_index(std::u8string_view name) const { return detail::to_index(name, get_size()); }

		template <size_t index> constexpr auto& get_item() { return detail::checked_tuple_get<index, tuple_t>(_items); };
		template <size_t index, typename Item> constexpr void get_item(Item& item) { item = detail::checked_tuple_get<index, tuple_t, Item>(_items); };
		template <typename Item> constexpr void get_item(size_t index, Item& item) { detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { item = elem; }); }
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

		constexpr auto& as_tuple() { return _items; }
		constexpr auto as_fixed_tuple() { return fixed_tuple<tuple_t>{ _items }; }
		constexpr auto& as_named_tuple() { return *this; }

	private:
		static constexpr size_t _size = std::tuple_size_v<tuple_t>;

		static constexpr size_t rebase(size_t index) { return detail::rebase(index, _size); }

	public:
		template <size_t index>
		using item_type = std::remove_reference_t<decltype(std::get<detail::rebase_v<index>>(_items))>;

		constexpr size_t get_size() const { return _size; }
		constexpr void set_size(size_t size) { if (size != _size) { throw std::out_of_range("a named tuple cannot be resized"); } }
		constexpr size_t get_index(std::u8string_view name) const
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

		template <size_t index> constexpr auto& get_item() { return detail::checked_tuple_get<index, tuple_t>(_items); };
		template <size_t index, typename Item> constexpr void get_item(Item& item) { item = detail::checked_tuple_get<index, tuple_t, Item>(_items); };
		template <typename Item> constexpr void get_item(size_t index, Item& item) { detail::visit<Item, tuple_t>(_items, index, [&](Item& elem) { item = elem; }); }
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
		constexpr auto as_tuple(std::array<T, N>&& arr, std::index_sequence<Is...>)
		{
			return std::make_tuple(T{ std::move(arr[Is]) }...);
		}

		template <class T, size_t N>
		constexpr auto as_tuple(std::array<T, N>&& arr)
		{
			return as_tuple(std::move(arr), std::make_index_sequence<N>{});
		}
	}

	template <typename... Items>
	constexpr auto to_fixed_tuple(Items&&... items)
	{
		return fixed_tuple<std::tuple<Items...>>{ {std::move(items)...} };
	}

	template <typename... Items>
	constexpr auto to_fixed_tuple(std::tuple<Items...>&& src)
	{
		return fixed_tuple<std::tuple<Items...>>{ std::move(src) };
	}

	template <typename Tuple>
	constexpr auto to_fixed_tuple(fixed_tuple<Tuple>&& src)
	{
		return std::move(src);
	}

	template <typename Tuple, typename Names>
	constexpr auto to_fixed_tuple(named_tuple<Names, Tuple>&& src)
	{
		return std::move(src.as_fixed_tuple());
	}
}
