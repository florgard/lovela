#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

class ParserTest : public TestingBase
{
public:
	bool Success(const char* name, std::string_view code, const Node& expectedTree)
	{
		// Success is failure with no errors.
		return Failure(name, code, expectedTree, {});
	}

	bool Failure(const char* name, std::string_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors)
	{
		return Failure(name, code, std::initializer_list<std::reference_wrapper<const Node>> { expectedTree }, expectedErrors);
	}

	bool Success(const char* name, std::string_view code, const std::ranges::range auto& expectedRange)
	{
		// Success is failure with no errors.
		return Failure(name, code, expectedRange, {});
	}

	bool Failure(const char* name, std::string_view code, const std::ranges::range auto& expectedRange, const std::vector<IParser::Error>& expectedErrors);
};

static ParserTest s_test;

bool ParserTest::Failure(const char* name, std::string_view code, const std::ranges::range auto& expectedRange, const std::vector<IParser::Error>& expectedErrors)
{
	std::istringstream input(std::string(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto nodes = to_vector(parser->Parse());

	bool success = nodes.begin() != nodes.end(); 

	if (!success)
	{
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Parser test \"" << color.name << name << color.none << "\": "
			<< "The parser didn't yield an AST.\n"
			<< "Expected:\n" << color.expect;
		PrintAST(expectedRange);
		std::cerr << color.none << '\n';
	}
	else
	{
		int index = 0;
		success = TestAST(index, name, nodes, expectedRange);

		if (!success)
		{
			std::cerr << color.fail << "ERROR: " << color.none
				<< "Parser test \"" << color.name << name << color.none << "\": "
				<< "AST mismatch.\n"
				<< "Actual:\n" << color.actual;
			PrintAST(nodes);
			std::cerr <<  color.none << "Expected:\n" << color.expect;
			PrintAST(expectedRange);
			std::cerr << color.none << '\n';
		}
	}

	auto& errors = parser->GetErrors();
	const auto actualCount = errors.size();
	const auto expectedCount = expectedErrors.size();
	const auto count = std::max(actualCount, expectedCount);

	for (int i = 0; i < count; i++)
	{
		const auto actual = i < actualCount ? errors[i] : IParser::Error{};
		const auto expected = i < expectedCount ? expectedErrors[i] : IParser::Error{};
		if (actual.code != expected.code)
		{
			success = false;

			PrintIncorrectErrorCodeMessage(std::cerr, "Parser", name, i, actual.code, expected.code);
			PrintErrorMessage(std::cerr, actual);
		}
		else if (expected.token.line && actual.token.line != expected.token.line)
		{
			success = false;

			PrintIncorrectErrorLineMessage(std::cerr, "Parser", name, i, actual.token.line, expected.token.line);
			PrintErrorMessage(std::cerr, actual);
		}
	}

	return success;
}

suite parser_function_declaration_input_types_tests = [] {
	"trivial function declaration"_test = [] {
		expect(s_test.Success("trivial function declaration",
			"func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func" }
		));
	};
	"function with any in type"_test = [] {
		expect(s_test.Success("function with any in type",
			"[] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func" }
		));
	};
	"function with given in type"_test = [] {
		expect(s_test.Success("function with given in type",
			"[type] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType{.kind = TypeSpec::Kind::Named, .name = "type"} }
		));
	};
	"function with empty in type"_test = [] {
		expect(s_test.Success("function with empty in type",
			"[()] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::None} }
		));
	};
	"function with tagged in type"_test = [] {
		expect(s_test.Success("function with tagged in type",
			"[1] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Tagged, .name = "1"} }
		));
	};
	"function with built-in in type"_test = [] {
		expect(s_test.Success("function with built-in in type",
			"[/type/i32] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType {.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = true}} }
		));
	};
};

