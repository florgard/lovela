#pragma once
#include "ILexer.h"
#include "IParser.h"

class TestingBase
{
protected:
	void TestLexer(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors = {});
	std::unique_ptr<Node> TestParser(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors = {});
	void TestCodeGenerator(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors = 0);
	void TestCodeGeneratorImport(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors = 0);
	void TestCodeGeneratorExport(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors = 0);

	void PrintTree(const Node& tree) { int i{}; PrintTree(i, tree, {}); }

private:
	bool TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree);
	void PrintTree(int& index, const Node& tree, std::wstring indent);

	template <typename Code>
	static std::wstring GetIncorrectErrorCodeMessage(const char* phase, const char* name, int index, Code actual, Code expected)
	{
		std::wostringstream s;
		s << phase << " test \"" << name << "\" error: Error " << index + 1 << " code is " << to_wstring(actual)
			<< ", expected " << to_wstring(expected) << ".\n";
		return s.str();
	}

	static std::wstring GetIncorrectErrorLineMessage(const char* phase, const char* name, int index, int actual, int expected)
	{
		std::wostringstream s;
		s << phase << " test \"" << name << "\" error: Error " << index + 1 << " line number is " << actual << ", expected " << expected << ".\n";
		return s.str();
	}

	template <typename ErrorType>
	static std::wstring GetErrorMessage(const ErrorType& error)
	{
		std::wostringstream s;
		s << to_wstring(error.code) << ": " << error.message << '\n'
			<< '(' << error.token.line << ':' << error.token.column << ") \"..." << error.token.code << "\" <-- At this place" << '\n';
		return s.str();
	}
};
