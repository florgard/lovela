#include "pch.h"
#include "ParseException.h"
#include "Parser.h"

static const std::set<Token::Type> beginFunctionDeclarationTokens
{
	Token::Type::ParenSquareOpen,
	Token::Type::Identifier,
	Token::Type::OperatorArrow,
	Token::Type::OperatorArithmetic,
	Token::Type::OperatorBitwise,
	Token::Type::OperatorComparison,
};

static const std::set<Token::Type> literalTokens
{
	Token::Type::LiteralInteger,
	Token::Type::LiteralDecimal,
	Token::Type::LiteralString,
};

static const std::set<Token::Type> operandTokens
{
	Token::Type::ParenRoundOpen,
	Token::Type::LiteralInteger,
	Token::Type::LiteralDecimal,
	Token::Type::LiteralString,
};

static const std::set<Token::Type> binaryOperatorTokens
{
	Token::Type::OperatorArithmetic,
	Token::Type::OperatorBitwise,
	Token::Type::OperatorComparison,
};

static const std::set<Token::Type> operatorTokens
{
	Token::Type::Identifier,
	Token::Type::OperatorArrow,
	Token::Type::OperatorArithmetic,
	Token::Type::OperatorBitwise,
	Token::Type::OperatorComparison,
};

static const std::set<Token::Type> expressionTerminatorTokens
{
	Token::Type::End,
	Token::Type::SeparatorDot,
	Token::Type::SeparatorComma,
	Token::Type::ParenRoundClose,
};

static const std::set<Node::Type> operandNodes
{
	Node::Type::Expression,
	Node::Type::Tuple,
	Node::Type::Literal,
};

static const std::set<Node::Type> operatorNodes
{
	Node::Type::FunctionCall,
	Node::Type::BinaryOperation,
};

Parser::Parser(TokenGenerator&& tokenGenerator) noexcept : ParserBase(std::move(tokenGenerator))
{
}

void Parser::TraverseDepthFirstPreorder(Node& tree, std::function<void(Node& node)> visitor) noexcept
{
	visitor(tree);

	for (auto& child : tree.children)
	{
		TraverseDepthFirstPreorder(child, visitor);
	}
}

void Parser::TraverseDepthFirstPostorder(Node& tree, std::function<void(Node& node)> visitor) noexcept
{
	for (auto& child : tree.children)
	{
		TraverseDepthFirstPostorder(child, visitor);
	}

	visitor(tree);
}

