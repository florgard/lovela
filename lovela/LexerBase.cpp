#include "Lexer.h"
#include <string_view>
#include <regex>

using namespace std::literals::string_view_literals;

static_assert(LexerBase::Trim(L""sv) == L""sv);
static_assert(LexerBase::Trim(L" \t\r\n\f\v"sv) == L""sv);
static_assert(LexerBase::Trim(L"a b c \r\n"sv) == L"a b c"sv);

static_assert(LexerBase::GetTokenType('(') == Token::Type::ParenRoundOpen);
static_assert(LexerBase::GetTokenType('.') == Token::Type::SeparatorDot);
static_assert(LexerBase::GetTokenType(' ') == Token::Type::Empty);

Token LexerBase::GetToken(wchar_t lexeme) noexcept
{
	auto type = GetTokenType(lexeme);
	if (type == Token::Type::Empty)
	{
		return {};
	}

	return { .type = type, .value = std::wstring(1, lexeme) };
}

Token LexerBase::GetToken(const std::wstring_view& lexeme) noexcept
{
	static const std::vector<std::pair<std::wregex, Token::Type>> tokenRegexes{
		{ std::wregex{ LR"(\d+)" }, Token::Type::LiteralInteger },
		{ std::wregex{ LR"(\d+\.\d+)" }, Token::Type::LiteralDecimal },
		{ std::wregex{ LR"(\w+)" }, Token::Type::Identifier },
		{ std::wregex{ LR"(<|>|<>|<=|>=|=)" }, Token::Type::OperatorComparison },
		{ std::wregex{ LR"(\+|-|\*|/|/*)" }, Token::Type::OperatorArithmetic },
		{ std::wregex{ LR"(\*\*|\+\+|--)" }, Token::Type::OperatorBitwise },
	};

	auto trimmed = Trim(lexeme);

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

	for (const auto& pair : tokenRegexes)
	{
		if (std::regex_match(trimmed.begin(), trimmed.end(), pair.first))
		{
			return { .type = pair.second, .value = std::wstring(trimmed.data(), trimmed.size()) };
		}
	}

	AddError(Error::Code::SyntaxError, L"Syntax error near \"" + std::wstring(trimmed.data(), trimmed.size()) + L"\".");

	return {};
}
