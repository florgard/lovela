#include "pch.h"
#include "ParseException.h"

ParseException::ParseException(const Token& token) : token(token)
{
}

ParseException::ParseException(const Token& token, std::string_view message) : token(token), message(ToWString(message))
{
}

ParseException::ParseException(const Token& token, std::wstring_view message) : token(token), message(message)
{
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token) : ParseException(token)
{
	std::wostringstream s;
	s << "Unexpected token " << ToWString(token.type);
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, Token::Type expected) : ParseException(token)
{
	std::wostringstream s;
	s << "Unexpected token " << ToWString(token.type) << ", expected " << ToWString(expected);
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, const std::set<Token::Type>& expected) : ParseException(token)
{
	std::wostringstream s;
	s << "Unexpected token " << ToWString(token.type) << ", expected ";
	bool first = true;
	for (const auto& type : expected)
	{
		if (!first)
		{
			s << ", ";
		}
		first = false;
		s << ToWString(type);
	}
	message = s.str();
}

InvalidCurrentTokenException::InvalidCurrentTokenException(const Token& token) : ParseException(token)
{
	std::wostringstream s;
	s << "Invalid current token " << ToWString(token.type);
	message = s.str();
}
