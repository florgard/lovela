#pragma once
#include "ILexer.h"

class Testing
{
public:
	void TestToken();
	void TestLexer();
	void TestParser();

private:
	void TestLexer(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors);
	void TestParser(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors);
	bool TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree);

	template <typename Code>
	static std::wstring GetIncorrectErrorCodeMessage(const char* phase, const char* name, int index, Code actual, Code expected)
	{
		std::wostringstream s;
		s << phase << " test \"" << name << "\" error: Error " << index + 1 << " code is " << ToWString(magic_enum::enum_name(actual))
			<< ", expected " << ToWString(magic_enum::enum_name(expected)) << ".\n";
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
		s << ToWString(magic_enum::enum_name(error.code)) << ": " << error.message << '\n'
			<< '(' << error.token.line << ':' << error.token.column << ") \"..." << error.token.code << "\" <-- At this place" << '\n';
		return s.str();
	}
};
