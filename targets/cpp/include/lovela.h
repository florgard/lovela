#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <exception>

namespace lovela
{
	struct streams
	{
		std::wistream& in = std::wcin;
		std::wostream& out = std::wcout;
		std::wostream& err = std::wcerr;
	};

	class error : public std::exception
	{
		std::string type;
		int code{};

	public:
		error() noexcept
			: std::exception()
		{
		}
		error(const char* const message) noexcept
			: std::exception(message)
		{
		}
		error(const char* const message, const char* const type, int code) noexcept
			: std::exception(message)
			, type(type)
			, code(code)
		{
		}
	};

	struct context
	{
		std::vector<std::string> parameters;
		streams streams;
		error error;
	};

	struct None
	{
	};
}
