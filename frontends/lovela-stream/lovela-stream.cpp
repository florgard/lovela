#include "libpu8.h"
#include "../../lovela/lovela-dependencies.h"
#include "../../lovela/Lexer.h"
#include "../../lovela/Parser.h"
#include "../../lovela/CodeGeneratorCpp.h"

int main_utf8(int argc, char** argv)
{
	std::unique_ptr<StreamLexer> lexer;
	RangeParser parser;
	CodeGeneratorCpp coder;
	std::cin >> lexer >> parser >> coder >> std::cout;

	return 0;
}
