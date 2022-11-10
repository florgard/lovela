#pragma once
#include "Token.h"

struct ParseException
{
	std::string message;
	Token token;

	ParseException(const Token& token);
	ParseException(const Token& token, std::string_view message);
};

struct UnexpectedTokenException : public ParseException
{
	UnexpectedTokenException(const Token& token);
	UnexpectedTokenException(const Token& token, Token::Type expected);
	UnexpectedTokenException(const Token& token, const std::set<Token::Type>& expected);

	template <size_t Size>
	constexpr UnexpectedTokenException(const Token& token, const static_set<Token::Type, Size>& expected) : ParseException(token)
	{
		std::ostringstream s;
		s << "Unexpected token " << to_string(token.type) << ", expected ";
		bool first = true;
		for (const auto& type : expected.data)
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
	InvalidCurrentTokenException(const Token& token);
};
