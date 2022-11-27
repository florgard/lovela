#pragma once
#include "Token.h"
#include "NameSpace.h"
#include "TypeSpec.h"
#include "ApiSpec.h"
#include "FunctionDeclaration.h"
#include "VariableDeclaration.h"

struct Node
{
	enum class Type
	{
		Empty,
		Error,
		FunctionDeclaration,
		Expression,
		ExpressionInput,
		Tuple,
		Literal,
		FunctionCall,
		VariableReference,
		BinaryOperation,
	} type{};

	std::string value;
	TypeSpec outType{};
	Token token{};

	// Function declaration
	NameSpace nameSpace{};
	TypeSpec inType{};
	ParameterList parameters{};
	ApiSpec api{};

	// Function call
	std::shared_ptr<FunctionDeclaration> callee;

	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;

	struct Error
	{
		enum class Code
		{
			NoError,
			ParseError,
		} code{};

		std::string message;

		[[nodiscard]] constexpr auto operator<=>(const Error& rhs) const noexcept = default;
	} error{};

	[[nodiscard]] bool operator==(const Node& rhs) const noexcept
	{
		// Compare owned data (not the child nodes, token or callee)
		return rhs.type == type
			&& rhs.value == value
			&& rhs.outType == outType
			&& rhs.nameSpace == nameSpace
			&& rhs.inType == inType
			&& rhs.api == api
			&& std::equal(rhs.parameters.begin(), rhs.parameters.end(), parameters.begin(), [](const auto& v1, const auto& v2) { return *v1 == *v2; });
	}

	[[nodiscard]] bool operator!=(const Node& rhs) const noexcept { return !operator==(rhs); }
	[[nodiscard]] operator bool() const noexcept { return type != Type::Empty; }

	[[nodiscard]] FunctionDeclaration ToFunctionDeclaration() const
	{
		return FunctionDeclaration
		{
			.name = value,
			.nameSpace = nameSpace,
			.outType = outType,
			.inType = inType,
			.parameters = parameters,
			.api = api,
		};
	}

	[[nodiscard]] std::string GetQualifiedName() const
	{
		std::ostringstream s;

		nameSpace.Print(s);
		s << value;

		return s.str();
	}

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << to_string(type) << ',';
		outType.Print(stream);
		stream << ',';
		token.Print(stream);
		stream << ',';
		nameSpace.Print(stream);
		stream << ',';
		inType.Print(stream);
		stream << ',' << '[';
		for (auto& param : parameters)
		{
			param->Print(stream);
			stream << ',';
		}
		stream << ']' << ',';
		api.Print(stream);
		stream << ']';
	}
};
