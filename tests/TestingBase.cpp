#include "pch.h"
#include "TestingBase.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

bool TestingBase::TestAST(int& index, const char* name, const Node& node, const Node& expectedNode)
{
	if (node != expectedNode)
	{
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Parser test \"" << color.name << name << color.none << "\": "
			<< "Some property of node " << index + 1 << " of type " << color.actual << to_string(node.type) << color.none
			<< " differs from the expected node of type " << color.expect << to_string(expectedNode.type) << color.none << ".\n"
			<< "Actual:\n" << color.actual;
		node.Print(std::cerr);
		std::cerr << color.none << '\n'
			<< "Expected:\n" << color.expect;
		expectedNode.Print(std::cerr);
		std::cerr << color.none << '\n';
		return false;
	}

	index++;

	// Fail if one pointer is set but not the other
	if (!!node.left != !!expectedNode.left || !!node.right != !!expectedNode.right)
	{
		return false;
	}

	if (node.left && !TestAST(index, name, *node.left, *expectedNode.left))
	{
		return false;
	}

	if (node.right && !TestAST(index, name, *node.right, *expectedNode.right))
	{
		return false;
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

void TestingBase::PrintAST(int& index, const Node& node, int indent)
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

	if (node.left)
	{
		PrintAST(index, *node.left, indent + 1);
	}
	if (node.right)
	{
		PrintAST(index, *node.right, indent + 1);
	}

	PrintIndent(indent);

	std::cerr << "),\n";
}
