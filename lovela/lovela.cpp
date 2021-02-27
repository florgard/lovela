#include <iostream>
#include <sstream>
#include <cassert>
#include "Lexer.h"

void TestCode(ILexer& lexer, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	const auto tokens = lexer.Lex(input);
	assert(tokens == expectedTokens);
	auto& errors = lexer.GetErrors();
	assert(errors.size() == expectedErrors.size());
	for (int i = 0; i < errors.size() && i < expectedErrors.size(); i++)
	{
		auto& error = errors[i];
		auto& expectedError = expectedErrors[i];
		const bool equal = error.code == expectedError.code && error.line == expectedError.line;
		if (!equal)
		{
			std::wcerr << "Error " << static_cast<int>(error.code) << " at line " << error.line << ": " << error.message << '\n';
		}
		assert(error.code == expectedError.code && error.line == expectedError.line);
	}
}

void TestLexer(ILexer& lexer)
{
	TestCode(lexer, L"", {}, {});
	TestCode(lexer, L".", { {.type = TokenType::SeparatorDot, .value = L"." }}, {});

	TestCode(lexer, L"abc", { {.type = TokenType::Identifier, .value = L"abc" }}, {});
	TestCode(lexer, L"\u65E5\u672C", { {.type = TokenType::Identifier, .value = L"\u65E5\u672C" }}, {});
	//TestCode(lexer, L"\u0061\u0300", { { .type = TokenType::Identifier, .value = L"\u0061\u0300" }}, {});

	TestCode(lexer, L"123", {
		{.type = TokenType::LiteralInteger, .value = L"123" }
		}, {});
	TestCode(lexer, L"123.", {
		{.type = TokenType::LiteralInteger, .value = L"123"},
		{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode(lexer, L"123.456", { {.type = TokenType::LiteralDecimal, .value = L"123.456" }}, {});
	TestCode(lexer, L"123.456.", {
		{.type = TokenType::LiteralDecimal, .value = L"123.456"},
		{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode(lexer, L"123.456.7", {
		{.type = TokenType::LiteralDecimal, .value = L"123.456"},
		{.type = TokenType::SeparatorDot, .value = L"."},
		{.type = TokenType::LiteralInteger, .value = L"7"},
		}, {});

	TestCode(lexer, L"''", { {.type = TokenType::LiteralString, .value = L"" }}, {});
	TestCode(lexer, L"''''", { {.type = TokenType::LiteralString, .value = L"'" }}, {});
	TestCode(lexer, L"'abc'", { {.type = TokenType::LiteralString, .value = L"abc" }}, {});
	TestCode(lexer, L"'ab c'", { {.type = TokenType::LiteralString, .value = L"ab c" }}, {});
	TestCode(lexer, L"'ab''c'", { {.type = TokenType::LiteralString, .value = L"ab'c" }}, {});
	TestCode(lexer, L"'ab' 'c'", {
		{.type = TokenType::LiteralString, .value = L"ab"},
		{.type = TokenType::LiteralString, .value = L"c"},
		}, {});
	TestCode(lexer, L"'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 1 } });
	TestCode(lexer, L"\r\n'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 2 } });
	TestCode(lexer, L"\n'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 2 } });
	TestCode(lexer, L"\r'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 1 } });
	TestCode(lexer, L"\t'ab\r\n\tc'\r\n", { {.type = TokenType::LiteralString, .value = L"ab\r\n\tc" }}, {});

	TestCode(lexer, L"func: 123.", {
			{.type = TokenType::Identifier, .value = L"func"},
			{.type = TokenType::OperatorColon, .value = L":"},
			{.type = TokenType::LiteralInteger, .value = L"123"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode(lexer, L"func : 123.4.", {
			{.type = TokenType::Identifier, .value = L"func"},
			{.type = TokenType::OperatorColon, .value = L":"},
			{.type = TokenType::LiteralDecimal, .value = L"123.4"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode(lexer, L"\r\nfunc44: (123.4).", {
			{.type = TokenType::Identifier, .value = L"func44"},
			{.type = TokenType::OperatorColon, .value = L":"},
			{.type = TokenType::ParenRoundOpen, .value = L"("},
			{.type = TokenType::LiteralDecimal, .value = L"123.4"},
			{.type = TokenType::ParenRoundClose, .value = L")"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});

	TestCode(lexer, L"ident123.", {
			{.type = TokenType::Identifier, .value = L"ident123"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode(lexer, L"<< ident123. >>", {}, {});
	TestCode(lexer, L"<<\r\nident123.\r\n>>", {}, {});
	TestCode(lexer, L"<< ident123. >> ident456.", {
			{.type = TokenType::Identifier, .value = L"ident456"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode(lexer, L"<<<< 123 << 456 >>>>.>> ident456.", {
			{.type = TokenType::Identifier, .value = L"ident456"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode(lexer, L"<<<<123>>ident234<<<<123<<456>>>:>>.", {
			{.type = TokenType::Identifier, .value = L"ident234"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode(lexer, L"<<<<123>>ident234<<<<123<<456>>>:>.", {
			{.type = TokenType::Identifier, .value = L"ident234"},
		}, { {.code = ILexer::Error::Code::OpenComment, .line = 1} });
	TestCode(lexer, L"1 < 2", {
			{.type = TokenType::LiteralInteger, .value = L"1"},
			{.type = TokenType::OperatorComparison, .value = L"<"},
			{.type = TokenType::LiteralInteger, .value = L"2"},
		}, {});
}

int main()
{
	Lexer lexer2;
	TestLexer(lexer2);
}
