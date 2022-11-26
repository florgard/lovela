#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

class LexerTest : public TestingBase
{
public:
	bool Success(const char* name, std::string_view code, const std::vector<Token>& expectedTokens)
	{
		return Failure(name, code, expectedTokens, {});
	}

	bool Failure(const char* name, std::string_view code, const std::vector<Token>& expectedTokens, const std::vector<Token>& expectedErrors);
};

static LexerTest s_test;

bool LexerTest::Failure(const char* name, std::string_view code, const std::vector<Token>& expectedTokens, const std::vector<Token>& expectedErrors)
{
	std::istringstream input(std::string(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto tokens = to_vector(lexer->Lex());

	bool success = true;
	const Token emptyToken{};

	auto actualCount = tokens.size();
	auto expectedCount = expectedTokens.size();
	auto count = std::max(actualCount, expectedCount);
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
		}
	}

	const auto errors = to_vector(tokens | std::views::filter([](auto& t) { return t.type == Token::Type::Error; }));

	actualCount = errors.size();
	expectedCount = expectedErrors.size();
	count = std::max(actualCount, expectedCount);
	for (size_t i = 0; i < count; i++)
	{
		const auto& actual = i < actualCount ? errors[i] : emptyToken;
		const auto& expected = i < expectedCount ? expectedErrors[i] : emptyToken;
		if (actual.error != expected.error)
		{
			success = false;

			PrintIncorrectErrorCodeMessage(std::cerr, "Lexer", name, i, actual.error, expected.error);
			PrintErrorMessage(std::cerr, actual);
		}
		else if (expected.line && actual.line != expected.line)
		{
			success = false;

			PrintIncorrectErrorLineMessage(std::cerr, "Lexer", name, i, actual.line, expected.line);
			PrintErrorMessage(std::cerr, actual);
		}
	}

	if (!success)
	{
		std::cerr << '\n';
	}

	return success;
}

static const Token endToken{ .type = Token::Type::End };
static constexpr auto idType = Token::Type::Identifier;

static constexpr Token ErrorToken(LexerError error)
{
	return {.type = Token::Type::Error, .error = error};
}

suite lexer_rudimental_tests = [] {
	"empty expression"_test = [] {
		expect(s_test.Success("empty expression",
			"",
			{
				endToken
			}
		));
	};
	"single character"_test = [] {
		expect(s_test.Success("single character",
			".",
			{
				{.type = Token::Type::SeparatorDot, .value = "." },
				endToken
			}
		));
	};
};

suite lexer_identifier_tests = [] {
	"simple identifier"_test = [] {
		expect(s_test.Success("simple identifier",
			"abc",
			{
				{.type = idType, .value = "abc" },
				endToken
			}
		));
	};
	"two identifiers"_test = [] {
		expect(s_test.Success("two identifiers",
			"abc def",
			{
				{.type = idType, .value = "abc" },
				{.type = idType, .value = "def" },
				endToken
			}
		));
	};
	"alphanumerical identifier"_test = [] {
		expect(s_test.Success("alphanumerical identifier",
			"abc123",
			{
				{.type = idType, .value = "abc123" },
				endToken
			}
		));
	};
	"kebab case identifier"_test = [] {
		expect(s_test.Success("kebab case identifier",
			"abc-123",
			{
				{.type = idType, .value = "abc-123" },
				endToken
			}
		));
	};
	"snake case identifier"_test = [] {
		expect(s_test.Success("snake case identifier",
			"abc_123",
			{
				{.type = idType, .value = "abc_123" },
				endToken
			}
		));
	};
	"operator character identifier"_test = [] {
		expect(s_test.Success("operator character identifier",
			"abc>=123",
			{
				{.type = idType, .value = "abc>=123" },
				endToken
			}
		));
	};
	"Unicode identifier"_test = [] {
		expect(s_test.Success("Unicode identifier",
			"\xE6\x97\xA5\xE6\x9C\xAC", // Nihon in nihongo, U+65E5 U+672C
			{
				{.type = idType, .value = "\xE6\x97\xA5\xE6\x9C\xAC" },
				endToken
			}
		));
	};
	"Unicode combining mark identifier"_test = [] {
		expect(s_test.Success("Unicode combining mark identifier",
			"a\xCC\x80", // a with combining mark, U+0300
			{
				{.type = idType, .value = "a\xCC\x80" },
				endToken
			}
		));
	};
	"invalid identifier 1"_test = [] {
		expect(s_test.Failure("invalid identifier 1",
			"1abc",
			{
				{.type = Token::Type::LiteralInteger, .value = "1"},
				{.type = Token::Type::Error, .error = LexerError::SyntaxError },
				{.type = idType, .value = "abc"},
				endToken
			},
			{
				{.error = LexerError::SyntaxError}
			}
		));
	};
	"invalid identifier 2"_test = [] {
		expect(s_test.Failure("invalid identifier 2",
			"=abc",
			{
				{.type = Token::Type::Error, .error = LexerError::SyntaxError },
				endToken
			},
			{
				{.error = LexerError::SyntaxError}
			}
		));
	};
};

