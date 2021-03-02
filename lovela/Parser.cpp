#include <cassert>
#include "ParseException.h"
#include "Parser.h"

bool Node::operator==(const Node& rhs) const noexcept
{
	// Compare everything but the children
	return rhs.type == type
		&& rhs.name == name
		&& rhs.functionType == functionType
		&& rhs.objectType == objectType
		&& rhs.parameters == parameters;
}

Parser::Parser(TokenGenerator&& tokenGenerator) noexcept : tokenGenerator(std::move(tokenGenerator))
{
}

Node Parser::Parse() noexcept
{
	auto node = Node{ .type{Node::Type::Root} };

	tokenIterator = tokenGenerator.begin();
	while (tokenIterator != tokenGenerator.end())
	{
		try
		{
			if (Accept(Token::Type::ParenSquareOpen)
				|| Accept(Token::Type::Identifier))
			{
				node.children.emplace_back(ParseFunction());
			}
			// TODO: OperatorArrowRight -> Import
			// TODO: OperatorArrorLeft -> Export
			else
			{
				throw UnexpectedTokenException(*tokenIterator);
			}
		}
		catch (const InvalidCurrentTokenException& e)
		{
			errors.emplace_back(Error{ .message = e.message });

			// Internal logic error: The caller and parsing function doesn't agree on the type of token to handle.
			assert(false);
			return node;
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
	if (tokenIterator == tokenGenerator.end() || tokenIterator->type != type)
	{
		throw UnexpectedTokenException(*tokenIterator, type);
	}

	currentToken = *tokenIterator;
	tokenIterator++;
}

bool Parser::Accept(Token::Type type)
{
	if (tokenIterator == tokenGenerator.end() || tokenIterator->type != type)
	{
		return false;
	}

	currentToken = *tokenIterator;
	tokenIterator++;
	return true;
}

TypeSpec Parser::ParseTypeSpec()
{
	TypeSpec typeSpec;

	if (currentToken.type != Token::Type::ParenSquareOpen)
	{
		throw InvalidCurrentTokenException(currentToken);
	}

	// []
	if (Accept(Token::Type::ParenSquareClose))
	{
		typeSpec.any = true;
	}
	// [identifier]
	else if (Accept(Token::Type::Identifier))
	{
		typeSpec.name = currentToken.value;
		Expect(Token::Type::ParenSquareClose);
	}
	// [()]
	else if (Accept(Token::Type::ParenRoundOpen))
	{
		typeSpec.empty = true;
		Expect(Token::Type::ParenRoundClose);
		Expect(Token::Type::ParenSquareClose);
	}
	else
	{
		throw UnexpectedTokenException(*tokenIterator);
	}

	return typeSpec;
}

ParameterList Parser::ParseParameterList()
{
	ParameterList parameters;

	if (currentToken.type != Token::Type::ParenRoundOpen)
	{
		throw InvalidCurrentTokenException(currentToken);
	}

	// ()
	if (Accept(Token::Type::ParenRoundClose))
	{
		return parameters;
	}

	for (;;)
	{
		Parameter parameter;
		bool defined = false;

		// Optional name
		if (Accept(Token::Type::Identifier))
		{
			parameter.name = currentToken.value;
			defined = true;
		}

		// Optional type
		if (Accept(Token::Type::ParenSquareOpen))
		{
			parameter.type = ParseTypeSpec();
			defined = true;
		}
		else
		{
			parameter.type.any = true;
		}

		// Name and/or type must be specified
		if (defined)
		{
			parameters.emplace_back(parameter);
		}
		else
		{
			throw UnexpectedTokenException(currentToken);
		}

		if (Accept(Token::Type::SeparatorComma))
		{
		}
		else if (Accept(Token::Type::ParenRoundClose))
		{
			break;
		}
		else
		{
			throw UnexpectedTokenException(currentToken);
		}
	}

	return parameters;
}

Node Parser::ParseFunction()
{
	auto node = Node{ .type{Node::Type::Function} };

	if (currentToken.type == Token::Type::ParenSquareOpen)
	{
		node.objectType = ParseTypeSpec();

		// [objectType] identifier
		if (Accept(Token::Type::Identifier))
		{
			node.name = currentToken.value;
		}
	}
	else if (currentToken.type == Token::Type::Identifier)
	{
		node.name = currentToken.value;
		node.objectType.any = true;
	}
	else
	{
		throw InvalidCurrentTokenException(currentToken);
	}

	// [objectType] identifier (parameterList)
	if (Accept(Token::Type::ParenRoundOpen))
	{
		node.parameters = ParseParameterList();
	}

	// TODO: function type

	return node;
}
