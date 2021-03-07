#include "pch.h"
#include "Lexer.h"
#include "Parser.h"
#include "Testing.h"

void Testing::RunTests()
{
	RunTypeTests();
	RunLexerTests();
	RunParserTests();
}

void Testing::RunTypeTests()
{
	assert(LexerBase::GetTokenType('(') == Token::Type::ParenRoundOpen);
	assert(LexerBase::GetTokenType('.') == Token::Type::SeparatorDot);
	assert(LexerBase::GetTokenType(' ') == Token::Type::Empty);

	assert((Token{} == Token{}));
	assert((Token{ Token::Type::Identifier } != Token{}));
	assert((Token{ {}, L"a" } != Token{}));
	assert((Token{ Token::Type::Identifier, L"a" } == Token{ Token::Type::Identifier, L"a" }));
	assert((Token{ Token::Type::Identifier, L"a" } != Token{ Token::Type::Identifier, L"b" }));
	assert((Token{ Token::Type::Identifier, L"a" } != Token{ Token::Type::LiteralString, L"a" }));
	assert((Token{} == false));
	assert((Token{ {}, L"a" } == false));
	assert((Token{ Token::Type::Identifier } == true));

	static constexpr std::array<std::pair<int, double>, 2> values{ { {1, 1.1}, {2, 2.2} } };
	static constexpr auto map = StaticMap<int, double, values.size()>{{values}};
	static_assert(map.at(1) == 1.1);
	static_assert(map.at(2) == 2.2);
	static_assert(map.at_or(2, 1.1) == 2.2);
	static_assert(map.at_or(3, 1.1) == 1.1);
	try {
		// Must throw
		if (map.at(3) == 3.3) {}
		assert(false);
	}
	catch (...) {}
}

