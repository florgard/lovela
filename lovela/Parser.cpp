#include <iostream>
#include <sstream>
#include <stdexcept>
#include "magic_enum.hpp"
#include "Utility.h"
#include "Parser.h"

struct ParseException
{
	std::wstring message;
	const Token& token;

	ParseException(const Token& token);
	ParseException(const Token& token, std::string_view message);
	ParseException(const Token& token, std::wstring_view message);
};

ParseException::ParseException(const Token& token) : token(token)
{
}

ParseException::ParseException(const Token& token, std::string_view message) : token(token), message(ToWString(message))
{
}

ParseException::ParseException(const Token& token, std::wstring_view message) : token(token), message(message)
{
}

struct UnexpectedTokenException : public ParseException
{
	UnexpectedTokenException(const Token& token);
	UnexpectedTokenException(const Token& token, Token::Type expected);
};

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

Parser::Parser(TokenGenerator&& tokenGenerator) noexcept : tokenGenerator(std::move(tokenGenerator))
{
}

Node Parser::Parse() noexcept
{
	auto node = Node{ .type{Node::Type::Root} };

	auto& i = tokenIterator = tokenGenerator.begin();
	while (tokenIterator != tokenGenerator.end())
	{
		try
		{
			if (Accept(Token::Type::Identifier))
			{
				node.children.emplace_back(ParseFunction());
			}
			else
			{
				throw UnexpectedTokenException(*i);
			}
		}
		catch (const ParseException& e)
		{
			errors.emplace_back(Error{ .message = e.message });

			// Skip the faulty token.
			tokenIterator++;
		}
	}

	return node;
}

void Parser::Expect(Token::Type type)
{
	if (tokenIterator->type != type)
	{
		throw UnexpectedTokenException(*tokenIterator, type);
	}

	currentToken = &*tokenIterator;
	tokenIterator++;
}

bool Parser::Accept(Token::Type type)
{
	if (tokenIterator->type != type)
	{
		return false;
	}

	currentToken = &*tokenIterator;
	tokenIterator++;
	return true;
}

Node Parser::ParseFunction()
{
	auto node = Node{ .type{Node::Type::Function} };

	// TODO

	return node;
}
