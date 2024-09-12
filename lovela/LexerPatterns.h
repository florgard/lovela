#pragma once
#include "Token.h"

// https://stackoverflow.com/questions/399078/what-special-characters-must-be-escaped-in-regular-expressions
// https://en.cppreference.com/w/cpp/regex/ecmascript
// https://www.regular-expressions.info/posixbrackets.html
// https://www.regular-expressions.info/unicode.html
// https://en.wikipedia.org/wiki/Combining_character

struct LexerPatterns
{
	// Special characters

	using Char = char;

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

	// Compound token identification patterns

	struct Chars
	{
		const char next;
		const char nextAfter;
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

	// Lexeme patterns

	template <typename CharT>
	struct LexemeRegex
	{
		const std::basic_regex<CharT> regex;

		constexpr LexemeRegex(const CharT* re) noexcept : regex(re)
		{
		}
	};

	const LexemeRegex<char> operatorComparison{ R"(<|>|<>|<=|>=|=)" };
	const LexemeRegex<char> operatorArithmetic{ R"(\+|-|\*|/|/*)" };
	const LexemeRegex<char> operatorBitwise{ R"(\*\*|\+\+|--)" };
	const LexemeRegex<char> operatorLeftArrow{ R"(<-)" };
	const LexemeRegex<char> operatorRightArrow{ R"(->)" };
	const LexemeRegex<char> identifierAnsi{ R"([[:alpha:]][\w<>=\+\-\*/]*)" };
	const LexemeRegex<wchar_t> identifierUnicode{ LR"([[:alpha:]][\w<>=\+\-\*/\u0300–\u036F\u1AB0–\u1AFF\u1DC0–\u1DFF\u20D0–\u20FF\uFE20–\uFE2F]*)" };

	// Character pattern to token type mapping
	
	using CharPattern = std::pair<Char, Token::Type>;

	static constexpr std::array<CharPattern, 14> charPatterns
	{ {
		{ parenRoundOpen , Token::Type::ParenRoundOpen },
		{ parenRoundClose, Token::Type::ParenRoundClose },
		{ parenSquareOpen, Token::Type::ParenSquareOpen },
		{ parenSquareClose, Token::Type::ParenSquareClose },
		{ parenCurlyOpen, Token::Type::ParenCurlyOpen },
		{ parenCurlyClose, Token::Type::ParenCurlyClose },
		{ separatorDot, Token::Type::SeparatorDot },
		{ separatorComma, Token::Type::SeparatorComma },
		{ separatorExclamation, Token::Type::SeparatorExclamation },
		{ separatorQuestion, Token::Type::SeparatorQuestion },
		{ separatorVerticalLine, Token::Type::SeparatorVerticalLine },
		{ separatorSlash, Token::Type::SeparatorSlash },
		{ separatorColon, Token::Type::SeparatorColon },
		{ separatorHash, Token::Type::SeparatorHash },
	} };

	// Lexeme pattern to token type mapping

	template <typename CharT>
	struct LexemePattern
	{
		const LexemeRegex<CharT>& pattern;
		const Token::Type tokenType;
	};

	const std::array<LexemePattern<char>, 6> lexemePatternsAnsi
	{ {
		{ operatorComparison, Token::Type::OperatorComparison },
		{ operatorArithmetic, Token::Type::OperatorArithmetic },
		{ operatorBitwise, Token::Type::OperatorBitwise },
		{ operatorLeftArrow, Token::Type::OperatorLeftArrow },
		{ operatorRightArrow, Token::Type::OperatorRightArrow },
		{ identifierAnsi, Token::Type::Identifier },
	} };

	const std::array<LexemePattern<wchar_t>, 1> lexemePatternsUnicode
	{ {
		{ identifierUnicode, Token::Type::Identifier },
	} };
};
