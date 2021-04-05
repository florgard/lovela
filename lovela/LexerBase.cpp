#include "pch.h"
#include "Lexer.h"

using namespace std::literals::string_view_literals;

static_assert(LexerBase::Trim(L""sv) == L""sv);
static_assert(LexerBase::Trim(L" \t\r\n\f\v"sv) == L""sv);
static_assert(LexerBase::Trim(L"a b c \r\n"sv) == L"a b c"sv);

Token::Type LexerBase::GetTokenType(wchar_t lexeme) noexcept
{
	static constexpr std::array<std::pair<wchar_t, Token::Type>, 13> values
	{ {
		{'(', Token::Type::ParenRoundOpen },
		{')', Token::Type::ParenRoundClose },
		{'[', Token::Type::ParenSquareOpen },
		{']', Token::Type::ParenSquareClose },
		{'{', Token::Type::ParenCurlyOpen },
		{'}', Token::Type::ParenCurlyClose },
		{'.', Token::Type::SeparatorDot },
		{',', Token::Type::SeparatorComma },
		{'!', Token::Type::SeparatorExclamation },
		{'?', Token::Type::SeparatorQuestion },
		{'|', Token::Type::SeparatorVerticalLine },
		{':', Token::Type::SeparatorColon },
		{'#', Token::Type::SeparatorHash },
	} };

	static constexpr auto map = StaticMap<wchar_t, Token::Type, values.size()>{ {values} };
	return map.at_or(lexeme, Token::Type::Empty);
}

std::wstring_view LexerBase::GetStringField(wchar_t code) noexcept
{
	static constexpr std::array<std::pair<wchar_t, std::wstring_view>, 3> values
	{ {
		{'t', L"\t"},
		{'n', L"\n"},
		{'r', L"\r"},
	} };

	static constexpr auto map = StaticMap<wchar_t, std::wstring_view, values.size()>{ {values} };
	return map.at_or(code, {});
}

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
	static const std::vector<std::tuple<std::wregex, Token::Type, std::wstring_view>> definitions
	{
		{ std::wregex{ LR"(\d+)" }, Token::Type::LiteralInteger, integerTypeName },
		{ std::wregex{ LR"(\d+\.\d+)" }, Token::Type::LiteralDecimal, decimalTypeName },
		{ std::wregex{ LR"(\w[\w<>=\+\-\*/]*)" }, Token::Type::Identifier, {} },
		{ std::wregex{ LR"(<|>|<>|<=|>=|=)" }, Token::Type::OperatorComparison, {} },
		{ std::wregex{ LR"(\+|-|\*|/|/*)" }, Token::Type::OperatorArithmetic, {} },
		{ std::wregex{ LR"(\*\*|\+\+|--)" }, Token::Type::OperatorBitwise, {} },
		{ std::wregex{ LR"(<-|->)" }, Token::Type::OperatorArrow, {} },
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

	for (const auto& definition : definitions)
	{
		if (std::regex_match(trimmed.begin(), trimmed.end(), std::get<0>(definition)))
		{
			return { .type = std::get<1>(definition), .value = std::wstring(trimmed.data(), trimmed.size()), .outType = to_wstring(std::get<2>(definition)) };
		}
	}

	AddError(Error::Code::SyntaxError, L"Syntax error near \"" + std::wstring(trimmed.data(), trimmed.size()) + L"\".");

	return {};
}

void LexerBase::AddError(Error::Code code, const std::wstring& message)
{
	errors.emplace_back(Error{ .code = code, .message = message, .token{.line = currentLine, .column = currentColumn, .code{currentCode.begin(), currentCode.end()} } });
}
