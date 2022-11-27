#pragma once
#include "Token.h"

struct ParseException : public std::exception
{
	std::string message;
	Token token;

	ParseException(const Token& token) noexcept;
	ParseException(const Token& token, std::string_view message) noexcept;

	const char* what() const noexcept override
	{
		return message.c_str();
	}
};

struct UnexpectedTokenException : public ParseException
{
	UnexpectedTokenException(const Token& token) noexcept;
	UnexpectedTokenException(const Token& token, Token::Type expectedType) noexcept;
	UnexpectedTokenException(const Token& token, Token::Type expectedType, std::string_view expectedValue) noexcept;
	UnexpectedTokenException(const Token& token, const std::set<Token::Type>& expectedTypes) noexcept;

	template <size_t Size>
	constexpr UnexpectedTokenException(const Token& token, const static_set<Token::Type, Size>& expectedTypes) noexcept : ParseException(token)
	{
		std::ostringstream s;
		s << "Unexpected token " << to_string(token.type) << ", expected ";
		bool first = true;
		for (const auto& type : expectedTypes.data)
		{
			if (!first)
			{
				s << ", ";
			}
			first = false;
			s << to_string(type);
		}
		message = s.str();
	}
};

struct InvalidCurrentTokenException : public ParseException
{
	InvalidCurrentTokenException(const Token& token) noexcept;
};

struct ErrorTokenException : public ParseException
{
	ErrorTokenException(const Token& token) noexcept;
};

struct NoTokenException : public ParseException
{
	NoTokenException() noexcept;
};
