#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"

class ParserTest : public TestingBase
{
public:
	static bool Success(const char* name, std::wstring_view code, const Node& expectedTree)
	{
		// Success is failure with no errors.
		return Failure(name, code, expectedTree, {});
	}

	static bool Failure(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors)
	{
		return Failure(name, code, std::initializer_list<std::reference_wrapper<const Node>> { expectedTree }, expectedErrors);
	}

	static bool Success(const char* name, std::wstring_view code, const std::ranges::range auto& expectedRange)
	{
		// Success is failure with no errors.
		return Failure(name, code, expectedRange, {});
	}

	static bool Failure(const char* name, std::wstring_view code, const std::ranges::range auto& expectedRange, const std::vector<IParser::Error>& expectedErrors);
};

bool ParserTest::Failure(const char* name, std::wstring_view code, const std::ranges::range auto& expectedRange, const std::vector<IParser::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto nodes = to_vector(parser->Parse());

	bool success = nodes.begin() != nodes.end();

	if (!success)
	{
		std::wcerr << "ERROR: Parser test \"" << name << "\" error: The parser didn't yield an AST.\n\nExpected:\n";
		PrintAST(expectedRange);
	}
	else
	{
		int index = 0;
		success = TestAST(index, name, nodes, expectedRange);

		if (!success)
		{
			std::wcerr << "ERROR: Parser test \"" << name << "\" error: AST mismatch.\n\nActual:\n";
			PrintAST(nodes);
			std::wcerr << "\nExpected:\n";
			PrintAST(expectedRange);
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
			std::wcerr << GetIncorrectErrorCodeMessage("Parser", name, i, actual.code, expected.code) << GetErrorMessage(actual);
		}
		else if (expected.token.line && actual.token.line != expected.token.line)
		{
			success = false;
			std::wcerr << GetIncorrectErrorLineMessage("Parser", name, i, actual.token.line, expected.token.line) << GetErrorMessage(actual);
		}
	}

	return success;
}

suite parser_function_declaration_tests = [] {
	"trivial function declaration"_test = [] {
		expect(ParserTest::Success("trivial function declaration",
			L"func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func" }
		));
	};
	"function with any in type"_test = [] {
		expect(ParserTest::Success("function with any in type",
			L"[] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func" }
		));
	};
	"function with given in type"_test = [] {
		expect(ParserTest::Success("function with given in type",
			L"[type] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .inType{.name = L"type"} }
		));
	};
	"function with empty in type"_test = [] {
		expect(ParserTest::Success("function with empty in type",
			L"[()] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .inType = TypeSpec::NoneType() }
		));
	};
	"function with out type"_test = [] {
		expect(ParserTest::Success("function with out type",
			L"func [type]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .outType{.name = L"type"} }
		));
	};
	"function with in and out type"_test = [] {
		expect(ParserTest::Success("function with in and out type",
			L"[in] func [out]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .outType{.name = L"out"}, .inType{.name = L"in"} }
		));
	};
	"function with primitive types"_test = [] {
		expect(ParserTest::Success("function with primitive types",
			L"#8# func #32",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .outType{.name = L"#32"}, .inType{.name = L"#8#"} }
		));
	};
	"function with primitive types in brackets"_test = [] {
		expect(ParserTest::Success("function with primitive types in brackets",
			L"[#32] func [#8]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .outType{.name = L"#8"}, .inType{.name = L"#32"} }
		));
	};
	"anonymous function"_test = [] {
		expect(ParserTest::Success("anonymous function",
			L"[]()",
			Node{ .type = Node::Type::FunctionDeclaration }
		));
	};
	"2 function declarations"_test = [] {
		expect(ParserTest::Success("2 function declarations",
			L"func1\r\nfunc2",
			std::array<Node, 2> {
				Node{ .type = Node::Type::FunctionDeclaration, .value = L"func1" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = L"func2" }
			}
		));
	};
	"3 function declarations"_test = [] {
		expect(ParserTest::Success("3 function declarations",
			L"func1\r\nfunc2 func3",
			std::array<Node, 3> {
				Node{ .type = Node::Type::FunctionDeclaration, .value = L"func1" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = L"func2" },
				Node{ .type = Node::Type::FunctionDeclaration, .value = L"func3" }
			}
		));
	};
	"function with empty body"_test = [] {
		expect(ParserTest::Success("function with empty body",
			L"func:.",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func",
			.left = make<Node>::unique({.type = Node::Type::Empty})
			}
		));
	};
	"function with parameters"_test = [] {
		expect(ParserTest::Success("function with parameters",
			L"func(name_only, name [type], [type_only])",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			}}
		));
	};
	"complete function declaration"_test = [] {
		expect(ParserTest::Success("complete function declaration",
			L"[inType] func (name_only, name [type], [type_only]) [functionType]",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .outType{.name = L"functionType"}, .inType{.name = L"inType"}, .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			}}
		));
	};
};

suite parser_import_export_tests = [] {
	"imported function"_test = [] {
		expect(ParserTest::Success("imported function",
			L"-> func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Import }
		));
	};
	"exported function"_test = [] {
		expect(ParserTest::Success("exported function",
			L"<- [] func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Export }
		));
	};
	"imported C function"_test = [] {
		expect(ParserTest::Success("imported C function",
			L"-> 'C' func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Import | Api::C }
		));
	};
	"imported dynamically linked C function"_test = [] {
		expect(ParserTest::Success("imported dynamically linked C function",
			L"-> 'C Dynamic' func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Import | Api::C | Api::Dynamic }
		));
	};
	"imported standard C++ function"_test = [] {
		expect(ParserTest::Success("imported standard C++ function",
			L"-> 'Standard C++' func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .api = Api::Import | Api::Cpp | Api::Standard }
		));
	};
	"invalid import specifier"_test = [] {
		expect(ParserTest::Failure("invalid import specifier",
			L"-> 'Standard Something' func1 func2", // func2 added to have any output
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func2" },
			{ IParser::Error{.code = IParser::Error::Code::ParseError } }
		));
	};
};

