#include "pch.h"
#include "Lexer.h"
#include "Parser.h"
#include "CodeGenerator.h"
#include "Testing.h"

void Testing::RunTests()
{
	RunTypeTests();
	RunLexerTests();
	RunParserTests();
	RunCodeGeneratorTests();
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
	static const Token endToken{ .type = Token::Type::End };

	TestLexer("empty expression", L"", { endToken });
	TestLexer("single character", L".", { {.type = Token::Type::SeparatorDot, .value = L"." }, endToken });

	TestLexer("simple identifier", L"abc", { {.type = Token::Type::Identifier, .value = L"abc" }, endToken });
	TestLexer("Unicode identifier", L"\u65E5\u672C", { {.type = Token::Type::Identifier, .value = L"\u65E5\u672C" }, endToken });
	//TestLexer(L"\u0061\u0300", { { .type = Token::Type::Identifier, .value = L"\u0061\u0300" }, endToken });

	TestLexer("integer literal", L"123", { {.type = Token::Type::LiteralInteger, .value = L"123" }, endToken });
	TestLexer("integer literal and full stop", L"123.", {
		{.type = Token::Type::LiteralInteger, .value = L"123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
		});
	TestLexer("decimal literal", L"123.456", { {.type = Token::Type::LiteralDecimal, .value = L"123.456" }, endToken });
	TestLexer("decimal literal and full stop", L"123.456.", {
		{.type = Token::Type::LiteralDecimal, .value = L"123.456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
		});
	TestLexer("decimal literal, full stop, digit", L"123.456.7", {
		{.type = Token::Type::LiteralDecimal, .value = L"123.456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		{.type = Token::Type::LiteralInteger, .value = L"7"},
		endToken
		});

	TestLexer("empty string literal", L"''", { {.type = Token::Type::LiteralString, .value = L"" }, endToken });
	TestLexer("single escaped quotation mark", L"''''", { {.type = Token::Type::LiteralString, .value = L"'" }, endToken });
	TestLexer("simple string literal", L"'abc'", { {.type = Token::Type::LiteralString, .value = L"abc" }, endToken });
	TestLexer("string literal with whitespace", L"'ab c'", { {.type = Token::Type::LiteralString, .value = L"ab c" }, endToken });
	TestLexer("string literal with escaped quotation mark", L"'ab''c'", { {.type = Token::Type::LiteralString, .value = L"ab'c" }, endToken });
	TestLexer("separated string literals", L"'ab' 'c'", {
		{.type = Token::Type::LiteralString, .value = L"ab"},
		{.type = Token::Type::LiteralString, .value = L"c"},
		endToken
		});
	TestLexer("comment in string literal", L"'<< abc >>'", { {.type = Token::Type::LiteralString, .value = L"<< abc >>" }, endToken });
	TestLexer("non-closed string literal", L"'", {}, { {.code = ILexer::Error::Code::StringLiteralOpen } });
	TestLexer("non-closed string literal on line 1", L"'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 1} } });
	TestLexer("non-closed string literal on line 2", L"\r\n'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 2} } });
	TestLexer("non-closed string literal on line 2", L"\n'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 2} } });
	TestLexer("non-closed string literal on line 1", L"\r'abc", {}, { {.code = ILexer::Error::Code::StringLiteralOpen, .token{.line = 1} } });
	TestLexer("whitespace outside and within string literal", L"\t'ab\r\n\tc'\r\n", { {.type = Token::Type::LiteralString, .value = L"ab\r\n\tc" }, endToken });

	TestLexer("escaped curly bracket", L"'{{'", { {.type = Token::Type::LiteralString, .value = L"{" }, endToken });
	TestLexer("escaped curly bracket", L"'{{}'", { {.type = Token::Type::LiteralString, .value = L"{}" }, endToken });
	TestLexer("single closing curly bracket", L"'}'", { {.type = Token::Type::LiteralString, .value = L"}" }, endToken });
	TestLexer("string field", L"'{n}'", { {.type = Token::Type::LiteralString, .value = L"\n" }, endToken });
	TestLexer("string fields", L"'{t}{n}{r}'", { {.type = Token::Type::LiteralString, .value = L"\t\n\r" }, endToken });
	TestLexer("embedded string fields", L"'abc{r}{n}def'", { {.type = Token::Type::LiteralString, .value = L"abc\r\ndef" }, endToken });
	TestLexer("non-closed string field", L"'{n'", {}, { {.code = ILexer::Error::Code::StringFieldIllformed}, {.code = ILexer::Error::Code::StringLiteralOpen} });
	TestLexer("ill-formed string field", L"'{nn}'", { {.type = Token::Type::LiteralString, .value = L"}"}, endToken }, { {.code = ILexer::Error::Code::StringFieldIllformed} });
	TestLexer("unknown string field", L"'{m}'", { {.type = Token::Type::LiteralString, .value = L"m}"}, endToken }, { {.code = ILexer::Error::Code::StringFieldUnknown} });

	TestLexer("unindexed string interpolation", L"'{}'", {
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		endToken
		});
	TestLexer("embedded unindexed string interpolation", L"'abc{}'", {
		{.type = Token::Type::LiteralString, .value = L"abc" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		endToken
		});
	TestLexer("unindexed string interpolations", L"'abc{}{}'", {
		{.type = Token::Type::LiteralString, .value = L"abc" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"2" },
		{.type = Token::Type::LiteralString, .value = L"" },
		endToken
		});
	TestLexer("unindexed string interpolation", L"'{2}'", {
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"2" },
		{.type = Token::Type::LiteralString, .value = L"" },
		endToken
		});
	TestLexer("unindexed string interpolations", L"'abc{4}{1}'", {
		{.type = Token::Type::LiteralString, .value = L"abc" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"4" },
		{.type = Token::Type::LiteralString, .value = L"" },
		{.type = Token::Type::LiteralStringInterpolation, .value = L"1" },
		{.type = Token::Type::LiteralString, .value = L"" },
		endToken
		});

	TestLexer("trivial function declaration", L"func", {
		{.type = Token::Type::Identifier, .value = L"func"},
		endToken
		});
	TestLexer("trivial integer function", L"func: 123.", {
		{.type = Token::Type::Identifier, .value = L"func"},
		{.type = Token::Type::SeparatorColon, .value = L":"},
		{.type = Token::Type::LiteralInteger, .value = L"123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
		});
	TestLexer("trivial decimal function with whitespace", L"func : 123.4.", {
		{.type = Token::Type::Identifier, .value = L"func"},
		{.type = Token::Type::SeparatorColon, .value = L":"},
		{.type = Token::Type::LiteralDecimal, .value = L"123.4"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
		});
	TestLexer("trivial decimal function with mixed name and group", L"\r\nfunc44: (123.4).", {
		{.type = Token::Type::Identifier, .value = L"func44"},
		{.type = Token::Type::SeparatorColon, .value = L":"},
		{.type = Token::Type::ParenRoundOpen, .value = L"("},
		{.type = Token::Type::LiteralDecimal, .value = L"123.4"},
		{.type = Token::Type::ParenRoundClose, .value = L")"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
		});
	TestLexer("imported function", L"-> func", {
		{.type = Token::Type::OperatorArrow, .value = L"->"},
		{.type = Token::Type::Identifier, .value = L"func"},
		endToken
		});
	TestLexer("exported function", L"<- []func", {
		{.type = Token::Type::OperatorArrow, .value = L"<-"},
		{.type = Token::Type::ParenSquareOpen, .value = L"["},
		{.type = Token::Type::ParenSquareClose, .value = L"]"},
		{.type = Token::Type::Identifier, .value = L"func"},
		endToken
		});
	TestLexer("function with namespace", L"namespace|func", {
		{.type = Token::Type::Identifier, .value = L"namespace"},
		{.type = Token::Type::SeparatorVerticalLine, .value = L"|"},
		{.type = Token::Type::Identifier, .value = L"func"},
		endToken
		});

	TestLexer("mixed character identifier", L"ident123.", {
		{.type = Token::Type::Identifier, .value = L"ident123"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
		});
	TestLexer("commented out identifier", L"<< ident123. >>", { endToken });
	TestLexer("commented out identifier and whitespace", L"<<\r\nident123.\r\n>>", { endToken });
	TestLexer("commented and non-commented identifier", L"<< ident123. >> ident456.", {
		{.type = Token::Type::Identifier, .value = L"ident456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
		});
	TestLexer("nested comments", L"<<<< 123 << 456 >>>>.>> ident456.", {
		{.type = Token::Type::Identifier, .value = L"ident456"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
		});
	TestLexer("multiple comments", L"<<<<123>>ident234<<<<123<<456>>>:>>.", {
		{.type = Token::Type::Identifier, .value = L"ident234"},
		{.type = Token::Type::SeparatorDot, .value = L"."},
		endToken
		});
	TestLexer("non-closed comment", L"<<<<123>>ident234<<<<123<<456>>>:>.", {
		{.type = Token::Type::Identifier, .value = L"ident234"},
		}, { {.code = ILexer::Error::Code::CommentOpen, .token{.line = 1}} });
	TestLexer("comparison operator", L"1 < 2", {
		{.type = Token::Type::LiteralInteger, .value = L"1"},
		{.type = Token::Type::OperatorComparison, .value = L"<"},
		{.type = Token::Type::LiteralInteger, .value = L"2"},
		endToken
		});

	TestLexer("comparison declaration", L"<(operand)", {
		{.type = Token::Type::OperatorComparison, .value = L"<"},
		{.type = Token::Type::ParenRoundOpen, .value = L"("},
		{.type = Token::Type::Identifier, .value = L"operand"},
		{.type = Token::Type::ParenRoundClose, .value = L")"},
		endToken
		});
}

void Testing::RunParserTests()
{
	TestParser("trivial function declaration", L"func",
		Node{.type = Node::Type::FunctionDeclaration, .value = L"func"}
		);
	TestParser("function with any object type", L"[] func",
		Node{.type = Node::Type::FunctionDeclaration, .value = L"func"}
		);
	TestParser("function with given object type", L"[type] func",
		Node{.type = Node::Type::FunctionDeclaration, .value = L"func", .inType{.name = L"type"}}
		);
	TestParser("function with empty object type", L"[()] func",
		Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .inType = TypeSpec::NoneType() }
		);
	TestParser("anonymous function", L"[]()",
		Node{.type = Node::Type::FunctionDeclaration}
		);
	TestParser("2 function declarations", L"func1\r\nfunc2",
		Node{ .type = Node::Type::FunctionDeclaration, .value = L"func2", .right =
			make<Node>::unique({ .type = Node::Type::FunctionDeclaration, .value = L"func1" })
		});
	TestParser("3 function declarations", L"func1\r\nfunc2 func3",
		Node{ .type = Node::Type::FunctionDeclaration, .value = L"func3", .right =
			make<Node>::unique(Node{ .type = Node::Type::FunctionDeclaration, .value = L"func2", .right =
				make<Node>::unique({.type = Node::Type::FunctionDeclaration, .value = L"func1" })
			})
		});
	TestParser("function with empty body", L"func:.",
		Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left =
			make<Node>::unique({.type = Node::Type::Empty})
		});

	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			} };
		TestParser("function with parameters", L"func(name_only, name [type], [type_only])", f);
	}

	TestParser("function with type", L"func [type]",
		Node{.type = Node::Type::FunctionDeclaration, .value = L"func", .outType{.name = L"type"}}
		);

	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .outType{.name = L"functionType"}, .inType{.name = L"inType"}, .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			} };
		TestParser("complete function declaration", L"[inType] func (name_only, name [type], [type_only]) [functionType]", f);
	}

	TestParser("imported function", L"-> func",
		Node{.type = Node::Type::FunctionDeclaration, .value = L"func", .imported = true}
		);
	TestParser("exported function", L"<- [] func",
		Node{.type = Node::Type::FunctionDeclaration, .value = L"func", .exported = true}
		);

	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .nameSpace{ L"namespace" } };
		TestParser("function with 1 namespace", L"namespace|func", f);
	}
	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .nameSpace{ L"namespace1", L"namespaceN" } };
		TestParser("function with 2 namespaces", L"namespace1|namespaceN|func", f);
	}
	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"<", .parameters{
				make<VariableDeclaration>::shared({.name = L"operand"}),
			} };
		TestParser("binary operator", L"<(operand)", f);
	}
	{
		auto f = Node{ .type = Node::Type::FunctionDeclaration, .value = L"<", .nameSpace{ L"namespace" }, .parameters{
				make<VariableDeclaration>::shared({.name = L"operand"}),
			} };
		TestParser("binary operator with namespace", L"namespace|< (operand)", f);
	}
	{
		auto f2 = Node{ .type = Node::Type::FunctionDeclaration, .value = L"<", .nameSpace{ L"namespace1" } };
		auto f1 = Node{ .type = Node::Type::FunctionDeclaration, .value = L"namespace2", .parameters{
				make<VariableDeclaration>::shared({.name = L"operand"}),
			}, .right = make<Node>::unique(f2) };
		TestParser("invalid binary operator as namespace", L"namespace1|<|namespace2 (operand)", f1, { IParser::Error{.code = IParser::Error::Code::ParseError } });
	}

	{
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique({.type = Node::Type::FunctionCall, .value = L"body" }) };
		TestParser("function with trivial body", L"func: body.", fd);
	}

	{
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"inner", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"outer", .left = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc1) };
		TestParser("function with 2 chained calls", L"func: inner outer.", fd);
	}

	{
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique({.type = Node::Type::FunctionCall, .value = L"body" }) };
		TestParser("function with group", L"func: (body).", fd);
		TestParser("function with group 2", L"func: (body.).", fd);
	}

	{
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2" };
		auto fc1  = Node{ .type = Node::Type::FunctionCall, .value = L"expr1", .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc1) };
		TestParser("function with compound expression", L"func: (expr1. expr2).", fd);
	}

	{
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2" };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1" };
		auto t = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(t) };
		TestParser("function with tuple", L"func: (expr1, expr2).", fd);
	}

	{
		auto fc3 = Node{ .type = Node::Type::FunctionCall, .value = L"expr3" };
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2" };
		auto t2 = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc2), .right = make<Node>::unique(fc3) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1" };
		auto t1 = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(t2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(t1) };
		TestParser("function with triple", L"func: (expr1, expr2, expr3).", fd);
	}

	{
		auto fc3 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2b" };
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2a", .right = make<Node>::unique(fc3) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1" };
		auto t = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(t) };
		TestParser("function with tuple and compound expression", L"func: (expr1, expr2a. expr2b).", fd);
	}

	{
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"doWork" };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			}, .left = make<Node>::unique(fc) };
		TestParser("function with parameters and body", L"func(name_only, name [type], [type_only]): doWork.", fd);
	}

	{
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"doWork" };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .inType = TypeSpec::NoneType(), .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			}, .left = make<Node>::unique(fc) };
		TestParser("function without object but with parameters and body", L"[()] func(name_only, name [type], [type_only]): doWork.", fd);
	}

	{
		auto l = Node{ .type = Node::Type::Literal, .value = L"1", .outType{.name = L"32"} };
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"call", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto bo = Node{ .type = Node::Type::BinaryOperation, .value = L"+", .left = make<Node>::unique(fc), .right = make<Node>::unique(l) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(bo) };
		TestParser("binary operation with function call", L"func: call + 1.", fd);
	}
}

