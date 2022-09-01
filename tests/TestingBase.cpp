#include "pch.h"
#include "TestingBase.h"

bool TestingBase::TestAST(int& index, const char* name, const Node& tree, const Node& expectedTree)
{
	if (tree != expectedTree)
	{
		std::wcerr << "Parser test \"" << name << "\" error: Some property of node " << index + 1 << " of type " << to_wstring(tree.type)
			<< " differs from the expected node of type " << to_wstring(expectedTree.type) << ".\n";
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

void TestingBase::PrintTree(int& index, const Node& tree, std::wstring indent)
{
	std::wcerr << indent << '(' << index + 1 << ' ' << to_wstring(tree.type) << " " << tree.value << '\n';
	index++;

	if (tree.left)
	{
		PrintTree(index, *tree.left, indent + L"  ");
	}
	if (tree.right)
	{
		PrintTree(index, *tree.right, indent + L"  ");
	}

	std::wcerr << indent << "),\n";
}
