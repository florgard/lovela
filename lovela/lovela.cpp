#include "Lexer.h"
#include "Parser.h"
#include "Testing.h"
#include <sstream>

void TestLexer()
{
	Testing testing;

	testing.TestLexer("empty expression", L"", {}, {});
	testing.TestLexer("single character", L".", { {.type = Token::Type::SeparatorDot, .value = L"." } }, {});

	testing.TestLexer("simple identifier", L"abc", { {.type = Token::Type::Identifier, .value = L"abc" } }, {});
	testing.TestLexer("Unicode identifier", L"\u65E5\u672C", { {.type = Token::Type::Identifier, .value = L"\u65E5\u672C" } }, {});
	//testing.TestLexer(L"\u0061\u0300", { { .type = Token::Type::Identifier, .value = L"\u0061\u0300" }}, {});

	testing.TestLexer("integer literal", L"123", {
		{.type = Token::Type::LiteralInteger, .value = L"123" }
		}, {});
	testing.TestLexer("integer literal and full stop", L"123.", {
		{.type = Token::Type::LiteralInteger, .value = L"123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	testing.TestLexer("decimal literal", L"123.456", { {.type = Token::Type::LiteralDecimal, .value = L"123.456" } }, {});
	testing.TestLexer("decimal literal and full stop", L"123.456.", {
		{.type = Token::Type::LiteralDecimal, .value = L"123.456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	testing.TestLexer("decimal literal, full stop, digit", L"123.456.7", {
		{.type = Token::Type::LiteralDecimal, .value = L"123.456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		{.type = Token::Type::LiteralInteger, .value = L"7"},
		}, {});

	testing.TestLexer("empty string literal", L"''", { {.type = Token::Type::LiteralString, .value = L"" } }, {});
	testing.TestLexer("single escaped quotation mark", L"''''", { {.type = Token::Type::LiteralString, .value = L"'" } }, {});
	testing.TestLexer("simple string literal", L"'abc'", { {.type = Token::Type::LiteralString, .value = L"abc" } }, {});
	testing.TestLexer("string literal with whitespace", L"'ab c'", { {.type = Token::Type::LiteralString, .value = L"ab c" } }, {});
	testing.TestLexer("string literal with escaped quotation mark", L"'ab''c'", { {.type = Token::Type::LiteralString, .value = L"ab'c" } }, {});
	testing.TestLexer("separated string literals", L"'ab' 'c'", {
		{.type = Token::Type::LiteralString, .value = L"ab"},
		{.type = Token::Type::LiteralString, .value = L"c"},
		}, {});
	testing.TestLexer("comment in string literal", L"'<< abc >>'", { {.type = Token::Type::LiteralString, .value = L"<< abc >>" } }, {});
	testing.TestLexer("non-closed string literal", L"'", {}, { {.code = ILexer::Error::Code::StringLiteralOpen } });
	testing.TestLexer("non-closed string literal on line 1", L"'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .line = 1 } });
	testing.TestLexer("non-closed string literal on line 2", L"\r\n'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .line = 2 } });
	testing.TestLexer("non-closed string literal on line 2", L"\n'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .line = 2 } });
	testing.TestLexer("non-closed string literal on line 1", L"\r'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .line = 1 } });
	testing.TestLexer("whitespace outside and within string literal", L"\t'ab\r\n\tc'\r\n", { {.type = Token::Type::LiteralString, .value = L"ab\r\n\tc" } }, {});

	testing.TestLexer("escaped curly bracket", L"'{{'", { {.type = Token::Type::LiteralString, .value = L"{" } }, {});
	testing.TestLexer("escaped curly bracket", L"'{{}'", { {.type = Token::Type::LiteralString, .value = L"{}" } }, {});
	testing.TestLexer("single closing curly bracket", L"'}'", { {.type = Token::Type::LiteralString, .value = L"}" } }, {});
	testing.TestLexer("string field", L"'{n}'", { {.type = Token::Type::LiteralString, .value = L"\n" } }, {});
	testing.TestLexer("string fields", L"'{t}{n}{r}'", { {.type = Token::Type::LiteralString, .value = L"\t\n\r" } }, {});
	testing.TestLexer("embedded string fields", L"'abc{r}{n}def'", { {.type = Token::Type::LiteralString, .value = L"abc\r\ndef" } }, {});
	testing.TestLexer("non-closed string field", L"'{n'", {}, { {.code = ILexer::Error::Code::StringFieldIllformed}, {.code = ILexer::Error::Code::StringLiteralOpen} });
	testing.TestLexer("ill-formed string field", L"'{nn}'", { {.type = Token::Type::LiteralString, .value = L"}"} }, { {.code = ILexer::Error::Code::StringFieldIllformed} });
	testing.TestLexer("unknown string field", L"'{m}'", { {.type = Token::Type::LiteralString, .value = L"m}"} }, { {.code = ILexer::Error::Code::StringFieldUnknown} });

	testing.TestLexer("unindexed string interpolation", L"'{}'", {
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});
	testing.TestLexer("embedded unindexed string interpolation", L"'abc{}'", {
		{.type = Token::Type::LiteralString, .value = L"abc" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});
	testing.TestLexer("unindexed string interpolations", L"'abc{}{}'", {
		{.type = Token::Type::LiteralString, .value = L"abc" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"2" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});
	testing.TestLexer("unindexed string interpolation", L"'{2}'", {
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"2" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});
	testing.TestLexer("unindexed string interpolations", L"'abc{4}{1}'", {
		{.type = Token::Type::LiteralString, .value = L"abc" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"4" },
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});

	testing.TestLexer("trivial integer function", L"func: 123.", {
		{.type = Token::Type::Identifier, .value = L"func"},
		{.type = Token::Type::OperatorColon, .value = L":"},
		{.type = Token::Type::LiteralInteger, .value = L"123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	testing.TestLexer("trivial decimal function with whitespace", L"func : 123.4.", {
		{.type = Token::Type::Identifier, .value = L"func"},
		{.type = Token::Type::OperatorColon, .value = L":"},
		{.type = Token::Type::LiteralDecimal, .value = L"123.4"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	testing.TestLexer("trivial decimal function with mixed name and group", L"\r\nfunc44: (123.4).", {
		{.type = Token::Type::Identifier, .value = L"func44"},
		{.type = Token::Type::OperatorColon, .value = L":"},
		{.type = Token::Type::ParenRoundOpen, .value = L"("},
		{.type = Token::Type::LiteralDecimal, .value = L"123.4"},
		{.type = Token::Type::ParenRoundClose, .value = L")"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});

	testing.TestLexer("mixed character identifier", L"ident123.", {
		{.type = Token::Type::Identifier, .value = L"ident123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	testing.TestLexer("commented out identifier", L"<< ident123. >>", {}, {});
	testing.TestLexer("commented out identifier and whitespace", L"<<\r\nident123.\r\n>>", {}, {});
	testing.TestLexer("commented and non-commented identifier", L"<< ident123. >> ident456.", {
		{.type = Token::Type::Identifier, .value = L"ident456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	testing.TestLexer("nested comments", L"<<<< 123 << 456 >>>>.>> ident456.", {
		{.type = Token::Type::Identifier, .value = L"ident456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	testing.TestLexer("multiple comments", L"<<<<123>>ident234<<<<123<<456>>>:>>.", {
		{.type = Token::Type::Identifier, .value = L"ident234"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	testing.TestLexer("non-closed comment", L"<<<<123>>ident234<<<<123<<456>>>:>.", {
		{.type = Token::Type::Identifier, .value = L"ident234"},
		}, { {.code = ILexer::Error::Code::CommentOpen, .line = 1} });
	testing.TestLexer("comparison operator", L"1 < 2", {
		{.type = Token::Type::LiteralInteger, .value = L"1"},
		{.type = Token::Type::OperatorComparison, .value = L"<"},
		{.type = Token::Type::LiteralInteger, .value = L"2"},
		}, {});
}

void TestParser()
{
	std::wistringstream input(L"func: 123.");
	Lexer lexer(input);
	Parser parser(lexer.Lex());
	parser.Parse();
}

int main()
{
	TestLexer();
	TestParser();
}
