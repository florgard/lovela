#pragma once

struct LexerPatterns
{
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

	// https://stackoverflow.com/questions/399078/what-special-characters-must-be-escaped-in-regular-expressions

	const Regex<1> whitespace{ R"(\s)" };
	const Regex<1> separator{ R"([()[\]{}.,:;!?/|#])" };
	const Regex<1> digit{ R"(\d)" };

	const Chars beginComment{ '<', '<' };
	const Chars endComment{ '>', '>' };

	const Regex<2> beginLiteralNumber{ R"((\d.|[+\-]\d))" };
	const Regex<2> beginDecimalPart{ R"(\.\d)" };
	const Regex<2> beginDecimalExponent{ R"([eE][+\-\d])" };

	const char beginString{ '\'' };
	const Regex<1> stringField{ R"([tnr])" };
};
