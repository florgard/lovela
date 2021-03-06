#pragma once

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
		Function,
		Statement,
		Expression,
		Group,
		List,
	} type{};

	std::wstring name;
	std::wstring value;
	TypeSpec dataType;

	// Function declaration
	std::vector<std::wstring> nameSpace;
	TypeSpec objectType;
	ParameterList parameters;
	bool imported{};
	bool exported{};

	std::vector<Node> children;

	[[nodiscard]] bool operator==(const Node& rhs) const noexcept;
	[[nodiscard]] bool operator!=(const Node& rhs) const noexcept
	{
		return !operator==(rhs);
	}
};
