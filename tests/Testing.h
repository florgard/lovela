#pragma once
#include "TestingBase.h"

class Testing : public TestingBase
{
public:
	void RunTests();

private:
	void RunLexerTests();
	void RunParserTests();
	void RunCodeGeneratorTests();
};
