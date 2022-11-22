#include "pch.h"
#include "TestingBase.h"

#define BOOST_UT_DISABLE_MODULE
#include "ut.hpp"
using namespace boost::ut;

bool TestingBase::TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree)
{
	if (tree != expectedTree)
	{
		std::cerr << color.fail << "ERROR: " << color.none
			<< "Parser test \"" << color.name << name << color.none << "\": "
			<< "Some property of node " << index + 1 << " of type " << color.actual << to_string(tree.type) << color.none
			<< " differs from the expected node of type " << color.expect << to_string(expectedTree.type) << color.none << ".\n";
		return false;
	}

	index++;

	// Fail if one pointer is set but not the other
	if (!!tree.left != !!expectedTree.left || !!tree.right != !!expectedTree.right)
	{
		return false;
	}

	if (tree.left && !TestAST(index, name, *tree.left, *expectedTree.left))
	{
		return false;
	}

	if (tree.right && !TestAST(index, name, *tree.right, *expectedTree.right))
	{
		return false;
	}

	return true;
}

void TestingBase::PrintAST(int& index, const Node& tree, std::string indent)
{
	std::cerr << indent << '(' << index + 1 << ' ' << to_string(tree.type);
	
	if (!tree.value.empty())
	{
		std::cerr << ' ' << tree.value;
	}

	if (tree.type == Node::Type::Error)
	{
		std::cerr << " [" << to_string(tree.error.code) << ": " << tree.error.message << ']';
	}

	std::cerr << '\n';

	index++;

	if (tree.left)
	{
		PrintAST(index, *tree.left, indent + "  ");
	}
	if (tree.right)
	{
		PrintAST(index, *tree.right, indent + "  ");
	}

	std::cerr << indent << "),\n";
}
