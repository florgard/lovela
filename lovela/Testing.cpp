#include "Testing.h"
#include <iostream>
#include <sstream>
#include <cassert>

void TestCode(ILexer& lexer, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	auto tokens = lexer.Lex(input);

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
			std::wcerr << "Token mismatch. Is (" << static_cast<int>(actual.type) << ' ' << actual.value
				<< "), expected (" << static_cast<int>(expected.type) << ' ' << expected.value << ").\n";
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
			std::wcerr << "Error mismatch. Code is " << static_cast<int>(actual.code) << ", expected " << static_cast<int>(expected.code) << ".\n"
				<< '(' << actual.line << ':' << actual.column << ") error " << static_cast<int>(actual.code) << ": " << actual.message << '\n';
		}
		else if (expected.line && actual.line != expected.line)
		{
			success = false;
			std::wcerr << "Error mismatch. Error line is " << actual.line << ", expected " << expected.line << ".\n"
				<< '(' << actual.line << ':' << actual.column << ") error " << static_cast<int>(actual.code) << ": " << actual.message << '\n';
		}
	}

	assert(success);
}
