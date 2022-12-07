#pragma once
#include "Node.h"
#include "ILexer.h"

struct IParser : public IEnumerator<Token>
{
	using Generator = tl::generator<Node>;

	[[nodiscard]] virtual Generator Parse() noexcept = 0;
};

template <class ParserT, class TokenRangeT = ILexer::Generator>
using BasicRangeParser = RangeEnumerator<ParserT, TokenRangeT>;
