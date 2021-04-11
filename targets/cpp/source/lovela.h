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
		template <size_t index>
		struct rebase_t
		{
			static_assert(index > 0, "index out of range");
			static constexpr size_t value = index - 1;
		};

		template <size_t index>
		static constexpr auto rebase_v = rebase_t<index>::value;

		constexpr size_t rebase(size_t index, size_t size)
		{
			if (!index || index > size)
			{
				throw std::out_of_range("index out of range");
			}

			return index - 1;
		}

		template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
		template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;
	}

	template <typename Item>
	class fixed_array
	{
		std::vector<Item> _items;

		constexpr size_t rebase(size_t index) const { return detail::rebase(index, _items.size()); }

	public:
		fixed_array(size_t size) noexcept : _items(size) {}
		~fixed_array() noexcept = default;
		fixed_array(const fixed_array& src) noexcept = default;
		fixed_array(fixed_array&& src) noexcept = default;
		fixed_array& operator=(const fixed_array& src) noexcept = default;
		fixed_array& operator=(fixed_array&& src) noexcept = default;

		size_t get_size() const { return _items.size(); }
		void set_size(size_t size) { size == get_size() || (throw std::out_of_range("a fixed sized array cannot be resized"), false); }
		void get_item(size_t index, Item& item) { item = _items.at(rebase(index)); }
		void set_item(size_t index, const Item& item) { _items[rebase(index)] = item; }
		void set_item(size_t index, Item&& item) { _items[rebase(index)] = std::move(item); }
		void add_item(const Item&) { throw std::out_of_range("a fixed sized array cannot be appended to"); }
		void add_item(Item&&) { throw std::out_of_range("a fixed sized array cannot be appended to"); }
	};

	template <typename Item>
	class dynamic_array
	{
		std::vector<Item> _items;

		constexpr size_t rebase(size_t index) const  { return detail::rebase(index, _items.size()); }

	public:
		size_t get_size() const { return _items.size(); }
		void set_size(size_t size) { _items.resize(size); }
		void get_item(size_t index, Item& item) { item = _items.at(rebase(index)); }
		void set_item(size_t index, const Item& item) { _items[rebase(index)] = item; }
		void set_item(size_t index, Item&& item) { _items[rebase(index)] = std::move(item); }
		void add_item(const Item& item) { _items.push_back(item); }
		void add_item(Item&& item) { _items.emplace_back(std::move(item)); }
	};

