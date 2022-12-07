#pragma once
#include "Token.h"

struct ILexer : public IEnumerator<char>
{
	using Generator = tl::generator<Token>;

	[[nodiscard]] virtual Generator Lex() noexcept = 0;
};
