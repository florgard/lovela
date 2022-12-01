#pragma once

struct LexerPatterns
{
	struct Chars
	{
		char first;
		char second;
	};

	struct Regex
	{
		/// <summary>
		/// The regular expression to match.
		/// </summary>
		const std::regex regex;
		/// <summary>
		/// The number of characters to match. Must be 1 or 2.
		/// </summary>
		const size_t length;
	};

	// https://stackoverflow.com/questions/399078/what-special-characters-must-be-escaped-in-regular-expressions

	const Regex whitespace{ std::regex(R"(\s)"), 1 };
	const Regex separator{ std::regex(R"([()[\]{}.,:;!?/|#])"), 1 };
	const Regex digit{ std::regex(R"(\d)"), 1 };

	const Chars beginComment{ '<', '<' };
	const Chars endComment{ '>', '>' };

	const Regex beginLiteralNumber{ std::regex(R"((\d.|[+\-]\d))"), 2 };
	const Regex beginDecimalPart{ std::regex(R"(\.\d)"), 2 };
	const Regex beginDecimalExponent{ std::regex(R"([eE][+\-\d])"), 2 };

	const char beginString{ '\'' };
	const Regex stringField{ std::regex(R"([tnr])"), 1 };
};
