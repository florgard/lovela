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

struct Node
{
	enum class Type
	{
		Empty,
		Root,
		FunctionDeclaration,
		Expression,
		Tuple,
		List,
		FunctionCall,
		BinaryOperation,
		Literal,
	} type{};

	std::wstring name;
	std::wstring value;
	TypeSpec dataType;
	Token token;

	// Function declaration
	std::vector<std::wstring> nameSpace;
	TypeSpec objectType;
	ParameterList parameters;
	bool imported{};
	bool exported{};

	//std::unique_ptr<Node> left;
	//std::unique_ptr<Node> right;
	std::vector<std::unique_ptr<Node>> children;

	[[nodiscard]] bool operator==(const Node& rhs) const noexcept;
	[[nodiscard]] bool operator!=(const Node& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	[[nodiscard]] operator bool() const noexcept { return type != Type::Empty; }

	static std::unique_ptr<Node> make_unique(Node& node) noexcept { return move_to_unique(node); }
	static std::unique_ptr<Node> make_unique(Node&& node) noexcept { return move_to_unique(node); }

	static std::vector<std::unique_ptr<Node>> make_vector(Node&& node1) noexcept
	{
		std::vector<std::unique_ptr<Node>> v;
		v.emplace_back(move_to_unique(node1));
		return v;
	}

	static std::vector<std::unique_ptr<Node>> make_vector(Node&& node1, Node&& node2) noexcept
	{
		std::vector<std::unique_ptr<Node>> v;
		v.emplace_back(move_to_unique(node1));
		v.emplace_back(move_to_unique(node2));
		return v;
	}

	static std::vector<std::unique_ptr<Node>> make_vector(Node& node1) noexcept
	{
		return make_vector(std::move(node1));
	}

	static std::vector<std::unique_ptr<Node>> make_vector(Node& node1, Node& node2) noexcept
	{
		return make_vector(std::move(node1), std::move(node2));
	}
};
