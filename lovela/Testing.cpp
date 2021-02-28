#include <iostream>
#include <sstream>
#include <cassert>
#include "magic_enum.hpp"
#include "Testing.h"
#include "Parser.h"
#include "Lexer.h"
#include "StaticMap.h"

void Testing::TestToken()
{
	assert((Token{} == Token{}));
	assert((Token{ Token::Type::Identifier } != Token{}));
	assert((Token{ {}, L"a" } != Token{}));
	assert((Token{ Token::Type::Identifier, L"a" } == Token{ Token::Type::Identifier, L"a" }));
	assert((Token{ Token::Type::Identifier, L"a" } != Token{ Token::Type::Identifier, L"b" }));
	assert((Token{ Token::Type::Identifier, L"a" } != Token{ Token::Type::LiteralString, L"a" }));
	assert((Token{}.empty()));
	assert((Token{ {}, L"a" }.empty()));
	assert((!Token{ Token::Type::Identifier }.empty()));

	static constexpr std::array<std::pair<int, double>, 2> values{ { {1, 1.1}, {2, 2.2} } };
	static constexpr auto map1 = StaticMap<int, double, values.size()>{{values}};
	static_assert(map1.at(1) == 1.1);
	static_assert(map1.at(2) == 2.2);
	try {
		// Must throw
		if (map1.at(3) == 3.3) {}
		assert(false);
	}
	catch (...) {}
}

