#pragma once
#include "Token.h"

struct TypeSpec
{
	enum class Kind
	{
		Any,
		None,
		Tagged,
		Named,
		Primitive,
		Invalid,
	} kind{};

	std::string name;

	struct Primitive
	{
		unsigned char bits{};
		bool signedType{};
		bool floatType{};
		bool arrayType{};
		size_t arrayLength{};

		[[nodiscard]] constexpr auto operator<=>(const Primitive& rhs) const noexcept = default;
	} primitive{};

	[[nodiscard]] constexpr bool Is(Kind k) const noexcept { return kind == k; }

	[[nodiscard]] constexpr auto operator<=>(const TypeSpec& rhs) const noexcept = default;

private:
	static constexpr const char* noneTypeName = "()";
};

struct VariableDeclaration
{
	std::string name;
	TypeSpec type;

	[[nodiscard]] auto operator<=>(const VariableDeclaration& rhs) const noexcept = default;
};

using ParameterList = std::vector<std::shared_ptr<VariableDeclaration>>;

struct ApiSpec
{
	static constexpr int None = 0;
	static constexpr int Import = 1 << 0;
	static constexpr int Export = 1 << 1;
	static constexpr int Dynamic = 1 << 2;
	static constexpr int Standard = 1 << 3;
	static constexpr int C = 1 << 4;
	static constexpr int Cpp = 1 << 5;

	constexpr ApiSpec() noexcept = default;
	constexpr ApiSpec(int flags) noexcept : flags(flags) {}

	[[nodiscard]] constexpr auto operator<=>(const ApiSpec& rhs) const noexcept = default;
	
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

private:
	int flags{};
};

struct FunctionDeclaration
{
	std::string name;
	std::vector<std::string> nameSpace;
	TypeSpec outType;
	TypeSpec inType;
	ParameterList parameters;
	ApiSpec api{};
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

	std::string value;
	TypeSpec outType;
	Token token;

	// Function declaration
	std::vector<std::string> nameSpace;
	TypeSpec inType;
	ParameterList parameters;
	ApiSpec api{};

	// Function call
	std::shared_ptr<FunctionDeclaration> callee;

	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;

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
		std::ostringstream name;
		for (auto& part : nameSpace)
		{
			name << part << '|';
		}
		name << value;
		return name.str();
	}
};
