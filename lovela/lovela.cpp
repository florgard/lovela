#include "Lexer.h"
#include "Testing.h"

void TestLexer(ILexer& lexer)
{
	TestCode("empty expression", lexer, L"", {}, {});
	TestCode("single character", lexer, L".", { {.type = TokenType::SeparatorDot, .value = L"." }}, {});

	TestCode("simple identifier", lexer, L"abc", { {.type = TokenType::Identifier, .value = L"abc" }}, {});
	TestCode("Unicode identifier", lexer, L"\u65E5\u672C", { {.type = TokenType::Identifier, .value = L"\u65E5\u672C" }}, {});
	//TestCode(lexer, L"\u0061\u0300", { { .type = TokenType::Identifier, .value = L"\u0061\u0300" }}, {});

	TestCode("integer literal", lexer, L"123", {
		{.type = TokenType::LiteralInteger, .value = L"123" }
		}, {});
	TestCode("integer literal and full stop", lexer, L"123.", {
		{.type = TokenType::LiteralInteger, .value = L"123"},
		{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode("decimal literal", lexer, L"123.456", { {.type = TokenType::LiteralDecimal, .value = L"123.456" }}, {});
	TestCode("decimal literal and full stop", lexer, L"123.456.", {
		{.type = TokenType::LiteralDecimal, .value = L"123.456"},
		{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode("decimal literal, full stop, digit", lexer, L"123.456.7", {
		{.type = TokenType::LiteralDecimal, .value = L"123.456"},
		{.type = TokenType::SeparatorDot, .value = L"."},
		{.type = TokenType::LiteralInteger, .value = L"7"},
		}, {});

	TestCode("empty string literal", lexer, L"''", { {.type = TokenType::LiteralString, .value = L"" }}, {});
	TestCode("single escaped quotation mark", lexer, L"''''", { {.type = TokenType::LiteralString, .value = L"'" }}, {});
	TestCode("simple string literal", lexer, L"'abc'", { {.type = TokenType::LiteralString, .value = L"abc" }}, {});
	TestCode("string literal with whitespace", lexer, L"'ab c'", { {.type = TokenType::LiteralString, .value = L"ab c" }}, {});
	TestCode("string literal with escaped quotation mark", lexer, L"'ab''c'", { {.type = TokenType::LiteralString, .value = L"ab'c" }}, {});
	TestCode("separated string literals", lexer, L"'ab' 'c'", {
		{.type = TokenType::LiteralString, .value = L"ab"},
		{.type = TokenType::LiteralString, .value = L"c"},
		}, {});
	TestCode("comment in string literal", lexer, L"'<< abc >>'", { {.type = TokenType::LiteralString, .value = L"<< abc >>" } }, {});
	TestCode("non-closed string literal", lexer, L"'", {}, { {.code = ILexer::Error::Code::OpenStringLiteral } });
	TestCode("non-closed string literal on line 1", lexer, L"'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 1 } });
	TestCode("non-closed string literal on line 2", lexer, L"\r\n'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 2 } });
	TestCode("non-closed string literal on line 2", lexer, L"\n'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 2 } });
	TestCode("non-closed string literal on line 1", lexer, L"\r'abc", {}, { {.code = ILexer::Error::Code::OpenStringLiteral, .line = 1 } });
	TestCode("whitespace outside and within string literal", lexer, L"\t'ab\r\n\tc'\r\n", { {.type = TokenType::LiteralString, .value = L"ab\r\n\tc" }}, {});

	TestCode("escaped curly bracket", lexer, L"'{{'", { {.type = TokenType::LiteralString, .value = L"{" } }, {});
	TestCode("escaped curly bracket", lexer, L"'{{}'", { {.type = TokenType::LiteralString, .value = L"{}" } }, {});
	TestCode("single closing curly bracket", lexer, L"'}'", { {.type = TokenType::LiteralString, .value = L"}" } }, {});
	TestCode("string field", lexer, L"'{n}'", { {.type = TokenType::LiteralString, .value = L"\n" } }, {});
	TestCode("string fields", lexer, L"'{t}{n}{r}'", { {.type = TokenType::LiteralString, .value = L"\t\n\r" } }, {});
	TestCode("embedded string fields", lexer, L"'abc{r}{n}def'", { {.type = TokenType::LiteralString, .value = L"abc\r\ndef" } }, {});
	TestCode("non-closed string field", lexer, L"'{n'", {}, { {.code = ILexer::Error::Code::StringFieldIllformed}, {.code = ILexer::Error::Code::OpenStringLiteral} });
	TestCode("ill-formed string field", lexer, L"'{nn}'", { {.type = TokenType::LiteralString, .value = L"}"} }, { {.code = ILexer::Error::Code::StringFieldIllformed} });
	TestCode("unknown string field", lexer, L"'{m}'", { {.type = TokenType::LiteralString, .value = L"m}"} }, { {.code = ILexer::Error::Code::StringFieldUnknown} });

	TestCode("unindexed string interpolation", lexer, L"'{}'", {
		{.type = TokenType::LiteralString, .value = L"" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"1" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});
	TestCode("embedded unindexed string interpolation", lexer, L"'abc{}'", {
		{.type = TokenType::LiteralString, .value = L"abc" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"1" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});
	TestCode("unindexed string interpolations", lexer, L"'abc{}{}'", {
		{.type = TokenType::LiteralString, .value = L"abc" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"1" },
		{.type = TokenType::LiteralString, .value = L"" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"2" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});
	TestCode("unindexed string interpolation", lexer, L"'{2}'", {
		{.type = TokenType::LiteralString, .value = L"" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"2" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});
	TestCode("unindexed string interpolations", lexer, L"'abc{4}{1}'", {
		{.type = TokenType::LiteralString, .value = L"abc" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"4" },
		{.type = TokenType::LiteralString, .value = L"" },
		{.type = TokenType::LiteralStringInterpolation, .value = L"1" },
		{.type = TokenType::LiteralString, .value = L"" },
		}, {});

	TestCode("trivial integer function", lexer, L"func: 123.", {
			{.type = TokenType::Identifier, .value = L"func"},
			{.type = TokenType::OperatorColon, .value = L":"},
			{.type = TokenType::LiteralInteger, .value = L"123"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode("trivial decimal function with whitespace", lexer, L"func : 123.4.", {
			{.type = TokenType::Identifier, .value = L"func"},
			{.type = TokenType::OperatorColon, .value = L":"},
			{.type = TokenType::LiteralDecimal, .value = L"123.4"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode("trivial decimal function with mixed name and group", lexer, L"\r\nfunc44: (123.4).", {
			{.type = TokenType::Identifier, .value = L"func44"},
			{.type = TokenType::OperatorColon, .value = L":"},
			{.type = TokenType::ParenRoundOpen, .value = L"("},
			{.type = TokenType::LiteralDecimal, .value = L"123.4"},
			{.type = TokenType::ParenRoundClose, .value = L")"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});

	TestCode("mixed character identifier", lexer, L"ident123.", {
			{.type = TokenType::Identifier, .value = L"ident123"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode("commented out identifier", lexer, L"<< ident123. >>", {}, {});
	TestCode("commented out identifier and whitespace", lexer, L"<<\r\nident123.\r\n>>", {}, {});
	TestCode("commented and non-commented identifier", lexer, L"<< ident123. >> ident456.", {
			{.type = TokenType::Identifier, .value = L"ident456"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode("nested comments", lexer, L"<<<< 123 << 456 >>>>.>> ident456.", {
			{.type = TokenType::Identifier, .value = L"ident456"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode("multiple comments", lexer, L"<<<<123>>ident234<<<<123<<456>>>:>>.", {
			{.type = TokenType::Identifier, .value = L"ident234"},
			{.type = TokenType::SeparatorDot, .value = L"."},
		}, {});
	TestCode("non-closed comment", lexer, L"<<<<123>>ident234<<<<123<<456>>>:>.", {
			{.type = TokenType::Identifier, .value = L"ident234"},
		}, { {.code = ILexer::Error::Code::OpenComment, .line = 1} });
	TestCode("comparison operator", lexer, L"1 < 2", {
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
