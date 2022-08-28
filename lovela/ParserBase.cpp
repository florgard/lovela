#include "pch.h"
#include "ParserBase.h"
#include "ParseException.h"

ParserBase::ParserBase(std::unique_ptr<ITokenIterator> tokenIterator) noexcept : tokenIterator(std::move(tokenIterator))
{
}

// Sets the current token to the next token that is expected to be of the given type.
// Throws if the next token isn't of the given type or if there's no next token.
void ParserBase::Expect(Token::Type type)
{
	if (!Accept(type))
	{
		throw UnexpectedTokenException(*GetTokenIterator(), type);
	}
}

// Sets the current token to the next token that is expected to be of one of the given types.
// Throws if the next token isn't of one of the given types or if there's no next token.
void ParserBase::Expect(const std::set<Token::Type>& types)
{
	if (!Accept(types))
	{
		throw UnexpectedTokenException(*GetTokenIterator(), types);
	}
}

// Sets the current token to the next token if it's of the given type.
// Returns true if the token was set, false otherwise.
bool ParserBase::Accept(Token::Type type)
{
	if (Peek(type))
	{
		Skip();
		return true;
	}

	return false;
}

// Sets the current token to the next token if it's of one of the given types.
// Returns true if the token was set, false otherwise.
bool ParserBase::Accept(const std::set<Token::Type>& types)
{
	auto iter = std::find_if(types.begin(), types.end(), [&](Token::Type type) { return Accept(type); });
	return iter != types.end();
}

// Skips the current token and sets the next token as current token.
void ParserBase::Skip()
{
	currentToken = *GetTokenIterator();
	GetTokenIterator()++;
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
void ParserBase::Skip(const std::set<Token::Type>& types)
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
void ParserBase::Assert(const std::set<Token::Type>& types)
{
	if (!IsToken(types))
	{
		throw InvalidCurrentTokenException(currentToken);
	}
}

// Checks whether there's a next token.
// Returns true if there's a token, false otherwise.
bool ParserBase::Peek()
{
	return !GetTokenIterator().empty();
}

// Checks whether the next token is of the given type.
// Returns true if the token is of the given type, false otherwise.
bool ParserBase::Peek(Token::Type type)
{
	if (!Peek())
	{
		return false;
	}

	const auto nextType = GetTokenIterator()->type;
	return type == nextType;
}

// Checks whether the next token is of one of the given types.
// Returns true if the token is of one of the given types, false otherwise.
bool ParserBase::Peek(const std::set<Token::Type>& types)
{
	if (!Peek())
	{
		return false;
	}

	const auto nextType = GetTokenIterator()->type;
	return types.contains(nextType);
}

// Checks whether the current token is of the given type.
// Returns true if the token is of the given type, false otherwise.
bool ParserBase::IsToken(Token::Type type)
{
	return currentToken.type == type;
}

// Checks whether the current token is of one of the given types.
// Returns true if the token is of one of the given types, false otherwise.
bool ParserBase::IsToken(const std::set<Token::Type>& types)
{
	return types.contains(currentToken.type);
}
