#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"
#include "../lovela/CodeGeneratorFactory.h"

suite CodeGeneratorCpp_program_tests = [] {
	"program test"_test = [] {
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
		std::cout << code << '\n';

		std::istringstream input(code);
		auto lexer = LexerFactory::Create(input);
		for (auto& error : lexer->GetErrors())
		{
			std::cerr << error.message << '\n';
		}

		expect(lexer->GetErrors().empty());

		auto parser = ParserFactory::Create(lexer->Lex());
		auto nodes = to_vector(parser->Parse());

		for (auto& error : parser->GetErrors())
		{
			std::cerr << error.message << '\n';
		}

		expect(parser->GetErrors().empty());

		TestingBase::PrintAST(nodes);
		std::cout << '\n';

		std::stringstream stream;

		auto codeGen = CodeGeneratorFactory::Create(stream, "Cpp");
		Traverse<Node>::DepthFirstPostorder(nodes, [&](auto& node) { codeGen->Visit(node); });

		for (auto& error : codeGen->GetErrors())
		{
			std::cerr << error << '\n';
		}

		expect(codeGen->GetErrors().empty());

		auto genCode = stream.str();
		std::cout << genCode;

		std::ofstream program(R"(..\targets\cpp\program\lovela-program.cpp)");
		codeGen->GenerateProgramFile(program);
		program.close();

		std::ofstream imports(R"(..\targets\cpp\program\lovela-imports.h)");
		codeGen->GenerateImportsFile(imports);
		imports.close();

		std::ofstream exports(R"(..\targets\cpp\program\lovela-exports.h)");
		codeGen->GenerateExportsFile(exports);
		exports.close();
	};
};
