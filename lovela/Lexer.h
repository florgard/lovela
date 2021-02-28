#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
	std::wistream& charStream;

public:
	Lexer(std::wistream& charStream) noexcept;
	TokenGenerator Lex() noexcept override;
};