suite lexer_numeric_literals_tests = [] {
	"integer literal"_test = [] {
		expect(s_test.Success("integer literal",
			"123",
			{
				{.type = Token::Type::LiteralInteger, .value = "123" },
				endToken
			}
		));
	};
	"integer literal and full stop"_test = [] {
		expect(s_test.Success("integer literal and full stop",
			"123.",
			{
				{.type = Token::Type::LiteralInteger, .value = "123"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"decimal literal"_test = [] {
		expect(s_test.Success("decimal literal",
			"123.456",
			{
				{.type = Token::Type::LiteralDecimal, .value = "123.456" },
				endToken
			}
		));
	};
	"decimal literal and full stop"_test = [] {
		expect(s_test.Success("decimal literal and full stop",
			"123.456.",
			{
				{.type = Token::Type::LiteralDecimal, .value = "123.456"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"decimal literal, full stop, digit"_test = [] {
		expect(s_test.Success("decimal literal, full stop, digit",
			"123.456.7",
			{
				{.type = Token::Type::LiteralDecimal, .value = "123.456"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				{.type = Token::Type::LiteralInteger, .value = "7"},
				endToken
			}
		));
	};
};

suite lexer_string_literal_tests = [] {
	"empty string literal"_test = [] {
		expect(s_test.Success("empty string literal",
			"''",
			{
				{.type = Token::Type::LiteralString, .value = "" },
				endToken
			}
		));
	};
	"single escaped quotation mark"_test = [] {
		expect(s_test.Success("single escaped quotation mark",
			"''''",
			{
				{.type = Token::Type::LiteralString, .value = "'" },
				endToken
			}
		));
	};
	"simple string literal"_test = [] {
		expect(s_test.Success("simple string literal",
			"'abc'",
			{
				{.type = Token::Type::LiteralString, .value = "abc" },
				endToken
			}
		));
	};
	"string literal with whitespace"_test = [] {
		expect(s_test.Success("string literal with whitespace",
			"'ab c'",
			{
				{.type = Token::Type::LiteralString, .value = "ab c" },
				endToken
			}
		));
	};
	"string literal with escaped quotation mark"_test = [] {
		expect(s_test.Success("string literal with escaped quotation mark",
			"'ab''c'",
			{
				{.type = Token::Type::LiteralString, .value = "ab'c" },
				endToken
			}
		));
	};
	"separated string literals"_test = [] {
		expect(s_test.Success("separated string literals",
			"'ab' 'c'",
			{
				{.type = Token::Type::LiteralString, .value = "ab"},
				{.type = Token::Type::LiteralString, .value = "c"},
				endToken
			}
		));
	};
	"comment in string literal"_test = [] {
		expect(s_test.Success("comment in string literal",
			"'<< abc >>'",
			{
				{.type = Token::Type::LiteralString, .value = "<< abc >>" },
				endToken
			}
		));
	};
	"non-closed string literal"_test = [] {
		expect(s_test.Failure("non-closed string literal",
			"'",
			{
				ErrorToken(LexerError::StringLiteralOpen),
				endToken
			},
			{
				{.error = LexerError::StringLiteralOpen }
			}
		));
	};
	"non-closed string literal on line 1"_test = [] {
		expect(s_test.Failure("non-closed string literal on line 1",
			"'abc",
			{
				ErrorToken(LexerError::StringLiteralOpen),
				endToken
			},
			{
				{.error = LexerError::StringLiteralOpen, .line = 1}
			}
		));
	};
	"non-closed string literal on line 2 CRLF"_test = [] {
		expect(s_test.Failure("non-closed string literal on line 2 CRLF",
			"\r\n'abc",
			{
				ErrorToken(LexerError::StringLiteralOpen),
				endToken
			},
			{
				{.error = LexerError::StringLiteralOpen, .line = 2}
			}
		));
	};
	"non-closed string literal on line 2 LF"_test = [] {
		expect(s_test.Failure("non-closed string literal on line 2 LF", "\n'abc",
			{
				ErrorToken(LexerError::StringLiteralOpen),
				endToken
			},
			{
				{.error = LexerError::StringLiteralOpen, .line = 2}
			}
		));
	};
	"non-closed string literal on line 1 CR"_test = [] {
		expect(s_test.Failure("non-closed string literal on line 1 CR",
			"\r'abc",
			{
				ErrorToken(LexerError::StringLiteralOpen),
				endToken
			},
			{
				{.error = LexerError::StringLiteralOpen, .line = 1}
			}
		));
	};
	"whitespace outside and within string literal"_test = [] {
		expect(s_test.Success("whitespace outside and within string literal",
			"\t'ab\r\n\tc'\r\n",
			{
				{.type = Token::Type::LiteralString, .value = "ab\r\n\tc" },
				endToken
			}
		));
	};
};

suite lexer_string_field_tests = [] {
	"escaped curly bracket"_test = [] {
		expect(s_test.Success("escaped curly bracket",
			"'{{'",
			{
				{.type = Token::Type::LiteralString, .value = "{" },
				endToken
			}
		));
	};
	"escaped curly bracket"_test = [] {
		expect(s_test.Success("escaped curly bracket",
			"'{{}'",
			{
				{.type = Token::Type::LiteralString, .value = "{}" },
				endToken
			}
		));
	};
	"single closing curly bracket"_test = [] {
		expect(s_test.Success("single closing curly bracket",
			"'}'",
			{
				{.type = Token::Type::LiteralString, .value = "}" },
				endToken
			}
		));
	};
	"string field"_test = [] {
		expect(s_test.Success("string field",
			"'{n}'",
			{
				{.type = Token::Type::LiteralString, .value = "\n" },
				endToken
			}
		));
	};
	"string fields"_test = [] {
		expect(s_test.Success("string fields",
			"'{t}{n}{r}'",
			{
				{.type = Token::Type::LiteralString, .value = "\t\n\r" },
				endToken
			}
		));
	};
	"embedded string fields"_test = [] {
		expect(s_test.Success("embedded string fields",
			"'abc{r}{n}def'",
			{
				{.type = Token::Type::LiteralString, .value = "abc\r\ndef" },
				endToken
			}
		));
	};
	"non-closed string field"_test = [] {
		expect(s_test.Failure("non-closed string field",
			"'{n'",
			{
				ErrorToken(LexerError::StringFieldIllformed),
				{.type = Token::Type::LiteralString},
				endToken
			},
			{
				{.error = LexerError::StringFieldIllformed}
			}
		));
	};
	"ill-formed string field"_test = [] {
		expect(s_test.Failure("ill-formed string field",
			"'{nn}'",
			{
				ErrorToken(LexerError::StringFieldIllformed),
				{.type = Token::Type::LiteralString, .value = "n}"},
				endToken
			},
			{
				{.error = LexerError::StringFieldIllformed}
			}
		));
	};
	"unknown string field"_test = [] {
		expect(s_test.Failure("unknown string field",
			"'{m}'",
			{
				ErrorToken(LexerError::StringFieldUnknown),
				{.type = Token::Type::LiteralString, .value = "m}"},
				endToken
			},
			{
				{.error = LexerError::StringFieldUnknown}
			}
		));
	};
};

suite lexer_string_interpolation_tests = [] {
	"unindexed string interpolation"_test = [] {
		expect(s_test.Success("unindexed string interpolation",
			"'{}'",
			{
				{.type = Token::Type::LiteralString, .value = "" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "1" },
				{.type = Token::Type::LiteralString, .value = "" },
				endToken
			}
		));
	};
	"embedded unindexed string interpolation"_test = [] {
		expect(s_test.Success("embedded unindexed string interpolation",
			"'abc{}'",
			{
				{.type = Token::Type::LiteralString, .value = "abc" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "1" },
				{.type = Token::Type::LiteralString, .value = "" },
				endToken
			}
		));
	};
	"unindexed string interpolations"_test = [] {
		expect(s_test.Success("unindexed string interpolations",
			"'abc{}{}'",
			{
				{.type = Token::Type::LiteralString, .value = "abc" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "1" },
				{.type = Token::Type::LiteralString, .value = "" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "2" },
				{.type = Token::Type::LiteralString, .value = "" },
				endToken
			}
		));
	};
	"indexed string interpolation"_test = [] {
		expect(s_test.Success("indexed string interpolation",
			"'{2}'",
			{
				{.type = Token::Type::LiteralString, .value = "" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "2" },
				{.type = Token::Type::LiteralString, .value = "" },
				endToken
			}
		));
	};
	"indexed string interpolations"_test = [] {
		expect(s_test.Success("indexed string interpolations",
			"'abc{4}{1}'",
			{
				{.type = Token::Type::LiteralString, .value = "abc" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "4" },
				{.type = Token::Type::LiteralString, .value = "" },
				{.type = Token::Type::LiteralStringInterpolation, .value = "1" },
				{.type = Token::Type::LiteralString, .value = "" },
				endToken
			}
		));
	};
};

suite lexer_function_declarations_tests = [] {
	"trivial function declaration"_test = [] {
		expect(s_test.Success("trivial function declaration",
			"func",
			{
				{.type = idType, .value = "func"},
				endToken
			}
		));
	};
	"trivial integer function"_test = [] {
		expect(s_test.Success("trivial integer function",
			"func: 123.",
			{
				{.type = idType, .value = "func"},
				{.type = Token::Type::SeparatorColon, .value = ":"},
				{.type = Token::Type::LiteralInteger, .value = "123"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"trivial decimal function with whitespace"_test = [] {
		expect(s_test.Success("trivial decimal function with whitespace",
			"func : 123.4.",
			{
				{.type = idType, .value = "func"},
				{.type = Token::Type::SeparatorColon, .value = ":"},
				{.type = Token::Type::LiteralDecimal, .value = "123.4"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"trivial decimal function with mixed name and group"_test = [] {
		expect(s_test.Success("trivial decimal function with mixed name and group",
			"\r\nfunc44: (123.4).",
			{
				{.type = idType, .value = "func44"},
				{.type = Token::Type::SeparatorColon, .value = ":"},
				{.type = Token::Type::ParenRoundOpen, .value = "("},
				{.type = Token::Type::LiteralDecimal, .value = "123.4"},
				{.type = Token::Type::ParenRoundClose, .value = ")"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"imported function"_test = [] {
		expect(s_test.Success("imported function",
			"-> func",
			{
				{.type = Token::Type::OperatorArrow, .value = "->"},
				{.type = idType, .value = "func"},
				endToken
			}
		));
	};
	"exported function"_test = [] {
		expect(s_test.Success("exported function",
			"<- []func",
			{
				{.type = Token::Type::OperatorArrow, .value = "<-"},
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				{.type = idType, .value = "func"},
				endToken
			}
		));
	};
	"function with namespace"_test = [] {
		expect(s_test.Success("function with namespace",
			"namespace/func",
			{
				{.type = idType, .value = "namespace"},
				{.type = Token::Type::SeparatorSlash, .value = "/"},
				{.type = idType, .value = "func"},
				endToken
			}
		));
	};
};

suite lexer_comment_tests = [] {
	"mixed character identifier"_test = [] {
		expect(s_test.Success("mixed character identifier",
			"ident123.",
			{
				{.type = idType, .value = "ident123"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"commented out identifier"_test = [] {
		expect(s_test.Success("commented out identifier",
			"<< ident123. >>",
			{
				endToken
			}
		));
	};
	"commented out identifier and whitespace"_test = [] {
		expect(s_test.Success("commented out identifier and whitespace",
			"<<\r\nident123.\r\n>>",
			{
				endToken
			}
		));
	};
	"commented and non-commented identifier"_test = [] {
		expect(s_test.Success("commented and non-commented identifier",
			"<< ident123. >> ident456.",
			{
				{.type = idType, .value = "ident456"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"nested comments"_test = [] {
		expect(s_test.Success("nested comments",
			"<<<< 123 << 456 >>>>.>> ident456.",
			{
				{.type = idType, .value = "ident456"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"multiple comments"_test = [] {
		expect(s_test.Success("multiple comments",
			"<<<<123>>ident234<<<<123<<456>>>:>>.",
			{
				{.type = idType, .value = "ident234"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"non-closed comment"_test = [] {
		expect(s_test.Failure("non-closed comment",
			"<<<<123>>ident234<<<<123<<456>>>:>.",
			{
				{.type = idType, .value = "ident234"},
				ErrorToken(LexerError::CommentOpen),
				endToken
			},
			{
				{.error = LexerError::CommentOpen, .line = 1}
			}
		));
	};
};

suite lexer_comparison_tests = [] {
	"comparison operator"_test = [] {
		expect(s_test.Success("comparison operator",
			"1 < 2",
			{
				{.type = Token::Type::LiteralInteger, .value = "1"},
				{.type = Token::Type::OperatorComparison, .value = "<"},
				{.type = Token::Type::LiteralInteger, .value = "2"},
				endToken
			}
		));
	};
	"comparison declaration"_test = [] {
		expect(s_test.Success("comparison declaration",
			"<(operand)",
			{
				{.type = Token::Type::OperatorComparison, .value = "<"},
				{.type = Token::Type::ParenRoundOpen, .value = "("},
				{.type = idType, .value = "operand"},
				{.type = Token::Type::ParenRoundClose, .value = ")"},
				endToken
			}
		));
	};
};

suite lexer_primitive_types_tests = [] {
	"primitive type, int"_test = [] {
		expect(s_test.Success("primitive type, int",
			"[1000000]",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "1000000"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				endToken
			}
		));
	};
	"primitive type, negative int"_test = [] {
		expect(s_test.Success("primitive type, negative int",
			"[-1000000]",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "-1000000"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				endToken
			}
		));
	};
	"primitive type, double"_test = [] {
		expect(s_test.Success("primitive type, double",
			"[1.0e300]",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralDecimal, .value = "1.0e300"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				endToken
			}
		));
	};
	"primitive type, int8 array"_test = [] {
		expect(s_test.Success("primitive type, int8 array",
			"[100]#",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "100"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				{.type = Token::Type::SeparatorHash, .value = "#"},
				endToken
			}
		));
	};
	"primitive type, int8 array of arrays"_test = [] {
		expect(s_test.Success("primitive type, int8 array of arrays",
			"[100]##",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "100"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				{.type = Token::Type::SeparatorHash, .value = "#"},
				{.type = Token::Type::SeparatorHash, .value = "#"},
				endToken
			}
		));
	};
	"primitive type, int32 array size 8"_test = [] {
		expect(s_test.Success("primitive type, int32 array size 8",
			"[100000]#8",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::LiteralInteger, .value = "100000"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				{.type = Token::Type::SeparatorHash, .value = "#"},
				{.type = Token::Type::LiteralInteger, .value = "8"},
				endToken
			}
		));
	};
};

suite lexer_builtin_types_tests = [] {
	"built-in type, /type/i32"_test = [] {
		expect(s_test.Success("built-in type, /type/i32",
			"[/type/i32]",
			{
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::SeparatorSlash, .value = "/"},
				{.type = Token::Type::Identifier, .value = "type"},
				{.type = Token::Type::SeparatorSlash, .value = "/"},
				{.type = Token::Type::Identifier, .value = "i32"},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				endToken
			}
		));
	};
};
