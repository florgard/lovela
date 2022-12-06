#include "pch.h"
#include "TestingBase.h"
#include "../lovela/Lexer.h"
#include "../lovela/Parser.h"
#include "../lovela/CodeGeneratorCpp.h"

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

		StringLexer lexer;
		RangeParser parser;
		std::vector<Node> nodes;
		CodeGeneratorCpp coder;
		std::stringstream output;
		code >> lexer >> parser >> nodes >> coder >> output;

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

		for (auto& error : coder.GetErrors())
		{
			std::cerr << error << '\n';
		}

		expect(coder.GetErrors().empty());

		auto genCode = output.str();
		std::cout << "Generated code:\n" << color.output << genCode << color.none;

		std::ofstream program(R"(..\targets\cpp\program\lovela-program.cpp)");
		coder.GenerateProgramFile(program);
		program.close();

		std::ofstream imports(R"(..\targets\cpp\program\lovela-imports.h)");
		coder.GenerateImportsFile(imports);
		imports.close();

		std::ofstream exports(R"(..\targets\cpp\program\lovela-exports.h)");
		coder.GenerateExportsFile(exports);
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
