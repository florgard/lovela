#pragma once
#include "ILexer.h"
#include "IParser.h"

class TestingBase
{
protected:
	void TestLexer(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors);
	void TestParser(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors);

private:
	bool TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree);
	void PrintTree(const Node& tree, std::wstring indent = {});

	template <typename Code>
	static std::wstring GetIncorrectErrorCodeMessage(const char* phase, const char* name, int index, Code actual, Code expected)
	{
		std::wostringstream s;
		s << phase << " test \"" << name << "\" error: Error " << index + 1 << " code is " << ToWString(actual)
			<< ", expected " << ToWString(expected) << ".\n";
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
		s << ToWString(error.code) << ": " << error.message << '\n'
			<< '(' << error.token.line << ':' << error.token.column << ") \"..." << error.token.code << "\" <-- At this place" << '\n';
		return s.str();
	}
};
