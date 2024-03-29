#include "pch.h"
#include "TestingBase.h"

class ParserTest : public TestingBase
{
public:
	bool YieldsNodes(std::string_view name, std::string_view code, const Node& expectedTree)
	{
		return YieldsNodes(name, code, std::initializer_list<std::reference_wrapper<const Node>> { expectedTree });
	}

	bool YieldsNodes(std::string_view name, std::string_view code, const std::ranges::range auto& expectedRange);
};

static ParserTest parserTest;

bool ParserTest::YieldsNodes(std::string_view name, std::string_view code, const std::ranges::range auto& expectedRange)
{
	StringLexer lexer;
	std::vector<Token> tokens;
	VectorParser parser;
	std::vector<Node> nodes;
	code >> lexer >> tokens >> parser >> nodes;

	Token failingToken{};
	std::ostringstream s;
	const bool success = TestSyntaxTree(s, name, nodes, expectedRange, failingToken);
	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Parser test \"" << color.name << name << color.none << "\"\n"
			<< "Input code:\n";
		lexer.PrintErrorSourceCode(std::cerr, failingToken);
		std::cerr << s.str()
			<< "Tokens:\n" << color.actual;
		PrintTokens(tokens);
		std::cerr << color.none << '\n'
			<< "Actual syntax tree:\n" << color.actual;
		PrintSyntaxTree(nodes);
		std::cerr << color.none
			<< "Expected syntax tree:\n" << color.expect;
		PrintSyntaxTree(expectedRange);
		std::cerr << color.none << '\n';
	}

	return success;
}

static Node ExprInputNode()
{
	return { .type = Node::Type::ExpressionInput };
}

using namespace boost::ut;

suite parser_comments_tests = [] {
	"multiple comments"_test = [] {
		expect(parserTest.YieldsNodes("multiple comments",
			"<<<<123>>ident234<<<<345<<456>>>:>>.",
			std::array<Node, 5>
			{
				Node{ .type = Node::Type::Comment, .value = "123" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "ident234" },
				Node{ .type = Node::Type::Comment, .value = "345" },
				Node{ .type = Node::Type::Comment, .value = "456" },
				Node{ .type = Node::Type::Comment, .value = ":" },
			}
		));
	};
};

suite parser_lexer_error_tests = [] {
	"invalid identifier"_test = [] {
		expect(parserTest.YieldsNodes("invalid identifier",
			"1abc",
			std::array<Node, 3>
			{
				Node{ .type = Node::Type::Error, .error{.code = Node::Error::Code::ParseError } },
				Node{ .type = Node::Type::Error, .error{.code = Node::Error::Code::ParseError } },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "abc" },
			}
		));
	};
};

suite parser_function_declaration_input_types_tests = [] {
	"trivial function declaration"_test = [] {
		expect(parserTest.YieldsNodes("trivial function declaration",
			"func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func" }
		));
	};
	"function with any in type"_test = [] {
		expect(parserTest.YieldsNodes("function with any in type",
			"[] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func" }
		));
	};
	"function with given in type"_test = [] {
		expect(parserTest.YieldsNodes("function with given in type",
			"[type] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType{.kind = TypeSpec::Kind::Named, .name = "type"} }
		));
	};
	"function with empty in type"_test = [] {
		expect(parserTest.YieldsNodes("function with empty in type",
			"[()] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::None} }
		));
	};
	"function with literal integer in type"_test = [] {
		expect(parserTest.YieldsNodes("function with literal integer in type",
			"[1] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true}} }
		));
	};
	"function with literal decimal in type 1"_test = [] {
		expect(parserTest.YieldsNodes("function with literal decimal in type 1",
			"[8.388607] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .floatType = true}} }
		));
	};
	"function with literal decimal in type 2"_test = [] {
		expect(parserTest.YieldsNodes("function with literal decimal in type 2",
			"[8.388608] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .floatType = true}} }
		));
	};
	"function with literal decimal in type 3"_test = [] {
		expect(parserTest.YieldsNodes("function with literal decimal in type 3",
			"[838860.7] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .floatType = true}} }
		));
	};
	"function with literal decimal in type 4"_test = [] {
		expect(parserTest.YieldsNodes("function with literal decimal in type 4",
			"[838860.8] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .floatType = true}} }
		));
	};
	"function with literal decimal in type 5"_test = [] {
		expect(parserTest.YieldsNodes("function with literal decimal in type 5",
			"[-123.4567891e-6] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 64, .floatType = true}} }
		));
	};
	"function with tagged in type"_test = [] {
		expect(parserTest.YieldsNodes("function with tagged in type",
			"[#1] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Tagged, .name = "1"} }
		));
	};
	"function with built-in in type"_test = [] {
		expect(parserTest.YieldsNodes("function with built-in in type",
			"[/type/i32] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Primitive, .nameSpace{.parts{"type"}, .root = true}, .primitive{.bits = 32, .signedType = true}}}
		));
	};
};

