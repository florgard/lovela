#include <iostream>
#include <sstream>
#include <stdexcept>
#include "magic_enum.hpp"
#include "Utility.h"
#include "Parser.h"

Node::Node(Type type) noexcept : type(type)
{
}

struct RootNode : Node
{
	RootNode() noexcept : Node(Node::Type::Root) {}
};

struct FunctionNode : public Node
{
	std::wstring name;

	FunctionNode() noexcept : Node(Node::Type::Function) {}
};

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

std::unique_ptr<Node> Parser::Parse() noexcept
{
	auto node = std::make_unique<RootNode>();

	auto& i = tokenIterator;
	auto& g = tokenGenerator;

	for (i = g.begin(); i != g.end(); i++)
	{
		try
		{
			if (Accept(Token::Type::Identifier))
			{
				node->children.emplace_back(ParseFunction());
			}
			else
			{
				throw UnexpectedTokenException(*i);
			}
		}
		catch (const ParseException& e)
		{
			errors.emplace_back(Error{ .message = e.message });
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

std::unique_ptr<Node> Parser::ParseFunction()
{
	auto node = std::make_unique<FunctionNode>();

	// TODO

	return node;
}
