#pragma once
#include "Token.h"

struct ILexer : public IEnumerator<char>
{
	using Generator = tl::generator<Token>;

	[[nodiscard]] virtual Generator Lex() noexcept = 0;
};

template <class LexerT>
using BasicStringLexer = RangeEnumerator<LexerT, std::string_view>;

template <class LexerT, class BaseT = RangeEnumerator<LexerT, std::ranges::istream_view<char>>>
struct BasicStreamLexer
{
	std::unique_ptr<BaseT> _base;

	BasicStreamLexer() noexcept = default;

	BasicStreamLexer(std::istream& stream) noexcept
	{
		Initialize(stream);
	}

	void Initialize(std::istream& stream) noexcept
	{
		_base = std::make_unique<BaseT>(std::ranges::istream_view<char>(stream >> std::noskipws));
	}

	auto Lex() noexcept
	{
		return _base->Lex();
	}
};

inline std::vector<Token>& operator>>(ILexer::Generator&& input, std::vector<Token>& output)
{
	output = std::move(to_vector(std::move(input)));
	return output;
}
