#include "pch.h"
#include "Parser.h"
#include "ParseException.h"

// Token sets

static constexpr std::array<Token::Type, 9> s_FunctionDeclarationTokens
{
	Token::Type::ParenSquareOpen,
	Token::Type::Identifier,
	Token::Type::OperatorLeftArrow,
	Token::Type::OperatorRightArrow,
	Token::Type::OperatorArithmetic,
	Token::Type::OperatorBitwise,
	Token::Type::OperatorComparison,
	Token::Type::SeparatorColon,
};

static constexpr std::array<Token::Type, 2> s_TypeSpecTokens
{
	Token::Type::ParenSquareOpen,
};

static constexpr std::array<Token::Type, 3> s_ExternalFunctionDeclarationTokens
{
	Token::Type::ParenSquareOpen,
	Token::Type::Identifier,
};

static constexpr std::array<Token::Type, 3> s_LiteralTokens
{
	Token::Type::LiteralInteger,
	Token::Type::LiteralDecimal,
	Token::Type::LiteralString,
};

static constexpr std::array<Token::Type, 4> s_OperandTokens
{
	Token::Type::ParenRoundOpen,
	Token::Type::LiteralInteger,
	Token::Type::LiteralDecimal,
	Token::Type::LiteralString,
};

static constexpr std::array<Token::Type, 3> s_BinaryOperatorTokens
{
	Token::Type::OperatorArithmetic,
	Token::Type::OperatorBitwise,
	Token::Type::OperatorComparison,
};

static constexpr std::array<Token::Type, 6> s_OperatorTokens
{
	Token::Type::Identifier,
	Token::Type::OperatorLeftArrow,
	Token::Type::OperatorRightArrow,
	Token::Type::OperatorArithmetic,
	Token::Type::OperatorBitwise,
	Token::Type::OperatorComparison,
};

static constexpr std::array<Token::Type, 4> s_ExpressionTerminatorTokens
{
	Token::Type::End,
	Token::Type::SeparatorDot,
	Token::Type::SeparatorComma,
	Token::Type::ParenRoundClose,
};

static constexpr std::array<Token::Type, 2> s_TaggedTypeSpecTokens
{
	Token::Type::LiteralInteger,
	Token::Type::Identifier,
};

static constexpr static_set<Token::Type, s_FunctionDeclarationTokens.size()> s_FunctionDeclarationTokenSet{ {s_FunctionDeclarationTokens} };
static constexpr static_set<Token::Type, s_TypeSpecTokens.size()> s_TypeSpecTokenSet{ {s_TypeSpecTokens} };
static constexpr static_set<Token::Type, s_ExternalFunctionDeclarationTokens.size()> s_ExternalFunctionDeclarationTokenSet{ {s_ExternalFunctionDeclarationTokens} };
static constexpr static_set<Token::Type, s_LiteralTokens.size()> s_LiteralTokenSet{ {s_LiteralTokens} };
static constexpr static_set<Token::Type, s_OperandTokens.size()> s_OperandTokenSet{ {s_OperandTokens} };
static constexpr static_set<Token::Type, s_BinaryOperatorTokens.size()> s_BinaryOperatorTokenSet{ {s_BinaryOperatorTokens} };
static constexpr static_set<Token::Type, s_OperatorTokens.size()> s_OperatorTokenSet{ {s_OperatorTokens} };
static constexpr static_set<Token::Type, s_ExpressionTerminatorTokens.size()> s_ExpressionTerminatorTokenSet{ {s_ExpressionTerminatorTokens} };
static constexpr static_set<Token::Type, s_TaggedTypeSpecTokens.size()> s_TaggedTypeSpecTokenSet{ {s_TaggedTypeSpecTokens} };

static constexpr auto& GetFunctionDeclarationTokens()
{
	return s_FunctionDeclarationTokenSet;
}

static constexpr auto& GetTypeSpecTokens()
{
	return s_TypeSpecTokenSet;
}

static constexpr auto& GetExternalFunctionDeclarationTokens()
{
	return s_ExternalFunctionDeclarationTokenSet;
}

static constexpr auto& GetLiteralTokens()
{
	return s_LiteralTokenSet;
}

static constexpr auto& GetOperandTokens()
{
	return s_OperandTokenSet;
}

static constexpr auto& GetBinaryOperatorTokens()
{
	return s_BinaryOperatorTokenSet;
}