suite parser_function_declaration_other_types_tests = [] {
	"function with out type"_test = [] {
		expect(parserTest.YieldsNodes("function with out type",
			"func [type]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .outType{.kind = TypeSpec::Kind::Named, .name = "type"} }
		));
	};
	"function with in and out type"_test = [] {
		expect(parserTest.YieldsNodes("function with in and out type",
			"[in] func [out]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .outType{.kind = TypeSpec::Kind::Named, .name = "out"}, .inType{.kind = TypeSpec::Kind::Named, .name = "in"} }
		));
	};
	"function with primitive types 1"_test = [] {
		expect(parserTest.YieldsNodes("function with primitive types 1",
			"[/type/i8]# func [/type/i32]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .outType{.kind = TypeSpec::Kind::Primitive, .nameSpace{.parts{"type"}, .root = true}, .primitive{.bits = 32, .signedType = true}}, .inType{.kind = TypeSpec::Kind::Primitive, .nameSpace{.parts{"type"}, .root = true}, .arrayDims{0}, .primitive{ .bits = 8, .signedType = true } }}
		));
	};
	"function with primitive types 2"_test = [] {
		expect(parserTest.YieldsNodes("function with primitive types 2",
			"[/type/i32] func [/type/i8]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .outType{.kind = TypeSpec::Kind::Primitive, .nameSpace{.parts{"type"}, .root = true}, .primitive{.bits = 8, .signedType = true }}, .inType{.kind = TypeSpec::Kind::Primitive, .nameSpace{.parts{"type"}, .root = true}, .primitive{ .bits = 32, .signedType = true }} }
		));
	};
	"anonymous function"_test = [] {
		expect(parserTest.YieldsNodes("anonymous function",
			"[]()",
			Node{ .type = Node::Type::FunctionDeclaration }
		));
	};
	"2 function declarations"_test = [] {
		expect(parserTest.YieldsNodes("2 function declarations",
			"func1\r\nfunc2",
			std::array<Node, 2> {
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func1" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func2" }
			}
		));
	};
	"3 function declarations"_test = [] {
		expect(parserTest.YieldsNodes("3 function declarations",
			"func1\r\nfunc2 func3",
			std::array<Node, 3> {
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func1" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func2" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func3" }
			}
		));
	};
	"function with empty body"_test = [] {
		expect(parserTest.YieldsNodes("function with empty body",
			"func:.",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func",
			.children{{.type = Node::Type::Empty}}
			}
		));
	};
	"function with parameters"_test = [] {
		expect(parserTest.YieldsNodes("function with parameters",
			"func(name_only, name [type], [type_only])",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .parameters{
				make<VariableDeclaration>::shared({.name = "name_only"}),
				make<VariableDeclaration>::shared({.name = "name", .type{.kind = TypeSpec::Kind::Named, .name = "type"}}),
				make<VariableDeclaration>::shared({.type{.kind = TypeSpec::Kind::Named, .name = "type_only"}})
			}}
		));
	};
	"complete function declaration"_test = [] {
		expect(parserTest.YieldsNodes("complete function declaration",
			"[inType] func (name_only, name [type], [type_only]) [functionType]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .outType{.kind = TypeSpec::Kind::Named, .name = "functionType"}, .inType{.kind = TypeSpec::Kind::Named, .name = "inType"}, .parameters{
				make<VariableDeclaration>::shared({.name = "name_only"}),
				make<VariableDeclaration>::shared({.name = "name", .type{.kind = TypeSpec::Kind::Named, .name = "type"}}),
				make<VariableDeclaration>::shared({.type{.kind = TypeSpec::Kind::Named, .name = "type_only"}})
			}}
		));
	};
};