suite parser_function_namespace_tests = [] {
	"function with 1 namespace"_test = [] {
		expect(ParserTest::Success("function with 1 namespace", 
			L"namespace|func", 
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .nameSpace{ L"namespace" } }));
	};
	"function with 2 namespaces"_test = [] {
		expect(ParserTest::Success("function with 2 namespaces",
			L"namespace1|namespaceN|func",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .nameSpace{ L"namespace1", L"namespaceN" } }));
	};
};

suite parser_binary_operator_tests = [] {
	"binary operator"_test = [] {
		expect(ParserTest::Success("binary operator",
			L"<(operand)",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"<",
			.parameters{make<VariableDeclaration>::shared({.name = L"operand"})}
			}
		));
	};
	"binary operator with namespace"_test = [] {
		expect(ParserTest::Success("binary operator with namespace",
			L"namespace|< (operand)",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"<", .nameSpace{ L"namespace" },
			.parameters{make<VariableDeclaration>::shared({.name = L"operand"})}
			}
		));
	};
	"invalid binary operator as namespace"_test = [] {
		expect(ParserTest::Failure("invalid binary operator as namespace",
			L"namespace1|<|namespace2 (operand)",
			std::array<Node, 2> {
				Node{ .type = Node::Type::FunctionDeclaration, .value = L"<", .nameSpace{ L"namespace1" } },
				Node{ .type = Node::Type::FunctionDeclaration, .value = L"namespace2", .parameters{make<VariableDeclaration>::shared({.name = L"operand"})} }
			},
			{ IParser::Error{.code = IParser::Error::Code::ParseError } }
		));
	};
};

suite parser_function_body_tests = [] {
	"function with trivial body"_test = [] {
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"body", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc) };
		expect(ParserTest::Success("function with trivial body",
			L"func: body.",
			fd));
	};

	"function with 2 chained calls"_test = [] {
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"inner", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"outer", .left = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc1) };
		expect(ParserTest::Success("function with 2 chained calls",
			L"func: inner outer.",
			fd));
	};

	"function with group"_test = [] {
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"body", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc) };
		expect(ParserTest::Success("function with group",
			L"func: (body).",
			fd));
		expect(ParserTest::Success("function with group 2",
			L"func: (body.).",
			fd));
			};

	"function with compound expression"_test = [] {
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(fc1) };
		expect(ParserTest::Success("function with compound expression",
			L"func: (expr1. expr2).",
			fd));
	};

	"function with tuple"_test = [] {
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(t) };
		expect(ParserTest::Success("function with tuple",
			L"func: (expr1, expr2).",
			fd));
	};

	"function with triple"_test = [] {
		auto fc3 = Node{ .type = Node::Type::FunctionCall, .value = L"expr3", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t2 = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc2), .right = make<Node>::unique(fc3) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t1 = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(t2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(t1) };
		expect(ParserTest::Success("function with triple",
			L"func: (expr1, expr2, expr3).",
			fd));
	};

	"function with tuple and compound expression"_test = [] {
		auto fc3 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2b", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fc2 = Node{ .type = Node::Type::FunctionCall, .value = L"expr2a", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}), .right = make<Node>::unique(fc3) };
		auto fc1 = Node{ .type = Node::Type::FunctionCall, .value = L"expr1", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto t = Node{ .type = Node::Type::Tuple, .left = make<Node>::unique(fc1), .right = make<Node>::unique(fc2) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(t) };
		expect(ParserTest::Success("function with tuple and compound expression",
			L"func: (expr1, expr2a. expr2b).",
			fd));
	};

	"function with parameters and body"_test = [] {
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"doWork", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			}, .left = make<Node>::unique(fc) };
		expect(ParserTest::Success("function with parameters and body",
			L"func(name_only, name [type], [type_only]): doWork.",
			fd));
	};

	"function without object but with parameters and body"_test = [] {
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"doWork", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .inType = TypeSpec::NoneType(), .parameters{
				make<VariableDeclaration>::shared({.name = L"name_only"}),
				make<VariableDeclaration>::shared({.name = L"name", .type{.name = L"type"}}),
				make<VariableDeclaration>::shared({.type{.name = L"type_only"}})
			}, .left = make<Node>::unique(fc) };
		expect(ParserTest::Success("function without object but with parameters and body",
			L"[()] func(name_only, name [type], [type_only]): doWork.",
			fd));
	};

	"binary operation with function call"_test = [] {
		auto l = Node{ .type = Node::Type::Literal, .value = L"1", .outType{.name = L"#32"} };
		auto fc = Node{ .type = Node::Type::FunctionCall, .value = L"call", .left = make<Node>::unique(Node{.type = Node::Type::ExpressionInput}) };
		auto bo = Node{ .type = Node::Type::BinaryOperation, .value = L"+", .left = make<Node>::unique(fc), .right = make<Node>::unique(l) };
		auto fd = Node{ .type = Node::Type::FunctionDeclaration, .value = L"func", .left = make<Node>::unique(bo) };
		expect(ParserTest::Success("binary operation with function call",
			L"func: call + 1.",
			fd));
	};
};