suite parser_function_declaration_other_types_tests = [] {
		"function with out type"_test = [] {
		expect(s_test.Success("function with out type",
			"func [type]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .outType{.kind = TypeSpec::Kind::Named, .name = "type"} }
		));
	};
	"function with in and out type"_test = [] {
		expect(s_test.Success("function with in and out type",
			"[in] func [out]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .outType{.kind = TypeSpec::Kind::Named, .name = "out"}, .inType{.kind = TypeSpec::Kind::Named, .name = "in"} }
		));
	};
	"function with primitive types"_test = [] {
		expect(s_test.Success("function with primitive types",
			"[/type/i8]# func [/type/i32]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .outType{.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 32, .signedType = true}}, .inType{.kind = TypeSpec::Kind::Primitive, .arrayDims{0}, .primitive{ .bits = 8, .signedType = true } }}
		));
	};
	"function with primitive types in brackets"_test = [] {
		expect(s_test.Success("function with primitive types in brackets",
			"[[/type/i32]] func [/type/i8]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .outType{.kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true }}, .inType{.kind = TypeSpec::Kind::Primitive, .primitive{ .bits = 32, .signedType = true }} }
		));
	};
	"anonymous function"_test = [] {
		expect(s_test.Success("anonymous function",
			"[]()",
			Node{ .type = Node::Type::FunctionDeclaration }
		));
	};
	"2 function declarations"_test = [] {
		expect(s_test.Success("2 function declarations",
			"func1\r\nfunc2",
			std::array<Node, 2> {
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func1" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func2" }
			}
		));
	};
	"3 function declarations"_test = [] {
		expect(s_test.Success("3 function declarations",
			"func1\r\nfunc2 func3",
			std::array<Node, 3> {
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func1" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func2" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "func3" }
			}
		));
	};
	"function with empty body"_test = [] {
		expect(s_test.Success("function with empty body",
			"func:.",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func",
			.left = make<Node>::unique({.type = Node::Type::Empty})
			}
		));
	};
	"function with parameters"_test = [] {
		expect(s_test.Success("function with parameters",
			"func(name_only, name [type], [type_only])",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .parameters{
				make<VariableDeclaration>::shared({.name = "name_only"}),
				make<VariableDeclaration>::shared({.name = "name", .type{.kind = TypeSpec::Kind::Named, .name = "type"}}),
				make<VariableDeclaration>::shared({.type{.kind = TypeSpec::Kind::Named, .name = "type_only"}})
			}}
		));
	};
	"complete function declaration"_test = [] {
		expect(s_test.Success("complete function declaration",
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
		expect(s_test.Success("imported function",
			"-> func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .api = ApiSpec::Import }
		));
	};
	"exported function"_test = [] {
		expect(s_test.Success("exported function",
			"<- [] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .api = ApiSpec::Export }
		));
	};
	"imported C function"_test = [] {
		expect(s_test.Success("imported C function",
			"-> 'C' func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .api = ApiSpec::Import | ApiSpec::C }
		));
	};
	"imported dynamically linked C function"_test = [] {
		expect(s_test.Success("imported dynamically linked C function",
			"-> 'C Dynamic' func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .api = ApiSpec::Import | ApiSpec::C | ApiSpec::Dynamic }
		));
	};
	"imported standard C++ function"_test = [] {
		expect(s_test.Success("imported standard C++ function",
			"-> 'Standard C++' func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .api = ApiSpec::Import | ApiSpec::Cpp | ApiSpec::Standard }
		));
	};
	"invalid import specifier"_test = [] {
		expect(s_test.Failure("invalid import specifier",
			"-> 'Standard Something' func1 func2", // func2 added to have any output
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func2" },
			{ IParser::Error{.code = IParser::Error::Code::ParseError } }
		));
	};
};

suite parser_function_namespace_tests = [] {
	"function with 1 namespace"_test = [] {
		expect(s_test.Success("function with 1 namespace", 
			"namespace|func", 
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .nameSpace{.parts{"namespace"}} }));
	};
	"function with 2 namespaces"_test = [] {
		expect(s_test.Success("function with 2 namespaces",
			"namespace1|namespaceN|func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .nameSpace{.parts{"namespace1", "namespaceN"}} }));
	};
};

suite parser_binary_operator_tests = [] {
	"binary operator"_test = [] {
		expect(s_test.Success("binary operator",
			"<(operand)",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "<",
			.parameters{make<VariableDeclaration>::shared({.name = "operand"})}
			}
		));
	};
	"binary operator with namespace"_test = [] {
		expect(s_test.Success("binary operator with namespace",
			"namespace|< (operand)",
			Node{ .type = Node::Type::FunctionDeclaration, .value = "<", .nameSpace{.parts{"namespace"}},
			.parameters{make<VariableDeclaration>::shared({.name = "operand"})}
			}
		));
	};
	"invalid binary operator as namespace"_test = [] {
		expect(s_test.Failure("invalid binary operator as namespace",
			"namespace1|<|namespace2 (operand)",
			std::array<Node, 2> {
				Node{ .type = Node::Type::FunctionDeclaration, .value = "<", .nameSpace{.parts{"namespace1"}} },
				Node{ .type = Node::Type::FunctionDeclaration, .value = "namespace2", .parameters{make<VariableDeclaration>::shared({.name = "operand"})} }
			},
			{ IParser::Error{.code = IParser::Error::Code::ParseError } }
		));
	};
};

suite parser_function_body_tests = [] {
	"function with trivial body"_test = [] {
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = "body", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .left = make<Node>::unique(fc) };
		expect(s_test.Success("function with trivial body",
			"func: body.",
			fd));
	};

	"function with 2 chained calls"_test = [] {
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = "inner", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = "outer", .left = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .left = make<Node>::unique(fc1) };
		expect(s_test.Success("function with 2 chained calls",
			"func: inner outer.",
			fd));
	};

	"function with group"_test = [] {
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = "body", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .left = make<Node>::unique(fc) };
		expect(s_test.Success("function with group",
			"func: (body).",
			fd));
		expect(s_test.Success("function with group 2",
			"func: (body.).",
			fd));
			};

	"function with compound expression"_test = [] {
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = "expr2", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = "expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .left = make<Node>::unique(fc1) };
		expect(s_test.Success("function with compound expression",
			"func: (expr1. expr2).",
			fd));
	};

	"function with tuple"_test = [] {
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = "expr2", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = "expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .left = make<Node>::unique(t) };
		expect(s_test.Success("function with tuple",
			"func: (expr1, expr2).",
			fd));
	};

	"function with triple"_test = [] {
		auto fc3 = Node{ .type = Node::Type::FunctionCall, .value = "expr3", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = "expr2", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t2 = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc2), .right = make<Node>::unique(fc3) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = "expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t1 = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(t2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .left = make<Node>::unique(t1) };
		expect(s_test.Success("function with triple",
			"func: (expr1, expr2, expr3).",
			fd));
	};

	"function with tuple and compound expression"_test = [] {
		auto fc3 = Node{ .type = Node::Type::FunctionCall, .value = "expr2b", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = "expr2a", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}), .right = make<Node>::unique(fc3) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = "expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .left = make<Node>::unique(t) };
		expect(s_test.Success("function with tuple and compound expression",
			"func: (expr1, expr2a. expr2b).",
			fd));
	};

	"function with parameters and body"_test = [] {
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = "doWork", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .parameters{
				make<VariableDeclaration>::shared({.name = "name_only"}),
				make<VariableDeclaration>::shared({.name = "name", .type{.kind = TypeSpec::Kind::Named, .name = "type"}}),
				make<VariableDeclaration>::shared({.type{.kind = TypeSpec::Kind::Named, .name = "type_only"}})
			}, .left = make<Node>::unique(fc) };
		expect(s_test.Success("function with parameters and body",
			"func(name_only, name [type], [type_only]): doWork.",
			fd));
	};

	"function without object but with parameters and body"_test = [] {
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = "doWork", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .inType = {.kind = TypeSpec::Kind::None}, .parameters{
				make<VariableDeclaration>::shared({.name = "name_only"}),
				make<VariableDeclaration>::shared({.name = "name", .type{.kind = TypeSpec::Kind::Named, .name = "type"}}),
				make<VariableDeclaration>::shared({.type{.kind = TypeSpec::Kind::Named, .name = "type_only"}})
			}, .left = make<Node>::unique(fc) };
		expect(s_test.Success("function without object but with parameters and body",
			"[()] func(name_only, name [type], [type_only]): doWork.",
			fd));
	};

	"binary operation with function call"_test = [] {
		auto l = Node{ .type = Node::Type::Literal, .value = "1", .outType{ .kind = TypeSpec::Kind::Primitive, .primitive{.bits = 8, .signedType = true }} };
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = "call", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto bo = Node{ .type = Node::Type::BinaryOperation, .value = "+", .left = make<Node>::unique(fc), .right = make<Node>::unique(l) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = "func", .left = make<Node>::unique(bo) };
		expect(s_test.Success("binary operation with function call",
			"func: call + 1.",
			fd));
	};
};
