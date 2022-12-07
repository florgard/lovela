#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
	friend class LexerTest;

public:
	using Generator = tl::generator<Token>;

	[[nodiscard]] Generator Lex() noexcept override;

private:
	[[nodiscard]] void LexLiteralString() noexcept;
	[[nodiscard]] void LexLiteralNumber() noexcept;
	[[nodiscard]] void LexComment() noexcept;
	[[nodiscard]] void LexSeparator() noexcept;
	[[nodiscard]] void LexWhitespace() noexcept;

	[[nodiscard]] Token GetToken(char lexeme) noexcept override;
	[[nodiscard]] Token GetToken(std::string_view lexeme) noexcept override;

	[[nodiscard]] static constexpr Token::Type GetTokenType(char lexeme) noexcept
	{
		constexpr auto map = static_map<char, Token::Type, LexerPatterns::charPatterns.size()>{ {LexerPatterns::charPatterns} };
		return map.at_or(lexeme, Token::Type::Empty);
	}

	[[nodiscard]] constexpr static std::string_view GetStringField(char code) noexcept
	{
		constexpr std::array<std::pair<char, std::string_view>, 3> values
		{ {
			{'t', "\t"},
			{'n', "\n"},
			{'r', "\r"},
		} };

		constexpr auto map = static_map<char, std::string_view, values.size()>{ {values} };
		return map.at_or(code, {});
	}

	LexerPatterns patterns;
};

template <class LexerT = Lexer>
using StringLexer = RangeEnumerator<LexerT, std::string_view>;

inline auto operator>>(std::string_view string, StringLexer<>& lexer)
{
	lexer.Initialize(std::move(string));
	return lexer.Lex();
}

template <class LexerT = Lexer, class BaseT = RangeEnumerator<LexerT, std::ranges::istream_view<char>>>
struct StreamLexer
{
	std::unique_ptr<BaseT> _base;

	StreamLexer() noexcept = default;

	StreamLexer(StreamLexer<>& lexer) noexcept
	{
		Initialize(lexer);
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

inline auto operator>>(std::istream& stream, StreamLexer<>& lexer)
{
	lexer.Initialize(stream);
	return lexer.Lex();
}

inline std::vector<Token>& operator>>(ILexer::Generator&& input, std::vector<Token>& output)
{
	output = std::move(to_vector(std::move(input)));
	return output;
}
