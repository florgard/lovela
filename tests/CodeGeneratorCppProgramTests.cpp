#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/Parser.h"
#include "../lovela/CodeGeneratorFactory.h"

class CodeGeneratorCppTest : public TestingBase
{
public:
	void GenerateCode()
	{
		using namespace boost::ut;

		//	std::string code = R"(
		//[()] pi: 3.14.
		//mul (factor): * factor.
		//transform (mul, sub): (* mul. - sub).
		//[](): pi transform (2, 0.28) + 1.
		//)";

			// Internal error:
			// -> [#32] puts [#8#]. : 'Hello, Wordl!" puts.

		std::string code = R"(
-> 'Standard C' puts.
: 'Hello, World!' puts.
)";
		std::cout << "Input code:\n" << color.code << code << color.none << '\n';

		std::istringstream input(code);
		auto lexer = LexerFactory::Create(input);
		RangeParser parser;
		parser.Initialize(lexer->Lex());
		auto nodes = to_vector(parser.Parse());

		bool parseErrors = false;

		for (auto& errorNode : nodes | std::views::filter([](Node& n) { return n.type == Node::Type::Error; }))
		{
			parseErrors = true;
			std::cerr << errorNode.error.message << '\n';
		}

		expect(!parseErrors);

		std::cout << "AST:\n" << color.output;
		PrintAST(nodes);
		std::cout << color.none << '\n';

		std::stringstream stream;

		auto codeGen = CodeGeneratorFactory::Create(stream, "Cpp");
		Traverse<Node>::DepthFirstPostorder(nodes, [&](auto& node) { codeGen->Visit(node); });

		for (auto& error : codeGen->GetErrors())
		{
			std::cerr << error << '\n';
		}

		expect(codeGen->GetErrors().empty());

		auto genCode = stream.str();
		std::cout << "Generated code:\n" << color.output << genCode << color.none;

		std::ofstream program(R"(..\targets\cpp\program\lovela-program.cpp)");
		codeGen->GenerateProgramFile(program);
		program.close();

		std::ofstream imports(R"(..\targets\cpp\program\lovela-imports.h)");
		codeGen->GenerateImportsFile(imports);
		imports.close();

		std::ofstream exports(R"(..\targets\cpp\program\lovela-exports.h)");
		codeGen->GenerateExportsFile(exports);
		exports.close();
	}
};

static CodeGeneratorCppTest s_test;

using namespace boost::ut;

suite CodeGeneratorCpp_program_tests = [] {
	"program test"_test = [] {
		s_test.GenerateCode();
	};
};
