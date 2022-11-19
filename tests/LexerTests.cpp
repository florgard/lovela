#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

class LexerTest : public TestingBase
{
public:
	static bool Success(const char* name, std::string_view code, const std::vector<Token>& expectedTokens)
	{
		return Failure(name, code, expectedTokens, {});
	}

	static bool Failure(const char* name, std::string_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors);
};

bool LexerTest::Failure(const char* name, std::string_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors)
{
	std::istringstream input(std::string(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto tokens = to_vector(lexer->Lex());

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
			std::cerr << "ERROR: Lexer test \"" << name << "\" error: Token " << i + 1 << " is " << to_string(actual.type) << " \"" << actual.value
				<< "\", expected " << to_string(expected.type) << " \"" << expected.value << "\".\n";
		}
	}

	auto& errors = lexer->GetErrors();

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
			std::cerr << GetIncorrectErrorCodeMessage("Lexer", name, i, actual.code, expected.code) << GetErrorMessage(actual);
		}
		else if (expected.token.line && actual.token.line != expected.token.line)
		{
			success = false;
			std::cerr << GetIncorrectErrorLineMessage("Lexer", name, i, actual.token.line, expected.token.line) << GetErrorMessage(actual);
		}
	}

	return success;
}

static const Token endToken{ .type = Token::Type::End };
static constexpr auto ident = Token::Type::Identifier;

