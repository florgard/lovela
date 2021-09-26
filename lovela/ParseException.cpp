import ParseException;
import Utility;
import <string_view>;
import <sstream>;

ParseException::ParseException(const Token& token) : token(token)
{
}

ParseException::ParseException(const Token& token, std::string_view message) : token(token), message(to_wstring(message))
{
}

ParseException::ParseException(const Token& token, std::wstring_view message) : token(token), message(message)
{
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token) : ParseException(token)
{
	std::wostringstream s;
	s << "Unexpected token " << to_wstring(token.type);
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, Token::Type expected) : ParseException(token)
{
	std::wostringstream s;
	s << "Unexpected token " << to_wstring(token.type) << ", expected " << to_wstring(expected);
	message = s.str();
}

UnexpectedTokenException::UnexpectedTokenException(const Token& token, const std::set<Token::Type>& expected) : ParseException(token)
{
	std::wostringstream s;
	s << "Unexpected token " << to_wstring(token.type) << ", expected ";
	bool first = true;
	for (const auto& type : expected)
	{
		if (!first)
		{
			s << ", ";
		}
		first = false;
		s << to_wstring(type);
	}
	message = s.str();
}

InvalidCurrentTokenException::InvalidCurrentTokenException(const Token& token) : ParseException(token)
{
	std::wostringstream s;
	s << "Invalid current token " << to_wstring(token.type);
	message = s.str();
}
