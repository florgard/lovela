#pragma once
#define LOVELA
#include <string>
#include <vector>
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
	template <typename Item>
	class fixed_size_array
	{
		std::vector<Item> items;

		size_t check(size_t index)
		{
			if (!index || index > items.size())
			{
				throw std::out_of_range("a fixed sized array index was out of range");
			}

			return index - 1;
		}

	public:
		fixed_size_array(size_t size) noexcept : items(size) {}
		~fixed_size_array() noexcept = default;
		fixed_size_array(const fixed_size_array& src) noexcept = default;
		fixed_size_array(fixed_size_array&& src) noexcept = default;
		fixed_size_array& operator=(const fixed_size_array& src) noexcept = default;
		fixed_size_array& operator=(fixed_size_array&& src) noexcept = default;

		size_t get_size() const { return items.size(); }
		void set_size(size_t size) { size == get_size() || (throw std::out_of_range("a fixed sized array cannot be resized"), false); }
		const Item& get_item(size_t index) { return items.at(check(index)); }
		void set_item(size_t index, const Item& item) { items[check(index)] = item; }
		void set_item(size_t index, Item&& item) { items[check(index)] = item; }
		void add_item(const Item&) { throw std::out_of_range("a fixed sized array cannot be appended to"); }
		void add_item(Item&&) { throw std::out_of_range("a fixed sized array cannot be appended to"); }
	};

	template <typename Item>
	class dynamic_array
	{
		std::vector<Item> items;

		size_t check(size_t index)
		{
			if (!index || index > items.size())
			{
				throw std::out_of_range("a dynamic array index was out of range");
			}

			return index - 1;
		}

	public:
		size_t get_size() const { return items.size(); }
		void set_size(size_t size) { items.resize(size); }
		const Item& get_item(size_t index) { return items.at(check(index)); }
		void set_item(size_t index, const Item& item) { items[check(index)] = item; }
		void set_item(size_t index, Item&& item) { items[check(index)] = item; }
		void add_item(const Item& item) { items.push_back(item); }
		void add_item(Item&& item) { items.emplace_back(item); }
	};

	template <typename... Types>
	class named_tuple
	{
		std::tuple<Types...> items;
		std::vector<std::string> names;

	public:
		constexpr size_t get_size() const
		{
			return std::tuple_size_v<decltype(items)>;
		}

		constexpr void set_size(size_t size)
		{
			size == get_size() || (throw std::out_of_range("a fixed sized array cannot be resized"), false);
		}

		template <int index>
		constexpr const auto& get_item()
		{
			static_assert(index > 0, "invalid index");
			return std::get<index - 1>(items);
		};

		template <int index>
		constexpr void set_item(const auto& item)
		{
			static_assert(index > 0, "invalid index");
			std::get<index - 1>(items) = item;
		};

		template <int index>
		constexpr void set_item(auto&& item)
		{
			static_assert(index > 0, "invalid index");
			std::get<index - 1>(items) = item;
		};

		constexpr void add_item(const auto&) { throw std::out_of_range("a named tuple cannot be appended to"); }
		//constexpr void add_item(auto&&) { throw std::out_of_range("a named tuple cannot be appended to"); }
	};

	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

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
			std::visit(overloaded{
				[&](auto&) {},
				[&](std::istream* stream) { *stream >> word; },
				[&](std::wistream* stream) { *stream >> buffer; to_string(word, buffer); }
				}, io);
		}

		void read_line(std::string& line)
		{
			std::visit(overloaded{
				[&](auto&) {},
				[&](std::istream* stream) { std::getline(*stream, line); },
				[&](std::wistream* stream) { std::getline(*stream, buffer); to_string(line, buffer); }
				}, io);
		}

		void read_all(std::string& text)
		{
			std::visit(overloaded{
				[&](auto&) {},
				[&](std::istream* stream) { text.assign(std::istreambuf_iterator<char>(*stream), {}); },
				[&](std::wistream* stream) { buffer.assign(std::istreambuf_iterator<wchar_t>(*stream), {}); to_string(text, buffer); }
				}, io);
		}

		void read_bytes(std::string& bytes, size_t count)
		{
			std::visit(overloaded{
				[&](auto&) {},
				[&](std::istream* stream) { bytes.resize(count); stream->read(&bytes[0], count); }
				}, io);
		}

		void write(std::string_view text)
		{
			std::visit(overloaded{
				[&](auto) {},
				[&](std::ostream* stream) { *stream << text; },
				[&](std::wostream* stream) { to_wstring(buffer, text); *stream << buffer; }
				}, io);
		}

		void write_line(std::string_view line)
		{
			std::visit(overloaded{
				[&](auto&) {},
				[&](std::ostream* stream) { *stream << line << '\n'; },
				[&](std::wostream* stream) { to_wstring(buffer, line); *stream << buffer << '\n'; }
				}, io);
		}

		void write_bytes(std::string_view bytes, size_t count)
		{
			std::visit(overloaded{
				[&](auto&) {},
				[&](std::ostream* stream) { stream->write(bytes.data(), count); }
				}, io);
		}
	};

	class streams
	{
#if defined(_MSC_VER)
		std::vector<stream> items{ {}, { std::wcin }, { std::wcout }, { std::wcerr } };
#else
		std::vector<stream> items{ {}, { std::cin }, { std::cout }, { std::cerr } };
#endif

	public:
		stream& select(size_t index)
		{
			return index < items.size() ? items.at(index) : items.front();
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
