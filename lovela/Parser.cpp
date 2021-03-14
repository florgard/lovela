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

	if (tree.left)
	{
		TraverseDepthFirstPreorder(*tree.left, visitor);
	}
	if (tree.right)
	{
		TraverseDepthFirstPreorder(*tree.right, visitor);
	}
}

void Parser::TraverseDepthFirstPostorder(Node& tree, std::function<void(Node& node)> visitor) noexcept
{
	if (tree.left)
	{
		TraverseDepthFirstPostorder(*tree.left, visitor);
	}
	if (tree.right)
	{
		TraverseDepthFirstPostorder(*tree.right, visitor);
	}

	visitor(tree);
}

std::unique_ptr<Node> Parser::Parse() noexcept
{
	auto context = std::make_shared<Context>();
	// TODO: add built-in functions?

	// Use a list of top-level nodes to be able to continue parsing after an error.
	std::vector<std::unique_ptr<Node>> nodes;

	tokenIterator = tokenGenerator.begin();
	while (tokenIterator != tokenGenerator.end())
	{
		try
		{
			if (Accept(beginFunctionDeclarationTokens))
			{
				nodes.emplace_back(ParseFunctionDeclaration(context));
			}
			else if (Accept(Token::Type::End))
			{
				if (Peek())
				{
					throw UnexpectedTokenException(*tokenIterator);
				}
			}
			else if (Peek())
			{
				throw UnexpectedTokenException(*tokenIterator);
			}
			else
			{
				throw ParseException({}, "Unexpected end of token stream.");
			}
		}
		catch (const InvalidCurrentTokenException& e)
		{
			errors.emplace_back(Error{ .code = IParser::Error::Code::ParseError, .message = e.message, .token = e.token });

			// Skip faulty token.
			Skip();
		}
		catch (const ParseException& e)
		{
			errors.emplace_back(Error{ .code = IParser::Error::Code::ParseError, .message = e.message, .token = e.token });

			// Skip faulty token.
			Skip();
		}
	}

	if (nodes.empty())
	{
		return {};
	}

	// Link the nodes together

	std::unique_ptr<Node> node = std::move(nodes.back());
	nodes.pop_back();
	auto parent = node.get();

	while (!nodes.empty())
	{
		node->right = std::move(nodes.back());
		nodes.pop_back();
		parent = node->right.get();
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

std::unique_ptr<Node> Parser::ParseFunctionDeclaration(std::shared_ptr<Context> context)
{
	auto innerContext = std::make_shared<Context>();
	innerContext->parent = context;

	auto node = Node::make_unique({ .type = Node::Type::FunctionDeclaration });

	// <-
	// ->
	if (IsToken(Token::Type::OperatorArrow))
	{
		node->exported = currentToken.value == L"<-";
		node->imported = currentToken.value == L"->";
		Expect({ Token::Type::ParenSquareOpen, Token::Type::Identifier });
	}

	// [objectType]
	if (IsToken(Token::Type::ParenSquareOpen))
	{
		node->objectType = ParseTypeSpec();

		// [objectType] identifier
		if (Accept(Token::Type::Identifier))
		{
			node->name = currentToken.value;
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
			node->nameSpace.emplace_back(name);
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

		node->name = name;
		node->objectType.any = true;

		qualifiedName << name;
		context->symbols.insert(qualifiedName.str());
	}
	// binaryOperator
	else if (IsToken(binaryOperatorTokens))
	{
		node->name = currentToken.value;
		node->objectType.any = true;
	}
	else
	{
		Assert();
	}

	// [objectType] identifier (parameterList)
	if (Accept(Token::Type::ParenRoundOpen))
	{
		node->parameters = ParseParameterList();
	}

	// [objectType] identifier (parameterList) [dataType]
	if (Accept(Token::Type::ParenSquareOpen))
	{
		node->dataType = ParseTypeSpec();
	}
	else
	{
		node->dataType.any = true;
	}

	// [objectType] identifier (parameterList) [dataType]:
	if (Accept(Token::Type::SeparatorColon))
	{
		node->left = ParseExpression(innerContext);
	}

	return node;
}

std::unique_ptr<Node> Parser::ParseCompoundExpression(std::shared_ptr<Context> context)
{
	auto node = ParseExpression(context);

	if (!Peek(expressionTerminatorTokens))
	{
		node->right = ParseCompoundExpression(context);
	}

	return node;
}

std::unique_ptr<Node> Parser::ParseExpression(std::shared_ptr<Context> context)
{
	std::vector<std::unique_ptr<Node>> nodes;

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
		else if (Accept(Token::Type::SeparatorDot) || Peek(expressionTerminatorTokens))
		{
			break;
		}
		else
		{
			throw UnexpectedTokenException(*tokenIterator);
		}
	}

	auto expression = Node::make_unique({ .type = Node::Type::Expression });

	if (nodes.empty())
	{
		return expression;
	}

	if (nodes.back()->type == Node::Type::Expression)
	{
		// Use a possible top-most expression node as the return node
		expression = std::move(nodes.back());
		nodes.pop_back();
	}

	auto parent = expression.get();
	std::unique_ptr<Node> right;

	while (!nodes.empty())
	{
		auto node = std::move(nodes.back());
		nodes.pop_back();

		if (operandNodes.contains(node->type))
		{
			if (right)
			{
				throw UnexpectedTokenException(node->token);
			}

			right = std::move(node);
		}
		else if (operatorNodes.contains(node->type))
		{
			node->right = std::move(right);

			parent->left = std::move(node);
			parent = parent->left.get();
		}
		else
		{
			throw ParseException(node->token, "The expression node stack contains a node that is neither an operand nor an operator.");
		}
	}

	// Left-most operand
	if (right)
	{
		if (parent->left)
		{
			throw ParseException(parent->token, "The parent expression node already has a left hand side operand.");
		}

		parent->left = std::move(right);
	}

	return expression;
}

// Returns Expression or Tuple
std::unique_ptr<Node> Parser::ParseGroup(std::shared_ptr<Context> context)
{
	Assert(Token::Type::ParenRoundOpen);

	auto node = ParseTuple(context);

	Expect(Token::Type::ParenRoundClose);

	return node;
}

// Returns Expression or Tuple
std::unique_ptr<Node> Parser::ParseTuple(std::shared_ptr<Context> context)
{
	auto node = ParseCompoundExpression(context);

	if (Accept(Token::Type::SeparatorComma))
	{
		auto tuple = Node::make_unique({ .type = Node::Type::Tuple });
		tuple->left = std::move(node);
		tuple->right = ParseTuple(context);
		node = std::move(tuple);
	}

	return node;
}

std::unique_ptr<Node> Parser::ParseOperand(std::shared_ptr<Context> context)
{
	std::unique_ptr<Node> node;

	if (IsToken(Token::Type::ParenRoundOpen))
	{
		node = ParseGroup(context);
	}
	else if (IsToken(literalTokens))
	{
		node = Node::make_unique({ .type = Node::Type::Literal, .token = currentToken });
	}
	else
	{
		Assert();
	}

	return node;
}

std::unique_ptr<Node> Parser::ParseFunctionCall(std::shared_ptr<Context> context)
{
	Assert(Token::Type::Identifier);

	auto node = Node::make_unique({ .type = Node::Type::FunctionCall, .name = currentToken.value });

	// TODO: nameSpace, parameters

	return node;
}

std::unique_ptr<Node> Parser::ParseBinaryOperation(std::shared_ptr<Context> context)
{
	Assert(binaryOperatorTokens);

	return Node::make_unique({ .type = Node::Type::BinaryOperation, .name = currentToken.value, .token = currentToken });
}