static constexpr auto& GetOperatorTokens()
{
	return s_OperatorTokenSet;
}

static constexpr auto& GetExpressionTerminatorTokens()
{
	return s_ExpressionTerminatorTokenSet;
}

static constexpr auto& GetTaggedTypeSpecTokens()
{
	return s_TaggedTypeSpecTokenSet;
}

// Node sets

static constexpr std::array<Node::Type, 4> s_OperandNodes
{
	Node::Type::Expression,
	Node::Type::Tuple,
	Node::Type::Literal,
	Node::Type::VariableReference,
};

static constexpr std::array<Node::Type, 2> s_OperatorNodes
{
	Node::Type::FunctionCall,
	Node::Type::BinaryOperation,
};

static constexpr static_set<Node::Type, s_OperandNodes.size()> s_OperandNodeSet{ {s_OperandNodes} };
static constexpr static_set<Node::Type, s_OperatorNodes.size()> s_OperatorNodeSet{ {s_OperatorNodes} };

static constexpr auto& GetOperandNodes()
{
	return s_OperandNodeSet;
}

static constexpr auto& GetOperatorNodes()
{
	return s_OperatorNodeSet;
}

// Context

bool Parser::Context::HasFunctionSymbol(const std::string& symbol) const
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

bool Parser::Context::HasVariableSymbol(const std::string& symbol) const
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

void Parser::Context::AddFunctionSymbol(const std::string& symbol)
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

// Parser

IParser::OutputT Parser::Parse() noexcept
{
	currentContext = make<Context>::shared();

	// TODO: add built-in functions?

	bool parsing = true;

	while (parsing)
	{
		Node errorNode{};

		try
		{
			for (auto&& rn : ParseNonSemantic())
			{
				Node n = std::move(rn);
				co_yield n;
			}

			if (Accept(GetFunctionDeclarationTokens()))
			{
				auto p = ParseFunctionDeclaration(currentContext);
				Node n = std::move(*p);
				co_yield n;
			}
			else if (Accept(Token::Type::End))
			{
				if (Peek())
				{
					throw UnexpectedTokenException(GetNext());
				}
				else
				{
					co_return;
				}
			}
			else if (Peek())
			{
				throw UnexpectedTokenException(GetNext());
			}
			else
			{
				throw MissingTokenException();
			}
		}
		catch (const MissingTokenException& e)
		{
			errorNode = { .type = Node::Type::Error, .error = {.code = Node::Error::Code::ParseError, .message = e.message } };

			// Stop parsing after yielding the error node.
			parsing = false;
		}
		catch (const ParseException& e)
		{
			errorNode = { .type = Node::Type::Error, .error = {.code = Node::Error::Code::ParseError, .message = e.message } };

			// Skip faulty token.
			Skip();
		}

		if (errorNode) co_yield errorNode;
	}
}

[[nodiscard]] IParser::OutputT Parser::ParseNonSemantic()
{
	for (;;)
	{
		if (Accept(Token::Type::Comment))
		{
			Node n{ .type = Node::Type::Comment, .value = GetCurrent().value, .token = GetCurrent() };
			co_yield n;
		}
		else if (Accept(Token::Type::SeparatorDot))
		{
			// Skip arbitrary statement separators.
		}
		else
		{
			co_return;
		}
	}
}

