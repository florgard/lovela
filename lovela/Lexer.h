#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
	friend class LexerTest;

public:
	[[nodiscard]] OutputT Lex() noexcept override;

private:
	[[nodiscard]] void LexLiteralString() noexcept;
	[[nodiscard]] void LexLiteralNumber() noexcept;
	[[nodiscard]] OutputT LexComment() noexcept;
	[[nodiscard]] void LexSeparator() noexcept;

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

using StringLexer = BasicStringLexer<Lexer>;
using StreamLexer = BasicStreamLexer<Lexer>;

inline auto operator>>(std::string_view string, StringLexer& lexer)
{
	lexer.Initialize(std::move(string));
	return lexer.Lex();
}

inline auto operator>>(std::istream& stream, StreamLexer& lexer)
{
	lexer.Initialize(stream);
	return lexer.Lex();
}
