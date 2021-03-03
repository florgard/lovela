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
	if (tokenIterator == tokenGenerator.end() || tokenIterator->type != type)
	{
		return false;
	}

	currentToken = *tokenIterator;
	tokenIterator++;
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
