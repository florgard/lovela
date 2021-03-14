#pragma once
#include "TestingBase.h"

class Testing : public TestingBase
{
public:
	void RunTests();

private:
	void RunTypeTests();
	void RunLexerTests();
	void RunParserTests();
	void RunCodeGeneratorTests();
};
