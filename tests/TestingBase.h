#pragma once
#include "../lovela/Node.h"

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

	static void PrintAST(int& index, const Node& tree, std::string indent);

	template <typename Code>
	static std::string GetIncorrectErrorCodeMessage(const char* phase, const char* name, int index, Code actual, Code expected)
	{
		std::ostringstream s;
		s << phase << " test \"" << name << "\" error: Error " << index + 1 << " code is " << to_string(actual)
			<< ", expected " << to_string(expected) << ".\n";
		return s.str();
	}

	static std::string GetIncorrectErrorLineMessage(const char* phase, const char* name, int index, int actual, int expected)
	{
		std::ostringstream s;
		s << phase << " test \"" << name << "\" error: Error " << index + 1 << " line number is " << actual << ", expected " << expected << ".\n";
		return s.str();
	}

	template <typename ErrorType>
	static std::string GetErrorMessage(const ErrorType& error)
	{
		std::ostringstream s;
		s << to_string(error.code) << ": " << error.message << '\n'
			<< '(' << error.token.line << ':' << error.token.column << ") \"..." << error.token.code << "\" <-- At this place" << '\n';
		return s.str();
	}
};
