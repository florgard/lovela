#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"

class LexerTest : public TestingBase
{
public:
	static bool Success(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens)
	{
		return Failure(name, code, expectedTokens, {});
	}

	static bool Failure(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors);
};

bool LexerTest::Failure(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto tokenGenerator = lexer->Lex();
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
			std::wcerr << "Lexer test \"" << name << "\" error: Token " << i + 1 << " is " << to_wstring(actual.type) << " \"" << actual.value
				<< "\", expected " << to_wstring(expected.type) << " \"" << expected.value << "\".\n";
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
			std::wcerr << GetIncorrectErrorCodeMessage("Lexer", name, i, actual.code, expected.code) << GetErrorMessage(actual);
		}
		else if (expected.token.line && actual.token.line != expected.token.line)
		{
			success = false;
			std::wcerr << GetIncorrectErrorLineMessage("Lexer", name, i, actual.token.line, expected.token.line) << GetErrorMessage(actual);
		}
	}

	return success;
}

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
			}
		));
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

suite lexer_string_literal_tests = [] {
	"empty string literal"_test = [] {
		expect(LexerTest::Success("empty string literal",
			L"''",
			{
				{.type = Token::Type::LiteralString, .value = L"" },
				endToken
			}
		));
	};
	"single escaped quotation mark"_test = [] {
		expect(LexerTest::Success("single escaped quotation mark",
			L"''''",
			{
				{.type = Token::Type::LiteralString, .value = L"'" },
				endToken
			}
		));
	};
	"simple string literal"_test = [] {
		expect(LexerTest::Success("simple string literal",
			L"'abc'",
			{
				{.type = Token::Type::LiteralString, .value = L"abc" },
				endToken
			}
		));
	};
	"string literal with whitespace"_test = [] {
		expect(LexerTest::Success("string literal with whitespace",
			L"'ab c'",
			{
				{.type = Token::Type::LiteralString, .value = L"ab c" },
				endToken
			}
		));
	};
	"string literal with escaped quotation mark"_test = [] {
		expect(LexerTest::Success("string literal with escaped quotation mark",
			L"'ab''c'",
			{
				{.type = Token::Type::LiteralString, .value = L"ab'c" },
				endToken
			}
		));
	};
	"separated string literals"_test = [] {
		expect(LexerTest::Success("separated string literals",
			L"'ab' 'c'",
			{
				{.type = Token::Type::LiteralString, .value = L"ab"},
				{.type = Token::Type::LiteralString, .value = L"c"},
				endToken
			}
		));
	};
	"comment in string literal"_test = [] {
		expect(LexerTest::Success("comment in string literal",
			L"'<< abc >>'",
			{
				{.type = Token::Type::LiteralString, .value = L"<< abc >>" },
				endToken
			}
		));
	};
	"non-closed string literal"_test = [] {
		expect(LexerTest::Failure("non-closed string literal",
			L"'",
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
			L"'abc",
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
			L"\r\n'abc",
			{
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 2} }
			}
		));
	};
	"non-closed string literal on line 2"_test = [] {
		expect(LexerTest::Failure("non-closed string literal on line 2", L"\n'abc",
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
			L"\r'abc",
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
			L"\t'ab\r\n\tc'\r\n",
			{
				{.type = Token::Type::LiteralString, .value = L"ab\r\n\tc" },
				endToken
			}
		));
	};
};

