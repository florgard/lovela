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

	Next();
	return true;
}

// Sets the current token to the next token if it's of one of the given types.
// Returns true if the token was set, false otherwise.
bool ParserBase::Accept(const std::vector<Token::Type>& types)
{
	auto iter = std::find_if(types.begin(), types.end(), [&](Token::Type type) { return Accept(type); });
	return iter != types.end();
}

// Sets the current token to the next token.
void ParserBase::Next()
{
	currentToken = *tokenIterator;
	tokenIterator++;
}

// Asserts that the current token is of the given type.
// Throws if the token isn't of the given type.
void ParserBase::Assert(Token::Type type)
{
	if (!IsToken(type))
	{
		throw InvalidCurrentTokenException(currentToken);
	}
}

// Asserts that the current token is of one of the given types.
// Throws if the token isn't of one of the given types.
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
	auto iter = std::find_if(types.begin(), types.end(), [&](Token::Type type) { return currentToken.type == type; });
	return iter != types.end();
}
