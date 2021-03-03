#include "pch.h"
#include "Lexer.h"
#include "Parser.h"
#include "Testing.h"

int main()
{
	Testing testing;
	testing.TestToken();
	testing.TestLexer();
	testing.TestParser();
}