suite lexer_rudimental_tests = [] {
	"empty expression"_test = [] {
		expect(LexerTest::Success("empty expression",
			"",
			{
				endToken
			}
		));
	};
	"single character"_test = [] {
		expect(LexerTest::Success("single character",
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
		expect(LexerTest::Success("simple identifier",
			"abc",
			{
				{.type = ident, .value = "abc" },
				endToken
			}
		));
	};
	"two identifiers"_test = [] {
		expect(LexerTest::Success("two identifiers",
			"abc def",
			{
				{.type = ident, .value = "abc" },
				{.type = ident, .value = "def" },
				endToken
			}
		));
	};
	"alphanumerical identifier"_test = [] {
		expect(LexerTest::Success("alphanumerical identifier",
			"abc123",
			{
				{.type = ident, .value = "abc123" },
				endToken
			}
		));
	};
	"kebab case identifier"_test = [] {
		expect(LexerTest::Success("kebab case identifier",
			"abc-123",
			{
				{.type = ident, .value = "abc-123" },
				endToken
			}
		));
	};
	"snake case identifier"_test = [] {
		expect(LexerTest::Success("snake case identifier",
			"abc_123",
			{
				{.type = ident, .value = "abc_123" },
				endToken
			}
		));
	};
	"operator character identifier"_test = [] {
		expect(LexerTest::Success("operator character identifier",
			"abc>=123",
			{
				{.type = ident, .value = "abc>=123" },
				endToken
			}
		));
	};
	"Unicode identifier"_test = [] {
		expect(LexerTest::Success("Unicode identifier",
			"\xE6\x97\xA5\xE6\x9C\xAC", // Nihon in nihongo, U+65E5 U+672C
			{
				{.type = ident, .value = "\xE6\x97\xA5\xE6\x9C\xAC" },
				endToken
			}
		));
	};
	"Unicode combining mark identifier"_test = [] {
		expect(LexerTest::Success("Unicode combining mark identifier",
			"a\xCC\x80", // a with combining mark, U+0300
			{
				{.type = ident, .value = "a\xCC\x80" },
				endToken
			}
		));
	};
	"invalid identifier 1"_test = [] {
		expect(LexerTest::Failure("invalid identifier 1",
			"1abc",
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
			"=abc",
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
			"123",
			{
				{.type = Token::Type::LiteralInteger, .value = "123" },
				endToken
			}
		));
	};
	"integer literal and full stop"_test = [] {
		expect(LexerTest::Success("integer literal and full stop",
			"123.",
			{
				{.type = Token::Type::LiteralInteger, .value = "123"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"decimal literal"_test = [] {
		expect(LexerTest::Success("decimal literal",
			"123.456",
			{
				{.type = Token::Type::LiteralDecimal, .value = "123.456" },
				endToken
			}
		));
	};
	"decimal literal and full stop"_test = [] {
		expect(LexerTest::Success("decimal literal and full stop",
			"123.456.",
			{
				{.type = Token::Type::LiteralDecimal, .value = "123.456"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"decimal literal, full stop, digit"_test = [] {
		expect(LexerTest::Success("decimal literal, full stop, digit",
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
		expect(LexerTest::Success("empty string literal",
			"''",
			{
				{.type = Token::Type::LiteralString, .value = "" },
				endToken
			}
		));
	};
	"single escaped quotation mark"_test = [] {
		expect(LexerTest::Success("single escaped quotation mark",
			"''''",
			{
				{.type = Token::Type::LiteralString, .value = "'" },
				endToken
			}
		));
	};
	"simple string literal"_test = [] {
		expect(LexerTest::Success("simple string literal",
			"'abc'",
			{
				{.type = Token::Type::LiteralString, .value = "abc" },
				endToken
			}
		));
	};
	"string literal with whitespace"_test = [] {
		expect(LexerTest::Success("string literal with whitespace",
			"'ab c'",
			{
				{.type = Token::Type::LiteralString, .value = "ab c" },
				endToken
			}
		));
	};
	"string literal with escaped quotation mark"_test = [] {
		expect(LexerTest::Success("string literal with escaped quotation mark",
			"'ab''c'",
			{
				{.type = Token::Type::LiteralString, .value = "ab'c" },
				endToken
			}
		));
	};
	"separated string literals"_test = [] {
		expect(LexerTest::Success("separated string literals",
			"'ab' 'c'",
			{
				{.type = Token::Type::LiteralString, .value = "ab"},
				{.type = Token::Type::LiteralString, .value = "c"},
				endToken
			}
		));
	};
	"comment in string literal"_test = [] {
		expect(LexerTest::Success("comment in string literal",
			"'<< abc >>'",
			{
				{.type = Token::Type::LiteralString, .value = "<< abc >>" },
				endToken
			}
		));
	};
	"non-closed string literal"_test = [] {
		expect(LexerTest::Failure("non-closed string literal",
			"'",
			{
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringLiteralOpen }
			}
		));
	};
	"non-closed string literal on line 1"_test = [] {
		expect(LexerTest::Failure("non-closed string literal on line 1",
			"'abc",
			{
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 1} }
			}
		));
	};
	"non-closed string literal on line 2"_test = [] {
		expect(LexerTest::Failure("non-closed string literal on line 2",
			"\r\n'abc",
			{
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 2} }
			}
		));
	};
	"non-closed string literal on line 2"_test = [] {
		expect(LexerTest::Failure("non-closed string literal on line 2", "\n'abc",
			{
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 2} }
			}
		));
	};
	"non-closed string literal on line 1"_test = [] {
		expect(LexerTest::Failure("non-closed string literal on line 1",
			"\r'abc",
			{
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 1} }
			}
		));
	};
	"whitespace outside and within string literal"_test = [] {
		expect(LexerTest::Success("whitespace outside and within string literal",
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
		expect(LexerTest::Success("escaped curly bracket",
			"'{{'",
			{
				{.type = Token::Type::LiteralString, .value = "{" },
				endToken
			}
		));
	};
	"escaped curly bracket"_test = [] {
		expect(LexerTest::Success("escaped curly bracket",
			"'{{}'",
			{
				{.type = Token::Type::LiteralString, .value = "{}" },
				endToken
			}
		));
	};
	"single closing curly bracket"_test = [] {
		expect(LexerTest::Success("single closing curly bracket",
			"'}'",
			{
				{.type = Token::Type::LiteralString, .value = "}" },
				endToken
			}
		));
	};
	"string field"_test = [] {
		expect(LexerTest::Success("string field",
			"'{n}'",
			{
				{.type = Token::Type::LiteralString, .value = "\n" },
				endToken
			}
		));
	};
	"string fields"_test = [] {
		expect(LexerTest::Success("string fields",
			"'{t}{n}{r}'",
			{
				{.type = Token::Type::LiteralString, .value = "\t\n\r" },
				endToken
			}
		));
	};
	"embedded string fields"_test = [] {
		expect(LexerTest::Success("embedded string fields",
			"'abc{r}{n}def'",
			{
				{.type = Token::Type::LiteralString, .value = "abc\r\ndef" },
				endToken
			}
		));
	};
	"non-closed string field"_test = [] {
		expect(LexerTest::Failure("non-closed string field",
			"'{n'",
			{
				{.type = Token::Type::LiteralString},
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringFieldIllformed}
			}
		));
	};
	"ill-formed string field"_test = [] {
		expect(LexerTest::Failure("ill-formed string field",
			"'{nn}'",
			{
				{.type = Token::Type::LiteralString, .value = "n}"},
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringFieldIllformed}
			}
		));
	};
	"unknown string field"_test = [] {
		expect(LexerTest::Failure("unknown string field",
			"'{m}'",
			{
				{.type = Token::Type::LiteralString, .value = "m}"},
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringFieldUnknown}
			}
		));
	};
};

suite lexer_string_interpolation_tests = [] {
	"unindexed string interpolation"_test = [] {
		expect(LexerTest::Success("unindexed string interpolation",
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
		expect(LexerTest::Success("embedded unindexed string interpolation",
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
		expect(LexerTest::Success("unindexed string interpolations",
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
		expect(LexerTest::Success("indexed string interpolation",
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
		expect(LexerTest::Success("indexed string interpolations",
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
		expect(LexerTest::Success("trivial function declaration",
			"func",
			{
				{.type = ident, .value = "func"},
				endToken
			}
		));
	};
	"trivial integer function"_test = [] {
		expect(LexerTest::Success("trivial integer function",
			"func: 123.",
			{
				{.type = ident, .value = "func"},
				{.type = Token::Type::SeparatorColon, .value = ":"},
				{.type = Token::Type::LiteralInteger, .value = "123"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"trivial decimal function with whitespace"_test = [] {
		expect(LexerTest::Success("trivial decimal function with whitespace",
			"func : 123.4.",
			{
				{.type = ident, .value = "func"},
				{.type = Token::Type::SeparatorColon, .value = ":"},
				{.type = Token::Type::LiteralDecimal, .value = "123.4"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"trivial decimal function with mixed name and group"_test = [] {
		expect(LexerTest::Success("trivial decimal function with mixed name and group",
			"\r\nfunc44: (123.4).",
			{
				{.type = ident, .value = "func44"},
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
		expect(LexerTest::Success("imported function",
			"-> func",
			{
				{.type = Token::Type::OperatorArrow, .value = "->"},
				{.type = ident, .value = "func"},
				endToken
			}
		));
	};
	"exported function"_test = [] {
		expect(LexerTest::Success("exported function",
			"<- []func",
			{
				{.type = Token::Type::OperatorArrow, .value = "<-"},
				{.type = Token::Type::ParenSquareOpen, .value = "["},
				{.type = Token::Type::ParenSquareClose, .value = "]"},
				{.type = ident, .value = "func"},
				endToken
			}
		));
	};
	"function with namespace"_test = [] {
		expect(LexerTest::Success("function with namespace",
			"namespace|func",
			{
				{.type = ident, .value = "namespace"},
				{.type = Token::Type::SeparatorVerticalLine, .value = "|"},
				{.type = ident, .value = "func"},
				endToken
			}
		));
	};
};

suite lexer_comment_tests = [] {
	"mixed character identifier"_test = [] {
		expect(LexerTest::Success("mixed character identifier",
			"ident123.",
			{
				{.type = ident, .value = "ident123"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"commented out identifier"_test = [] {
		expect(LexerTest::Success("commented out identifier",
			"<< ident123. >>",
			{
				endToken
			}
		));
	};
	"commented out identifier and whitespace"_test = [] {
		expect(LexerTest::Success("commented out identifier and whitespace",
			"<<\r\nident123.\r\n>>",
			{
				endToken
			}
		));
	};
	"commented and non-commented identifier"_test = [] {
		expect(LexerTest::Success("commented and non-commented identifier",
			"<< ident123. >> ident456.",
			{
				{.type = ident, .value = "ident456"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"nested comments"_test = [] {
		expect(LexerTest::Success("nested comments",
			"<<<< 123 << 456 >>>>.>> ident456.",
			{
				{.type = ident, .value = "ident456"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"multiple comments"_test = [] {
		expect(LexerTest::Success("multiple comments",
			"<<<<123>>ident234<<<<123<<456>>>:>>.",
			{
				{.type = ident, .value = "ident234"},
				{.type = Token::Type::SeparatorDot, .value = "."},
				endToken
			}
		));
	};
	"non-closed comment"_test = [] {
		expect(LexerTest::Failure("non-closed comment",
			"<<<<123>>ident234<<<<123<<456>>>:>.",
			{
				{.type = ident, .value = "ident234"},
				endToken
			},
			{
				{.code = ILexer::Error::Code::CommentOpen, .token{.line = 1}}
			}
		));
	};
};

suite lexer_comparison_tests = [] {
	"comparison operator"_test = [] {
		expect(LexerTest::Success("comparison operator",
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
		expect(LexerTest::Success("comparison declaration",
			"<(operand)",
			{
				{.type = Token::Type::OperatorComparison, .value = "<"},
				{.type = Token::Type::ParenRoundOpen, .value = "("},
				{.type = ident, .value = "operand"},
				{.type = Token::Type::ParenRoundClose, .value = ")"},
				endToken
			}
		));
	};
};

suite lexer_primitive_types_tests = [] {
	"primitive type, int32"_test = [] {
		expect(LexerTest::Success("primitive type, int32",
			"#32",
			{
				{.type = Token::Type::PrimitiveType, .value = "#32"},
				endToken
			}
		));
	};
	"primitive type, uint32"_test = [] {
		expect(LexerTest::Success("primitive type, uint32",
			"#+32",
			{
				{.type = Token::Type::PrimitiveType, .value = "#+32"},
				endToken
			}
		));
	};
	"primitive type, int8"_test = [] {
		expect(LexerTest::Success("primitive type, int8",
			"#8",
			{
				{.type = Token::Type::PrimitiveType, .value = "#8"},
				endToken
			}
		));
	};
	"primitive type, double"_test = [] {
		expect(LexerTest::Success("primitive type, double",
			"#.64",
			{
				{.type = Token::Type::PrimitiveType, .value = "#.64"},
				endToken
			}
		));
	};
	"primitive type, int8 array"_test = [] {
		expect(LexerTest::Success("primitive type, int8 array",
			"#8#",
			{
				{.type = Token::Type::PrimitiveType, .value = "#8#"},
				endToken
			}
		));
	};
	"primitive type, int8 array of arrays"_test = [] {
		expect(LexerTest::Success("primitive type, int8 array of arrays",
			"#8##",
			{
				{.type = Token::Type::PrimitiveType, .value = "#8##"},
				endToken
			}
		));
	};
	"primitive type, int32 array size 8"_test = [] {
		expect(LexerTest::Success("primitive type, int32 array size 8",
			"#32#8",
			{
				{.type = Token::Type::PrimitiveType, .value = "#32#8"},
				endToken
			}
		));
	};
};
