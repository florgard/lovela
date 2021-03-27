#pragma once
#define LOVELA
#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <typeinfo>
#include <cstdint>

using float32_t = float;
using float64_t = double;

namespace lovela
{
	struct streams
	{
		std::wistream& in = std::wcin;
		std::wostream& out = std::wcout;
		std::wostream& err = std::wcerr;

		template <int index> auto& select() { static_assert(false, "invalid select index"); return *this; };
		template <> auto& select<1>() { return in; }
		template <> auto& select<2>() { return out; }
		template <> auto& select<3>() { return err; }
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
