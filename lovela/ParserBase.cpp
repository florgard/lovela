#include "pch.h"
#include "Lexer.h"
#include "ParseException.h"
#include "ParserBase.h"

ParserBase::ParserBase(TokenGenerator&& tokenGenerator) noexcept : tokenGenerator(std::move(tokenGenerator))
{
}

void ParserBase::Expect(Token::Type type)
{
	if (!Accept(type))
	{
		throw UnexpectedTokenException(*tokenIterator, type);
	}
}

void ParserBase::Expect(const std::vector<Token::Type>& types)
{
	if (!Accept(types))
	{
		throw UnexpectedTokenException(*tokenIterator, types);
	}
}

bool ParserBase::Accept(Token::Type type)
{
	if (tokenIterator == tokenGenerator.end())
	{
		currentToken = Token{ .type = Token::Type::End };
		return false;
	}
	else if (tokenIterator->type != type)
	{
		return false;
	}

	Next();
	return true;
}

bool ParserBase::Accept(const std::vector<Token::Type>& types)
{
	for (const auto& type : types)
	{
		if (Accept(type))
		{
			return true;
		}
	}

	return false;
}

void ParserBase::Next()
{
	currentToken = *tokenIterator;
	tokenIterator++;
}

void ParserBase::Assert(Token::Type type)
{
	if (currentToken.type != type)
	{
		throw InvalidCurrentTokenException(currentToken);
	}
}

void ParserBase::Assert(const std::vector<Token::Type>& types)
{
	for (const auto& type : types)
	{
		if (currentToken.type == type)
		{
			return;
		}
	}

	throw InvalidCurrentTokenException(currentToken);
}
