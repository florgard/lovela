#pragma once
#include "../lovela/ILexer.h"
#include "../lovela/IParser.h"

class TestingBase
{
public:
	static void PrintTree(const Node& tree) { int i{}; PrintTree(i, tree, {}); }

protected:
	static bool TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree);
	static void PrintTree(int& index, const Node& tree, std::wstring indent);

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
