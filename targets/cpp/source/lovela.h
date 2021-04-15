#pragma once
#define LOVELA
#include <string>
#include <array>
#include <vector>
#include <map>
#include <iostream>
#include <exception>
#include <typeinfo>
#include <cstdint>
#include <variant>
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

		template<class... Functions> struct overloaded : Functions... { using Functions::operator()...; };
		template<class... Functions> overloaded(Functions...)->overloaded<Functions...>;
	}

	template <typename Item, size_t _size>
	class fixed_array
	{
		std::array<Item, _size> _items;

		static constexpr size_t rebase(size_t index) { return detail::rebase(index, _size); }

	public:
		constexpr size_t get_size() const { return _size; }
		constexpr void set_size(size_t size) { if (size != _size) { throw std::out_of_range("a fixed array cannot be resized"); } }
		constexpr size_t get_index(std::u8string_view name) const { return detail::to_index(name, get_size()); }

		template <size_t index> void get_item(Item& item) { get_item(index, item); }
		void get_item(size_t index, Item& item) { item = _items.at(rebase(index)); }
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

		template <size_t index> void get_item(Item& item) { get_item(index, item); }
		void get_item(size_t index, Item& item) { item = _items.at(rebase(index)); }
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

	template <typename... Types>
	struct fixed_tuple
	{
		using items_t = std::tuple<Types...>;
		using tuple_t = items_t;
		tuple_t _items;

		tuple_t& as_tuple() { return _items; }

	private:
		static constexpr size_t _size = std::tuple_size_v<std::tuple<Types...>>;

		static constexpr size_t rebase(size_t index) { return detail::rebase(index, _size); }

		template <typename Item, size_t visitIndex = 0>
		void visit(size_t index, auto&& visitor)
		{
			if constexpr (!visitIndex)
			{
				index = rebase(index);
			}

			if constexpr (visitIndex >= _size)
			{
				visitor;
				throw std::out_of_range("index out of range");
			}
			else if (index == visitIndex)
			{
				if constexpr (detail::is_same_tuple_element<Item, items_t, visitIndex>)
				{
					visitor(std::get<visitIndex>(_items));
				}
				else
				{
					throw std::invalid_argument("invalid access type");
				}
			}
			else
			{
				visit<Item, visitIndex + 1>(index, visitor);
			}
		}

		template <size_t index, typename Item>
		constexpr auto& checked_get()
		{
			static constexpr size_t rebased = detail::rebase_v<index>;
			static_assert(rebased < _size, "index out of bounds");
			static_assert(detail::is_same_tuple_element<Item, items_t, rebased>, "invalid access type");
			return std::get<rebased>(_items);
		};

	public:
		constexpr size_t get_size() const { return _size; }
		constexpr void set_size(size_t size) { if (size != _size) { throw std::out_of_range("an indexed tuple cannot be resized"); } }
		constexpr size_t get_index(std::u8string_view name) const { return detail::to_index(name, get_size()); }

		template <size_t index, typename Item> constexpr void get_item(Item& item) { item = checked_get<index, Item>(); };
		template <typename Item> constexpr void get_item(size_t index, Item& item) { visit<Item>(index, [&](Item& elem) { item = elem; }); }
		template <typename Item> void get_item(std::u8string_view name, Item& item) { get_item(get_index(name), item); }

		template <size_t index, typename Item> constexpr void set_item(const Item& item) { checked_get<index, Item>() = item; };
		template <size_t index, typename Item> constexpr void set_item(Item&& item) { checked_get<index, Item>() = std::move(item); };
		template <typename Item> constexpr void set_item(size_t index, const Item& item) { visit<Item>(index, [&](Item& elem) { elem = item; }); }
		template <typename Item> constexpr void set_item(size_t index, Item&& item) { visit<Item>(index, [&](Item& elem) { elem = std::move(item); }); }
		template <typename Item> void set_item(std::u8string_view name, const Item& item) { set_item(get_index(name), item); }
		template <typename Item> void set_item(std::u8string_view name, Item&& item) { set_item(get_index(name), std::move(item)); }

		template <typename Item> constexpr void add_item(const Item&) { throw std::out_of_range("an indexed tuple cannot be appended to"); }
		template <typename Item> constexpr void add_item(Item&&) { throw std::out_of_range("an indexed tuple cannot be appended to"); }
	};

	template <typename Names, typename... Types>
	struct named_tuple
	{
		using fixed_tuple_t = fixed_tuple<Types...>;
		using tuple_t = fixed_tuple_t::tuple_t;

		fixed_tuple_t _tuple;

		tuple_t& as_tuple() { return _tuple._items; }

		constexpr size_t get_size() const { return _tuple.get_size(); }
		constexpr void set_size(size_t size) { _tuple.set_size(size); }
		constexpr size_t get_index(std::u8string_view name) const
		{
			static constexpr auto names = Names::names;

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

		template <size_t index, typename Item> constexpr void get_item(Item& item) { _tuple.get_item<index>(item); };
		template <typename Item> constexpr void get_item(size_t index, Item& item) { _tuple.get_item(index, item); }
		template <typename Item> constexpr void get_item(std::u8string_view name, Item& item) { get_item(get_index(name), item); }

		template <size_t index, typename Item> constexpr void set_item(const Item& item) { _tuple.set_item<index>(item); };
		template <size_t index, typename Item> constexpr void set_item(Item&& item) { _tuple.set_item<index>(std::move(item)); };
		template <typename Item> constexpr void set_item(size_t index, const Item& item) { _tuple.set_item(index, item); };
		template <typename Item> constexpr void set_item(size_t index, Item&& item) { _tuple.set_item(index, std::move(item)); };
		template <typename Item> constexpr void set_item(std::u8string_view name, const Item& item) { set_item(get_index(name), item); }
		template <typename Item> constexpr void set_item(std::u8string_view name, Item&& item) { set_item(get_index(name), std::move(item)); }

		template <typename Item> constexpr void add_item(const Item& item) { _tuple.add_item<Item>(item); }
		template <typename Item> constexpr void add_item(Item&& item) { _tuple.add_item<Item>(std::move(item)); }
	};

	class stream
	{
		std::variant<std::monostate, std::istream*, std::ostream*, std::wistream*, std::wostream*> io;
		std::wstring buffer;
		
		static void to_string(std::string& str, std::wstring_view value)
		{
			str.resize(value.length());
			std::transform(value.begin(), value.end(), str.begin(), [](auto elem) { return static_cast<char>(elem); });
		}

		static void to_wstring(std::wstring& str, std::string_view value)
		{
			str.resize(value.length());

			if constexpr (sizeof(wchar_t) == 2)
			{
				utf8::utf8to16(value.begin(), value.end(), str.begin());
			}
			else
			{
				utf8::utf8to32(value.begin(), value.end(), str.begin());
			}
		}

	public:
		stream() noexcept = default;
		~stream() noexcept = default;
		stream(std::istream& stream) noexcept : io(&stream) {}
		stream(std::ostream& stream) noexcept : io(&stream) {}
		stream(std::wistream& stream) noexcept : io(&stream) { buffer.reserve(4096); }
		stream(std::wostream& stream) noexcept : io(&stream) { buffer.reserve(4096); }
		stream(const stream& src) noexcept = default;
		stream(stream&& src) noexcept = default;
		stream& operator=(const stream& src) noexcept = default;
		stream& operator=(stream&& src) noexcept = default;

		void read_word(std::string& word)
		{
			std::visit(detail::overloaded{
				[&](auto&) {},
				[&](std::istream* stream) { *stream >> word; },
				[&](std::wistream* stream) { *stream >> buffer; to_string(word, buffer); }
				}, io);
		}

		void read_line(std::string& line)
		{
			std::visit(detail::overloaded{
				[&](auto&) {},
				[&](std::istream* stream) { std::getline(*stream, line); },
				[&](std::wistream* stream) { std::getline(*stream, buffer); to_string(line, buffer); }
				}, io);
		}

		void read_all(std::string& text)
		{
			std::visit(detail::overloaded{
				[&](auto&) {},
				[&](std::istream* stream) { text.assign(std::istreambuf_iterator<char>(*stream), {}); },
				[&](std::wistream* stream) { buffer.assign(std::istreambuf_iterator<wchar_t>(*stream), {}); to_string(text, buffer); }
				}, io);
		}

		void read_bytes(std::string& bytes, size_t count)
		{
			std::visit(detail::overloaded{
				[&](auto&) {},
				[&](std::istream* stream) { bytes.resize(count); stream->read(&bytes[0], count); }
				}, io);
		}

		void write(std::string_view text)
		{
			std::visit(detail::overloaded{
				[&](auto) {},
				[&](std::ostream* stream) { *stream << text; },
				[&](std::wostream* stream) { to_wstring(buffer, text); *stream << buffer; }
				}, io);
		}

		void write_line(std::string_view line)
		{
			std::visit(detail::overloaded{
				[&](auto&) {},
				[&](std::ostream* stream) { *stream << line << '\n'; },
				[&](std::wostream* stream) { to_wstring(buffer, line); *stream << buffer << '\n'; }
				}, io);
		}

		void write_bytes(std::string_view bytes, size_t count)
		{
			std::visit(detail::overloaded{
				[&](auto&) {},
				[&](std::ostream* stream) { stream->write(bytes.data(), count); }
				}, io);
		}
	};

	class streams
	{
#if defined(_MSC_VER)
		std::vector<stream> _items{ {}, { std::wcin }, { std::wcout }, { std::wcerr } };
#else
		std::vector<stream> _items{ {}, { std::cin }, { std::cout }, { std::cerr } };
#endif

	public:
		stream& select(size_t index)
		{
			return index < _items.size() ? _items.at(index) : _items.front();
		}
	};

	class error : public std::exception
	{
	public:
		std::string message;
		std::string type;
		int code{};
		std::exception inner;

		error() noexcept
			: std::exception()
		{
		}
		error(const char* const message, int code = 0) noexcept
			: std::exception(message)
			, message(message)
			, code(code)
		{
		}
		error(const char* const message, const char* const type, int code = 0) noexcept
			: std::exception(message)
			, message(message)
			, type(type)
			, code(code)
		{
		}
		error(const char* const message, const char* const type, const std::exception& inner, int code = 0) noexcept
			: std::exception(message)
			, message(message)
			, type(type)
			, code(code)
			, inner(inner)
		{
		}

		template <typename T>
		static error make_error(const T& exception, int code = 0)
		{
			static_assert(std::is_base_of_v<std::exception, T>);
			const std::exception& ex = exception;
			return error(ex.what(), typeid(T).name(), ex, code);
		}

		template <int index> auto& select() { static_assert(false, "invalid select index"); return *this; };
		template <> auto& select<1>() { return message; }
		template <> auto& select<2>() { return type; }
		template <> auto& select<3>() { return code; }
		template <> auto& select<4>() { return inner; }
	};

	struct context
	{
		error error;
		streams streams;
		std::vector<std::string> parameters;

		template <int index> auto& select() { static_assert(false, "invalid select index"); return *this; };
		// Index 1 is reserved for function input.
		template <> auto& select<2>() { return error; }
		template <> auto& select<3>() { return streams; }
		template <> auto& select<4>() { return parameters; }
	};

	struct None
	{
	};

	None main(context& context, None in);
}
