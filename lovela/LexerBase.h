#pragma once
#include "ILexer.h"

class LexerBase : public ILexer
{
public:
	void PrintErrorSourceCode(std::ostream& stream, const Token& token) noexcept override;

	[[nodiscard]] static constexpr std::string_view Trim(const std::string_view& input) noexcept
	{
		constexpr std::array<char, 7> whitespace{ ' ', '\t', '\r', '\n', '\f', '\v', '\0' };

		const auto start = input.find_first_not_of(whitespace.data());
		if (start == input.npos)
		{
			return {};
		}

		const auto end = input.find_last_not_of(whitespace.data(), input.npos, whitespace.size());
		return input.substr(start, end - start + 1);
	}

	[[nodiscard]] static constexpr std::wstring_view Trim(const std::wstring_view& input) noexcept
	{
		constexpr std::array<wchar_t, 7> whitespace{ L' ', L'\t', L'\r', L'\n', L'\f', L'\v', L'\0' };

		const auto start = input.find_first_not_of(whitespace.data());
		if (start == input.npos)
		{
			return {};
		}

		const auto end = input.find_last_not_of(whitespace.data(), input.npos, whitespace.size());
		return input.substr(start, end - start + 1);
	}

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

protected:
	[[nodiscard]] Token GetToken(char lexeme) noexcept;
	[[nodiscard]] Token GetToken(std::string_view lexeme) noexcept;

	size_t currentLine = 1;
	size_t currentColumn = 1;
	std::ostringstream currentSourceCode;

	size_t firstSourceCodeLine = 1;
	std::deque<std::string> sourceCodeLines;

	struct Color
	{
		std::string_view none = "\033[0m";
		std::string_view fail = "\033[97m\033[41m";
	} color;
};
