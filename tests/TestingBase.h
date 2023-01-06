#pragma once

class TestingBase
{
protected:
	struct Color
	{
		std::string_view none = "\033[0m";
		std::string_view pass = "\033[32m";
		std::string_view warn = "\033[33m";
		std::string_view fail = "\033[31m";
		std::string_view name = "\033[33m";
		std::string_view code = "\033[96m";
		std::string_view output = "\033[36m";
		std::string_view expect = "\033[92m";
		std::string_view actual = "\033[93m";
	} color;

	void PrintTokens(std::ranges::range auto& tokens)
	{
		for (auto& token : tokens)
		{
			std::cerr << to_string(token.type) << ' ';

			if (!token.value.empty())
			{
				std::cerr << '"' << token.value << '"' << ' ';
			}
		}
	}

	void PrintSyntaxTree(Node const& node)
	{
		int i{};
		PrintSyntaxTree(i, node, {});
	}

	void PrintSyntaxTree(std::ranges::range auto& nodes)
	{
		auto end = nodes.end();
		for (auto it = nodes.begin(); it != end; it++)
		{
			PrintSyntaxTree(*it);
		}
	}

	/// <summary>
	/// Compares the actual syntax tree with the expected one.
	/// </summary>
	/// <param name="name">The name of the running test, for error messages.</param>
	/// <param name="node">The root node for which to print the syntax tree.</param>
	/// <param name="expectedNode">The expected root node with its expected syntax tree.</param>
	/// <returns>true if the actual and expected syntax tree match, false otherwise.</returns>
	bool TestSyntaxTree(std::ostream& stream, std::string_view name, Node const& node, Node const& expectedNode, Token& failingToken)
	{
		int index = 0;
		return TestSyntaxTree(stream, index, name, node, expectedNode, failingToken);
	}

	/// <summary>
	/// Compares each syntax tree in the actual range with the corresponding one in the expected range.
	/// </summary>
	/// <param name="name">The name of the running test, for error messages.</param>
	/// <param name="nodes">The root nodes for which to print the syntax tree.</param>
	/// <param name="expectedRange">The expected root nodes with their expected syntax trees.</param>
	/// <returns>true if the actual and expected syntax trees match, false otherwise.</returns>
	bool TestSyntaxTree(std::ostream& stream, std::string_view name, std::ranges::range auto& nodes, std::ranges::range auto const& expectedRange, Token& failingToken)
	{
		int index = 0;
		return TestSyntaxTree(stream, index, name, nodes, expectedRange, failingToken);
	}

private:
	void PrintIndent(int indent);
	void PrintSyntaxTree(int& index, Node const& node, int indent);
	bool TestSyntaxTree(std::ostream& stream, int& index, std::string_view name, Node const& node, Node const& expectedNode, Token& failingToken);

	bool TestSyntaxTree(std::ostream& stream, int& index, std::string_view name, std::ranges::range auto& range, std::ranges::range auto const& expectedRange, Token& failingToken)
	{
		static const Node emptyNode{};

		auto actualIt = range.begin();
		auto actualEnd = range.end();
		auto expectedIt = expectedRange.begin();
		auto expectedEnd = expectedRange.end();

		while (actualIt != actualEnd || expectedIt != expectedEnd)
		{
			const auto& actual = actualIt != actualEnd ? *actualIt++ : emptyNode;
			const auto& expected = expectedIt != expectedEnd ? static_cast<const Node&>(*expectedIt++) : emptyNode;

			if (!TestSyntaxTree(stream, index, name, actual, expected, failingToken))
			{
				return false;
			}
		}

		return true;
	}

	template <typename Code>
	void PrintIncorrectErrorCodeMessage(std::ostream& stream, std::string_view phase, std::string_view name, size_t index, Code actual, Code expected)
	{
		stream << color.fail << "ERROR: " << color.none
			<< phase << " test \"" << color.name << name << color.none << "\": "
			<< "Error " << index + 1 << " code is " << color.actual << to_string(actual) << color.none << ", expected " << color.expect << to_string(expected) << color.none << ".\n";
	}

	void PrintIncorrectErrorLineMessage(std::ostream& stream, std::string_view phase, std::string_view name, size_t index, size_t actual, size_t expected)
	{
		stream << color.fail << "ERROR: " << color.none
			<< phase << " test \"" << color.name << name << color.none << "\": "
			<< "Error " << index + 1 << " line number is " << color.actual << actual << color.none << ", expected " << color.expect << expected << color.none << ".\n";
	}

	template <typename ErrorType>
	void PrintErrorMessage(std::ostream& stream, ErrorType const& error)
	{
		stream << to_string(error.code) << ": " << error.message << '\n';
	}
};
