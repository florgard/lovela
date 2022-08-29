#include "pch.h"
#include "TestingBase.h"

static const Token endToken{ .type = Token::Type::End };
static constexpr auto ident = Token::Type::Identifier;

suite lexer_rudimental_tests = [] {
	"empty expression"_test = [] {
		expect(LexerTest::Success("empty expression",
			L"",
			{
				endToken
			}
		));
	};
	"single character"_test = [] {
		expect(LexerTest::Success("single character",
			L".",
			{
				{.type = Token::Type::SeparatorDot, .value = L"." },
				endToken
			}
		));
	};
};

suite lexer_identifier_tests = [] {
	"simple identifier"_test = [] {
		expect(LexerTest::Success("simple identifier",
			L"abc",
			{
				{.type = ident, .value = L"abc" },
				endToken
			}
		));
	};
	"two identifiers"_test = [] {
		expect(LexerTest::Success("two identifiers",
			L"abc def",
			{
				{.type = ident, .value = L"abc" },
				{.type = ident, .value = L"def" },
				endToken
			}
		));
	};
	"alphanumerical identifier"_test = [] {
		expect(LexerTest::Success("alphanumerical identifier",
			L"abc123",
			{
				{.type = ident, .value = L"abc123" },
				endToken
			}
		));
	};
	"kebab case identifier"_test = [] {
		expect(LexerTest::Success("kebab case identifier",
			L"abc-123",
			{
				{.type = ident, .value = L"abc-123" },
				endToken
			}
		));
	};
	"snake case identifier"_test = [] {
		expect(LexerTest::Success("snake case identifier",
			L"abc_123",
			{
				{.type = ident, .value = L"abc_123" },
				endToken
			}));
	};
	"operator character identifier"_test = [] {
		expect(LexerTest::Success("operator character identifier",
			L"abc>=123",
			{
				{.type = ident, .value = L"abc>=123" },
				endToken
			}
		));
	};
	"Unicode identifier"_test = [] {
		expect(LexerTest::Success("Unicode identifier",
			L"\u65E5\u672C",
			{
				{.type = ident, .value = L"\u65E5\u672C" },
				endToken
			}
		));
	};
	"Unicode combining mark identifier"_test = [] {
		expect(LexerTest::Success("Unicode combining mark identifier",
			L"a\u0300",
			{
				{.type = ident, .value = L"a\u0300" },
				endToken
			}
		));
	};
	"invalid identifier 1"_test = [] {
		expect(LexerTest::Failure("invalid identifier 1",
			L"1abc",
			{
				endToken
			},
			{
				{.code = ILexer::Error::Code::SyntaxError}
			}
			));
	};
	"invalid identifier 2"_test = [] {
		expect(LexerTest::Failure("invalid identifier 2",
			L"=abc",
			{
				endToken
			},
			{
				{.code = ILexer::Error::Code::SyntaxError}
			}
			));
	};
};

suite lexer_numeric_literals_tests = [] {
	"integer literal"_test = [] {
		expect(LexerTest::Success("integer literal",
			L"123",
			{
				{.type = Token::Type::LiteralInteger, .value = L"123" },
				endToken
			}
		));
	};
	"integer literal and full stop"_test = [] {
		expect(LexerTest::Success("integer literal and full stop",
			L"123.",
			{
				{.type = Token::Type::LiteralInteger, .value = L"123"},
				{.type = Token::Type::SeparatorDot, .value = L"."},
				endToken
			}
		));
	};
	"decimal literal"_test = [] {
		expect(LexerTest::Success("decimal literal",
			L"123.456",
			{
				{.type = Token::Type::LiteralDecimal, .value = L"123.456" },
				endToken
			}
		));
	};
	"decimal literal and full stop"_test = [] {
		expect(LexerTest::Success("decimal literal and full stop",
			L"123.456.",
			{
				{.type = Token::Type::LiteralDecimal, .value = L"123.456"},
				{.type = Token::Type::SeparatorDot, .value = L"."},
				endToken
			}
		));
	};
	"decimal literal, full stop, digit"_test = [] {
		expect(LexerTest::Success("decimal literal, full stop, digit",
			L"123.456.7",
			{
				{.type = Token::Type::LiteralDecimal, .value = L"123.456"},
				{.type = Token::Type::SeparatorDot, .value = L"."},
				{.type = Token::Type::LiteralInteger, .value = L"7"},
				endToken
			}
		));
	};
};
