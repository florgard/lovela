#include "pch.h"
#include "TestingBase.h"
#include "../lovela/Lexer.h"

class LexerTest : public TestingBase
{
public:
	bool YieldsTokens(const char* name, std::string_view code, const std::vector<Token>& expectedTokens);
};

static LexerTest lexerTest;

bool LexerTest::YieldsTokens(const char* name, std::string_view code, const std::vector<Token>& expectedTokens)
{
	StringLexer lexer;
	std::vector<Token> tokens;
	code >> lexer >> tokens;

	bool success = true;

	auto actualCount = tokens.size();
	auto expectedCount = expectedTokens.size();
	auto count = std::max(actualCount, expectedCount);
	const Token emptyToken{};

	for (size_t i = 0; i < count; i++)
	{
		const auto& actual = i < actualCount ? tokens[i] : emptyToken;
		const auto& expected = i < expectedCount ? expectedTokens[i] : emptyToken;
		if (actual != expected)
		{
			success = false;
			std::cerr << color.fail << "ERROR: " << color.none
				<< "Lexer test \"" << color.name << name << color.none << "\": "
				<< "Token " << i + 1 << " is " << to_string(actual.type) << " \"" << actual.value << "\", expected " << to_string(expected.type) << " \"" << expected.value << "\".\n"
				<< "Actual:\n" << color.actual;
			actual.Print(std::cerr);
			std::cerr << color.none << '\n'
				<< "Expected:\n" << color.expect;
			expected.Print(std::cerr);
			std::cerr << color.none << '\n';
			lexer.PrintErrorSourceCode(std::cerr, actual);
		}
	}

	if (!success)
	{
		std::cerr << "Input code:\n" << color.code << code << color.none << '\n';
		std::cerr << '\n';
	}

	return success;
}

static constexpr Token EndToken()
{
	return { .type = Token::Type::End };
}

static constexpr Token IdToken(std::string&& identifier)
{
	return { .type = Token::Type::Identifier, .value = std::move(identifier) };
}

static constexpr Token ErrorToken(Token::Error::Code error, size_t line = 1)
{
	return { .type = Token::Type::Error, .error{.code = error, .line = line} };
}

using namespace boost::ut;

suite lexer_rudimental_tests = [] {
	"empty expression"_test = [] {
		expect(lexerTest.YieldsTokens("empty expression",
			"",
			{
				EndToken()
			}
		));
	};
	"single character"_test = [] {
		expect(lexerTest.YieldsTokens("single character",
			".",
			{
				{.type = Token::Type::SeparatorDot, .value = "." },
				EndToken()
			}
		));
	};
};

suite lexer_identifier_tests = [] {
	"simple identifier"_test = [] {
		expect(lexerTest.YieldsTokens("simple identifier",
			"abc",
			{
				IdToken("abc"),
				EndToken()
			}
		));
	};
	"two identifiers"_test = [] {
		expect(lexerTest.YieldsTokens("two identifiers",
			"abc def",
			{
				IdToken("abc"),
				IdToken("def"),
				EndToken()
			}
		));
	};
	"alphanumerical identifier"_test = [] {
		expect(lexerTest.YieldsTokens("alphanumerical identifier",
			"abc123",
			{
				IdToken("abc123"),
				EndToken()
			}
		));
	};
	"kebab case identifier"_test = [] {
		expect(lexerTest.YieldsTokens("kebab case identifier",
			"abc-123",
			{
				IdToken("abc-123"),
				EndToken()
			}
		));
	};
	"snake case identifier"_test = [] {
		expect(lexerTest.YieldsTokens("snake case identifier",
			"abc_123",
			{
				IdToken("abc_123"),
				EndToken()
			}
		));
	};
	"operator character identifier"_test = [] {
		expect(lexerTest.YieldsTokens("operator character identifier",
			"abc>=123",
			{
				IdToken("abc>=123"),
				EndToken()
			}
		));
	};
	"Unicode identifier"_test = [] {
		expect(lexerTest.YieldsTokens("Unicode identifier",
			"\xE6\x97\xA5\xE6\x9C\xAC", // Nihon in nihongo, U+65E5 U+672C
			{
				IdToken("\xE6\x97\xA5\xE6\x9C\xAC"),
				EndToken()
			}
		));
	};
	"Unicode combining mark identifier"_test = [] {
		expect(lexerTest.YieldsTokens("Unicode combining mark identifier",
			"a\xCC\x80", // a with combining mark, U+0300
			{
				IdToken("a\xCC\x80"),
				EndToken()
			}
		));
	};
	"invalid identifier 1"_test = [] {
		expect(lexerTest.YieldsTokens("invalid identifier 1",
			"1abc",
			{
				{.type = Token::Type::LiteralInteger, .value = "1"},
				ErrorToken(Token::Error::Code::SyntaxError),
				IdToken("abc"),
				EndToken()
			}
		));
	};
	"invalid identifier 2"_test = [] {
		expect(lexerTest.YieldsTokens("invalid identifier 2",
			"=abc",
			{
				ErrorToken(Token::Error::Code::SyntaxError),
				EndToken()
			}
		));
	};
};

