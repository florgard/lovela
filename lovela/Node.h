#pragma once
#include "Token.h"

template <typename CharT>
struct basic_type_spec
{
	using StringT = std::basic_string<CharT>;

	StringT name;

	bool Any() const { return name.empty(); }
	bool None() const { return name == noneTypeName; }
	void SetAny() { name.clear(); }
	void SetNone() { name = noneTypeName; }

	static basic_type_spec AnyType() { return {}; };
	static basic_type_spec NoneType() { return { .name = noneTypeName }; };

	// FIXME: Causes an internal compiler error in ut.hpp with VS 17.4
	//[[nodiscard]] auto operator<=>(const TypeSpec& rhs) const noexcept = default;
	[[nodiscard]] bool operator==(const basic_type_spec& rhs) const noexcept
	{
		return name == rhs.name;
	}

private:
	static constexpr CharT noneTypeName[3]{ '(', ')', '\0' };
};

using TypeSpec = basic_type_spec<DefaultCharT>;

template <typename CharT>
struct basic_variable_declaration
{
	using StringT = std::basic_string<CharT>;
	using TypeSpecT = basic_type_spec<CharT>;

	StringT name;
	TypeSpecT type;

	[[nodiscard]] auto operator<=>(const basic_variable_declaration& rhs) const noexcept = default;
};

using VariableDeclaration = basic_variable_declaration<DefaultCharT>;

template <typename CharT>
using basic_parameter_list = std::vector<std::shared_ptr<basic_variable_declaration<CharT>>>;

using ParameterList = basic_parameter_list<DefaultCharT>;

template <typename CharT>
struct basic_api
{
	static constexpr int None = 0;
	static constexpr int Import = 1 << 0;
	static constexpr int Export = 1 << 1;
	static constexpr int Dynamic = 1 << 2;
	static constexpr int Standard = 1 << 3;
	static constexpr int C = 1 << 4;
	static constexpr int Cpp = 1 << 5;

	constexpr basic_api() noexcept = default;
	constexpr basic_api(int flags) noexcept : flags(flags) {}

	[[nodiscard]] constexpr auto operator<=>(const basic_api& rhs) const noexcept = default;
	
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

using Api = basic_api<DefaultCharT>;

template <typename CharT>
struct basic_function_declaration
{
	using StringT = std::basic_string<CharT>;
	using TypeSpecT = basic_type_spec<CharT>;
	using ParameterListT = basic_parameter_list<CharT>;
	using ApiT = basic_api<CharT>;

	StringT name;
	std::vector<StringT> nameSpace;
	TypeSpecT outType;
	TypeSpecT inType;
	ParameterListT parameters;
	ApiT api{};
};

using FunctionDeclaration = basic_function_declaration<DefaultCharT>;

template <typename CharT>
struct basic_node
{
	using StringT = std::basic_string<CharT>;
	using TypeSpecT = basic_type_spec<CharT>;
	using TokenT = basic_token<CharT>;
	using ParameterListT = basic_parameter_list<CharT>;
	using ApiT = basic_api<CharT>;
	using FunctionDeclarationT = basic_function_declaration<CharT>;

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

	StringT value;
	TypeSpecT outType;
	TokenT token;

	// Function declaration
	std::vector<StringT> nameSpace;
	TypeSpecT inType;
	ParameterListT parameters;
	ApiT api{};

	// Function call
	std::shared_ptr<FunctionDeclarationT> callee;

	std::unique_ptr<basic_node> left;
	std::unique_ptr<basic_node> right;

	[[nodiscard]] bool operator==(const basic_node& rhs) const noexcept
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

	[[nodiscard]] bool operator!=(const basic_node& rhs) const noexcept { return !operator==(rhs); }
	[[nodiscard]] operator bool() const noexcept { return type != Type::Empty; }

	[[nodiscard]] FunctionDeclarationT ToFunctionDeclaration() const
	{
		return FunctionDeclarationT
		{
			.name = value,
			.nameSpace = nameSpace,
			.outType = outType,
			.inType = inType,
			.parameters = parameters,
			.api = api,
		};
	}

	[[nodiscard]] StringT GetQualifiedName() const
	{
		std::basic_ostringstream<CharT> name;
		for (auto& part : nameSpace)
		{
			name << part << '|';
		}
		name << value;
		return name.str();
	}
};

using Node = basic_node<DefaultCharT>;
