#include <iostream>
#include <sstream>
#include <cassert>
#include "Testing.h"
#include "Lexer.h"
#include "magic_enum.hpp"

void Test(const char* name, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors)
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
			std::cerr << "Test \"" << name << "\" error: Token " << i << " is " << magic_enum::enum_name(actual.type) << " \"";
			std::wcerr << actual.value;
			std::cerr << "\", expected " << magic_enum::enum_name(expected.type) << " \"";
			std::wcerr << expected.value << "\".\n";
		}
	}

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
			std::cerr << "Test \"" << name << "\" error: Error " << i << " code is " << magic_enum::enum_name(actual.code) << ", expected " << magic_enum::enum_name(expected.code) << ".\n";
			std::cerr << '(' << actual.line << ':' << actual.column << ") error " << magic_enum::enum_name(actual.code) << ": ";
			std::wcerr << actual.message << '\n';
		}
		else if (expected.line && actual.line != expected.line)
		{
			success = false;
			std::cerr << "Test \"" << name << "\" error: Error " << i << " line number is " << actual.line << ", expected " << expected.line << ".\n";
			std::cerr << '(' << actual.line << ':' << actual.column << ") error " << magic_enum::enum_name(actual.code) << ": ";
			std::wcerr << actual.message << '\n';
		}
	}

	assert(success);
}
