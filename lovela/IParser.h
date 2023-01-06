#pragma once
#include "Node.h"
#include "ILexer.h"

struct IParser : public IEnumerator<Token>
{
	using OutputT = tl::generator<Node>;

	[[nodiscard]] virtual OutputT Parse() noexcept = 0;
};

template <class ParserT, class TokenRangeT = ILexer::OutputT>
using BasicRangeParser = RangeEnumerator<ParserT, TokenRangeT>;

template <class ParserT, class TokenRangeT = ILexer::OutputT>
using BasicRangeRefParser = RangeRefEnumerator<ParserT, TokenRangeT>;

inline std::vector<Node>& operator>>(IParser::OutputT&& input, std::vector<Node>& output)
{
	output = std::move(to_vector(std::move(input)));
	return output;
}
