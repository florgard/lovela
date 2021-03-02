#pragma once
#include "ILexer.h"
#include <string_view>
#include <memory>

class Testing
{
public:
	void TestToken();
	void TestLexer();
	void TestParser();

private:
	void TestLexer(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors);
	void TestParser(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<ILexer::Error>& expectedErrors);
	bool TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree);
};
