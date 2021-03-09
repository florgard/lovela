#include "pch.h"
#include "Lexer.h"
#include "ParseException.h"
#include "ParserBase.h"

ParserBase::ParserBase(TokenGenerator&& tokenGenerator) noexcept : tokenGenerator(std::move(tokenGenerator))
{
}

// Sets the current token to the next token that is expected to be of the given type.
// Throws if the next token isn't of the given type or if there's no next token.
void ParserBase::Expect(Token::Type type)
{
	if (!Accept(type))
	{
		throw UnexpectedTokenException(*tokenIterator, type);
	}
}

// Sets the current token to the next token that is expected to be of one of the given types.
// Throws if the next token isn't of one of the given types or if there's no next token.
void ParserBase::Expect(const std::vector<Token::Type>& types)
{
	if (!Accept(types))
	{
		throw UnexpectedTokenException(*tokenIterator, types);
	}
}

// Sets the current token to the next token if it's of the given type.
// Returns true if the token was set, false otherwise.
bool ParserBase::Accept(Token::Type type)
{
	if (!Peek(type))
	{
		currentToken = Token{ .type = Token::Type::End };
		return false;
	}

	Skip();
	return true;
}

// Sets the current token to the next token if it's of one of the given types.
// Returns true if the token was set, false otherwise.
bool ParserBase::Accept(const std::vector<Token::Type>& types)
{
	auto iter = std::find_if(types.begin(), types.end(), [&](Token::Type type) { return Accept(type); });
	return iter != types.end();
}

// Skips the current token and sets the next token as current token.
void ParserBase::Skip()
{
	currentToken = *tokenIterator;
	tokenIterator++;
}

// Skips the current token and sets the next token as current token, if the next token is of the given type.
void ParserBase::Skip(Token::Type type)
{
	if (Peek(type))
	{
		Skip();
	}
}

// Skips the current token and sets the next token as current token, if the next token is of one of the given types.
void ParserBase::Skip(const std::vector<Token::Type>& types)
{
	if (Peek(types))
	{
		Skip();
	}
}

// Throws InvalidCurrentTokenException.
void ParserBase::Assert()
{
	throw InvalidCurrentTokenException(currentToken);
}

// Asserts that the current token is of the given type.
// Throws InvalidCurrentTokenException if the token isn't of the given type.
void ParserBase::Assert(Token::Type type)
{
	if (!IsToken(type))
	{
		throw InvalidCurrentTokenException(currentToken);
	}
}

// Asserts that the current token is of one of the given types.
// Throws InvalidCurrentTokenException if the token isn't of one of the given types.
void ParserBase::Assert(const std::vector<Token::Type>& types)
{
	if (!IsToken(types))
	{
		throw InvalidCurrentTokenException(currentToken);
	}
}

// Checks whether the next token is of the given type.
// Returns true if the token is of the given type, false otherwise.
bool ParserBase::Peek(Token::Type type)
{
	return tokenIterator != tokenGenerator.end() && tokenIterator->type == type;
}

// Checks whether the next token is of one of the given types.
// Returns true if the token is of one of the given types, false otherwise.
bool ParserBase::Peek(const std::vector<Token::Type>& types)
{
	auto iter = std::find_if(types.begin(), types.end(), [&](Token::Type type) { return Peek(type); });
	return iter != types.end();
}

// Checks whether the current token is of the given type.
// Returns true if the token is of the given type, false otherwise.
bool ParserBase::IsToken(Token::Type type)
{
	return currentToken.type == type;
}

// Checks whether the current token is of one of the given types.
// Returns true if the token is of one of the given types, false otherwise.
bool ParserBase::IsToken(const std::vector<Token::Type>& types)
{
	auto iter = std::find_if(types.begin(), types.end(), [&](Token::Type type) { return IsToken(type); });
	return iter != types.end();
}
