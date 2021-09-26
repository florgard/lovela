export module ParseException;

export import Token;
import <string>;
import <string_view>;
import <set>;

export struct ParseException
{
	std::wstring message;
	Token token;

	ParseException(const Token& token);
	ParseException(const Token& token, std::string_view message);
	ParseException(const Token& token, std::wstring_view message);
};

export struct UnexpectedTokenException : public ParseException
{
	UnexpectedTokenException(const Token& token);
	UnexpectedTokenException(const Token& token, Token::Type expected);
	UnexpectedTokenException(const Token& token, const std::set<Token::Type>& expected);
};

export struct InvalidCurrentTokenException : public ParseException
{
	InvalidCurrentTokenException(const Token& token);
};
