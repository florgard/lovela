#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <typeinfo>

namespace lovela
{
	struct streams
	{
		std::wistream& in = std::wcin;
		std::wostream& out = std::wcout;
		std::wostream& err = std::wcerr;

		template <int select> auto& get() { static_assert(false, "Invalid select index"); return *this; };
		template <> auto& get<1>() { return in; }
		template <> auto& get<2>() { return out; }
		template <> auto& get<3>() { return err; }
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

		template <int select> auto& get() { static_assert(false, "Invalid select index"); return *this; };
		template <> auto& get<1>() { return message; }
		template <> auto& get<2>() { return type; }
		template <> auto& get<3>() { return code; }
		template <> auto& get<4>() { return inner; }
	};

	struct context
	{
		std::vector<std::string> parameters;
		streams streams;
		error error;

		template <int select> auto& get() { static_assert(false, "Invalid select index"); return *this; };
		template <> auto& get<1>() { return parameters; }
		template <> auto& get<2>() { return streams; }
		template <> auto& get<3>() { return error; }
	};

	struct None
	{
	};
}
