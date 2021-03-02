#include <sstream>
#include "magic_enum.hpp"
#include "Utility.h"
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