Node Parser::Parse() noexcept
{
	auto context = std::make_shared<Context>();
	// TODO: add built-in functions?

	auto node = Node{ .type{Node::Type::Root} };

	tokenIterator = tokenGenerator.begin();
	while (tokenIterator != tokenGenerator.end())
	{
		try
		{
			if (Accept(beginFunctionDeclarationTokens))
			{
				node.children.emplace_back(ParseFunctionDeclaration(context));
			}
			else
			{
				throw UnexpectedTokenException(*tokenIterator);
			}
		}
		catch (const InvalidCurrentTokenException& e)
		{
			errors.emplace_back(Error{ .code = IParser::Error::Code::ParseError, .message = e.message, .token = e.token });

			// Internal logic error: The caller and parsing function doesn't agree on the type of token to handle.
			assert(false);
			return node;
		}
		catch (const ParseException& e)
		{
			errors.emplace_back(Error{ .code = IParser::Error::Code::ParseError, .message = e.message, .token = e.token });

			// Skip faulty token.
			Skip();
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

Node Parser::ParseFunctionDeclaration(std::shared_ptr<Context> context)
{
	auto innerContext = std::make_shared<Context>();
	innerContext->parent = context;

	auto node = Node{ .type{Node::Type::FunctionDeclaration} };

	// <-
	// ->
	if (IsToken(Token::Type::OperatorArrow))
	{
		node.exported = currentToken.value == L"<-";
		node.imported = currentToken.value == L"->";
		Expect({ Token::Type::ParenSquareOpen, Token::Type::Identifier });
	}

	// [objectType]
	if (IsToken(Token::Type::ParenSquareOpen))
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
	else if (IsToken(Token::Type::Identifier))
	{
		auto name = currentToken.value;
		std::wostringstream qualifiedName;

		// namespace1|namespaceN|identifier
		// namespace1|namespaceN|binaryOperator
		while (Accept(Token::Type::SeparatorVerticalLine))
		{
			node.nameSpace.emplace_back(name);
			qualifiedName << name << '|';

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

		qualifiedName << name;
		context->symbols.insert(qualifiedName.str());
	}
	// binaryOperator
	else if (IsToken(binaryOperatorTokens))
	{
		node.name = currentToken.value;
		node.objectType.any = true;
	}
	else
	{
		Assert();
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
	else
	{
		node.dataType.any = true;
	}

	// [objectType] identifier (parameterList) [dataType]:
	if (Accept(Token::Type::SeparatorColon))
	{
		node.children.emplace_back(ParseExpression(innerContext));
	}

	return node;
}

Node Parser::ParseCompoundExpression(std::shared_ptr<Context> context)
{
	auto node = ParseExpression(context);

	if (!IsToken(expressionTerminatorTokens))
	{
		node.children.emplace_back(ParseCompoundExpression(context));
	}

	return node;
}

Node Parser::ParseExpression(std::shared_ptr<Context> context)
{
	std::vector<Node> nodes;

	for (;;)
	{
		if (Accept(operandTokens))
		{
			nodes.emplace_back(ParseOperand(context));
		}
		else if (Accept(Token::Type::Identifier))
		{
			nodes.emplace_back(ParseFunctionCall(context));
		}
		else if (Accept(binaryOperatorTokens))
		{
			nodes.emplace_back(ParseBinaryOperation(context));
		}
		// TODO: Selector, bind
		else if (Peek(expressionTerminatorTokens))
		{
			break;
		}
		else
		{
			throw UnexpectedTokenException(*tokenIterator);
		}
	}

	Node expression{ .type = Node::Type::Expression };
	auto* parent = &expression;
	Node right;

	for (auto iter = nodes.rbegin(); iter != nodes.rend(); iter++)
	{
		if (operandNodes.contains(iter->type))
		{
			if (right.type != Node::Type::Empty)
			{
				throw UnexpectedTokenException(iter->token);
			}

			right = *iter;
		}
		else if (operatorNodes.contains(iter->type))
		{
			auto& current = parent->children.emplace_back(*iter);

			if (right)
			{
				parent->children.emplace_back(right);
			}

			parent = &current;
			assert(!right);
		}
		else
		{
			throw ParseException(iter->token, "The expression node stack contains a node that is neither an operand nor an operator.");
		}
	}

	// Left-most operand
	if (right)
	{
		parent->children.emplace_back(right);
	}

	Skip(Token::Type::SeparatorDot);

	return expression;
}

// Returns Expression or Tuple
Node Parser::ParseGroup(std::shared_ptr<Context> context)
{
	Assert(Token::Type::ParenRoundOpen);

	auto node = ParseCompoundExpression(context);

	if (Accept(Token::Type::SeparatorComma))
	{
		Node tuple{ .type = Node::Type::Tuple };
		tuple.children.emplace_back(node);
		tuple.children.emplace_back(ParseTuple(context));
		node = tuple;
	}

	Expect(Token::Type::ParenRoundClose);

	return node;
}

// Returns Expression or Tuple
Node Parser::ParseTuple(std::shared_ptr<Context> context)
{
	auto node = ParseCompoundExpression(context);

	if (Accept(Token::Type::SeparatorComma))
	{
		Node tuple{ .type = Node::Type::Tuple };
		tuple.children.emplace_back(node);
		tuple.children.emplace_back(ParseTuple(context));
		node = tuple;
	}

	return node;
}

Node Parser::ParseOperand(std::shared_ptr<Context> context)
{
	Node node;

	if (IsToken(Token::Type::ParenRoundOpen))
	{
		node = ParseGroup(context);
	}
	else if (IsToken(literalTokens))
	{
		node = Node{ .type = Node::Type::Literal, .token = currentToken };
	}
	else
	{
		Assert();
	}

	return node;
}

Node Parser::ParseFunctionCall(std::shared_ptr<Context> context)
{
	Assert(Token::Type::Identifier);

	Node node{ .type = Node::Type::FunctionCall, .name = currentToken.value };

	// TODO: nameSpace, parameters

	return node;
}

Node Parser::ParseBinaryOperation(std::shared_ptr<Context> context)
{
	Assert(binaryOperatorTokens);

	return { .type = Node::Type::BinaryOperation, .name = currentToken.value, .token = currentToken };
}
