#pragma once
#include "LexerBase.h"
#include "LexerRegexes.h"

class Lexer : public LexerBase
{
	friend class LexerFactory;
	friend class LexerTest;

protected:
	Lexer(std::istream& charStream) noexcept;

public:
	[[nodiscard]] TokenGenerator Lex() noexcept override;

private:
	[[nodiscard]] bool Accept() noexcept;
	[[nodiscard]] bool Accept(char character) noexcept;
	[[nodiscard]] bool Accept(const std::regex& regex, size_t length) noexcept;
	[[nodiscard]] bool AcceptBegin(char character) noexcept;
	[[nodiscard]] bool AcceptBegin(const std::regex& regex, size_t length) noexcept;
	[[nodiscard]] bool Expect(char character) noexcept;
	[[nodiscard]] bool Expect(const std::regex& regex, size_t length) noexcept;

	[[nodiscard]] void LexLiteralString() noexcept;
	[[nodiscard]] void LexLiteralNumber() noexcept;
	[[nodiscard]] void LexComment() noexcept;
	[[nodiscard]] void LexSeparator() noexcept;
	[[nodiscard]] void LexWhitespace() noexcept;

	[[nodiscard]] static constexpr Token::Type GetTokenType(char lexeme) noexcept
	{
		constexpr std::array<std::pair<char, Token::Type>, 14> values
		{ {
			{'(', Token::Type::ParenRoundOpen },
			{')', Token::Type::ParenRoundClose },
			{'[', Token::Type::ParenSquareOpen },
			{']', Token::Type::ParenSquareClose },
			{'{', Token::Type::ParenCurlyOpen },
			{'}', Token::Type::ParenCurlyClose },
			{'.', Token::Type::SeparatorDot },
			{',', Token::Type::SeparatorComma },
			{'!', Token::Type::SeparatorExclamation },
			{'?', Token::Type::SeparatorQuestion },
			{'|', Token::Type::SeparatorVerticalLine },
			{'/', Token::Type::SeparatorSlash },
			{':', Token::Type::SeparatorColon },
			{'#', Token::Type::SeparatorHash },
		} };

		constexpr auto map = static_map<char, Token::Type, values.size()>{ {values} };
		return map.at_or(lexeme, Token::Type::Empty);
	}

	[[nodiscard]] Token GetToken(char lexeme) noexcept override;
	[[nodiscard]] Token GetToken(std::string_view lexeme) noexcept override;

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

	LexerRegexes regexes;
};
