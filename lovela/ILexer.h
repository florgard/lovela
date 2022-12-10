#pragma once
#include "Token.h"

struct ILexer : public IEnumerator<char>
{
	using OutputT = tl::generator<Token>;

	[[nodiscard]] virtual OutputT Lex() noexcept = 0;
};

template <class LexerT>
using BasicStringLexer = ConstRangeEnumerator<LexerT, std::string_view>;

template <class LexerT>
using BasicStreamLexer = StreamEnumerator<LexerT, std::istream>;

inline std::vector<Token>& operator>>(ILexer::OutputT&& input, std::vector<Token>& output)
{
	output = std::move(to_vector(std::move(input)));
	return output;
}
