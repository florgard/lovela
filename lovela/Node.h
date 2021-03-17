#pragma once
#include "Token.h"

class TypeSpec
{
	static constexpr wchar_t noneTypeName[3]{ L"()" };

public:
	std::wstring name;

	bool Any() const { return name.empty(); }
	bool None() const { return name == noneTypeName; }
	void SetAny() { name.clear(); }
	void SetNone() { name = noneTypeName; }

	static TypeSpec AnyType() { return {}; };
	static TypeSpec NoneType() { return { .name = noneTypeName }; };

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
	TypeSpec outType;
	TypeSpec inType;
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
		ExpressionInput,
		Tuple,
		Literal,
		FunctionCall,
		BinaryOperation,
	} type{};

	std::wstring value;
	TypeSpec outType;
	Token token;

	// Function declaration
	std::vector<std::wstring> nameSpace;
	TypeSpec inType;
	ParameterList parameters;
	bool imported{};
	bool exported{};

	// Function call
	std::shared_ptr<FunctionDeclaration> callee;

	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;

	[[nodiscard]] bool operator==(const Node& rhs) const noexcept;
	[[nodiscard]] bool operator!=(const Node& rhs) const noexcept { return !operator==(rhs); }
	[[nodiscard]] operator bool() const noexcept { return type != Type::Empty; }
	[[nodiscard]] static std::unique_ptr<Node> make_unique(Node& node) noexcept { return move_to_unique(node); }
	[[nodiscard]] static std::unique_ptr<Node> make_unique(Node&& node) noexcept { return move_to_unique(node); }
	[[nodiscard]] FunctionDeclaration ToFunctionDeclaration() const;
};
