#pragma once
#include "LexerBase.h"

class Lexer : public LexerBase
{
	friend class LexerFactory;
	friend class LexerTest;

	Lexer(std::istream& charStream) noexcept;

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
		constexpr std::array<std::pair<LexerPatterns::Char, Token::Type>, 14> values
		{ {
			{ LexerPatterns::parenRoundOpen , Token::Type::ParenRoundOpen },
			{ LexerPatterns::parenRoundClose, Token::Type::ParenRoundClose },
			{ LexerPatterns::parenSquareOpen, Token::Type::ParenSquareOpen },
			{ LexerPatterns::parenSquareClose, Token::Type::ParenSquareClose },
			{ LexerPatterns::parenCurlyOpen, Token::Type::ParenCurlyOpen },
			{ LexerPatterns::parenCurlyClose, Token::Type::ParenCurlyClose },
			{ LexerPatterns::separatorDot, Token::Type::SeparatorDot },
			{ LexerPatterns::separatorComma, Token::Type::SeparatorComma },
			{ LexerPatterns::separatorExclamation, Token::Type::SeparatorExclamation },
			{ LexerPatterns::separatorQuestion, Token::Type::SeparatorQuestion },
			{ LexerPatterns::separatorVerticalLine, Token::Type::SeparatorVerticalLine },
			{ LexerPatterns::separatorSlash, Token::Type::SeparatorSlash },
			{ LexerPatterns::separatorColon, Token::Type::SeparatorColon },
			{ LexerPatterns::separatorHash, Token::Type::SeparatorHash },
		} };

		constexpr auto map = static_map<char, Token::Type, values.size()>{ {values} };
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