void Testing::TestLexer()
{
	TestLexer("empty expression", L"", {}, {});
	TestLexer("single character", L".", { {.type = Token::Type::SeparatorDot, .value = L"." } }, {});

	TestLexer("simple identifier", L"abc", { {.type = Token::Type::Identifier, .value = L"abc" } }, {});
	TestLexer("Unicode identifier", L"\u65E5\u672C", { {.type = Token::Type::Identifier, .value = L"\u65E5\u672C" } }, {});
	//TestLexer(L"\u0061\u0300", { { .type = Token::Type::Identifier, .value = L"\u0061\u0300" }}, {});

	TestLexer("integer literal", L"123", {
		{.type = Token::Type::LiteralInteger, .value = L"123" }
		}, {});
	TestLexer("integer literal and full stop", L"123.", {
		{.type = Token::Type::LiteralInteger, .value = L"123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("decimal literal", L"123.456", { {.type = Token::Type::LiteralDecimal, .value = L"123.456" } }, {});
	TestLexer("decimal literal and full stop", L"123.456.", {
		{.type = Token::Type::LiteralDecimal, .value = L"123.456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("decimal literal, full stop, digit", L"123.456.7", {
		{.type = Token::Type::LiteralDecimal, .value = L"123.456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		{.type = Token::Type::LiteralInteger, .value = L"7"},
		}, {});

	TestLexer("empty string literal", L"''", { {.type = Token::Type::LiteralString, .value = L"" } }, {});
	TestLexer("single escaped quotation mark", L"''''", { {.type = Token::Type::LiteralString, .value = L"'" } }, {});
	TestLexer("simple string literal", L"'abc'", { {.type = Token::Type::LiteralString, .value = L"abc" } }, {});
	TestLexer("string literal with whitespace", L"'ab c'", { {.type = Token::Type::LiteralString, .value = L"ab c" } }, {});
	TestLexer("string literal with escaped quotation mark", L"'ab''c'", { {.type = Token::Type::LiteralString, .value = L"ab'c" } }, {});
	TestLexer("separated string literals", L"'ab' 'c'", {
		{.type = Token::Type::LiteralString, .value = L"ab"},
		{.type = Token::Type::LiteralString, .value = L"c"},
		}, {});
	TestLexer("comment in string literal", L"'<< abc >>'", { {.type = Token::Type::LiteralString, .value = L"<< abc >>" } }, {});
	TestLexer("non-closed string literal", L"'", {}, { {.code = ILexer::Error::Code::StringLiteralOpen } });
	TestLexer("non-closed string literal on line 1", L"'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .line = 1 } });
	TestLexer("non-closed string literal on line 2", L"\r\n'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .line = 2 } });
	TestLexer("non-closed string literal on line 2", L"\n'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .line = 2 } });
	TestLexer("non-closed string literal on line 1", L"\r'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .line = 1 } });
	TestLexer("whitespace outside and within string literal", L"\t'ab\r\n\tc'\r\n", { {.type = Token::Type::LiteralString, .value = L"ab\r\n\tc" } }, {});

	TestLexer("escaped curly bracket", L"'{{'", { {.type = Token::Type::LiteralString, .value = L"{" } }, {});
	TestLexer("escaped curly bracket", L"'{{}'", { {.type = Token::Type::LiteralString, .value = L"{}" } }, {});
	TestLexer("single closing curly bracket", L"'}'", { {.type = Token::Type::LiteralString, .value = L"}" } }, {});
	TestLexer("string field", L"'{n}'", { {.type = Token::Type::LiteralString, .value = L"\n" } }, {});
	TestLexer("string fields", L"'{t}{n}{r}'", { {.type = Token::Type::LiteralString, .value = L"\t\n\r" } }, {});
	TestLexer("embedded string fields", L"'abc{r}{n}def'", { {.type = Token::Type::LiteralString, .value = L"abc\r\ndef" } }, {});
	TestLexer("non-closed string field", L"'{n'", {}, { {.code = ILexer::Error::Code::StringFieldIllformed}, {.code = ILexer::Error::Code::StringLiteralOpen} });
	TestLexer("ill-formed string field", L"'{nn}'", { {.type = Token::Type::LiteralString, .value = L"}"} }, { {.code = ILexer::Error::Code::StringFieldIllformed} });
	TestLexer("unknown string field", L"'{m}'", { {.type = Token::Type::LiteralString, .value = L"m}"} }, { {.code = ILexer::Error::Code::StringFieldUnknown} });

	TestLexer("unindexed string interpolation", L"'{}'", {
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});
	TestLexer("embedded unindexed string interpolation", L"'abc{}'", {
		{.type = Token::Type::LiteralString, .value = L"abc" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});
	TestLexer("unindexed string interpolations", L"'abc{}{}'", {
		{.type = Token::Type::LiteralString, .value = L"abc" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"2" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});
	TestLexer("unindexed string interpolation", L"'{2}'", {
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"2" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});
	TestLexer("unindexed string interpolations", L"'abc{4}{1}'", {
		{.type = Token::Type::LiteralString, .value = L"abc" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"4" },
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		}, {});

	TestLexer("trivial integer function", L"func: 123.", {
		{.type = Token::Type::Identifier, .value = L"func"},
		{.type = Token::Type::OperatorColon, .value = L":"},
		{.type = Token::Type::LiteralInteger, .value = L"123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("trivial decimal function with whitespace", L"func : 123.4.", {
		{.type = Token::Type::Identifier, .value = L"func"},
		{.type = Token::Type::OperatorColon, .value = L":"},
		{.type = Token::Type::LiteralDecimal, .value = L"123.4"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("trivial decimal function with mixed name and group", L"\r\nfunc44: (123.4).", {
		{.type = Token::Type::Identifier, .value = L"func44"},
		{.type = Token::Type::OperatorColon, .value = L":"},
		{.type = Token::Type::ParenRoundOpen, .value = L"("},
		{.type = Token::Type::LiteralDecimal, .value = L"123.4"},
		{.type = Token::Type::ParenRoundClose, .value = L")"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});

	TestLexer("mixed character identifier", L"ident123.", {
		{.type = Token::Type::Identifier, .value = L"ident123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("commented out identifier", L"<< ident123. >>", {}, {});
	TestLexer("commented out identifier and whitespace", L"<<\r\nident123.\r\n>>", {}, {});
	TestLexer("commented and non-commented identifier", L"<< ident123. >> ident456.", {
		{.type = Token::Type::Identifier, .value = L"ident456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("nested comments", L"<<<< 123 << 456 >>>>.>> ident456.", {
		{.type = Token::Type::Identifier, .value = L"ident456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("multiple comments", L"<<<<123>>ident234<<<<123<<456>>>:>>.", {
		{.type = Token::Type::Identifier, .value = L"ident234"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("non-closed comment", L"<<<<123>>ident234<<<<123<<456>>>:>.", {
		{.type = Token::Type::Identifier, .value = L"ident234"},
		}, { {.code = ILexer::Error::Code::CommentOpen, .line = 1} });
	TestLexer("comparison operator", L"1 < 2", {
		{.type = Token::Type::LiteralInteger, .value = L"1"},
		{.type = Token::Type::OperatorComparison, .value = L"<"},
		{.type = Token::Type::LiteralInteger, .value = L"2"},
		}, {});
}

void Testing::TestParser()
{
	std::wistringstream input(L"func: 123.");
	Lexer lexer(input);
	Parser parser(lexer.Lex());
	auto node = parser.Parse();
	assert(node->type == Node::Type::Root);
	assert(node->children.size() == 1);
	assert(node->children.front()->type == Node::Type::Function);
}

void Testing::TestLexer(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	Lexer lexer(input);
	auto tokenGenerator = lexer.Lex();
	auto tokens = std::vector<Token>(tokenGenerator.begin(), tokenGenerator.end());

	bool success = true;

	auto actualCount = tokens.size();
	auto expectedCount = expectedTokens.size();
	auto count = std::max(actualCount, expectedCount);
	for (int i = 0; i < count; i++)
	{
		const auto actual = i < actualCount ? tokens[i] : Token{};
		const auto expected = i < expectedCount ? expectedTokens[i] : Token{};
		if (actual != expected)
		{
			success = false;
			std::cerr << "Test \"" << name << "\" error: Token " << i + 1 << " is " << magic_enum::enum_name(actual.type) << " \"";
			std::wcerr << actual.value;
			std::cerr << "\", expected " << magic_enum::enum_name(expected.type) << " \"";
			std::wcerr << expected.value << "\".\n";
		}
	}

	auto& errors = lexer.GetErrors();

	actualCount = errors.size();
	expectedCount = expectedErrors.size();
	count = std::max(actualCount, expectedCount);
	for (int i = 0; i < count; i++)
	{
		const auto actual = i < actualCount ? errors[i] : ILexer::Error{};
		const auto expected = i < expectedCount ? expectedErrors[i] : ILexer::Error{};
		if (actual.code != expected.code)
		{
			success = false;
			std::cerr << "Test \"" << name << "\" error: Error " << i + 1 << " code is " << magic_enum::enum_name(actual.code) << ", expected " << magic_enum::enum_name(expected.code) << ".\n";
			std::cerr << '(' << actual.line << ':' << actual.column << ") error " << magic_enum::enum_name(actual.code) << ": ";
			std::wcerr << actual.message << '\n';
		}
		else if (expected.line && actual.line != expected.line)
		{
			success = false;
			std::cerr << "Test \"" << name << "\" error: Error " << i + 1 << " line number is " << actual.line << ", expected " << expected.line << ".\n";
			std::cerr << '(' << actual.line << ':' << actual.column << ") error " << magic_enum::enum_name(actual.code) << ": ";
			std::wcerr << actual.message << '\n';
		}
	}

	assert(success);
}
