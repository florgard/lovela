#include "pch.h"
#include "ParseException.h"
#include "Parser.h"

static const std::vector<Token::Type> noAcceptedTokens
{
};

static const std::vector<Token::Type> beginFunctionDeclarationTokens
{
	Token::Type::OperatorArrow,
	Token::Type::ParenSquareOpen,
	Token::Type::Identifier,
	Token::Type::OperatorArithmetic,
	Token::Type::OperatorBitwise,
	Token::Type::OperatorComparison,
};

static const std::vector<Token::Type> literalTokens
{
	Token::Type::LiteralInteger,
	Token::Type::LiteralDecimal,
	Token::Type::LiteralString,
};

static const std::vector<Token::Type> binaryOperatorTokens
{
	Token::Type::OperatorArithmetic,
	Token::Type::OperatorBitwise,
	Token::Type::OperatorComparison,
};

static const std::vector<Token::Type> statementTerminatorTokens
{
	Token::Type::End,
	Token::Type::SeparatorDot,
	Token::Type::SeparatorComma,
	Token::Type::ParenRoundClose,
};

Parser::Parser(TokenGenerator&& tokenGenerator) noexcept : ParserBase(std::move(tokenGenerator))
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
			if (Accept(beginFunctionDeclarationTokens))
			{
				node.children.emplace_back(ParseFunctionDeclaration());
			}
			else
			{
				throw UnexpectedTokenException(*tokenIterator);
			}
		}
		catch (const InvalidCurrentTokenException& e)
		{
			errors.emplace_back(Error{ .message = e.message, .token = e.token });

			// Internal logic error: The caller and parsing function doesn't agree on the type of token to handle.
			assert(false);
			return node;
		}
		catch (const ParseException& e)
		{
			errors.emplace_back(Error{ .message = e.message, .token = e.token });

			// Skip the faulty token.
			Next();
		}
	}

	return node;
}

TypeSpec Parser::ParseTypeSpec()
{
	TypeSpec typeSpec;

	Assert(Token::Type::ParenSquareOpen);

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
		typeSpec.none = true;
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

	Assert(Token::Type::ParenRoundOpen);

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

Node Parser::ParseFunctionDeclaration()
{
	auto node = Node{ .type{Node::Type::Function} };

	// <-
	// ->
	if (currentToken.type == Token::Type::OperatorArrow)
	{
		node.exported = currentToken.value == L"<-";
		node.imported = currentToken.value == L"->";
		Expect({ Token::Type::ParenSquareOpen, Token::Type::Identifier });
	}

	// [objectType]
	if (currentToken.type == Token::Type::ParenSquareOpen)
	{
		node.objectType = ParseTypeSpec();

		// [objectType] identifier
		if (Accept(Token::Type::Identifier))
		{
			node.name = currentToken.value;
		}
	}
	// identifier
	// namespace|identifier
	// namespace|binaryOperator
	else if (currentToken.type == Token::Type::Identifier)
	{
		auto name = currentToken.value;

		// namespace1|namespaceN|identifier
		// namespace1|namespaceN|binaryOperator
		while (Accept(Token::Type::SeparatorVerticalLine))
		{
			node.nameSpace.emplace_back(name);

			// binaryOperator
			if (Accept(binaryOperatorTokens))
			{
				name = currentToken.value;
				break;
			}
			// identifier
			// namespaceN|identifier
			else
			{
				Expect(Token::Type::Identifier);
				name = currentToken.value;
			}
		}

		node.name = name;
		node.objectType.any = true;
	}
	// binaryOperator
	else if (contains(binaryOperatorTokens, currentToken.type))
	{
		node.name = currentToken.value;
		node.objectType.any = true;
	}
	else
	{
		Assert(noAcceptedTokens);
	}

	// [objectType] identifier (parameterList)
	if (Accept(Token::Type::ParenRoundOpen))
	{
		node.parameters = ParseParameterList();
	}

	// [objectType] identifier (parameterList) [dataType]
	if (Accept(Token::Type::ParenSquareOpen))
	{
		node.dataType = ParseTypeSpec();
	}

	// [objectType] identifier (parameterList) [dataType]:
	if (Accept(Token::Type::SeparatorColon))
	{
		Next();
		node.children.emplace_back(ParseStatement());
	}

	return node;
}

Node Parser::ParseStatement(/*objectType?*/)
{
	Node node{ .type = Node::Type::Statement };

	node.children.emplace_back(ParseExpression());

	return node;
}

Node Parser::ParseStatements(/*objectType?*/)
{
	auto node = ParseStatement();

	if (!contains(statementTerminatorTokens, currentToken.type))
	{
		node.children.emplace_back(ParseStatements());
	}

	return node;
}

Node Parser::ParseExpression()
{
	Node node{ .type = Node::Type::Expression };

	if (Accept(Token::Type::ParenRoundOpen))
	{
		node.children.emplace_back(ParseGroup());
	}
	// Identifier
	// Colon

	return node;
}

Node Parser::ParseGroup()
{
	Assert(Token::Type::ParenRoundOpen);

	Node node{ .type = Node::Type::Group };

	do
	{
		node.children.emplace_back(ParseStatements());

	} while (Accept(Token::Type::SeparatorComma));

	Expect(Token::Type::ParenRoundClose);

	return node;
}
