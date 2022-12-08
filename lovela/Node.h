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
		Comment,
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

		[[nodiscard]] constexpr bool operator==(const Error& rhs) const noexcept
		{
			return code == rhs.code;
		}

		[[nodiscard]] constexpr bool operator!=(const Error& rhs) const noexcept
		{
			return !operator==(rhs);
		}
	} error{};

	[[nodiscard]] constexpr bool operator==(const Node& rhs) const noexcept
	{
		// Compare owned data (not the child nodes, token or callee)
		return type == rhs.type
			&& value == rhs.value
			&& outType == rhs.outType
			&& nameSpace == rhs.nameSpace
			&& inType == rhs.inType
			&& api == rhs.api
			&& error == rhs.error
			&& std::equal(parameters.begin(), parameters.end(), rhs.parameters.begin(), [](auto& p1, auto& p2) { return *p1 == *p2; });
	}

	[[nodiscard]] constexpr bool operator!=(const Node& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	[[nodiscard]] constexpr operator bool() const noexcept
	{
		return type != Type::Empty;
	}

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
		stream << '\"' << value << '\"' << ',';
		stream << "outType=";
		outType.Print(stream);
		stream << ',';
		stream << "token=";
		token.Print(stream);
		stream << ',';
		stream << "nameSpace=";
		nameSpace.Print(stream);
		stream << ',';
		stream << "inType=";
		inType.Print(stream);
		stream << ',';
		stream << "parameters=[";
		for (auto& param : parameters)
		{
			param->Print(stream);
			stream << ',';
		}
		stream << ']' << ',';
		stream << "apiSpec=";
		api.Print(stream);
		stream << ',';
		stream << "errorCode=" << to_string(error.code) << ']';
	}
};
