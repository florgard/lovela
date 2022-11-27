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

protected:
	size_t currentLine = 1;
	size_t currentTokenColumn = 1;
	size_t nextTokenColumn = 1;
	std::ostringstream currentSourceCode;

	size_t firstSourceCodeLine = 1;
	std::deque<std::string> sourceCodeLines;

	struct Color
	{
		std::string_view none = "\033[0m";
		std::string_view fail = "\033[97m\033[41m";
		std::string_view code = "\033[96m";
	} color;
};
