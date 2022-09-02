#pragma once
#define LOVELA
#include <string>
#include <array>
#include <vector>
#include <map>
#include <iostream>
#include <exception>
#include <cstdint>
#include <variant>
#include <algorithm>
#include "utfcpp/utf8.h"
#include "lovela-types.h"

namespace lovela
{
	namespace detail
	{
		template<class... Functions> struct overloaded : Functions... { using Functions::operator()...; };
		template<class... Functions> overloaded(Functions...)->overloaded<Functions...>;
	}

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

		template <int index> auto& select() = delete;
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

		template <int index> auto& select() = delete;
		// Index 1 is reserved for function input.
		template <> auto& select<2>() { return error; }
		template <> auto& select<3>() { return streams; }
		template <> auto& select<4>() { return parameters; }
	};

	None main(context& context, None in);
}
