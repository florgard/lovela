#include "pch.h"
#include "Lexer.h"
#include "Parser.h"
#include "TestingBase.h"

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
			std::wcerr << "Test \"" << name << "\" error: Token " << i + 1 << " is " << to_wstring(actual.type) << " \"" << actual.value
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

Node TestingBase::TestParser(const char* name, std::wstring_view code, const Node& expectedTree, const std::vector<IParser::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	Lexer lexer(input);
	Parser parser(lexer.Lex());
	auto tree = parser.Parse();

	int index = 0;
	bool success = TestAST(index, name, tree, expectedTree);

	if (!success)
	{
		std::wcerr << "AST mismatch.\nActual:\n";
		index = 0;
		PrintTree(index, tree);
		std::wcerr << "Expected:\n";
		index = 0;
		PrintTree(index, expectedTree);
		assert(success);
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
		const auto& actual = tree;
		const auto& expected = expectedTree;

		std::wcerr << "Test \"" << name << "\" error: Some property of node " << index + 1 << " of type " << to_wstring(actual.type)
			<< " differs from the expected node of type " << to_wstring(expected.type) << ".\n";
		return false;
	}

	index++;

	auto actualCount = tree.children.size();
	auto expectedCount = expectedTree.children.size();
	auto count = std::max(actualCount, expectedCount);
	for (int i = 0; i < count; i++)
	{
		const auto& actual = i < actualCount ? tree.children[i] : Node{};
		const auto& expected = i < expectedCount ? expectedTree.children[i] : Node{};
		if (!TestAST(index, name, actual, expected))
		{
			return false;
		}
	}

	return true;
}

void TestingBase::PrintTree(int& index, const Node& tree, std::wstring indent)
{
	std::wcerr << indent << '(' << index + 1 << ' ' << to_wstring(tree.type) << " " << tree.name << '\n';
	index++;

	for (auto& node : tree.children)
	{
		PrintTree(index, node, indent + L"  ");
	}

	std::wcerr << indent << "),\n";
}
