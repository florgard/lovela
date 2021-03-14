#pragma once
#include "Token.h"

struct TypeSpec
{
	std::wstring name;
	bool any{};
	bool none{};

	[[nodiscard]] auto operator<=>(const TypeSpec& rhs) const noexcept = default;
};

struct Parameter
{
	std::wstring name;
	TypeSpec type;

	[[nodiscard]] auto operator<=>(const Parameter& rhs) const noexcept = default;
};

using ParameterList = std::vector<Parameter>;

struct FunctionDeclaration
{
	std::wstring name;
	std::vector<std::wstring> nameSpace;
	TypeSpec dataType;
	TypeSpec objectType;
	ParameterList parameters;
	bool imported{};
	bool exported{};
};

struct Node
{
	enum class Type
	{
		Empty,
		FunctionDeclaration,
		Expression,
		Tuple,
		Literal,
		FunctionCall,
		BinaryOperation,
	} type{};

	std::wstring value;
	TypeSpec dataType;
	Token token;

	// Function declaration
	std::vector<std::wstring> nameSpace;
	TypeSpec objectType;
	ParameterList parameters;
	bool imported{};
	bool exported{};

	// Function call
	std::shared_ptr<FunctionDeclaration> callee;

	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;

	[[nodiscard]] bool operator==(const Node& rhs) const noexcept;
	[[nodiscard]] bool operator!=(const Node& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	[[nodiscard]] operator bool() const noexcept { return type != Type::Empty; }

	static std::unique_ptr<Node> make_unique(Node& node) noexcept { return move_to_unique(node); }
	static std::unique_ptr<Node> make_unique(Node&& node) noexcept { return move_to_unique(node); }

	FunctionDeclaration ToFunctionDeclaration() const;
};
