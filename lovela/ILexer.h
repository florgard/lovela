#pragma once
#include "Token.h"

using TokenGenerator = tl::generator<Token>;

class ILexer
{
public:
	virtual ~ILexer() = default;

	[[nodiscard]] virtual TokenGenerator Lex() noexcept = 0;

	virtual void PrintErrorSourceCode(std::ostream& stream, const Token& token) noexcept = 0;

protected:
	virtual void AddToken(Token&& token) noexcept = 0;
};