void Testing::RunCodeGeneratorTests()
{
	TestCodeGenerator("trivial function", L"func",
		L"template <typename Out, typename In> Out f_func(lovela::context& context, In in);");
	TestCodeGenerator("function with return type", L"func [type]",
		L"template <typename In> t_type f_func(lovela::context& context, In in);");
	TestCodeGenerator("function with object type", L"[type] func",
		L"template <typename Out> Out f_func(lovela::context& context, t_type in);");
	TestCodeGenerator("function with untyped parameter", L"func (arg)",
		L"template <typename Out, typename In, typename Param1> Out f_func(lovela::context& context, In in, Param1 p_arg);");
	TestCodeGenerator("function with typed parameter", L"func (arg [type])",
		L"template <typename Out, typename In> Out f_func(lovela::context& context, In in, t_type p_arg);");
	TestCodeGenerator("trivial function", L"func: + 1.",
		L"template <typename Out, typename In> Out f_func(lovela::context& context, In in) { context; auto& v1 = in; v1; const auto v2 = v1 + 1 ; return v2; }");
	TestCodeGenerator("function call", L"[#8] func [#8]: f(1, 'a', g).",
		LR"code(int8_t f_func(lovela::context& context, int8_t in) { context; auto& v1 = in; v1; const auto v2 = f_f( context, 1 , "a" , f_g( context) ) ; return v2; })code");
	TestCodeGenerator("exported function none -> none", L"<- [()] ex [()]:.",
		L"lovela::None f_ex(lovela::context& context, lovela::None in) { context; auto& v1 = in; v1; return {}; } void ex() { lovela::context context; lovela::None in; f_ex(context, in); }");
	TestCodeGenerator("exported function any -> any", L"<- ex: + 1.",
		L"template <typename Out, typename In> Out f_ex(lovela::context& context, In in) { context; auto& v1 = in; v1; const auto v2 = v1 + 1 ; return v2; } void* ex(void* in) { lovela::context context; return f_ex(context, in); }");

//	std::wstring code = LR"(
//[()] pi: 3.14.
//mul (factor): * factor.
//transform (mul, sub): (* mul. - sub).
//[](): pi transform (2, 0.28) + 1.
//)";

	std::wstring code = LR"(
<- [#32] ex [#32]: + 1.
: 1 ex.
)";
	std::wcout << code << '\n';

	std::wistringstream input(code);
	Lexer lexer(input);
	for (auto& error : lexer.GetErrors())
	{
		std::wcerr << error.message << '\n';
	}

	Parser parser(lexer.Lex());
	auto tree = parser.Parse();
	for (auto& error : parser.GetErrors())
	{
		std::wcerr << error.message << '\n';
	}

	PrintTree(*tree);
	std::wcout << '\n';

	std::wstringstream stream;

	CodeGenerator gen(stream);
	Parser::TraverseDepthFirstPostorder(*tree, [&](Node& node) { gen.Visit(node); });

	for (auto& error : gen.GetErrors())
	{
		std::wcerr << error << '\n';
	}

	auto genCode = stream.str();
	std::wcout << genCode;

	std::wofstream program(R"(..\targets\cpp\program\lovela-program.cpp)");
	CodeGenerator::BeginProgramSourceFile(program);
	program << genCode;
	CodeGenerator::EndProgramSourceFile(program);
	program.close();

	std::wofstream exports(R"(..\targets\cpp\program\lovela-exports.h)");
	gen.GenerateLibraryHeaderFile(exports);
	exports.close();
}
