#pragma once
#include "../lovela/Node.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

class TestingBase
{
public:
	void PrintAST(const Node& tree)
	{
		int i{};
		PrintAST(i, tree, {});
	}

	void PrintAST(std::ranges::range auto& range)
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
	bool TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree);

	/// <summary>
	/// Compares each AST in the actual range with the corresponding one in the expected range.
	/// </summary>
	/// <param name="index"></param>
	/// <param name="name"></param>
	/// <param name="range"></param>
	/// <param name="expectedRange"></param>
	/// <returns>true if the actual and expected AST match, false otherwise.</returns>
	bool TestAST(int& index, const char* name, std::ranges::range auto& range, const std::ranges::range auto& expectedRange)
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

	void PrintAST(int& index, const Node& tree, int indent);

	template <typename Code>
	void PrintIncorrectErrorCodeMessage(std::ostream& stream, const char* phase, const char* name, int index, Code actual, Code expected)
	{
		stream << color.fail << "ERROR: " << color.none
			<< phase << " test \"" << color.name << name << color.none << "\": "
			<< "Error " << index + 1 << " code is " << color.actual << to_string(actual) << color.none << ", expected " << color.expect << to_string(expected) << color.none << ".\n";
	}

	void PrintIncorrectErrorLineMessage(std::ostream& stream, const char* phase, const char* name, int index, int actual, int expected)
	{
		stream << color.fail << "ERROR: " << color.none
			<< phase << " test \"" << color.name << name << color.none << "\": "
			<< "Error " << index + 1 << " line number is " << color.actual << actual << color.none << ", expected " << color.expect << expected << color.none << ".\n";
	}

	template <typename ErrorType>
	void PrintErrorMessage(std::ostream& stream, const ErrorType& error)
	{
		stream << to_string(error.code) << ": " << error.message << '\n' << '(' << error.token.line << ':' << error.token.column << ") \"..." << error.token.code << "\" <-- At this place" << '\n';
	}

	struct Color
	{
		// Copied from ut.hpp (c) Kris Jusiak 
		std::string_view none = "\033[0m";
		std::string_view pass = "\033[32m";
		std::string_view warn = "\033[33m";
		std::string_view fail = "\033[31m";
		std::string_view name = "\033[33m";
		std::string_view code = "\033[1;36m";
		std::string_view output = "\033[36m";
		std::string_view expect = "\033[1;32m";
		std::string_view actual = "\033[1;33m";
	} color;

private:
	void PrintIndent(int indent);
};
