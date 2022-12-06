#include <csignal>
#include "libpu8.h"
#include "../../lovela/lovela-dependencies.h"
#include "../../lovela/Lexer.h"
#include "../../lovela/Parser.h"
#include "../../lovela/CoderCpp.h"

int main_utf8(int argc, char** argv)
{
	// Intercept Ctrl-C to exit gracefully.
	signal(SIGINT, [](int) {});

	std::unique_ptr<StreamLexer> lexer;
	RangeParser parser;
	CodeGeneratorCpp coder;
	std::cin >> lexer >> parser >> coder >> std::cout;

	return 0;
}