#define INDEXED_TUPLE_GUARD_BEGIN(index_) index_; \
	if constexpr (index_ < _size) { \
	if constexpr (std::is_same_v<std::remove_cvref_t<Item>, std::remove_cvref_t<decltype(std::get<index_>(_items))>>) {
#define INDEXED_TUPLE_GUARD_END } else throw std::invalid_argument("indexed tuple: invalid access type"); }
#define INDEXED_TUPLE_GUARD_END_RANGE INDEXED_TUPLE_GUARD_END else throw std::out_of_range("index out of range");

#define INDEXED_TUPLE_SAFE_GET_ITEM(index_, item_) \
	INDEXED_TUPLE_GUARD_BEGIN(index_); item_ = std::get<index_>(_items); INDEXED_TUPLE_GUARD_END_RANGE;

#define INDEXED_TUPLE_SAFE_SET_ITEM(index_, item_) \
	INDEXED_TUPLE_GUARD_BEGIN(index_); std::get<index_>(_items) = item_; INDEXED_TUPLE_GUARD_END_RANGE;

#define INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(index_, item_) \
	INDEXED_TUPLE_GUARD_BEGIN(index_); item_ = std::get<index_>(_items); INDEXED_TUPLE_GUARD_END;

#define INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(index_, item_) \
	INDEXED_TUPLE_GUARD_BEGIN(index_); std::get<index_>(_items) = item_; INDEXED_TUPLE_GUARD_END;

	template <typename... Types>
	class indexed_tuple
	{
		std::tuple<Types...> _items;

		static constexpr size_t _size = std::tuple_size_v<std::tuple<Types...>>;
		static_assert(_size <= 10, "insufficient number of handled indices in indexed_tuple::get_item(size_t, Item&)");

		constexpr size_t rebase(size_t index) const { return detail::rebase(index, _size); }

	public:
		constexpr size_t get_size() const
		{
			return _size;
		}

		constexpr void set_size(size_t size)
		{
			if (size != _size)
			{
				throw std::out_of_range("an indexed tuple cannot be resized");
			}
		}

		template <typename Item>
		constexpr void get_item(size_t index, Item& item)
		{
			switch (rebase(index))
			{
			case 0: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(0, item); break;
			case 1: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(1, item); break;
			case 2: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(2, item); break;
			case 3: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(3, item); break;
			case 4: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(4, item); break;
			case 5: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(5, item); break;
			case 6: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(6, item); break;
			case 7: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(7, item); break;
			case 8: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(8, item); break;
			case 9: INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE(9, item); break;
			default:
				throw std::out_of_range("index out of range");
			}
		}

		template <size_t index, typename Item>
		constexpr void get_item(Item& item)
		{
			INDEXED_TUPLE_SAFE_GET_ITEM(detail::rebase_v<index>, item);
		};

		template <typename Item>
		constexpr void set_item(size_t index, const Item& item)
		{
			switch (rebase(index))
			{
			case 0: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(0, item); break;
			case 1: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(1, item); break;
			case 2: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(2, item); break;
			case 3: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(3, item); break;
			case 4: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(4, item); break;
			case 5: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(5, item); break;
			case 6: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(6, item); break;
			case 7: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(7, item); break;
			case 8: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(8, item); break;
			case 9: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(9, item); break;
			default:
				throw std::out_of_range("index out of range");
			}
		}

		template <typename Item>
		constexpr void set_item(size_t index, Item&& item)
		{
			switch (rebase(index))
			{
			case 0: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(0, std::move(item)); break;
			case 1: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(1, std::move(item)); break;
			case 2: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(2, std::move(item)); break;
			case 3: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(3, std::move(item)); break;
			case 4: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(4, std::move(item)); break;
			case 5: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(5, std::move(item)); break;
			case 6: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(6, std::move(item)); break;
			case 7: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(7, std::move(item)); break;
			case 8: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(8, std::move(item)); break;
			case 9: INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE(9, std::move(item)); break;
			default:
				throw std::out_of_range("index out of range");
			}
		}

		template <size_t index, typename Item>
		constexpr void set_item(const Item& item)
		{
			INDEXED_TUPLE_SAFE_SET_ITEM(detail::rebase_v<index>, item);
		};

		template <size_t index, typename Item>
		constexpr void set_item(Item&& item)
		{
			INDEXED_TUPLE_SAFE_SET_ITEM(detail::rebase_v<index>, std::move(item));
		};

		template <typename Item>
		constexpr void add_item(const Item&)
		{
			throw std::out_of_range("an indexed tuple cannot be appended to");
		}

		template <typename Item>
		constexpr void add_item(Item&&)
		{
			throw std::out_of_range("an indexed tuple cannot be appended to");
		}
	};

#undef INDEXED_TUPLE_SAFE_SET_ITEM_NO_RANGE
#undef INDEXED_TUPLE_SAFE_GET_ITEM_NO_RANGE
#undef INDEXED_TUPLE_SAFE_SET_ITEM
#undef INDEXED_TUPLE_SAFE_GET_ITEM
#undef INDEXED_TUPLE_GUARD_END_RANGE
#undef INDEXED_TUPLE_GUARD_END
#undef INDEXED_TUPLE_GUARD_BEGIN

	template <size_t NamedTupleTypeOrdinal>
	struct named_tuple_names
	{
		static constexpr std::array<std::u8string_view, 0> names{};
	};

	template <size_t NamedTupleTypeOrdinal, typename... Types>
	class named_tuple
	{
		indexed_tuple<Types...> _tuple;

	public:
		constexpr size_t get_size() const
		{
			return _tuple.get_size();
		}

		constexpr void set_size(size_t size)
		{
			_tuple.set_size(size);
		}

		template <typename Item>
		constexpr void get_item(const std::u8string& name, Item& item)
		{
			static constexpr auto names = named_tuple_names<NamedTupleTypeOrdinal>::names;

			auto iter = std::find(names.begin(), names.end(), name);
			if (iter == names.end())
			{
				throw std::out_of_range("named tuple: the name doesn't exist");
			}

			_tuple.get_item(std::distance(names.begin(), iter) + 1, item);
		}

		template <typename Item>
		constexpr void get_item(size_t index, Item& item)
		{
			_tuple.get_item(index, item);
		}

		template <size_t index, typename Item>
		constexpr void get_item(Item& item)
		{
			_tuple.get_item<index, Item>(item);
		};

		template <typename Item>
		constexpr void set_item(const std::u8string& name, const Item& item)
		{
			static constexpr auto names = named_tuple_names<NamedTupleTypeOrdinal>::names;

			auto iter = std::find(names.begin(), names.end(), name);
			if (iter == names.end())
			{
				throw std::out_of_range("named tuple: the name doesn't exist");
			}

			_tuple.set_item(std::distance(names.begin(), iter) + 1, item);
		}

		template <typename Item>
		constexpr void set_item(const std::u8string& name, Item&& item)
		{
			static constexpr auto names = named_tuple_names<NamedTupleTypeOrdinal>::names;

			auto iter = std::find(names.begin(), names.end(), name);
			if (iter == names.end())
			{
				throw std::out_of_range("named tuple: the name doesn't exist");
			}

			_tuple.set_item(std::distance(names.begin(), iter) + 1, std::move(item));
		}

		template <typename Item>
		constexpr void set_item(size_t index, const Item& item)
		{
			_tuple.set_item<Item>(index, item);
		};

		template <typename Item>
		constexpr void set_item(size_t index, Item&& item)
		{
			_tuple.set_item<Item>(index, std::move(item));
		};

		template <size_t index, typename Item>
		constexpr void set_item(const Item& item)
		{
			_tuple.set_item<index, Item>(item);
		};

		template <size_t index, typename Item>
		constexpr void set_item(Item&& item)
		{
			_tuple.set_item<index, Item>(std::move(item));
		};

		template <typename Item>
		constexpr void add_item(const Item& item)
		{
			_tuple.add_item<Item>(item);
		}

		template <typename Item>
		constexpr void add_item(Item&& item)
		{
			_tuple.add_item<Item>(std::move(item));
		}
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
