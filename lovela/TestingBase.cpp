#include "pch.h"
#include "Lexer.h"
#include "Parser.h"
#include "TestingBase.h"
#include "CodeGenerator.h"

void TestingBase::TestLexer(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	Lexer lexer(input);
	auto tokenGenerator = lexer.Lex();
	auto tokens = std::vector<Token>(tokenGenerator.begin(), tokenGenerator.end());

	bool success = true;

	auto actualCount = tokens.size();
	auto expectedCount = expectedTokens.size();
	auto count = std::max(actualCount, expectedCount);
	for (int i = 0; i < count; i++)
	{
		const auto actual = i < actualCount ? tokens[i] : Token{};
		const auto expected = i < expectedCount ? expectedTokens[i] : Token{};
		if (actual != expected)
		{
			success = false;
			std::wcerr << "Lexer test \"" << name << "\" error: Token " << i + 1 << " is " << to_wstring(actual.type) << " \"" << actual.value
				<< "\", expected " << to_wstring(expected.type) << " \"" << expected.value << "\".\n";
		}
	}

	assert(success);

	auto& errors = lexer.GetErrors();

	actualCount = errors.size();
	expectedCount = expectedErrors.size();
	count = std::max(actualCount, expectedCount);
	for (int i = 0; i < count; i++)
	{
		const auto actual = i < actualCount ? errors[i] : ILexer::Error{};
		const auto expected = i < expectedCount ? expectedErrors[i] : ILexer::Error{};
		if (actual.code != expected.code)
		{
			success = false;
			std::wcerr << GetIncorrectErrorCodeMessage("Lexer", name, i, actual.code, expected.code) << GetErrorMessage(actual);
		}
		else if (expected.token.line && actual.token.line != expected.token.line)
		{
			success = false;
			std::wcerr << GetIncorrectErrorLineMessage("Lexer", name, i, actual.token.line, expected.token.line) << GetErrorMessage(actual);
		}
	}

	assert(success);
}

std::unique_ptr<Node> TestingBase::TestParser(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	Lexer lexer(input);
	Parser parser(lexer.Lex());
	auto tree = parser.Parse();

	bool success = !!tree;

	if (!success)
	{
		std::wcerr << "Parser test \"" << name << "\" error: The parser didn't yield an AST.\n\nExpected:\n";
		PrintTree(expectedTree);
	}
	else
	{
		int index = 0;
		success = TestAST(index, name, *tree, expectedTree);

		if (!success)
		{
			std::wcerr << "Parser test \"" << name << "\" error: AST mismatch.\n\nActual:\n";
			PrintTree(*tree);
			std::wcerr << "\nExpected:\n";
			PrintTree(expectedTree);
		}
	}

	auto& errors = parser.GetErrors();
	const auto actualCount = errors.size();
	const auto expectedCount = expectedErrors.size();
	const auto count = std::max(actualCount, expectedCount);

	for (int i = 0; i < count; i++)
	{
		const auto actual = i < actualCount ? errors[i] : IParser::Error{};
		const auto expected = i < expectedCount ? expectedErrors[i] : IParser::Error{};
		if (actual.code != expected.code)
		{
			success = false;
			std::wcerr << GetIncorrectErrorCodeMessage("Parser", name, i, actual.code, expected.code) << GetErrorMessage(actual);
		}
		else if (expected.token.line && actual.token.line != expected.token.line)
		{
			success = false;
			std::wcerr << GetIncorrectErrorLineMessage("Parser", name, i, actual.token.line, expected.token.line) << GetErrorMessage(actual);
		}
	}

	assert(success);

	return tree;
}

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
	Lexer lexer(input);
	Parser parser(lexer.Lex());
	auto tree = parser.Parse();

	std::wostringstream output;
	CodeGenerator gen(output);
	Parser::TraverseDepthFirstPostorder(*tree, [&](Node& node) { gen.Visit(node); });

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

	success = gen.GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::wcerr << "Code generator test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : gen.GetErrors())
		{
			std::wcerr << error << '\n';
		}

		assert(success);
	}
}

void TestingBase::TestCodeGeneratorExport(const char* name, std::wstring_view code, std::wstring_view cppCode, int expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	Lexer lexer(input);
	Parser parser(lexer.Lex());
	auto tree = parser.Parse();

	std::wostringstream output;
	CodeGenerator gen(output);
	Parser::TraverseDepthFirstPostorder(*tree, [&](Node& node) { gen.Visit(node); });

	bool success = gen.GetExports().size() == 1;

	if (!success)
	{
		std::wcerr << "Code generator export test \"" << name << "\" error: The code didn't yield a single export.\n";

		assert(success);
		return;
	}

	auto generatedCode = gen.GetExports().front();
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

	success = gen.GetErrors().size() == expectedErrors;

	if (!success)
	{
		std::wcerr << "Code generator export test \"" << name << "\" error: The error count differs from the expected count.\nError messages:\n";

		for (auto& error : gen.GetErrors())
		{
			std::wcerr << error << '\n';
		}

		assert(success);
	}
}
