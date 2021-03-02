#pragma once
#include <vector>
#include "ILexer.h"

struct TypeSpec
{
	std::wstring name;
	bool any{};
	bool empty{};

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
		List,
		Function,
	} type{};

	std::wstring name;
	TypeSpec functionType;
	TypeSpec objectType;
	ParameterList parameters;

	std::vector<Node> children;

	bool operator==(const Node& rhs) const noexcept;
	bool operator!=(const Node& rhs) const noexcept
	{
		return !operator==(rhs);
	}
};

class Parser
{
public:
	struct Error
	{
		enum class Code
		{
			ParseError,
		} code{};

		std::wstring message;

		constexpr bool operator<=>(const Error& rhs) const noexcept = default;
	};

	Parser(TokenGenerator&& tokenGenerator) noexcept;

	[[nodiscard]] Node Parse() noexcept;

private:
	void Expect(Token::Type type);
	[[nodiscard]] bool Accept(Token::Type type);

	[[nodiscard]] TypeSpec ParseTypeSpec();
	[[nodiscard]] ParameterList ParseParameterList();
	[[nodiscard]] Node ParseFunction();

	TokenGenerator tokenGenerator;
	TokenGenerator::iterator tokenIterator;
	Token currentToken;
	std::vector<Error> errors;
};
