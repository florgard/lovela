#include "pch.h"
#include "TestingBase.h"

bool TestingBase::TestSyntaxTree(std::ostream& stream, int& index, std::string_view name, Node const& node, Node const& expectedNode, Token& failingToken)
{
	if (node != expectedNode)
	{
		failingToken = node.token;

		stream << "Some property of node " << index + 1 << " of type " << color.actual << to_string(node.type) << color.none
			<< " differs from the expected node of type " << color.expect << to_string(expectedNode.type) << color.none << ".\n"
			<< "Actual:\n" << color.actual;
		node.Print(stream);
		stream << color.none << '\n'
			<< "Expected:\n" << color.expect;
		expectedNode.Print(stream);
		stream << color.none << '\n';

		return false;
	}

	++index;

	if (node.children.size() != expectedNode.children.size())
	{
		failingToken = node.token;
		return false;
	}

	for (size_t i = 0, c = node.children.size(); i < c; ++i)
	{
		if (!TestSyntaxTree(stream, index, name, node.children[i], expectedNode.children[i], failingToken))
		{
			return false;
		}
	}

	return true;
}

void TestingBase::PrintIndent(int indent)
{
	for (int i = 0; i < indent; ++i)
	{
		std::cerr << "  ";
	}
}

void TestingBase::PrintSyntaxTree(int& index, Node const& node, int indent)
{
	PrintIndent(indent);

	std::cerr << '(' << index + 1 << ' ' << to_string(node.type);
	
	if (!node.value.empty())
	{
		std::cerr << " \"" << node.value << '\"';
	}

	if (node.type == Node::Type::Error)
	{
		std::cerr << " [" << to_string(node.error.code) << ": " << node.error.message << ']';
	}

	std::cerr << '\n';

	++index;

	for (auto& child : node.children)
	{
		PrintSyntaxTree(index, child, indent + 1);
	}

	PrintIndent(indent);

	std::cerr << "),\n";
}
