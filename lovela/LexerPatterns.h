#pragma once

struct LexerPatterns
{
	using Char = char;

	struct Chars
	{
		const char first;
		const char second;
	};

	template <size_t length>
	struct Regex
	{
		static_assert(length >= 1 && length <= 2, "The number of characters to match must be 1 or 2");

		const std::regex regex;

		constexpr Regex(const char* re) noexcept : regex(re)
		{
		}
	};

	// Special characters

	static constexpr Char parenRoundOpen{ '(' };
	static constexpr Char parenRoundClose{ ')' };
	static constexpr Char parenSquareOpen{ '[' };
	static constexpr Char parenSquareClose{ ']' };
	static constexpr Char parenCurlyOpen{ '{' };
	static constexpr Char parenCurlyClose{ '}' };
	static constexpr Char parenAngleOpen{ '<' };
	static constexpr Char parenAngleClose{ '>' };
	static constexpr Char separatorDot{ '.' };
	static constexpr Char separatorComma{ ',' };
	static constexpr Char separatorExclamation{ '!' };
	static constexpr Char separatorQuestion{ '?' };
	static constexpr Char separatorVerticalLine{ '|' };
	static constexpr Char separatorSlash{ '/' };
	static constexpr Char separatorColon{ ':' };
	static constexpr Char separatorHash{ '#' };
	static constexpr Char separatorQuote{ '\'' };
	static constexpr Char separatorNewLine{ '\n' };

	// Token specific characters

	static constexpr Char commentOpen{ parenAngleOpen };
	static constexpr Char commentClose{ parenAngleClose };

	static constexpr Char stringOpen{ separatorQuote };
	static constexpr Char stringClose{ separatorQuote };

	static constexpr Char stringFieldOpen{ parenCurlyOpen };
	static constexpr Char stringFieldClose{ parenCurlyClose };

	// Compound token patterns

	// https://stackoverflow.com/questions/399078/what-special-characters-must-be-escaped-in-regular-expressions

	const Regex<1> whitespace{ R"(\s)" };
	const Regex<1> separator{ R"([()[\]{}.,:;!?/|#])" };
	const Regex<1> digit{ R"(\d)" };

	const Chars beginComment{ commentOpen, commentOpen };
	const Chars endComment{ commentClose, commentClose };

	const Regex<2> beginLiteralNumber{ R"((\d.|[+\-]\d))" };
	const Regex<2> beginDecimalPart{ R"(\.\d)" };
	const Regex<2> beginDecimalExponent{ R"([eE][+\-\d])" };

	static constexpr Char beginString{ stringOpen };
	const Regex<1> stringField{ R"([tnr])" };
};
