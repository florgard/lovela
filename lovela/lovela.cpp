#include "Lexer.h"
#include "Parser.h"
#include "Testing.h"
#include <sstream>

void TestLexer()
{
	Lexer lexer;

	Test("empty expression", lexer, L"", {}, {});
	Test("single character", lexer, L".", { {.type = TokenType::SeparatorDot, .value = L"." }}, {});

	Test("simple identifier", lexer, L"abc", { {.type = TokenType::Identifier, .value = L"abc" }}, {});
	Test("Unicode identifier", lexer, L"\u65E5\u672C", { {.type = TokenType::Identifier, .value = L"\u65E5\u672C" }}, {});
	//Test(lexer, L"\u0061\u0300", { { .type = TokenType::Identifier, .value = L"\u0061\u0300" }}, {});

	Test("integer literal", lexer, L"123", {
		{.type = TokenType::LiteralInteger, .value = L"123" }
		}, {});
	Test("integer literal and full stop", lexer, L"123.", {
		{.type = TokenType::LiteralInteger, .value = L"123"},
		{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	Test("decimal literal", lexer, L"123.456", { {.type = TokenType::LiteralDecimal, .value = L"123.456" }}, {});
	Test("decimal literal and full stop", lexer, L"123.456.", {
		{.type = TokenType::LiteralDecimal, .value = L"123.456"},
		{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	Test("decimal literal, full stop, digit", lexer, L"123.456.7", {
		{.type = TokenType::LiteralDecimal, .value = L"123.456"},
		{.type = TokenType::SeparatorDot, .value = L"."},
		{.type = TokenType::LiteralInteger, .value = L"7"},
		}, {});

	Test("empty string literal", lexer, L"''", { {.type = TokenType::LiteralString, .value = L"" }}, {});
	Test("single escaped quotation mark", lexer, L"''''", { {.type = TokenType::LiteralString, .value = L"'" }}, {});
	Test("simple string literal", lexer, L"'abc'", { {.type = TokenType::LiteralString, .value = L"abc" }}, {});
	Test("string literal with whitespace", lexer, L"'ab c'", { {.type = TokenType::LiteralString, .value = L"ab c" }}, {});
	Test("string literal with escaped quotation mark", lexer, L"'ab''c'", { {.type = TokenType::LiteralString, .value = L"ab'c" }}, {});
	Test("separated string literals", lexer, L"'ab' 'c'", {
		{.type = TokenType::LiteralString, .value = L"ab"},
		{.type = TokenType::LiteralString, .value = L"c"},
		}, {});
	Test("comment in string literal", lexer, L"'<< abc >>'", { {.type = TokenType::LiteralString, .value = L"<< abc >>" } }, {});
	Test("non-closed string literal", lexer, L"'", {}, { {.code = ILexer::Error::Code::OpenStringLiteral } });
	Test("non-closed string literal on line 1", lexer, L"'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 1 } });
	Test("non-closed string literal on line 2", lexer, L"\r\n'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 2 } });
	Test("non-closed string literal on line 2", lexer, L"\n'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 2 } });
	Test("non-closed string literal on line 1", lexer, L"\r'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 1 } });
	Test("whitespace outside and within string literal", lexer, L"\t'ab\r\n\tc'\r\n", { {.type = TokenType::LiteralString, .value = L"ab\r\n\tc" }}, {});

	Test("escaped curly bracket", lexer, L"'{{'", { {.type = TokenType::LiteralString, .value = L"{" } }, {});
	Test("escaped curly bracket", lexer, L"'{{}'", { {.type = TokenType::LiteralString, .value = L"{}" } }, {});
	Test("single closing curly bracket", lexer, L"'}'", { {.type = TokenType::LiteralString, .value = L"}" } }, {});
	Test("string field", lexer, L"'{n}'", { {.type = TokenType::LiteralString, .value = L"\n" } }, {});
	Test("string fields", lexer, L"'{t}{n}{r}'", { {.type = TokenType::LiteralString, .value = L"\t\n\r" } }, {});
	Test("embedded string fields", lexer, L"'abc{r}{n}def'", { {.type = TokenType::LiteralString, .value = L"abc\r\ndef" } }, {});
	Test("non-closed string field", lexer, L"'{n'", {}, { {.code = ILexer::Error::Code::StringFieldIllformed}, {.code = ILexer::Error::Code::OpenStringLiteral} });
	Test("ill-formed string field", lexer, L"'{nn}'", { {.type = TokenType::LiteralString, .value = L"}"} }, { {.code = ILexer::Error::Code::StringFieldIllformed} });
	Test("unknown string field", lexer, L"'{m}'", { {.type = TokenType::LiteralString, .value = L"m}"} }, { {.code = ILexer::Error::Code::StringFieldUnknown} });

	Test("unindexed string interpolation", lexer, L"'{}'", {
		{.type = TokenType::LiteralString, .value = L"" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"1" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});
	Test("embedded unindexed string interpolation", lexer, L"'abc{}'", {
		{.type = TokenType::LiteralString, .value = L"abc" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"1" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});
	Test("unindexed string interpolations", lexer, L"'abc{}{}'", {
		{.type = TokenType::LiteralString, .value = L"abc" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"1" },
		{.type = TokenType::LiteralString, .value = L"" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"2" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});
	Test("unindexed string interpolation", lexer, L"'{2}'", {
		{.type = TokenType::LiteralString, .value = L"" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"2" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});
	Test("unindexed string interpolations", lexer, L"'abc{4}{1}'", {
		{.type = TokenType::LiteralString, .value = L"abc" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"4" },
		{.type = TokenType::LiteralString, .value = L"" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"1" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});

	Test("trivial integer function", lexer, L"func: 123.", {
			{.type = TokenType::Identifier, .value = L"func"},
			{.type = TokenType::OperatorColon, .value = L":"},
			{.type = TokenType::LiteralInteger, .value = L"123"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	Test("trivial decimal function with whitespace", lexer, L"func : 123.4.", {
			{.type = TokenType::Identifier, .value = L"func"},
			{.type = TokenType::OperatorColon, .value = L":"},
			{.type = TokenType::LiteralDecimal, .value = L"123.4"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	Test("trivial decimal function with mixed name and group", lexer, L"\r\nfunc44: (123.4).", {
			{.type = TokenType::Identifier, .value = L"func44"},
			{.type = TokenType::OperatorColon, .value = L":"},
			{.type = TokenType::ParenRoundOpen, .value = L"("},
			{.type = TokenType::LiteralDecimal, .value = L"123.4"},
			{.type = TokenType::ParenRoundClose, .value = L")"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});

	Test("mixed character identifier", lexer, L"ident123.", {
			{.type = TokenType::Identifier, .value = L"ident123"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	Test("commented out identifier", lexer, L"<< ident123. >>", {}, {});
	Test("commented out identifier and whitespace", lexer, L"<<\r\nident123.\r\n>>", {}, {});
	Test("commented and non-commented identifier", lexer, L"<< ident123. >> ident456.", {
			{.type = TokenType::Identifier, .value = L"ident456"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	Test("nested comments", lexer, L"<<<< 123 << 456 >>>>.>> ident456.", {
			{.type = TokenType::Identifier, .value = L"ident456"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	Test("multiple comments", lexer, L"<<<<123>>ident234<<<<123<<456>>>:>>.", {
			{.type = TokenType::Identifier, .value = L"ident234"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	Test("non-closed comment", lexer, L"<<<<123>>ident234<<<<123<<456>>>:>.", {
			{.type = TokenType::Identifier, .value = L"ident234"},
		}, { {.code = ILexer::Error::Code::OpenComment, .line = 1} });
	Test("comparison operator", lexer, L"1 < 2", {
			{.type = TokenType::LiteralInteger, .value = L"1"},
			{.type = TokenType::OperatorComparison, .value = L"<"},
			{.type = TokenType::LiteralInteger, .value = L"2"},
		}, {});
}

void TestParser()
{
	std::wistringstream input(L"func: 123.");
	Lexer lexer;
	Parser parser;
	parser.Parse(lexer.Lex(input));
}

int main()
{
	TestLexer();
	TestParser();
}
