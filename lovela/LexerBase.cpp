#include "pch.h"
#include "LexerBase.h"

using namespace std::literals::string_view_literals;

static_assert(LexerBase::Trim(""sv) == ""sv);
static_assert(LexerBase::Trim(" \t\r\n\f\v"sv) == ""sv);
static_assert(LexerBase::Trim("a b c \r\n"sv) == "a b c"sv);

Token LexerBase::GetToken(char lexeme) noexcept
{
	auto type = GetTokenType(lexeme);
	if (type == Token::Type::Empty)
	{
		return {};
	}

	return { .type = type, .value = std::string(1, lexeme) };
}

Token LexerBase::GetToken(std::string_view lexeme) noexcept
{
	// https://en.cppreference.com/w/cpp/regex/ecmascript
	// https://www.regular-expressions.info/posixbrackets.html
	// https://www.regular-expressions.info/unicode.html
	// https://en.wikipedia.org/wiki/Combining_character

	// ANSI token regexes
	static const std::vector<std::pair<std::regex, Token::Type>> definitions
	{
		{ std::regex{ R"(<|>|<>|<=|>=|=)" }, Token::Type::OperatorComparison },
		{ std::regex{ R"(\+|-|\*|/|/*)" }, Token::Type::OperatorArithmetic },
		{ std::regex{ R"(\*\*|\+\+|--)" }, Token::Type::OperatorBitwise },
		{ std::regex{ R"(<-|->)" }, Token::Type::OperatorArrow },
		{ std::regex{ R"([[:alpha:]][\w<>=\+\-\*/]*)" }, Token::Type::Identifier },
	};

	// Additional Unicode token regexes
	static const std::vector<std::pair<std::wregex, Token::Type>> unicodeDefinitions
	{
		{ std::wregex{ LR"([[:alpha:]][\w<>=\+\-\*/\u0300–\u036F\u1AB0–\u1AFF\u1DC0–\u1DFF\u20D0–\u20FF\uFE20–\uFE2F]*)" }, Token::Type::Identifier },
	};

	const auto trimmed = Trim(lexeme);

	if (trimmed.empty())
	{
		return {};
	}

	// Check for single char token

	if (trimmed.length() == 1)
	{
		auto token = GetToken(trimmed[0]);
		if (token)
		{
			return token;
		}
	}

	// Check for ANSI token

	for (const auto& definition : definitions)
	{
		if (std::regex_match(trimmed.begin(), trimmed.end(), definition.first))
		{
			return { .type = definition.second, .value = std::string(trimmed.data(), trimmed.size()) };
		}
	}

	// Check for Unicode token

	const auto wlexeme = to_wstring(lexeme);
	const auto wtrimmed = Trim(wlexeme);

	for (const auto& unicodeDefinition : unicodeDefinitions)
	{
		if (std::regex_match(wtrimmed.begin(), wtrimmed.end(), unicodeDefinition.first))
		{
			return { .type = unicodeDefinition.second, .value = std::string(trimmed.data(), trimmed.size()) };
		}
	}

	AddToken({ .type = Token::Type::Error, .value = std::format("Syntax error near \"{}\".", std::string(trimmed.data(), trimmed.size())), .error{.code = LexerError::SyntaxError} });

	return {};
}

void LexerBase::PrintErrorSourceCode(std::ostream& stream, const Token& token) noexcept
{
	const auto line = token.error.line;
	const auto column = token.error.column;

	if (line < firstSourceCodeLine || line > firstSourceCodeLine + sourceCodeLines.size() - 1)
	{
		stream << "No source code available for line " << line << ".\n";
		return;
	}

	const auto& sourceCode = sourceCodeLines[line - firstSourceCodeLine];

	const auto length = sourceCode.length();
	const auto begin = std::min(column - 1, length);
	const auto end = std::min(begin + token.value.length(), length);

	stream << '(' << line << ':' << column << ") " << sourceCode.substr(0, begin) << color.fail;

	if (begin < end)
	{
		stream << sourceCode.substr(begin, end);
	}
	else if (end < length)
	{
		// Highlight the character after if the token is empty.
		stream << sourceCode.substr(begin, end + 1);
	}
	else
	{
		// Add highlighted padding at the end of the line.
		stream << ' ';
	}

	stream << color.none;

	// Write the remaining part of the line, if any.
	if (end < length)
	{
		stream << sourceCode.substr(end);
	}
	
	stream << '\n';

	// Drop all lines before the requested one to avoid keeping a lot of lines in memory.
	// It's assumed that errors are always printed in a forward order.

	while (firstSourceCodeLine < line)
	{
		sourceCodeLines.pop_front();
		++firstSourceCodeLine;
	}
}