void Testing::RunLexerTests()
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
	TestLexer("non-closed string literal on line 1", L"'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 1} } });
	TestLexer("non-closed string literal on line 2", L"\r\n'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 2} } });
	TestLexer("non-closed string literal on line 2", L"\n'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 2} } });
	TestLexer("non-closed string literal on line 1", L"\r'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 1} } });
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
		{.type = Token::Type::SeparatorColon, .value = L":"},
		{.type = Token::Type::LiteralInteger, .value = L"123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("trivial decimal function with whitespace", L"func : 123.4.", {
		{.type = Token::Type::Identifier, .value = L"func"},
		{.type = Token::Type::SeparatorColon, .value = L":"},
		{.type = Token::Type::LiteralDecimal, .value = L"123.4"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		}, {});
	TestLexer("trivial decimal function with mixed name and group", L"\r\nfunc44: (123.4).", {
		{.type = Token::Type::Identifier, .value = L"func44"},
		{.type = Token::Type::SeparatorColon, .value = L":"},
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
		}, { {.code = ILexer::Error::Code::CommentOpen, .token{.line = 1}} });
	TestLexer("comparison operator", L"1 < 2", {
		{.type = Token::Type::LiteralInteger, .value = L"1"},
		{.type = Token::Type::OperatorComparison, .value = L"<"},
		{.type = Token::Type::LiteralInteger, .value = L"2"},
		}, {});

	TestLexer("comparison declaration", L"<(operand)", {
		{.type = Token::Type::OperatorComparison, .value = L"<"},
		{.type = Token::Type::ParenRoundOpen, .value = L"("},
		{.type = Token::Type::Identifier, .value = L"operand"},
		{.type = Token::Type::ParenRoundClose, .value = L")"},
		}, {});
}

void Testing::RunParserTests()
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

	{
		const Node f{ .type = Node::Type::Function, .name = L"func", .objectType{.any = true}, .parameters{
				Parameter{.name = L"untyped", .type{.any = true}},
				Parameter{.name = L"name", .type{.name = L"type"}},
				Parameter{.type{.name = L"unnamed"}}
			} };
		TestParser("function with parameters", L"func(untyped, name [type], [unnamed])", Node{ .type{Node::Type::Root}, .children{f} }, {});
	}

	TestParser("function with type", L"func [type]", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .dataType{.name = L"type"}, .objectType{.any = true}}
		} }, {});

	{
		const Node f{ .type = Node::Type::Function, .name = L"func", .dataType{.name = L"functionType"}, .objectType{.name = L"objectType"}, .parameters{
				Parameter{.name = L"untyped", .type{.any = true}},
				Parameter{.name = L"name", .type{.name = L"type"}},
				Parameter{.type{.name = L"unnamed"}}
			} };
		TestParser("complete function declaration", L"[objectType] func (untyped, name [type], [unnamed]) [functionType]", Node{ .type{Node::Type::Root}, .children{f} }, {});
	}

	TestParser("imported function", L"-> func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .objectType{.any = true}, .imported = true}
		} }, {});
	TestParser("exported function", L"<- [] func", Node{ .type{Node::Type::Root}, .children{
		Node{.type = Node::Type::Function, .name = L"func", .objectType{.any = true}, .exported = true}
		} }, {});

	{
		const Node f{ .type = Node::Type::Function, .name = L"func", .nameSpace{ L"namespace" }, .objectType{.any = true} };
		TestParser("function with 1 namespace", L"namespace|func", Node{ .type{Node::Type::Root}, .children{f} }, {});
	}
	{
		const Node f{ .type = Node::Type::Function, .name = L"func", .nameSpace{ L"namespace1", L"namespaceN" }, .objectType{.any = true} };
		TestParser("function with 2 namespaces", L"namespace1|namespaceN|func", Node{ .type{Node::Type::Root}, .children{f} }, {});
	}
	{
		const Node f{ .type = Node::Type::Function, .name = L"<", .objectType{.any = true}, .parameters{
				Parameter{.name = L"operand", .type{.any = true}},
			} };
		TestParser("binary operator", L"<(operand)", Node{ .type{Node::Type::Root}, .children{f} }, {});
	}
	{
		const Node f{ .type = Node::Type::Function, .name = L"<", .nameSpace{ L"namespace" }, .objectType{.any = true}, .parameters{
				Parameter{.name = L"operand", .type{.any = true}},
			} };
		TestParser("binary operator with namespace", L"namespace|< (operand)", Node{ .type{Node::Type::Root}, .children{f} }, {});
	}
	{
		const Node f{ .type = Node::Type::Function, .name = L"<", .nameSpace{ L"namespace1" }, .objectType{.any = true} };
		const Node n{ .type = Node::Type::Function, .name = L"namespace2", .objectType{.any = true}, .parameters{
				Parameter{.name = L"operand", .type{.any = true}},
			} };
		TestParser("invalid binary operator as namespace", L"namespace1|<|namespace2 (operand)", Node{ .type{Node::Type::Root}, .children{f, n, } },
			{ IParser::Error{.code = IParser::Error::Code::ParseError } });
	}

	{
		const Node e{ .type = Node::Type::Expression };
		const Node s{ .type = Node::Type::Statement, .children{e} };
		const Node f{ .type = Node::Type::Function, .name = L"func", .objectType{.any = true}, .children{s} };
		const Node r{ .type = Node::Type::Root, .children{f} };
		TestParser("function with trivial body", L"func: body.", r, {});
	}

	{
		const Node e2{ .type = Node::Type::Expression };
		const Node s2{ .type = Node::Type::Statement, .children{e2} };
		const Node g{ .type = Node::Type::Group, .children{s2} };
		const Node e1{ .type = Node::Type::Expression, .children{g} };
		const Node s1{ .type = Node::Type::Statement, .children{e1} };
		const Node f{ .type = Node::Type::Function, .name = L"func", .objectType{.any = true}, .children{s1} };
		const Node r{ .type = Node::Type::Root, .children{f} };
		TestParser("function with body within group", L"func: (body).", r, {});
	}
}
