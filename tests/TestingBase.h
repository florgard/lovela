#pragma once
#include "../lovela/NodeGenerator.h"

class TestingBase
{
public:
	static void PrintAST(const Node& tree)
	{
		int i{};
		PrintAST(i, tree, {});
	}

	static void PrintAST(std::ranges::range auto& range)
	{
		auto end = range.end();
		for (auto it = range.begin(); it != end; it++)
		{
			PrintAST(*it);
		}
	}

protected:
	/// <summary>
	/// Compares the actual AST with the expected one.
	/// </summary>
	/// <param name="index"></param>
	/// <param name="name"></param>
	/// <param name="tree"></param>
	/// <param name="expectedTree"></param>
	/// <returns>true if the actual and expected AST match, false otherwise.</returns>
	static bool TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree);

	/// <summary>
	/// Compares each AST in the actual range with the corresponding one in the expected range.
	/// </summary>
	/// <param name="index"></param>
	/// <param name="name"></param>
	/// <param name="range"></param>
	/// <param name="expectedRange"></param>
	/// <returns>true if the actual and expected AST match, false otherwise.</returns>
	static bool TestAST(int& index, const char* name, std::ranges::range auto& range, const std::ranges::range auto& expectedRange)
	{
		auto expected = expectedRange.begin();
		for (auto it = range.begin(), end = range.end(); it != end; it++, expected++)
		{
			if (!TestAST(index, name, *it, *expected))
			{
				return false;
			}
		}

		return true;
	}

	static void PrintAST(int& index, const Node& tree, std::wstring indent);

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
