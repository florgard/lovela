#pragma once
#include "ILexer.h"

class Testing
{
public:
	void TestLexer(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors);
};
