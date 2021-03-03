#pragma once
#include "Token.h"

struct ParseException
{
	std::wstring message;
	const Token& token;

	ParseException(const Token& token);
	ParseException(const Token& token, std::string_view message);
	ParseException(const Token& token, std::wstring_view message);
};

struct UnexpectedTokenException : public ParseException
{
	UnexpectedTokenException(const Token& token);
	UnexpectedTokenException(const Token& token, Token::Type expected);
	UnexpectedTokenException(const Token& token, const std::vector<Token::Type>& expected);
};

struct InvalidCurrentTokenException : public ParseException
{
	InvalidCurrentTokenException(const Token& token);
};
