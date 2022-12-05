#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
	friend class LexerFactory;
	friend class LexerTest;

public:
	[[nodiscard]] TokenGenerator Lex() noexcept override;

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

struct StreamLexer : public RangeEnumerator<Lexer, std::ranges::istream_view<char>>
{
	StreamLexer(std::istream& input) noexcept
		: RangeEnumerator(std::ranges::istream_view<char>(input >> std::noskipws))
	{
	}
};

inline auto operator>>(std::istream& input, std::unique_ptr<StreamLexer>& lexer)
{
	if (!lexer)
	{
		lexer = std::make_unique<StreamLexer>(input);
	}

	return lexer->Lex();
}
