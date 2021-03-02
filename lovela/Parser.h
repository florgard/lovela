#pragma once
#include <vector>
#include "ILexer.h"

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
		List,
		Function,
	} type{};

	std::wstring name;
	TypeSpec dataType;

	// Function declaration
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
	void Expect(const std::vector<Token::Type>& types);
	[[nodiscard]] bool Accept(Token::Type type);
	[[nodiscard]] bool Accept(const std::vector<Token::Type>& types);

	[[nodiscard]] TypeSpec ParseTypeSpec();
	[[nodiscard]] ParameterList ParseParameterList();
	[[nodiscard]] Node ParseFunctionDeclaration();

	TokenGenerator tokenGenerator;
	TokenGenerator::iterator tokenIterator;
	Token currentToken;
	std::vector<Error> errors;
};