suite lexer_numeric_literals_tests = [] {
	"integer literal"_test = [] {
		expect(lexerTest.YieldsTokens("integer literal",
			"123",
			{
				{.type = Token::Type::LiteralInteger, .value = "123" },
				EndToken()
			}
		));
	};
	"integer literal and full stop"_test = [] {
		expect(lexerTest.YieldsTokens("integer literal and full stop",
			"123.",
			{
				{.type = Token::Type::LiteralInteger, .value = "123"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				EndToken()
			}
		));
	};
	"decimal literal"_test = [] {
		expect(lexerTest.YieldsTokens("decimal literal",
			"123.456",
			{
				{.type = Token::Type::LiteralDecimal, .value = "123.456" },
				EndToken()
			}
		));
	};
	"decimal literal and full stop"_test = [] {
		expect(lexerTest.YieldsTokens("decimal literal and full stop",
			"123.456.",
			{
				{.type = Token::Type::LiteralDecimal, .value = "123.456"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				EndToken()
			}
		));
	};
	"decimal literal, full stop, digit"_test = [] {
		expect(lexerTest.YieldsTokens("decimal literal, full stop, digit",
			"123.456.7",
			{
				{.type = Token::Type::LiteralDecimal, .value = "123.456"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				{.type = Token::Type::LiteralInteger, .value = "7"},
				EndToken()
			}
		));
	};
};

suite lexer_string_literal_tests = [] {
	"empty string literal"_test = [] {
		expect(lexerTest.YieldsTokens("empty string literal",
			"''",
			{
				{.type = Token::Type::LiteralString, .value = "" },
				EndToken()
			}
		));
	};
	"single escaped quotation mark"_test = [] {
		expect(lexerTest.YieldsTokens("single escaped quotation mark",
			"''''",
			{
				{.type = Token::Type::LiteralString, .value = "'" },
				EndToken()
			}
		));
	};
	"simple string literal"_test = [] {
		expect(lexerTest.YieldsTokens("simple string literal",
			"'abc'",
			{
				{.type = Token::Type::LiteralString, .value = "abc" },
				EndToken()
			}
		));
	};
	"string literal with whitespace"_test = [] {
		expect(lexerTest.YieldsTokens("string literal with whitespace",
			"'ab c'",
			{
				{.type = Token::Type::LiteralString, .value = "ab c" },
				EndToken()
			}
		));
	};
	"string literal with escaped quotation mark"_test = [] {
		expect(lexerTest.YieldsTokens("string literal with escaped quotation mark",
			"'ab''c'",
			{
				{.type = Token::Type::LiteralString, .value = "ab'c" },
				EndToken()
			}
		));
	};
	"separated string literals"_test = [] {
		expect(lexerTest.YieldsTokens("separated string literals",
			"'ab' 'c'",
			{
				{.type = Token::Type::LiteralString, .value = "ab"},
				{.type = Token::Type::LiteralString, .value = "c"},
				EndToken()
			}
		));
	};
	"comment in string literal"_test = [] {
		expect(lexerTest.YieldsTokens("comment in string literal",
			"'<< abc >>'",
			{
				{.type = Token::Type::LiteralString, .value = "<< abc >>" },
				EndToken()
			}
		));
	};
	"non-closed string literal"_test = [] {
		expect(lexerTest.YieldsTokens("non-closed string literal",
			"'",
			{
				ErrorToken(Token::Error::Code::StringLiteralOpen),
				EndToken()
			}
		));
	};
	"non-closed string literal on line 1"_test = [] {
		expect(lexerTest.YieldsTokens("non-closed string literal on line 1",
			"'abc",
			{
				ErrorToken(Token::Error::Code::StringLiteralOpen),
				EndToken()
			}
		));
	};
	"non-closed string literal on line 2 CRLF"_test = [] {
		expect(lexerTest.YieldsTokens("non-closed string literal on line 2 CRLF",
			"\r\n'abc",
			{
				ErrorToken(Token::Error::Code::StringLiteralOpen, 2),
				EndToken()
			}
		));
	};
	"non-closed string literal on line 2 LF"_test = [] {
		expect(lexerTest.YieldsTokens("non-closed string literal on line 2 LF", "\n'abc",
			{
				ErrorToken(Token::Error::Code::StringLiteralOpen, 2),
				EndToken()
			}
		));
	};
	"non-closed string literal on line 1 CR"_test = [] {
		expect(lexerTest.YieldsTokens("non-closed string literal on line 1 CR",
			"\r'abc",
			{
				ErrorToken(Token::Error::Code::StringLiteralOpen),
				EndToken()
			}
		));
	};
	"whitespace outside and within string literal"_test = [] {
		expect(lexerTest.YieldsTokens("whitespace outside and within string literal",
			"\t'ab\r\n\tc'\r\n",
			{
				{.type = Token::Type::LiteralString, .value = "ab\r\n\tc" },
				EndToken()
			}
		));
	};
};

suite lexer_string_field_tests = [] {
	"escaped curly bracket"_test = [] {
		expect(lexerTest.YieldsTokens("escaped curly bracket",
			"'{{'",
			{
				{.type = Token::Type::LiteralString, .value = "{" },
				EndToken()
			}
		));
	};
	"escaped curly bracket"_test = [] {
		expect(lexerTest.YieldsTokens("escaped curly bracket",
			"'{{}'",
			{
				{.type = Token::Type::LiteralString, .value = "{}" },
				EndToken()
			}
		));
	};
	"single closing curly bracket"_test = [] {
		expect(lexerTest.YieldsTokens("single closing curly bracket",
			"'}'",
			{
				{.type = Token::Type::LiteralString, .value = "}" },
				EndToken()
			}
		));
	};
	"string field"_test = [] {
		expect(lexerTest.YieldsTokens("string field",
			"'{n}'",
			{
				{.type = Token::Type::LiteralString, .value = "\n" },
				EndToken()
			}
		));
	};
	"string fields"_test = [] {
		expect(lexerTest.YieldsTokens("string fields",
			"'{t}{n}{r}'",
			{
				{.type = Token::Type::LiteralString, .value = "\t\n\r" },
				EndToken()
			}
		));
	};
	"embedded string fields"_test = [] {
		expect(lexerTest.YieldsTokens("embedded string fields",
			"'abc{r}{n}def'",
			{
				{.type = Token::Type::LiteralString, .value = "abc\r\ndef" },
				EndToken()
			}
		));
	};
	"non-closed string field"_test = [] {
		expect(lexerTest.YieldsTokens("non-closed string field",
			"'{n'",
			{
				ErrorToken(Token::Error::Code::StringFieldIllformed),
				{.type = Token::Type::LiteralString},
				EndToken()
			}
		));
	};
	"ill-formed string field"_test = [] {
		expect(lexerTest.YieldsTokens("ill-formed string field",
			"'{nn}'",
			{
				ErrorToken(Token::Error::Code::StringFieldIllformed),
				{.type = Token::Type::LiteralString, .value = "n}"},
				EndToken()
			}
		));
	};
	"unknown string field"_test = [] {
		expect(lexerTest.YieldsTokens("unknown string field",
			"'{m}'",
			{
				ErrorToken(Token::Error::Code::StringFieldUnknown),
				{.type = Token::Type::LiteralString, .value = "m}"},
				EndToken()
			}
		));
	};
};

suite lexer_string_interpolation_tests = [] {
	"unindexed string interpolation"_test = [] {
		expect(lexerTest.YieldsTokens("unindexed string interpolation",
			"'{}'",
			{
				{.type = Token::Type::LiteralString, .value = "" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "1" },
				{.type = Token::Type::LiteralString, .value = "" },
				EndToken()
			}
		));
	};
	"embedded unindexed string interpolation"_test = [] {
		expect(lexerTest.YieldsTokens("embedded unindexed string interpolation",
			"'abc{}'",
			{
				{.type = Token::Type::LiteralString, .value = "abc" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "1" },
				{.type = Token::Type::LiteralString, .value = "" },
				EndToken()
			}
		));
	};
	"unindexed string interpolations"_test = [] {
		expect(lexerTest.YieldsTokens("unindexed string interpolations",
			"'abc{}{}'",
			{
				{.type = Token::Type::LiteralString, .value = "abc" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "1" },
				{.type = Token::Type::LiteralString, .value = "" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "2" },
				{.type = Token::Type::LiteralString, .value = "" },
				EndToken()
			}
		));
	};
	"indexed string interpolation"_test = [] {
		expect(lexerTest.YieldsTokens("indexed string interpolation",
			"'{2}'",
			{
				{.type = Token::Type::LiteralString, .value = "" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "2" },
				{.type = Token::Type::LiteralString, .value = "" },
				EndToken()
			}
		));
	};
	"indexed string interpolations"_test = [] {
		expect(lexerTest.YieldsTokens("indexed string interpolations",
			"'abc{4}{1}'",
			{
				{.type = Token::Type::LiteralString, .value = "abc" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "4" },
				{.type = Token::Type::LiteralString, .value = "" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "1" },
				{.type = Token::Type::LiteralString, .value = "" },
				EndToken()
			}
		));
	};
};

suite lexer_function_declarations_tests = [] {
	"trivial function declaration"_test = [] {
		expect(lexerTest.YieldsTokens("trivial function declaration",
			"func",
			{
				IdToken("func"),
				EndToken()
			}
		));
	};
	"trivial integer function"_test = [] {
		expect(lexerTest.YieldsTokens("trivial integer function",
			"func: 123.",
			{
				IdToken("func"),
				{.type = Token::Type::SeparatorColon, .value = ":"},
				{.type = Token::Type::LiteralInteger, .value = "123"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				EndToken()
			}
		));
	};
	"trivial decimal function with whitespace"_test = [] {
		expect(lexerTest.YieldsTokens("trivial decimal function with whitespace",
			"func : 123.4.",
			{
				IdToken("func"),
				{.type = Token::Type::SeparatorColon, .value = ":"},
				{.type = Token::Type::LiteralDecimal, .value = "123.4"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				EndToken()
			}
		));
	};
	"trivial decimal function with mixed name and group"_test = [] {
		expect(lexerTest.YieldsTokens("trivial decimal function with mixed name and group",
			"\r\nfunc44: (123.4).",
			{
				IdToken("func44"),
				{.type = Token::Type::SeparatorColon, .value = ":"},
				{.type = Token::Type::ParenRoundOpen, .value = "("},
				{.type = Token::Type::LiteralDecimal, .value = "123.4"},
				{.type = Token::Type::ParenRoundClose, .value = ")"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				EndToken()
			}
		));
	};
	"imported function"_test = [] {
		expect(lexerTest.YieldsTokens("imported function",
			"-> func",
			{
				{.type = Token::Type::OperatorArrow, .value = "->"},
				IdToken("func"),
				EndToken()
			}
		));
	};
	"exported function"_test = [] {
		expect(lexerTest.YieldsTokens("exported function",
			"<- []func",
			{
				{.type = Token::Type::OperatorArrow, .value = "<-"},
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				IdToken("func"),
				EndToken()
			}
		));
	};
	"function with namespace"_test = [] {
		expect(lexerTest.YieldsTokens("function with namespace",
			"namespace/func",
			{
				IdToken("namespace"),
				{.type = Token::Type::SeparatorSlash, .value = "/"},
				IdToken("func"),
				EndToken()
			}
		));
	};
};

suite lexer_comment_tests = [] {
	"mixed character identifier"_test = [] {
		expect(lexerTest.YieldsTokens("mixed character identifier",
			"ident123.",
			{
				IdToken("ident123"),
				{.type = Token::Type::SeparatorDot, .value = "."},
				EndToken()
			}
		));
	};
	"commented out identifier"_test = [] {
		expect(lexerTest.YieldsTokens("commented out identifier",
			"<< ident123. >>",
			{
				{.type = Token::Type::Comment, .value = " ident123. "},
				EndToken()
			}
		));
	};
	"commented out identifier and whitespace"_test = [] {
		expect(lexerTest.YieldsTokens("commented out identifier and whitespace",
			"<<\r\nident123.\r\n>>",
			{
				{.type = Token::Type::Comment, .value = "\r\nident123.\r\n"},
				EndToken()
			}
		));
	};
	"commented and non-commented identifier"_test = [] {
		expect(lexerTest.YieldsTokens("commented and non-commented identifier",
			"<< ident123. >> ident456.",
			{
				{.type = Token::Type::Comment, .value = " ident123. "},
				IdToken("ident456"),
				{.type = Token::Type::SeparatorDot, .value = "."},
				EndToken()
			}
		));
	};
	"nested comments"_test = [] {
		expect(lexerTest.YieldsTokens("nested comments",
			"<<<< 123 << 456 >>>>.>> ident456.",
			{
				{.type = Token::Type::Comment, .value = " 123 "},
				{.type = Token::Type::Comment, .value = " 456 "},
				{.type = Token::Type::Comment, .value = "."},
				IdToken("ident456"),
				{.type = Token::Type::SeparatorDot, .value = "."},
				EndToken()
			}
		));
	};
	"multiple comments"_test = [] {
		expect(lexerTest.YieldsTokens("multiple comments",
			"<<<<123>>ident234<<<<123<<456>>>:>>.",
			{
				{.type = Token::Type::Comment, .value = "123"},
				IdToken("ident234"),
				{.type = Token::Type::Comment, .value = "123"},
				{.type = Token::Type::Comment, .value = "456"},
				{.type = Token::Type::Comment, .value = ":"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				EndToken()
			}
		));
	};
	"non-closed comment"_test = [] {
		expect(lexerTest.YieldsTokens("non-closed comment",
			"<<<<123>>ident234<<<<123<<456>>>:>.",
			{
				{.type = Token::Type::Comment, .value = "123"},
				IdToken("ident234"),
				{.type = Token::Type::Comment, .value = "123"},
				{.type = Token::Type::Comment, .value = "456"},
				{.type = Token::Type::Comment, .value = ":>."},
				ErrorToken(Token::Error::Code::CommentOpen),
				EndToken()
			}
		));
	};
};

suite lexer_comparison_tests = [] {
	"comparison operator"_test = [] {
		expect(lexerTest.YieldsTokens("comparison operator",
			"1 < 2",
			{
				{.type = Token::Type::LiteralInteger, .value = "1"},
				{.type = Token::Type::OperatorComparison, .value = "<"},
				{.type = Token::Type::LiteralInteger, .value = "2"},
				EndToken()
			}
		));
	};
	"comparison declaration"_test = [] {
		expect(lexerTest.YieldsTokens("comparison declaration",
			"<(operand)",
			{
				{.type = Token::Type::OperatorComparison, .value = "<"},
				{.type = Token::Type::ParenRoundOpen, .value = "("},
				IdToken("operand"),
				{.type = Token::Type::ParenRoundClose, .value = ")"},
				EndToken()
			}
		));
	};
};

suite lexer_primitive_types_tests = [] {
	"primitive type, int"_test = [] {
		expect(lexerTest.YieldsTokens("primitive type, int",
			"[1000000]",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "1000000"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				EndToken()
			}
		));
	};
	"primitive type, negative int"_test = [] {
		expect(lexerTest.YieldsTokens("primitive type, negative int",
			"[-1000000]",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "-1000000"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				EndToken()
			}
		));
	};
	"primitive type, double"_test = [] {
		expect(lexerTest.YieldsTokens("primitive type, double",
			"[1.0e300]",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralDecimal, .value = "1.0e300"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				EndToken()
			}
		));
	};
	"primitive type, int8 array"_test = [] {
		expect(lexerTest.YieldsTokens("primitive type, int8 array",
			"[100]#",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "100"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				{.type = Token::Type::SeparatorHash, .value = "#"},
				EndToken()
			}
		));
	};
	"primitive type, int8 array of arrays"_test = [] {
		expect(lexerTest.YieldsTokens("primitive type, int8 array of arrays",
			"[100]##",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "100"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				{.type = Token::Type::SeparatorHash, .value = "#"},
				{.type = Token::Type::SeparatorHash, .value = "#"},
				EndToken()
			}
		));
	};
	"primitive type, int32 array size 8"_test = [] {
		expect(lexerTest.YieldsTokens("primitive type, int32 array size 8",
			"[100000]#8",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "100000"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				{.type = Token::Type::SeparatorHash, .value = "#"},
				{.type = Token::Type::LiteralInteger, .value = "8"},
				EndToken()
			}
		));
	};
};

suite lexer_builtin_types_tests = [] {
	"built-in type, /type/i32"_test = [] {
		expect(lexerTest.YieldsTokens("built-in type, /type/i32",
			"[/type/i32]",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::SeparatorSlash, .value = "/"},
				IdToken("type"),
				{.type = Token::Type::SeparatorSlash, .value = "/"},
				IdToken("i32"),
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				EndToken()
			}
		));
	};
};
