#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"

class ParserTest : public TestingBase
{
public:
	static bool Success(const char* name, std::wstring_view code, const Node& expectedTree)
	{
		return Failure(name, code, expectedTree, {});
	}

	static bool Failure(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors);
};

bool ParserTest::Failure(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto tree = parser->Parse();

	bool success = !!tree;

	if (!success)
	{
		std::wcerr << "Parser test \"" << name << "\" error: The parser didn't yield an AST.\n\nExpected:\n";
		PrintTree(expectedTree);
	}
	else
	{
		int index = 0;
		success = TestAST(index, name, *tree, expectedTree);

		if (!success)
		{
			std::wcerr << "Parser test \"" << name << "\" error: AST mismatch.\n\nActual:\n";
			PrintTree(*tree);
			std::wcerr << "\nExpected:\n";
			PrintTree(expectedTree);
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
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func2",
			.right = make<Node>::unique({.type = Node::Type::FunctionDeclaration, .value = L"func1" })
			}
		));
	};
	"3 function declarations"_test = [] {
		expect(ParserTest::Success("3 function declarations",
			L"func1\r\nfunc2 func3",
			Node{ .type = Node::Type::FunctionDeclaration, .value = L"func3",
			.right = make<Node>::unique(
				Node{.type = Node::Type::FunctionDeclaration, .value = L"func2",
				.right = make<Node>::unique({.type = Node::Type::FunctionDeclaration, .value = L"func1" })
				})
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
};
