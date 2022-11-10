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

Token LexerBase::GetToken(const std::string_view& lexeme) noexcept
{
	// https://en.cppreference.com/w/cpp/regex/ecmascript
	// https://www.regular-expressions.info/posixbrackets.html
	// https://www.regular-expressions.info/unicode.html
	// https://en.wikipedia.org/wiki/Combining_character
	static const std::vector<std::tuple<std::regex, Token::Type, std::string_view>> definitions
	{
		{ std::regex{ R"(\d+)" }, Token::Type::LiteralInteger, integerTypeName },
		{ std::regex{ R"(\d+\.\d+)" }, Token::Type::LiteralDecimal, decimalTypeName },
		{ std::regex{ R"(<|>|<>|<=|>=|=)" }, Token::Type::OperatorComparison, {} },
		{ std::regex{ R"(\+|-|\*|/|/*)" }, Token::Type::OperatorArithmetic, {} },
		{ std::regex{ R"(\*\*|\+\+|--)" }, Token::Type::OperatorBitwise, {} },
		{ std::regex{ R"(<-|->)" }, Token::Type::OperatorArrow, {} },
		{ std::regex{ R"([[:alpha:]][\w<>=\+\-\*/]*)" }, Token::Type::Identifier, {} },
	};

	static const std::vector<std::tuple<std::wregex, Token::Type, std::string_view>> unicodeDefinitions
	{
		{ std::wregex{ LR"([[:alpha:]][\w<>=\+\-\*/\u0300–\u036F\u1AB0–\u1AFF\u1DC0–\u1DFF\u20D0–\u20FF\uFE20–\uFE2F]*)" }, Token::Type::Identifier, {} },
	};

	const auto trimmed = Trim(lexeme);

	if (trimmed.empty())
	{
		return {};
	}

	if (trimmed.length() == 1)
	{
		auto token = GetToken(trimmed[0]);
		if (token)
		{
			return token;
		}
	}

	for (const auto& definition : definitions)
	{
		if (std::regex_match(trimmed.begin(), trimmed.end(), std::get<0>(definition)))
		{
			return { .type = std::get<1>(definition), .value = std::string(trimmed.data(), trimmed.size()), .outType = to_string(std::get<2>(definition)) };
		}
	}

	const auto wlexeme = to_wstring(lexeme);
	const auto wtrimmed = Trim(wlexeme);

	for (const auto& unicodeDefinition : unicodeDefinitions)
	{
		if (std::regex_match(wtrimmed.begin(), wtrimmed.end(), std::get<0>(unicodeDefinition)))
		{
			return { .type = std::get<1>(unicodeDefinition), .value = std::string(trimmed.data(), trimmed.size()), .outType = to_string(std::get<2>(unicodeDefinition)) };
		}
	}

	AddError(Error::Code::SyntaxError, "Syntax error near \"" + std::string(trimmed.data(), trimmed.size()) + "\".");

	return {};
}

void LexerBase::AddError(Error::Code code, const std::string& message)
{
	errors.emplace_back(Error{ .code = code, .message = message, .token{.line = currentLine, .column = currentColumn, .code{currentCode.begin(), currentCode.end()} } });
}