suite lexer_string_field_tests = [] {
	"escaped curly bracket"_test = [] {
		expect(LexerTest::Success("escaped curly bracket",
			L"'{{'",
			{
				{.type = Token::Type::LiteralString, .value = L"{" },
				endToken
			}
		));
	};
	"escaped curly bracket"_test = [] {
		expect(LexerTest::Success("escaped curly bracket",
			L"'{{}'",
			{
				{.type = Token::Type::LiteralString, .value = L"{}" },
				endToken
			}
		));
	};
	"single closing curly bracket"_test = [] {
		expect(LexerTest::Success("single closing curly bracket",
			L"'}'",
			{
				{.type = Token::Type::LiteralString, .value = L"}" },
				endToken
			}
		));
	};
	"string field"_test = [] {
		expect(LexerTest::Success("string field",
			L"'{n}'",
			{
				{.type = Token::Type::LiteralString, .value = L"\n" },
				endToken
			}
		));
	};
	"string fields"_test = [] {
		expect(LexerTest::Success("string fields",
			L"'{t}{n}{r}'",
			{
				{.type = Token::Type::LiteralString, .value = L"\t\n\r" },
				endToken
			}
		));
	};
	"embedded string fields"_test = [] {
		expect(LexerTest::Success("embedded string fields",
			L"'abc{r}{n}def'",
			{
				{.type = Token::Type::LiteralString, .value = L"abc\r\ndef" },
				endToken
			}
		));
	};
	"non-closed string field"_test = [] {
		expect(LexerTest::Failure("non-closed string field",
			L"'{n'",
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
			L"'{nn}'",
			{
				{.type = Token::Type::LiteralString, .value = L"n}"},
				endToken
			},
			{
				{.code = ILexer::Error::Code::StringFieldIllformed}
			}
		));
	};
	"unknown string field"_test = [] {
		expect(LexerTest::Failure("unknown string field",
			L"'{m}'",
			{
				{.type = Token::Type::LiteralString, .value = L"m}"},
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
			L"'{}'",
			{
				{.type = Token::Type::LiteralString, .value = L"" },
				{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
				{.type = Token::Type::LiteralString, .value = L"" },
				endToken
			}
		));
	};
	"embedded unindexed string interpolation"_test = [] {
		expect(LexerTest::Success("embedded unindexed string interpolation",
			L"'abc{}'",
			{
				{.type = Token::Type::LiteralString, .value = L"abc" },
				{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
				{.type = Token::Type::LiteralString, .value = L"" },
				endToken
			}
		));
	};
	"unindexed string interpolations"_test = [] {
		expect(LexerTest::Success("unindexed string interpolations",
			L"'abc{}{}'",
			{
				{.type = Token::Type::LiteralString, .value = L"abc" },
				{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
				{.type = Token::Type::LiteralString, .value = L"" },
				{.type = Token::Type::LiteralStringInterpolation, .value = L"2" },
				{.type = Token::Type::LiteralString, .value = L"" },
				endToken
			}
		));
	};
	"indexed string interpolation"_test = [] {
		expect(LexerTest::Success("indexed string interpolation",
			L"'{2}'",
			{
				{.type = Token::Type::LiteralString, .value = L"" },
				{.type = Token::Type::LiteralStringInterpolation, .value = L"2" },
				{.type = Token::Type::LiteralString, .value = L"" },
				endToken
			}
		));
	};
	"indexed string interpolations"_test = [] {
		expect(LexerTest::Success("indexed string interpolations",
			L"'abc{4}{1}'",
			{
				{.type = Token::Type::LiteralString, .value = L"abc" },
				{.type = Token::Type::LiteralStringInterpolation, .value = L"4" },
				{.type = Token::Type::LiteralString, .value = L"" },
				{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
				{.type = Token::Type::LiteralString, .value = L"" },
				endToken
			}
		));
	};
};

suite lexer_function_declarations_tests = [] {
	"trivial function declaration"_test = [] {
		expect(LexerTest::Success("trivial function declaration",
			L"func",
			{
				{.type = ident, .value = L"func"},
				endToken
			}
		));
	};
	"trivial integer function"_test = [] {
		expect(LexerTest::Success("trivial integer function",
			L"func: 123.",
			{
				{.type = ident, .value = L"func"},
				{.type = Token::Type::SeparatorColon, .value = L":"},
				{.type = Token::Type::LiteralInteger, .value = L"123"},
				{.type = Token::Type::SeparatorDot, .value = L"."},
				endToken
			}
		));
	};
	"trivial decimal function with whitespace"_test = [] {
		expect(LexerTest::Success("trivial decimal function with whitespace",
			L"func : 123.4.",
			{
				{.type = ident, .value = L"func"},
				{.type = Token::Type::SeparatorColon, .value = L":"},
				{.type = Token::Type::LiteralDecimal, .value = L"123.4"},
				{.type = Token::Type::SeparatorDot, .value = L"."},
				endToken
			}
		));
	};
	"trivial decimal function with mixed name and group"_test = [] {
		expect(LexerTest::Success("trivial decimal function with mixed name and group",
			L"\r\nfunc44: (123.4).",
			{
				{.type = ident, .value = L"func44"},
				{.type = Token::Type::SeparatorColon, .value = L":"},
				{.type = Token::Type::ParenRoundOpen, .value = L"("},
				{.type = Token::Type::LiteralDecimal, .value = L"123.4"},
				{.type = Token::Type::ParenRoundClose, .value = L")"},
				{.type = Token::Type::SeparatorDot, .value = L"."},
				endToken
			}
		));
	};
	"imported function"_test = [] {
		expect(LexerTest::Success("imported function",
			L"-> func",
			{
				{.type = Token::Type::OperatorArrow, .value = L"->"},
				{.type = ident, .value = L"func"},
				endToken
			}
		));
	};
	"exported function"_test = [] {
		expect(LexerTest::Success("exported function",
			L"<- []func",
			{
				{.type = Token::Type::OperatorArrow, .value = L"<-"},
				{.type = Token::Type::ParenSquareOpen, .value = L"["},
				{.type = Token::Type::ParenSquareClose, .value = L"]"},
				{.type = ident, .value = L"func"},
				endToken
			}
		));
	};
	"function with namespace"_test = [] {
		expect(LexerTest::Success("function with namespace",
			L"namespace|func",
			{
				{.type = ident, .value = L"namespace"},
				{.type = Token::Type::SeparatorVerticalLine, .value = L"|"},
				{.type = ident, .value = L"func"},
				endToken
			}
		));
	};
};

suite lexer_comment_tests = [] {
	"mixed character identifier"_test = [] {
		expect(LexerTest::Success("mixed character identifier",
			L"ident123.",
			{
		{.type = ident, .value = L"ident123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
			}
		));
	};
	"commented out identifier"_test = [] {
		expect(LexerTest::Success("commented out identifier",
			L"<< ident123. >>",
			{
				endToken
			}
		));
	};
	"commented out identifier and whitespace"_test = [] {
		expect(LexerTest::Success("commented out identifier and whitespace",
			L"<<\r\nident123.\r\n>>",
			{
				endToken
			}
		));
	};
	"commented and non-commented identifier"_test = [] {
		expect(LexerTest::Success("commented and non-commented identifier",
			L"<< ident123. >> ident456.",
			{
				{.type = ident, .value = L"ident456"},
				{.type = Token::Type::SeparatorDot, .value = L"."},
				endToken
			}
		));
	};
	"nested comments"_test = [] {
		expect(LexerTest::Success("nested comments",
			L"<<<< 123 << 456 >>>>.>> ident456.",
			{
				{.type = ident, .value = L"ident456"},
				{.type = Token::Type::SeparatorDot, .value = L"."},
				endToken
			}
		));
	};
	"multiple comments"_test = [] {
		expect(LexerTest::Success("multiple comments",
			L"<<<<123>>ident234<<<<123<<456>>>:>>.",
			{
				{.type = ident, .value = L"ident234"},
				{.type = Token::Type::SeparatorDot, .value = L"."},
				endToken
			}
		));
	};
	"non-closed comment"_test = [] {
		expect(LexerTest::Failure("non-closed comment",
			L"<<<<123>>ident234<<<<123<<456>>>:>.",
			{
				{.type = ident, .value = L"ident234"},
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
			L"1 < 2",
			{
				{.type = Token::Type::LiteralInteger, .value = L"1"},
				{.type = Token::Type::OperatorComparison, .value = L"<"},
				{.type = Token::Type::LiteralInteger, .value = L"2"},
				endToken
			}
		));
	};
	"comparison declaration"_test = [] {
		expect(LexerTest::Success("comparison declaration",
			L"<(operand)",
			{
				{.type = Token::Type::OperatorComparison, .value = L"<"},
				{.type = Token::Type::ParenRoundOpen, .value = L"("},
				{.type = ident, .value = L"operand"},
				{.type = Token::Type::ParenRoundClose, .value = L")"},
				endToken
			}
		));
	};
};

suite lexer_primitive_types_tests = [] {
	"primitive type, int32"_test = [] {
		expect(LexerTest::Success("primitive type, int32",
			L"#32",
			{
				{.type = Token::Type::PrimitiveType, .value = L"#32"},
				endToken
			}
		));
	};
	"primitive type, uint32"_test = [] {
		expect(LexerTest::Success("primitive type, uint32",
			L"#+32",
			{
				{.type = Token::Type::PrimitiveType, .value = L"#+32"},
				endToken
			}
		));
	};
	"primitive type, int8"_test = [] {
		expect(LexerTest::Success("primitive type, int8",
			L"#8",
			{
				{.type = Token::Type::PrimitiveType, .value = L"#8"},
				endToken
			}
		));
	};
	"primitive type, double"_test = [] {
		expect(LexerTest::Success("primitive type, double",
			L"#.64",
			{
				{.type = Token::Type::PrimitiveType, .value = L"#.64"},
				endToken
			}
		));
	};
	"primitive type, int8 array"_test = [] {
		expect(LexerTest::Success("primitive type, int8 array",
			L"#8#",
			{
				{.type = Token::Type::PrimitiveType, .value = L"#8#"},
				endToken
			}
		));
	};
	"primitive type, int8 array of arrays"_test = [] {
		expect(LexerTest::Success("primitive type, int8 array of arrays",
			L"#8##",
			{
				{.type = Token::Type::PrimitiveType, .value = L"#8##"},
				endToken
			}
		));
	};
	"primitive type, int32 array size 8"_test = [] {
		expect(LexerTest::Success("primitive type, int32 array size 8",
			L"#32#8",
			{
				{.type = Token::Type::PrimitiveType, .value = L"#32#8"},
				endToken
			}
		));
	};
};
