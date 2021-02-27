#include "Testing.h"
#include <iostream>
#include <sstream>
#include <cassert>

void TestCode(ILexer& lexer, std::wstring_view code, const std::vector<Token>& expectedTokens, const std::vector<ILexer::Error>& expectedErrors)
{
	std::wistringstream input(std::wstring(code.data(), code.size()));
	const auto tokens = lexer.Lex(input);
	const bool tokensMatch = tokens == expectedTokens;
	assert(tokensMatch);
	if (!tokensMatch)
	{
		return;
	}

	auto& errors = lexer.GetErrors();
	const bool errorCountMatches = errors.size() == expectedErrors.size();
	assert(errorCountMatches);
	if (!errorCountMatches)
	{
		return;
	}

	for (int i = 0; i < errors.size(); i++)
	{
		auto& error = errors[i];
		auto& expectedError = expectedErrors[i];
		const bool errorMatches = error.code == expectedError.code && (!expectedError.line || error.line == expectedError.line);
		if (!errorMatches)
		{
			std::wcerr << '(' << error.line << ':' << error.column << ") error " << static_cast<int>(error.code) << ": " << error.message << '\n';
			assert(errorMatches);
			return;
		}
	}
}
