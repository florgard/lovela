#include "pch.h"
#include "Testing.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"
#include "../lovela/CodeGeneratorFactory.h"
#include "../lovela/LexerBase.h"
#include "../lovela/Algorithm.h"

void Testing::RunCodeGeneratorTests()
{


//	std::wstring code = LR"(
//[()] pi: 3.14.
//mul (factor): * factor.
//transform (mul, sub): (* mul. - sub).
//[](): pi transform (2, 0.28) + 1.
//)";

	// Internal error:
	// -> [#32] puts [#8#]. : 'Hello, Wordl!" puts.

	std::wstring code = LR"(
-> 'Standard C' puts.
: 'Hello, Wordl!' puts.
)";
	std::wcout << code << '\n';

	std::wistringstream input(code);
	auto lexer = LexerFactory::Create(input);
	for (auto& error : lexer->GetErrors())
	{
		std::wcerr << error.message << '\n';
	}

	auto parser = ParserFactory::Create(lexer->Lex());
	auto tree = parser->Parse();
	for (auto& error : parser->GetErrors())
	{
		std::wcerr << error.message << '\n';
	}

	PrintTree(*tree);
	std::wcout << '\n';

	std::wstringstream stream;

	auto codeGen = CodeGeneratorFactory::Create(stream, "Cpp");
	Traverse::DepthFirstPostorder(*tree, [&](Node& node) { codeGen->Visit(node); });

	for (auto& error : codeGen->GetErrors())
	{
		std::wcerr << error << '\n';
	}

	auto genCode = stream.str();
	std::wcout << genCode;

	std::wofstream program(R"(..\targets\cpp\program\lovela-program.cpp)");
	codeGen->GenerateProgramFile(program);
	program.close();

	std::wofstream imports(R"(..\targets\cpp\program\lovela-imports.h)");
	codeGen->GenerateImportsFile(imports);
	imports.close();

	std::wofstream exports(R"(..\targets\cpp\program\lovela-exports.h)");
	codeGen->GenerateExportsFile(exports);
	exports.close();
}
