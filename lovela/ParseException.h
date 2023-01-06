#pragma once
#include "Token.h"

template <> struct fmt::formatter<Token::Type> : formatter<string_view>
{
	auto format(Token::Type t, format_context& ctx) const
	{
		return formatter<string_view>::format(::to_string(t), ctx);
	}
};

struct ParseException : public std::exception
{
	std::string message;
	Token token;

	ParseException(Token const& token) noexcept;
	ParseException(Token const& token, std::string&& message) noexcept;

	const char* what() const noexcept override
	{
		return message.c_str();
	}
};

struct UnexpectedTokenException : public ParseException
{
	UnexpectedTokenException(Token const& token) noexcept;
	UnexpectedTokenException(Token const& token, std::string&& message) noexcept;
	UnexpectedTokenException(Token const& token, Token::Type expectedType) noexcept;
	UnexpectedTokenException(Token const& token, Token::Type expectedType, std::string_view expectedValue) noexcept;

	template <std::ranges::range RangeT>
	UnexpectedTokenException(Token const& token, const RangeT& expectedTypes) noexcept
		: UnexpectedTokenException(token, fmt::format("Expected {}.", fmt::join(expectedTypes.begin(), expectedTypes.end(), ", ")))
	{
	}
};

struct UnexpectedTokenAfterException : public UnexpectedTokenException
{
	UnexpectedTokenAfterException(Token const& token, Token::Type preceedingType) noexcept;
	UnexpectedTokenAfterException(Token const& token, Token::Type preceedingType, std::string&& message) noexcept;
	UnexpectedTokenAfterException(Token const& token, Token const& preceedingToken) noexcept;
	UnexpectedTokenAfterException(Token const& token, Token const& preceedingToken, std::string&& message) noexcept;
};

struct InvalidCurrentTokenException : public ParseException
{
	InvalidCurrentTokenException(Token const& token) noexcept;
};

struct ErrorTokenException : public ParseException
{
	ErrorTokenException(Token const& token) noexcept;
};

struct MissingTokenException : public ParseException
{
	MissingTokenException() noexcept;
};
