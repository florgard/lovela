#pragma once

struct LexerPatterns
{
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

	const Regex whitespace{ std::regex(whitespaceRegex), 1 };
	const Regex separator{ std::regex(separatorRegex), 1 };
	const Regex digit{ std::regex(digitRegex), 1 };

	const std::regex& GetBeginComment();
	const std::regex& GetEndComment();
	const std::regex& GetBeginLiteralNumber();
	const std::regex& GetBeginDecimalPart();
	const std::regex& GetBeginDecimalExponent();
	const std::regex& GetBeginString();
	const std::regex& GetStringField();

private:
	// https://stackoverflow.com/questions/399078/what-special-characters-must-be-escaped-in-regular-expressions
	static constexpr const char* whitespaceRegex{ R"(\s)" };
	static constexpr const char* separatorRegex{ R"([()[\]{}.,:;!?/|#])" };
	static constexpr const char* digitRegex{ R"(\d)" };
};
