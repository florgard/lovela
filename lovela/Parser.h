#pragma once
#include <vector>
#include <memory>
#include "ILexer.h"

struct Node
{
	enum class Type
	{
		Root,
		List,
		Function,
	} type;

	std::vector<std::unique_ptr<Node>> children;

	Node(Type type) noexcept;
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

	[[nodiscard]] std::unique_ptr<Node> Parse() noexcept;

private:
	void Expect(Token::Type type);
	[[nodiscard]] bool Accept(Token::Type type);

	[[nodiscard]] std::unique_ptr<Node> ParseFunction();

	TokenGenerator tokenGenerator;
	TokenGenerator::iterator tokenIterator;
	const Token* currentToken{};
	std::vector<Error> errors;
};
