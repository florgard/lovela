#pragma once
#include <vector>
#include <istream>
#include "Token.h"

class ILexer
{
public:
	virtual std::vector<Token> Lex(std::wistream& charStream) noexcept = 0;
};

class Lexer1 : public ILexer
{
public:
	std::vector<Token> Lex(std::wistream& charStream) noexcept override;
};

class Lexer2 : public ILexer
{
public:
	std::vector<Token> Lex(std::wistream& charStream) noexcept override;
};
