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
	Token::Type::SeparatorColon,
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
	Node::Type::VariableReference,
};

static const std::set<Node::Type> operatorNodes
{
	Node::Type::FunctionCall,
	Node::Type::BinaryOperation,
};

bool Parser::Context::HasFunctionSymbol(const std::wstring& symbol) const
{
	if (functionSymbols.contains(symbol))
	{
		return true;
	}
	else if (parent)
	{
		return parent->HasFunctionSymbol(symbol);
	}
	else
	{
		return false;
	}
}

bool Parser::Context::HasVariableSymbol(const std::wstring& symbol) const
{
	if (variableSymbols.contains(symbol))
	{
		return true;
	}
	else if (parent)
	{
		return parent->HasVariableSymbol(symbol);
	}
	else
	{
		return false;
	}
}

void Parser::Context::AddFunctionSymbol(const std::wstring& symbol)
{
	if (!HasFunctionSymbol(symbol))
	{
		functionSymbols.emplace(std::make_pair(symbol, std::shared_ptr<FunctionDeclaration>()));
	}
}

void Parser::Context::AddVariableSymbol(std::shared_ptr<VariableDeclaration> declaration)
{
	if (!HasVariableSymbol(declaration->name))
	{
		variableSymbols.emplace(std::make_pair(declaration->name, declaration));
	}
}

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
	auto context = make<Context>::shared();
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

	auto node = std::move(nodes.back());
	nodes.pop_back();
	auto parent = node.get();

	while (!nodes.empty())
	{
		parent->right = std::move(nodes.back());
		nodes.pop_back();
		parent = parent->right.get();
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
		typeSpec.SetAny();
	}
	// [#32]
	else if (Accept(Token::Type::SeparatorHash))
	{
		typeSpec.name = currentToken.value;
		Expect(Token::Type::LiteralInteger);
		typeSpec.name += currentToken.value;
		Expect(Token::Type::ParenSquareClose);
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
		typeSpec.SetNone();
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
		auto parameter = make<VariableDeclaration>::shared();
		bool defined = false;

		// Optional name
		if (Accept(Token::Type::Identifier))
		{
			parameter->name = currentToken.value;
			defined = true;
		}

		// Optional type
		if (Accept(Token::Type::ParenSquareOpen))
		{
			parameter->type = ParseTypeSpec();
			defined = true;
		}
		else
		{
			parameter->type.SetAny();
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
	auto node = make<Node>::unique({ .type = Node::Type::FunctionDeclaration, .token = currentToken });

	// <-
	// ->
	if (IsToken(Token::Type::OperatorArrow))
	{
		node->exported = currentToken.value == L"<-";
		node->imported = currentToken.value == L"->";
		Expect({ Token::Type::ParenSquareOpen, Token::Type::Identifier });
	}

	// [inType]
	if (IsToken(Token::Type::ParenSquareOpen))
	{
		node->inType = ParseTypeSpec();

		// [inType] identifier
		if (Accept(Token::Type::Identifier))
		{
			node->value = currentToken.value;
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

		node->value = name;

		qualifiedName << name;

		context->AddFunctionSymbol(qualifiedName.str());
	}
	// binaryOperator
	else if (IsToken(binaryOperatorTokens))
	{
		node->value = currentToken.value;
	}
	// :
	else if (IsToken(Token::Type::SeparatorColon))
	{
	}
	else
	{
		Assert();
	}

	// identifier (parameterList)
	if (Accept(Token::Type::ParenRoundOpen))
	{
		node->parameters = ParseParameterList();
	}

	// identifier [outType]
	if (Accept(Token::Type::ParenSquareOpen))
	{
		node->outType = ParseTypeSpec();
	}

	// identifier:
	if (IsToken(Token::Type::SeparatorColon) || Accept(Token::Type::SeparatorColon))
	{
		if (node->value.empty())
		{
			// The anonymous main function has no output type.
			node->outType.SetNone();
		}

		auto innerContext = make<Context>::shared({ .parent = context, .inType = node->inType });

		for (const auto& parameter : node->parameters)
		{
			innerContext->AddVariableSymbol(parameter);
		}

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
	auto firstToken = *tokenIterator;

	const auto& inType = context->inType;
	auto innerContext = make<Context>::shared({ .parent = context, .inType = inType });

	std::vector<std::unique_ptr<Node>> nodes;

	for (;;)
	{
		if (Accept(operandTokens))
		{
			nodes.emplace_back(ParseOperand(innerContext));
		}
		else if (Accept(Token::Type::Identifier))
		{
			if (context->HasVariableSymbol(currentToken.value))
			{
				nodes.emplace_back(ParseVariableReference(innerContext));
			}
			else
			{
				nodes.emplace_back(ParseFunctionCall(innerContext));
			}
		}
		else if (Accept(binaryOperatorTokens))
		{
			nodes.emplace_back(ParseBinaryOperation(innerContext));
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

	if (nodes.empty())
	{
		return make<Node>::unique({ .type = Node::Type::Empty, .token = firstToken });
	}
	if (nodes.size() == 1)
	{
		return std::move(nodes.front());
	}

	auto expression = make<Node>::unique({ .type = Node::Type::Expression, .outType = inType, .token = firstToken, .inType = inType });

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
			if (right)
			{
				if (node->right)
				{
					throw ParseException(node->token, "Attempt to replace an existing right node of an operator");
				}

				node->right = std::move(right);
			}

			parent->left = std::move(node);
			parent = parent->left.get();
		}
		else
		{
			throw ParseException(node->token, "The expression node stack contains a node that is neither an operand nor an operator.");
		}
	}

	if (right)
	{
		// Left-most operand
		// TODO: Check that default input is implicit and can be discarded?

		if (parent->left)
		{
			throw ParseException(parent->token, "The parent expression node already has a left hand side operand.");
		}

		parent->left = std::move(right);
	}
	else if (!parent->left)
	{
		// Default expression input
		// TODO: Check if the expected in-type is None and the implicit input should be discarded?

		parent->left = make<Node>::unique(Node{ .type = Node::Type::ExpressionInput, .token = firstToken });
	}

	expression = ReduceExpression(std::move(expression));

	// The data type of the expression is the data type of the first child node.
	if (expression->left)
	{
		expression->outType = expression->left->outType;
	}

	return expression;
}

std::unique_ptr<Node> Parser::ReduceExpression(std::unique_ptr<Node>&& expression)
{
	static const std::set<Node::Type> reducable{
		Node::Type::Empty,
		Node::Type::Expression
	};

	const bool leftEmpty = !expression->left || expression->left->type == Node::Type::Empty;
	const bool rightEmpty = !expression->right || expression->right->type == Node::Type::Empty;

	if (leftEmpty && rightEmpty)
	{
		// Nothing to reduce
		return expression;
	}
	else if (!leftEmpty && !rightEmpty)
	{
		// Both children defined, can't reduce.
		return expression;
	}
	else if (!reducable.contains(expression->type))
	{
		// This node type is not known to be reducable.
		return expression;
	}

	return std::move(leftEmpty ? expression->right : expression->left);
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
		auto tuple = make<Node>::unique({ .type = Node::Type::Tuple, .token = currentToken });
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
		node = make<Node>::unique(
			{
				.type = Node::Type::Literal,
				.value = currentToken.value,
				.outType{.name = currentToken.outType},
				.token = currentToken
			});
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

	auto node = make<Node>::unique({ .type = Node::Type::FunctionCall, .value = currentToken.value, .token = currentToken });

	// TODO: nameSpace

	if (Accept(Token::Type::ParenRoundOpen))
	{
		node->right = ParseGroup(context);
	}

	return node;
}

std::unique_ptr<Node> Parser::ParseBinaryOperation(std::shared_ptr<Context> context)
{
	Assert(binaryOperatorTokens);

	return make<Node>::unique({ .type = Node::Type::BinaryOperation, .value = currentToken.value, .token = currentToken });
}

std::unique_ptr<Node> Parser::ParseVariableReference(std::shared_ptr<Context> context)
{
	Assert(Token::Type::Identifier);

	return make<Node>::unique({ .type = Node::Type::VariableReference, .value = currentToken.value, .token = currentToken });
}
