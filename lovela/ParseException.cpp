#include "pch.h"
#include "ParseException.h"

ParseException::ParseException(const Token& token) noexcept
	: token(token)
{
}

ParseException::ParseException(const Token& token, std::string_view message) noexcept
	: token(token)
	, message(message)
{
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token) noexcept
	: ParseException(token)
{
	std::ostringstream s;
	s << "Unexpected token " << to_string(token.type);
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, Token::Type expectedTypes) noexcept
	: ParseException(token)
{
	std::ostringstream s;
	s << "Unexpected token " << to_string(token.type) << ", expected " << to_string(expectedTypes);
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, Token::Type expectedType, std::string_view expectedValue) noexcept
	: ParseException(token)
{
	std::ostringstream s;
	s << "Unexpected token " << to_string(token.type) << " = \"" << token.value << "\", expected " << to_string(expectedType) << " = \"" << expectedValue << "\"";
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, const std::set<Token::Type>& expectedTypes) noexcept
	: ParseException(token)
{
	std::ostringstream s;
	s << "Unexpected token " << to_string(token.type) << ", expected ";
	bool first = true;
	for (const auto& type : expectedTypes)
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

InvalidCurrentTokenException::InvalidCurrentTokenException(const Token& token) noexcept
	: ParseException(token)
{
	std::ostringstream s;
	s << "Invalid current token " << to_string(token.type);
	message = s.str();
}

ErrorTokenException::ErrorTokenException(const Token& token) noexcept
	: ParseException(token)
{
	std::ostringstream s;
	s << "Error token encountered. Message: " << token.error.message;
	message = s.str();
}

NoTokenException::NoTokenException() noexcept
	: ParseException({})
{
	message = "Unexpected end of token stream.";
}
