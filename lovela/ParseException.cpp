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
	s << "Unexpected token " << ToWString(magic_enum::enum_name(token.type));
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, Token::Type expected) : ParseException(token)
{
	std::wostringstream s;
	s << "Unexpected token " << ToWString(magic_enum::enum_name(token.type)) << ", expected " << ToWString(magic_enum::enum_name(expected));
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, const std::vector<Token::Type>& expected) : ParseException(token)
{
	std::wostringstream s;
	s << "Unexpected token " << ToWString(magic_enum::enum_name(token.type)) << ", expected ";
	bool first = true;
	for (const auto& type : expected)
	{
		if (!first)
		{
			s << ", ";
		}
		first = false;
		s << ToWString(magic_enum::enum_name(type));
	}
	message = s.str();
}

InvalidCurrentTokenException::InvalidCurrentTokenException(const Token& token) : ParseException(token)
{
	std::wostringstream s;
	s << "Invalid current token " << ToWString(magic_enum::enum_name(token.type));
	message = s.str();
}