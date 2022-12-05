#include "libpu8.h"
#include "../../lovela/lovela-dependencies.h"
#include "../../lovela/Lexer.h"
#include "../../lovela/Parser.h"
#include "../../lovela/CodeGeneratorFactory.h"

int main_utf8(int argc, char** argv)
{
	std::unique_ptr<StreamLexer> lexer;
	RangeParser parser;
	auto codeGen = CodeGeneratorFactory::Create(std::cout, "Cpp");
	std::cin >> lexer >> parser >> *codeGen;

	return 0;
}
