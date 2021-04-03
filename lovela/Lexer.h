#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
public:
	Lexer(std::wistream& charStream) noexcept;

	[[nodiscard]] TokenGenerator Lex() noexcept override;

private:
	Token AddToken(Token token) const;

	[[nodiscard]] bool Accept();
	[[nodiscard]] bool Accept(wchar_t token);
	[[nodiscard]] bool Accept(const std::vector<wchar_t>& tokens);
	void Expect(wchar_t token);
	void Expect(const std::vector<wchar_t>& tokens);

	std::wistream& charStream;
	wchar_t currentToken{};
	wchar_t previousToken{};
	wchar_t nextToken{};
	bool hasNextToken{};
	std::deque<wchar_t> currentCode;
};