TypeSpec Parser::GetPrimitiveDecimalTypeSpec(const std::string& value)
{
	// Check the mantissa precision to see whether a double is requried.

	const auto& literalRegex = regexes.GetLiteralDecimal();
	std::smatch literalMatch;
	if (!std::regex_match(value, literalMatch, literalRegex))
	{
		// FIXME: Throw?
		// Not a valid decimal literal.
		return { .kind = TypeSpec::Kind::Invalid };
	}

	// The integer mantissa is all the digits.
	const auto digits = literalMatch[1].str() + literalMatch[2].str();

	// Remove the leading and trailing zeros.
	const auto& trimmedRegex = regexes.GetLiteralIntegerTrimZeros();
	std::smatch trimmedMatch;
	if (!std::regex_match(digits, trimmedMatch, trimmedRegex))
	{
		// FIXME: Throw?
		// Invalid regex?
		return { .kind = TypeSpec::Kind::Invalid };
	}

	bool requireDouble = false;

	// Convert to integer and check the limit for 32 bit float.
	// https://blog.demofox.org/2017/11/21/floating-point-precision/
	const auto trimmedDigits = trimmedMatch[1].str();
	const auto mantissa = to_int<uint64_t>(trimmedDigits).unsignedValue.value_or(std::numeric_limits<uint64_t>::max());
	if (mantissa >= 8388608) // 2^23 = 23 bit mantissa for 32 bit float
	{
		// Higher precision requires a double.
		requireDouble = true;
	}

	// Check which range the literal will fit in.

	if (!requireDouble)
	{
		// Maybe a float will suffice, check if within range.

		float f{};
		auto [ptrFloat, errorFloat] = std::from_chars(value.data(), value.data() + value.size(), f);

		if (errorFloat == std::errc{})
		{
			return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .floatType = true} };
		}
	}

	// A double is required, but check if within range.

	double d{};
	auto [ptrDouble, errorDouble] = std::from_chars(value.data(), value.data() + value.size(), d);

	if (errorDouble == std::errc{})
	{
		return { .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .floatType = true} };
	}

	// FIXME: Throw?
	// Not a number or out of range.
	return { .kind = TypeSpec::Kind::Invalid };
}

TypeSpec Parser::ParseTypeSpec()
{
	Assert(GetTypeSpecTokens());

	TypeSpec t{ .kind = TypeSpec::Kind::Invalid };

	// []
	if (Peek(Token::Type::ParenSquareClose))
	{
		t = { .kind = TypeSpec::Kind::Any };
	}
	// [1]
	else if (Accept(Token::Type::LiteralInteger))
	{
		t = GetPrimitiveIntegerTypeSpec(GetCurrent().value);
	}
	// [1.0]
	else if (Accept(Token::Type::LiteralDecimal))
	{
		t = GetPrimitiveDecimalTypeSpec(GetCurrent().value);
	}
	// [#1]
	else if (Accept(Token::Type::SeparatorHash))
	{
		if (Accept(GetTaggedTypeSpecTokens()))
		{
			t = { .kind = TypeSpec::Kind::Tagged, .name = GetCurrent().value };
		}
		else
		{
			throw UnexpectedTokenException(GetNext());
		}
	}
	// [/type/i32]
	else if (Accept(Token::Type::SeparatorSlash))
	{
		Expect(Token::Type::Identifier, Token::Constant::TypeNameSpace);
		Expect(Token::Type::SeparatorSlash);
		Expect(Token::Type::Identifier);
		t = GetBuiltinTypeSpec(GetCurrent().value);

		if (t.Is(TypeSpec::Kind::Primitive))
		{
			t.nameSpace.root = true;
			t.nameSpace.parts.emplace_back(Token::Constant::TypeNameSpace);
		}
	}
	// [identifier]
	else if (Accept(Token::Type::Identifier))
	{
		t = { .kind = TypeSpec::Kind::Named, .name = GetCurrent().value };
	}
	// [()]
	else if (Accept(Token::Type::ParenRoundOpen))
	{
		Expect(Token::Type::ParenRoundClose);
		t = { .kind = TypeSpec::Kind::None };
	}
	else
	{
		throw UnexpectedTokenException(GetNext());
	}

	Expect(Token::Type::ParenSquareClose);

	// [identifier]#
	while (Accept(Token::Type::SeparatorHash))
	{
		t.arrayDims.emplace_back(0);

		if (Accept(Token::Type::LiteralInteger))
		{
			t.arrayDims.back() = to_int<int64_t>(GetCurrent().value).unsignedValue.value_or(0);

			if (!t.arrayDims.back())
			{
				// The given array length must be greater than zero.
				throw UnexpectedTokenException(GetCurrent());
			}
		}
	}

	return t;
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
			parameter->name = GetCurrent().value;
			defined = true;
		}

		// Optional type
		if (Accept(GetTypeSpecTokens()))
		{
			parameter->type = ParseTypeSpec();
			defined = true;
		}
		else
		{
			parameter->type = {.kind = TypeSpec::Kind::Any};
		}

		// Name and/or type must be specified
		if (defined)
		{
			parameters.emplace_back(parameter);
		}
		else
		{
			throw UnexpectedTokenException(GetCurrent());
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
			throw UnexpectedTokenException(GetCurrent());
		}
	}

	return parameters;
}

