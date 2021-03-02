#include "ParseException.h"
#include "Parser.h"

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
