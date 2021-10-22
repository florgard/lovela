export module Testing;

import TestingBase;

export class Testing : public TestingBase
{
public:
	void RunTests();

private:
	void RunLexerTests();
	void RunParserTests();
	void RunCodeGeneratorTests();
};