ApiSpec Parser::ParseApiSpec()
{
	ApiSpec apiSpec;

	if (Accept(Token::Type::LiteralString))
	{
		static const std::map<std::string, int> validApiTokens
		{
			{ "Dynamic", ApiSpec::Dynamic },
			{ "Standard", ApiSpec::Standard },
			{ "C", ApiSpec::C },
			{ "C++", ApiSpec::Cpp },
		};

		const auto apiTokens = split(GetCurrent().value, L' ');
		for (auto& apiToken : apiTokens)
		{
			if (validApiTokens.contains(apiToken))
			{
				apiSpec.Set(validApiTokens.at(apiToken));
			}
			else
			{
				throw ParseException(GetCurrent(), fmt::format("Invalid import/export API specification token \"{}\".", apiToken));
			}
		}
	}

	Expect(GetExternalFunctionDeclarationTokens());

	return apiSpec;
}

ApiSpec Parser::ParseImportApiSpec()
{
	auto apiSpec = ParseApiSpec();
	apiSpec.Set(ApiSpec::Import);
	return apiSpec;
}

ApiSpec Parser::ParseExportApiSpec()
{
	auto apiSpec = ParseApiSpec();
	apiSpec.Set(ApiSpec::Export);
	return apiSpec;
}

std::unique_ptr<Node> Parser::ParseFunctionDeclaration(std::shared_ptr<Context> context)
{
	auto node = make<Node>::unique({ .type = Node::Type::FunctionDeclaration, .token = GetCurrent() });

	// <-
	if (IsToken(Token::Type::OperatorLeftArrow))
	{
		node->apiSpec = ParseExportApiSpec();
	}
	// ->
	else if (IsToken(Token::Type::OperatorRightArrow))
	{
		node->apiSpec = ParseImportApiSpec();
	}

	// [inType]
	if (IsToken(GetTypeSpecTokens()))
	{
		node->inType = ParseTypeSpec();

		// [inType] identifier
		if (Accept(Token::Type::Identifier))
		{
			node->value = GetCurrent().value;
		}
	}
	// identifier
	// namespace/identifier
	// namespace/binaryOperator
	else if (IsToken(Token::Type::Identifier))
	{
		auto name = GetCurrent().value;

		// namespace1/namespaceN/identifier
		// namespace1/namespaceN/binaryOperator
		while (Accept(Token::Type::SeparatorSlash))
		{
			node->nameSpace.parts.emplace_back(name);

			// binaryOperator
			if (Accept(GetBinaryOperatorTokens()))
			{
				name = GetCurrent().value;
				break;
			}
			// identifier
			// namespaceN/identifier
			else
			{
				Expect(Token::Type::Identifier);
				name = GetCurrent().value;
			}
		}

		node->value = name;

		context->AddFunctionSymbol(node->GetQualifiedName());
	}
	// binaryOperator
	else if (IsToken(GetBinaryOperatorTokens()))
	{
		node->value = GetCurrent().value;
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
	if (Accept(GetTypeSpecTokens()))
	{
		node->outType = ParseTypeSpec();
	}

	// identifier:
	if (IsToken(Token::Type::SeparatorColon) || Accept(Token::Type::SeparatorColon))
	{
		if (node->value.empty())
		{
			// The anonymous main function has no output type.
			node->outType = { .kind = TypeSpec::Kind::None };
		}

		auto innerContext = make<Context>::shared({ .parent = context, .inType = node->inType });

		for (const auto& parameter : node->parameters)
		{
			innerContext->AddVariableSymbol(parameter);
		}

		node->left = ParseExpression(innerContext);
	}
	else if (Accept(Token::Type::SeparatorDot))
	{
		// Declaration only:
		// [#32] identifier [#32].
	}

	return node;
}

std::unique_ptr<Node> Parser::ParseCompoundExpression(std::shared_ptr<Context> context)
{
	auto node = ParseExpression(context);

	if (!Peek(GetExpressionTerminatorTokens()))
	{
		node->right = ParseCompoundExpression(context);
	}

	return node;
}

std::unique_ptr<Node> Parser::ParseExpression(std::shared_ptr<Context> context)
{
	auto firstToken = GetNext();

	const auto& inType = context->inType;
	auto innerContext = make<Context>::shared({ .parent = context, .inType = inType });

	std::vector<std::unique_ptr<Node>> nodes;

	for (;;)
	{
		if (Accept(GetOperandTokens()))
		{
			nodes.emplace_back(ParseOperand(innerContext));
		}
		else if (Accept(Token::Type::Identifier))
		{
			if (context->HasVariableSymbol(GetCurrent().value))
			{
				nodes.emplace_back(ParseVariableReference(innerContext));
			}
			else
			{
				nodes.emplace_back(ParseFunctionCall(innerContext));
			}
		}
		else if (Accept(GetBinaryOperatorTokens()))
		{
			nodes.emplace_back(ParseBinaryOperation(innerContext));
		}
		// TODO: Selector, bind
		else if (Accept(Token::Type::SeparatorDot) || Peek(GetExpressionTerminatorTokens()))
		{
			break;
		}
		else
		{
			throw UnexpectedTokenException(GetNext());
		}
	}

	if (nodes.empty())
	{
		return make<Node>::unique({ .type = Node::Type::Empty, .token = firstToken });
	}

	auto expression = make<Node>::unique({ .type = Node::Type::Expression, .outType = inType, .token = firstToken, .inType = inType });
	auto parent = expression.get();
	std::unique_ptr<Node> right;

	while (!nodes.empty())
	{
		auto node = std::move(nodes.back());
		nodes.pop_back();

		if (GetOperandNodes().contains(node->type))
		{
			if (right)
			{
				throw UnexpectedTokenException(node->token);
			}

			right = std::move(node);
		}
		else if (GetOperatorNodes().contains(node->type))
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
	else if (!parent->left || parent->left->type == Node::Type::Empty)
	{
		// Default expression input
		// TODO: Check if the expected in-type is None and the implicit input should be discarded?

		parent->left = make<Node>::unique(Node{ .type = Node::Type::ExpressionInput, .token = firstToken });
	}

	expression = ReduceExpression(std::move(expression));

	// The out type of an expression node is the data type of the first child node.
	if (expression->type == Node::Type::Expression && expression->left)
	{
		expression->outType = expression->left->outType;
	}

	return expression;
}

std::unique_ptr<Node> Parser::ReduceExpression(std::unique_ptr<Node>&& expression)
{
	if (expression->type != Node::Type::Expression)
	{
		// Only expression nodes can be reduced.
		return expression;
	}

	const bool leftEmpty = !expression->left || expression->left->type == Node::Type::Empty;
	const bool rightEmpty = !expression->right || expression->right->type == Node::Type::Empty;

	if (leftEmpty == rightEmpty)
	{
		// Can't reduce when either no or both children are defined.
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
		auto tuple = make<Node>::unique({ .type = Node::Type::Tuple, .token = GetCurrent() });
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
	else if (IsToken(GetLiteralTokens()))
	{
		node = make<Node>::unique(
			{
				.type = Node::Type::Literal,
				.value = GetCurrent().value,
				.token = GetCurrent()
			});

		switch (GetCurrent().type)
		{
		case Token::Type::LiteralInteger:
			node->outType = GetPrimitiveIntegerTypeSpec(GetCurrent().value);
			break;

		case Token::Type::LiteralDecimal:
			node->outType = GetPrimitiveDecimalTypeSpec(GetCurrent().value);
			break;

		case Token::Type::LiteralString:
			node->outType = TypeSpec{ .kind = TypeSpec::Kind::Primitive, .arrayDims{0}, .primitive{.bits = 8, .signedType = true} };
		}
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

	auto node = make<Node>::unique({ .type = Node::Type::FunctionCall, .value = GetCurrent().value, .token = GetCurrent() });

	// TODO: nameSpace

	if (Accept(Token::Type::ParenRoundOpen))
	{
		node->right = ParseGroup(context);
	}

	return node;
}

std::unique_ptr<Node> Parser::ParseBinaryOperation(std::shared_ptr<Context> context)
{
	Assert(GetBinaryOperatorTokens());

	return make<Node>::unique({ .type = Node::Type::BinaryOperation, .value = GetCurrent().value, .token = GetCurrent() });
}

std::unique_ptr<Node> Parser::ParseVariableReference(std::shared_ptr<Context> context)
{
	Assert(Token::Type::Identifier);

	return make<Node>::unique({ .type = Node::Type::VariableReference, .value = GetCurrent().value, .token = GetCurrent() });
}
