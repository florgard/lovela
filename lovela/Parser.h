#pragma once
#include <vector>
#include "ILexer.h"

struct Node
{
	enum class Type
	{
		Empty,
		Root,
		List,
		Function,
	} type{};

	std::vector<Node> children;
	std::wstring name;
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

	[[nodiscard]] Node ParseFunction();

	TokenGenerator tokenGenerator;
	TokenGenerator::iterator tokenIterator;
	const Token* currentToken{};
	std::vector<Error> errors;
};
