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

	AddError(Error::Code::SyntaxError, "Syntax error near \"" + std::string(trimmed.data(), trimmed.size()) + "\".");

	return {};
}

void LexerBase::AddError(Error::Code code, const std::string& message)
{
	errors.emplace_back(Error{ .code = code, .message = message, .token{.line = currentLine, .column = currentColumn, .code{currentCode.begin(), currentCode.end()} } });
}