suite parser_import_export_tests = [] {
	"imported function"_test = [] {
		expect(parserTest.YieldsNodes("imported function",
			"-> func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .apiSpec{ .flags = ApiSpec::Import } }
		));
	};
	"exported function"_test = [] {
		expect(parserTest.YieldsNodes("exported function",
			"<- [] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .apiSpec{.flags = ApiSpec::Export } }
		));
	};
	"imported C function"_test = [] {
		expect(parserTest.YieldsNodes("imported C function",
			"-> 'C' func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .apiSpec{.flags = ApiSpec::Import | ApiSpec::C } }
		));
	};
	"imported dynamically linked C function"_test = [] {
		expect(parserTest.YieldsNodes("imported dynamically linked C function",
			"-> 'C Dynamic' func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .apiSpec{.flags = ApiSpec::Import | ApiSpec::C | ApiSpec::Dynamic } }
		));
	};
	"imported standard C++ function"_test = [] {
		expect(parserTest.YieldsNodes("imported standard C++ function",
			"-> 'Standard C++' func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .apiSpec{.flags = ApiSpec::Import | ApiSpec::Cpp | ApiSpec::Standard } }
		));
	};
	"invalid import specifier"_test = [] {
		expect(parserTest.YieldsNodes("invalid import specifier",
			"-> 'Standard Something' func1 func2", // func2 added to have any output
			std::array<Node, 2>
			{
				Node{ .type = Node::Type::Error, .error{.code = Node::Error::Code::ParseError } },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func2" },
			}
		));
	};
};

suite parser_function_namespace_tests = [] {
	"function with 1 namespace"_test = [] {
		expect(parserTest.YieldsNodes("function with 1 namespace", 
			"namespace/func", 
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .nameSpace{.parts{"namespace"}} }));
	};
	"function with 2 namespaces"_test = [] {
		expect(parserTest.YieldsNodes("function with 2 namespaces",
			"namespace1/namespaceN/func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .nameSpace{.parts{"namespace1", "namespaceN"}} }));
	};
};

suite parser_binary_operator_tests = [] {
	"binary operator"_test = [] {
		expect(parserTest.YieldsNodes("binary operator",
			"<(operand)",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "<",
			.parameters{make<VariableDeclaration>::shared({.name = "operand"})}
			}
		));
	};
	"binary operator with namespace"_test = [] {
		expect(parserTest.YieldsNodes("binary operator with namespace",
			"namespace/< (operand)",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "<", .nameSpace{.parts{"namespace"}},
			.parameters{make<VariableDeclaration>::shared({.name = "operand"})}
			}
		));
	};
	"invalid binary operator as namespace"_test = [] {
		expect(parserTest.YieldsNodes("invalid binary operator as namespace",
			"namespace1/</namespace2 (operand)",
			std::array<Node, 3> {
				Node{ .type = Node::Type::FunctionDeclaration, .value = "<", .nameSpace{.parts{"namespace1"}} },
				Node{ .type = Node::Type::Error, .error{.code = Node::Error::Code::ParseError } },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "namespace2", .parameters{make<VariableDeclaration>::shared({.name = "operand"})} }
			}
		));
	};
};

