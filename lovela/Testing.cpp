#include "pch.h"
#include "Parser.h"
#include "Lexer.h"
#include "Testing.h"

struct MSVCBug
{
	enum class E { A, B, C, } e{};
	auto operator<=>(const MSVCBug& rhs) const noexcept = default;
	operator bool() const noexcept { return e != E::A; }
};

void Testing::TestToken()
{
	// When this breaks Token::operator bool() can be added again.
	assert(!(MSVCBug{ MSVCBug::E::B } != MSVCBug{ MSVCBug::E::C }));

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

	TestLexer("trivial function declaration", L"func", {
		{.type = Token::Type::Identifier, .value = L"func"},
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
	TestLexer("imported function", L"-> func", {
		{.type = Token::Type::OperatorArrow, .value = L"->"},
		{.type = Token::Type::Identifier, .value = L"func"},
		}, {});
	TestLexer("exported function", L"<- []func", {
		{.type = Token::Type::OperatorArrow, .value = L"<-"},
		{.type = Token::Type::ParenSquareOpen, .value = L"["},
		{.type = Token::Type::ParenSquareClose, .value = L"]"},
		{.type = Token::Type::Identifier, .value = L"func"},
		}, {});
	TestLexer("function with namespace", L"namespace|func", {
		{.type = Token::Type::Identifier, .value = L"namespace"},
		{.type = Token::Type::SeparatorVerticalLine, .value = L"|"},
		{.type = Token::Type::Identifier, .value = L"func"},
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
	TestParser("trivial function declaration", L"func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .objectType{.any = true}}
		} }, {});
	TestParser("function with any object type", L"[] func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .objectType{.any = true}}
		} }, {});
	TestParser("function with given object type", L"[type] func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .objectType{.name = L"type"}}
		} }, {});
	TestParser("function with empty object type", L"[()] func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .objectType{.none = true}}
		} }, {});
	TestParser("anonymous function", L"[]()", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .objectType{.any = true}}
		} }, {});
	TestParser("function with parameters", L"func(untyped, name [type], [unnamed])", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .objectType{.any = true}, .parameters{
			Parameter{.name = L"untyped", .type{.any = true}},
			Parameter{.name = L"name", .type{.name = L"type"}},
			Parameter{.type{.name = L"unnamed"}}
		} },
		} }, {});
	TestParser("function with type", L"func [type]", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .dataType{.name = L"type"}, .objectType{.any = true}}
		} }, {});
	TestParser("complete function declaration", L"[objectType] func (untyped, name [type], [unnamed]) [functionType]", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .dataType{.name = L"functionType"}, .objectType{.name = L"objectType"}, .parameters{
			Parameter{.name = L"untyped", .type{.any = true}},
			Parameter{.name = L"name", .type{.name = L"type"}},
			Parameter{.type{.name = L"unnamed"}}
		} },
		} }, {});
	TestParser("imported function", L"-> func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .objectType{.any = true}, .imported = true}
		} }, {});
	TestParser("exported function", L"<- [] func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .objectType{.any = true}, .exported = true}
		} }, {});

	TestParser("function with 1 namespace", L"namespace|func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .nameSpace{ L"namespace" }, .objectType{.any = true} }
		} }, {});
	TestParser("function with 2 namespaces", L"namespace1|namespaceN|func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .nameSpace{ L"namespace1", L"namespaceN" }, .objectType{.any = true} }
	} }, {});
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
			std::wcerr << "Test \"" << name << "\" error: Token " << i + 1 << " is " << ToWString(magic_enum::enum_name(actual.type)) << " \"" << actual.value
				<< "\", expected " << ToWString(magic_enum::enum_name(expected.type)) << " \"" << expected.value << "\".\n";
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
			std::wcerr << "Test \"" << name << "\" error: Error " << i + 1 << " code is " << ToWString(magic_enum::enum_name(actual.code))
				<< ", expected " << ToWString(magic_enum::enum_name(expected.code)) << ".\n"
				<< '(' << actual.line << ':' << actual.column << ") error " << ToWString(magic_enum::enum_name(actual.code)) << ": " << actual.message << '\n';
		}
		else if (expected.line && actual.line != expected.line)
		{
			success = false;
			std::wcerr << "Test \"" << name << "\" error: Error " << i + 1 << " line number is " << actual.line << ", expected " << expected.line << ".\n"
				<< '(' << actual.line << ':' << actual.column << ") error " << ToWString(magic_enum::enum_name(actual.code)) << ": " << actual.message << '\n';
		}
	}

	assert(success);
}

void Testing::TestParser(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors)
{
	expectedErrors; // TODO

	std::wistringstream input(std::wstring(code.data(), code.size()));
	Lexer lexer(input);
	Parser parser(lexer.Lex());
	auto tree = parser.Parse();

	int index = 0;
	bool success = TestAST(index, name, tree, expectedTree);
	success;
	assert(success);
}

bool Testing::TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree)
{
	if (tree != expectedTree)
	{
		const auto& actual = tree;
		const auto& expected = expectedTree;

		std::wcerr << "Test \"" << name << "\" error: Some property of token " << index + 1 << " of type " << ToWString(magic_enum::enum_name(actual.type))
			<< " differs from the expected token of type " << ToWString(magic_enum::enum_name(expected.type)) << ".\n";
		return false;
	}

	index++;

	auto actualCount = tree.children.size();
	auto expectedCount = expectedTree.children.size();
	auto count = std::max(actualCount, expectedCount);
	for (int i = 0; i < count; i++)
	{
		const auto& actual = i < actualCount ? tree.children[i] : Node{};
		const auto& expected = i < expectedCount ? expectedTree.children[i] : Node{};
		if (!TestAST(index, name, actual, expected))
		{
			return false;
		}
	}

	return true;
}
