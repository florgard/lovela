#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
public:
	Lexer(std::wistream& charStream) noexcept;

	[[nodiscard]] TokenGenerator Lex() noexcept override;

private:
	Token AddToken(Token token) const;

	std::wistream& charStream;
	std::deque<wchar_t> currentCode;
};
