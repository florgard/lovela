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

UnexpectedTokenException::UnexpectedTokenException(const Token& token, Token::Type expectedTypes) : ParseException(token)
{
	std::ostringstream s;
	s << "Unexpected token " << to_string(token.type) << ", expected " << to_string(expectedTypes);
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, Token::Type expectedType, std::string_view expectedValue) : ParseException(token)
{
	std::ostringstream s;
	s << "Unexpected token " << to_string(token.type) << " = \"" << token.value << "\", expected " << to_string(expectedType) << " = \"" << expectedValue << "\"";
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, const std::set<Token::Type>& expectedTypes) : ParseException(token)
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

InvalidCurrentTokenException::InvalidCurrentTokenException(const Token& token) : ParseException(token)
{
	std::ostringstream s;
	s << "Invalid current token " << to_string(token.type);
	message = s.str();
}