suite parser_function_body_tests = [] {
	"function with trivial body"_test = [] {
		Node fc{ .type = Node::Type::FunctionCall, .value = "body", .children{ExprInputNode()} };
		Node e{ .type = Node::Type::Expression, .children{fc} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{e} };
		expect(parserTest.YieldsNodes("function with trivial body",
			"func: body.",
			fd));
	};

	"function with 2 chained calls"_test = [] {
		Node fc1{ .type = Node::Type::FunctionCall, .value = "inner", .children{ExprInputNode()} };
		Node fc2{ .type = Node::Type::FunctionCall, .value = "outer", .children{ExprInputNode()} };
		Node e{ .type = Node::Type::Expression, .children{fc1, fc2} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{e} };
		expect(parserTest.YieldsNodes("function with 2 chained calls",
			"func: inner outer.",
			fd));
	};

	"function with group"_test = [] {
		Node fc{ .type = Node::Type::FunctionCall, .value = "body", .children{ExprInputNode()} };
		Node e{ .type = Node::Type::Expression, .children{fc} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{e} };
		expect(parserTest.YieldsNodes("function with group",
			"func: (body).",
			fd));
		expect(parserTest.YieldsNodes("function with group 2",
			"func: (body.).",
			fd));
			};

	"function with compound expression"_test = [] {
		Node fc1{ .type = Node::Type::FunctionCall, .value = "expr1", .children{ExprInputNode()} };
		Node e1{ .type = Node::Type::Expression, .children{fc1} };
		Node fc2{ .type = Node::Type::FunctionCall, .value = "expr2", .children{ExprInputNode()} };
		Node e2{ .type = Node::Type::Expression, .children{fc2} };
		Node el{ .type = Node::Type::ExpressionList, .children{e1, e2} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{el} };
		expect(parserTest.YieldsNodes("function with compound expression",
			"func: (expr1. expr2).",
			fd));
	};

	"function with tuple"_test = [] {
		Node fc1{ .type = Node::Type::FunctionCall, .value = "expr1", .children{ExprInputNode()} };
		Node e1{ .type = Node::Type::Expression, .children{fc1} };
		Node fc2{ .type = Node::Type::FunctionCall, .value = "expr2", .children{ExprInputNode()} };
		Node e2{ .type = Node::Type::Expression, .children{fc2} };
		Node t{ .type = Node::Type::Tuple, .children{e1, e2} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{t} };
		expect(parserTest.YieldsNodes("function with tuple",
			"func: (expr1, expr2).",
			fd));
	};

	"function with triple"_test = [] {
		Node fc1{ .type = Node::Type::FunctionCall, .value = "expr1", .children{ExprInputNode()} };
		Node e1{ .type = Node::Type::Expression, .children{fc1} };
		Node fc2{ .type = Node::Type::FunctionCall, .value = "expr2", .children{ExprInputNode()} };
		Node e2{ .type = Node::Type::Expression, .children{fc2} };
		Node fc3{ .type = Node::Type::FunctionCall, .value = "expr3", .children{ExprInputNode()} };
		Node e3{ .type = Node::Type::Expression, .children{fc3} };
		Node t{ .type = Node::Type::Tuple, .children{e1, e2, e3} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{t} };
		expect(parserTest.YieldsNodes("function with triple",
			"func: (expr1, expr2, expr3).",
			fd));
	};

	"function with tuple and compound expression"_test = [] {
		Node fc1{ .type = Node::Type::FunctionCall, .value = "expr1", .children{ExprInputNode()} };
		Node e1{ .type = Node::Type::Expression, .children{fc1} };
		Node fc2{ .type = Node::Type::FunctionCall, .value = "expr2a", .children{ExprInputNode()} };
		Node e2{ .type = Node::Type::Expression, .children{fc2} };
		Node fc3{ .type = Node::Type::FunctionCall, .value = "expr2b", .children{ExprInputNode()} };
		Node e3{ .type = Node::Type::Expression, .children{fc3} };
		Node el{ .type = Node::Type::ExpressionList, .children{e2, e3} };
		Node t{ .type = Node::Type::Tuple, .children{e1, el} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{t} };
		expect(parserTest.YieldsNodes("function with tuple and compound expression",
			"func: (expr1, expr2a. expr2b).",
			fd));
	};

	"function with empty parameters list"_test = [] {
		Node fc{ .type = Node::Type::FunctionCall, .value = "doWork", .children{ExprInputNode()} };
		Node e{ .type = Node::Type::Expression, .children{fc} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{e} };
		expect(parserTest.YieldsNodes("function with empty parameters list",
			"func(): doWork.",
			fd));
	};

	"function without object"_test = [] {
		Node fc{ .type = Node::Type::FunctionCall, .value = "doWork", .children{ExprInputNode()} };
		Node e{ .type = Node::Type::Expression, .children{fc} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType = {.kind = TypeSpec::Kind::None}, .children{e} };
		expect(parserTest.YieldsNodes("function without object",
			"[()] func: doWork.",
			fd));
	};

	"function with parameters and body"_test = [] {
		Node fc{ .type = Node::Type::FunctionCall, .value = "doWork", .children{ExprInputNode()} };
		Node e{ .type = Node::Type::Expression, .children{fc} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .parameters{
				make<VariableDeclaration>::shared({.name = "name_only"}),
				make<VariableDeclaration>::shared({.name = "name", .type{.kind = TypeSpec::Kind::Named, .name = "type"}}),
				make<VariableDeclaration>::shared({.type{.kind = TypeSpec::Kind::Named, .name = "type_only"}})
			}, .children{e} };
		expect(parserTest.YieldsNodes("function with parameters and body",
			"func(name_only, name [type], [type_only]): doWork.",
			fd));
	};

	"function without object but with parameters and body"_test = [] {
		Node fc{ .type = Node::Type::FunctionCall, .value = "doWork", .children{ExprInputNode()} };
		Node e{ .type = Node::Type::Expression, .children{fc} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType = {.kind = TypeSpec::Kind::None}, .parameters{
				make<VariableDeclaration>::shared({.name = "name_only"}),
				make<VariableDeclaration>::shared({.name = "name", .type{.kind = TypeSpec::Kind::Named, .name = "type"}}),
				make<VariableDeclaration>::shared({.type{.kind = TypeSpec::Kind::Named, .name = "type_only"}})
			}, .children{e} };
		expect(parserTest.YieldsNodes("function without object but with parameters and body",
			"[()] func(name_only, name [type], [type_only]): doWork.",
			fd));
	};

	"binary operation with function call"_test = [] {
		Node fc{ .type = Node::Type::FunctionCall, .value = "call", .children{ExprInputNode()} };
		Node l{ .type = Node::Type::Literal, .value = "1", .outType{.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true }} };
		Node bo{ .type = Node::Type::BinaryOperation, .value = "+", .children{ExprInputNode(), l} };
		Node e{ .type = Node::Type::Expression, .children{fc, bo} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{e} };
		expect(parserTest.YieldsNodes("binary operation with function call",
			"func: call + 1.",
			fd));
	};

	"increment function"_test = [] {
		Node l{ .type = Node::Type::Literal, .value = "1", .outType{.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true }} };
		Node bo{ .type = Node::Type::BinaryOperation, .value = "+", .children{ExprInputNode(), l} };
		Node e{ .type = Node::Type::Expression, .children{bo} };
		Node fd{ .type = Node::Type::FunctionDeclaration, .value = "func", .children{e} };
		expect(parserTest.YieldsNodes("increment function",
			"func: + 1.",
			fd));
	};
};

suite Parser_expression_tests = [] {
	"expression with input"_test = [] {
		Node f{ .type = Node::Type::FunctionDeclaration, .value = "func", .children
			{
				{.type = Node::Type::Expression, .children{
					{.type = Node::Type::FunctionCall, .value = "scale", .children{ExprInputNode()} },
					{.type = Node::Type::FunctionCall, .value = "rotate", .children{ExprInputNode()} },
					{.type = Node::Type::FunctionCall, .value = "translate", .children{ExprInputNode()} }
				} },
			} };
		expect(parserTest.YieldsNodes("expression with input",
			"func: (scale rotate translate).",
			f));
	};
};
