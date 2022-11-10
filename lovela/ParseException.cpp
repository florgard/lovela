#include "pch.h"
#include "ParseException.h"

ParseException::ParseException(const Token& token) : token(token)
{
}

ParseException::ParseException(const Token& token, std::string_view message) : token(token), message(message)
{
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token) : ParseException(token)
{
	std::ostringstream s;
	s << "Unexpected token " << to_string(token.type);
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, Token::Type expected) : ParseException(token)
{
	std::ostringstream s;
	s << "Unexpected token " << to_string(token.type) << ", expected " << to_string(expected);
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, const std::set<Token::Type>& expected) : ParseException(token)
{
	std::ostringstream s;
	s << "Unexpected token " << to_string(token.type) << ", expected ";
	bool first = true;
	for (const auto& type : expected)
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

InvalidCurrentTokenException::InvalidCurrentTokenException(const Token& token) : ParseException(token)
{
	std::ostringstream s;
	s << "Invalid current token " << to_string(token.type);
	message = s.str();
}
