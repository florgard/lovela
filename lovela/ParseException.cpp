#include "pch.h"
#include "ParseException.h"

ParseException::ParseException(Token const& token) noexcept
	: token(token)
{
}

ParseException::ParseException(Token const& token, std::string&& message) noexcept
	: token(token)
	, message(std::move(message))
{
}

UnexpectedTokenException::UnexpectedTokenException(Token const& token) noexcept
	: ParseException(token, fmt::format("Unexpected token {}.", token.type))
{
}

UnexpectedTokenException::UnexpectedTokenException(Token const& token, std::string&& message) noexcept
	: ParseException(token, fmt::format("Unexpected token {}. {}", token.type, message))
{
}

UnexpectedTokenException::UnexpectedTokenException(Token const& token, Token::Type expectedType) noexcept
	: UnexpectedTokenException(token, fmt::format("Expected {}.", expectedType))
{
}

UnexpectedTokenException::UnexpectedTokenException(Token const& token, Token::Type expectedType, std::string_view expectedValue) noexcept
	: UnexpectedTokenException(token, fmt::format("Expected {} = {}.", expectedType, expectedValue))
{
}

UnexpectedTokenAfterException::UnexpectedTokenAfterException(Token const& token, Token::Type preceedingType) noexcept
	: UnexpectedTokenException(token, fmt::format("It can't come after a {}.", preceedingType))
{
}

UnexpectedTokenAfterException::UnexpectedTokenAfterException(Token const& token, Token const& preceedingToken) noexcept
	: UnexpectedTokenException(token, preceedingToken.type)
{
}

InvalidCurrentTokenException::InvalidCurrentTokenException(Token const& token) noexcept
	: ParseException(token, fmt::format("Invalid current token {}.", token.type))
{
}

ErrorTokenException::ErrorTokenException(Token const& token) noexcept
	: ParseException(token, fmt::format("Error token encountered. Message: {}", token.error.message))
{
}

MissingTokenException::MissingTokenException() noexcept
	: ParseException({}, "Unexpected end of token stream.")
{
}
