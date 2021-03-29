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

struct VariableDeclaration
{
	std::wstring name;
	TypeSpec type;

	[[nodiscard]] auto operator<=>(const VariableDeclaration& rhs) const noexcept = default;
};

using ParameterList = std::vector<std::shared_ptr<VariableDeclaration>>;

class Api
{
	int flags{};

public:
	static constexpr int None = 0;
	static constexpr int Import = 1;
	static constexpr int Export = 2;
	static constexpr int Dynamic = 4;
	static constexpr int Standard = 8;
	static constexpr int C = 16;
	static constexpr int Cpp = 32;

	constexpr Api() noexcept = default;
	constexpr Api(int flags) noexcept : flags(flags) {}

	constexpr auto operator<=>(const Api& rhs) const noexcept = default;
	
	constexpr void Set(int flag)
	{
		flags |= flag;
	}

	constexpr bool Is(int flag) const
	{
		return (flags & flag) == flag;
	}

	constexpr bool IsExplicit() const
	{
		return flags > Export;
	}
};

struct FunctionDeclaration
{
	std::wstring name;
	std::vector<std::wstring> nameSpace;
	TypeSpec outType;
	TypeSpec inType;
	ParameterList parameters;
	Api api{};
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
		VariableReference,
		BinaryOperation,
	} type{};

	std::wstring value;
	TypeSpec outType;
	Token token;

	// Function declaration
	std::vector<std::wstring> nameSpace;
	TypeSpec inType;
	ParameterList parameters;
	Api api{};

	// Function call
	std::shared_ptr<FunctionDeclaration> callee;

	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;

	[[nodiscard]] bool operator==(const Node& rhs) const noexcept;
	[[nodiscard]] bool operator!=(const Node& rhs) const noexcept { return !operator==(rhs); }
	[[nodiscard]] operator bool() const noexcept { return type != Type::Empty; }
	[[nodiscard]] FunctionDeclaration ToFunctionDeclaration() const;
};
