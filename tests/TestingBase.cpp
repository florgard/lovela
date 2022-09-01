#include "pch.h"
#include "TestingBase.h"
#include "../lovela/LexerFactory.h"
#include "../lovela/ParserFactory.h"
#include "../lovela/CodeGeneratorFactory.h"
#include "../lovela/Algorithm.h"

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

void TestingBase::TestCodeGenerator(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto tree = parser->Parse();

	std::wostringstream output;
	auto codeGen = CodeGeneratorFactory::Create(output, "Cpp");
	Traverse::DepthFirstPostorder(*tree, [&](Node& node) { codeGen->Visit(node); });

	auto generatedCode = output.str();
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"^\\s+" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+$" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+" }, L" ");
	auto expectedCode = to_wstring(cppCode);
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"^\\s+" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+$" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+" }, L" ");

	bool success = generatedCode == expectedCode;

	if (!success)
	{
		std::wcerr << "Code generator test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n\nAST:\n";

		PrintTree(*tree);

		assert(success);
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::wcerr << "Code generator test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::wcerr << error << '\n';
		}

		assert(success);
	}
}

void TestingBase::TestCodeGeneratorImport(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto tree = parser->Parse();

	std::wostringstream output;
	auto codeGen = CodeGeneratorFactory::Create(output, "Cpp");
	Traverse::DepthFirstPostorder(*tree, [&](Node& node) { codeGen->Visit(node); });

	bool success = codeGen->GetImports().size() == 1 || codeGen->GetImports().empty() && cppCode.empty();

	if (!success)
	{
		std::wcerr << "Code generator import test \"" << name << "\" error: The code didn't yield a single export.\n";

		assert(success);
		return;
	}

	auto generatedCode = codeGen->GetImports().front();
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"^\\s+" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+$" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+" }, L" ");
	auto expectedCode = to_wstring(cppCode);
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"^\\s+" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+$" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+" }, L" ");

	success = generatedCode == expectedCode;

	if (!success)
	{
		std::wcerr << "Code generator import test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n";

		assert(success);
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::wcerr << "Code generator import test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::wcerr << error << '\n';
		}

		assert(success);
	}
}

void TestingBase::TestCodeGeneratorExport(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto lexer = LexerFactory::Create(input);
	auto parser = ParserFactory::Create(lexer->Lex());
	auto tree = parser->Parse();

	std::wostringstream output;
	auto codeGen = CodeGeneratorFactory::Create(output, "Cpp");
	Traverse::DepthFirstPostorder(*tree, [&](Node& node) { codeGen->Visit(node); });

	bool success = codeGen->GetExports().size() == 1 || codeGen->GetExports().empty() && cppCode.empty();

	if (!success)
	{
		std::wcerr << "Code generator export test \"" << name << "\" error: The code didn't yield a single export.\n";

		assert(success);
		return;
	}

	auto generatedCode = codeGen->GetExports().front();
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"^\\s+" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+$" }, L"");
	generatedCode = std::regex_replace(generatedCode, std::wregex{ L"\\s+" }, L" ");
	auto expectedCode = to_wstring(cppCode);
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"^\\s+" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+$" }, L"");
	expectedCode = std::regex_replace(expectedCode, std::wregex{ L"\\s+" }, L" ");

	success = generatedCode == expectedCode;

	if (!success)
	{
		std::wcerr << "Code generator export test \"" << name << "\" error: The generated code differs from the expected code.\nGenerated:\n" << generatedCode
			<< "\nExpected:\n" << expectedCode << "\n\nInput code:\n" << code << "\n";

		assert(success);
	}

	success = codeGen->GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::wcerr << "Code generator export test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : codeGen->GetErrors())
		{
			std::wcerr << error << '\n';
		}

		assert(success);
	}
}
